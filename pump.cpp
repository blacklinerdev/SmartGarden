#include "pump.h"

using namespace std;

Pump::Pump(int pumpPin) {
    /*  
    * Initialises the pump. The pumpPin must be the pin that the 
    * pump is connected to on the RPi. The wiringPiSetup function 
    * needs to be called before the wiringPi library can be used. 
    * This function sets up wiringPi using the wiringPi pin 
    * numbering scheme. Different pin numberings are available,
    * see:
    *
    *      wiringpi.com/reference/setup/
    *
    * for more details on different set up functions, and:
    *
    *      projects.drogan.net/raspberry-pi/wiringpi/pins/ 
    *
    * for details on pin numbering. 
    */
       
    pin = pumpPin;
    wiringPiSetup();
    pinMode(pin, OUTPUT);
}

int Pump::off() {
    try {
        digitalWrite(pin, LOW);
        on_flag = 0;
        time_to_run = 0;
        running_since = 0;
    } catch(...) {
        cout << "Pump may not shut off" << endl;
        return -1;
    }
    return 0;
}

int Pump::on() {

    try {
	    digitalWrite(pin, HIGH);
        on_flag = 1;
        running_since = time(0);
    }catch(...) {
        return -1;
    }
return 0;
}

bool Pump::is_on() {
    return on_flag;
}

void Pump::set_run_time(unsigned int seconds_to_run) {
    /* Having a seperate set_run_time method allows for seperating the 
     * way in which timed and non timed pump runs are handled. In 
     * conjuction with the update method, a timed run can be set up by 
     * setting the required run time for the pump using this method, 
     * turning the pump on with the on() method, and then continuosly 
     * calling the update method on the pump.
     */
    time_to_run = seconds_to_run;
}

void Pump::update() {
    /* The pump update function is used by the pump manager for manging 
     * pumping of predefined duration. If the pump is set to run for a 
     * predetermined length of time then this function will need to be 
     * called continuously in order to achieve the desired functionality.
     */
    unsigned int pump_has_been_running_for;
    if (is_on_timed() && has_reached_or_exceeded_time_to_run()) {
        off();
    }
}

bool Pump::is_on_timed() {
    return is_on() && time_to_run != 0;
}

bool Pump::has_reached_or_exceeded_time_to_run() {
    return get_running_time() >= time_to_run;
}

unsigned int Pump::get_running_time(){
    // Returns the number of seconds since the pump was turned on.
    unsigned int running_time = time(0) - running_since;
    return running_time;
}


void PumpOn::execute(shared_ptr <Pump> pump) {
    pump->on();
}


void PumpOff::execute(shared_ptr <Pump> pump) {
    pump->off();
}


PumpOnTimed::PumpOnTimed(unsigned int pumpTime) : PumpCommand() {
    /* All values of pumpTime from 1 to 60 (inclusive) can be excepted. 
     * Negative time values make not sense and will cause and error, 
     * any time over 60 will be capped at sixty. Entering zero will also
     * cause and error since running the pump for zero time is the same 
     * as not running the pump.
     */
    if (pumpTime > 60) {
        pump_time = 60;
    } else if (pumpTime <= 0) {
        // Not implemented, but should throw an error.
    } else {
        pump_time = pumpTime;
    }
    
}

void PumpOnTimed::execute(shared_ptr <Pump> pump) {
    pump->set_run_time(pump_time);
    pump->on();
}


PumpCommandQueue::PumpCommandQueue() {
    mtx = make_shared<mutex>();
    cv = make_shared<condition_variable>();
}

void PumpCommandQueue::push(shared_ptr<PumpCommand> command) {
    mtx->lock();
    commands.push(command);
    mtx->unlock();
}

shared_ptr<PumpCommand> PumpCommandQueue::pop() {
    mtx->lock();
    shared_ptr<PumpCommand> pump_command = commands.front();
    commands.pop();
    mtx->unlock();
    return pump_command;
}

bool PumpCommandQueue::is_empty() {
    return commands.empty();
}

PumpManager::PumpManager(shared_ptr<PumpCommandQueue> commandQueue, shared_ptr<Pump> pump_to_manage) {
    command_queue = commandQueue;
    pump = pump_to_manage;
}

void PumpManager::task() {
    /* The pump management task checks the command queue for new commands, 
     * executes them and will contiously update the pump.
     */
     
     shared_ptr<PumpCommand> pump_command; 
     
     if (!command_queue->is_empty()) {
         pump_command = command_queue->pop();
         pump_command->execute(pump);
     }
     pump->update();
}

void PumpManager::cont_task(shared_ptr<bool> exit_signal) {
    /* This cont_task is designed to run in its own thread, where it can 
     * manage the pump. This thread will be created by running:
     * 
     *      std::thread manager_thread(&PumpManager::cont_task, &manager, exit_signal);
     * 
     * The exit signal is a pointer to a boolean that ends the task when
     * it is set to true. The task checks the command queue, executes 
     * tasks, and updates the pump twice per second. Thus pump times 
     * should be precise to withing one second of the requests pumping 
     * time.
     */
    shared_ptr<PumpCommand> pump_command; 
    
    while(!*exit_signal) {
        if (!command_queue->is_empty()) {
            pump_command = command_queue->pop();
            pump_command->execute(pump);
        }
        pump->update();
        if (command_queue->is_empty()) {
            this_thread::sleep_for(chrono::milliseconds(500));
        } else {
        }
    }
}
