#include "pump.h"


/*
Details for the next commit.

Change implementation details for PumpCommandQueue

The pump command queue is designed to be a thread safe queue for sending
commands to a pump management task that is running in a seperate thread
from where the commands originate.

I have been researching how to implement such a queue, and previous 
versions of this code contained some implementation of what I had 
initially learnt. However, much of this was wrong and as a result some 
changes have been made.

## Changes to PumpCommandQueue

The mutex and condition variable (that are private members of the 
PumpCommandQueue have been changed from std::shared_ptr to variables. 
This matches pretty much every implementation of thread safe queues that 
I have seen on the net. The reason why these were made into shared_ptr 
in the first place was because the queue (once constructed) needs to be 
passed into the function that runs in the pump management thread. 
However, in my further research I have found that this can be done by 
passing a std::ref of the queue instead. Unfourtunatly, this has caused 
another problems that will be explained below, related to initialising 
the PumpManager class (that now owns a reference to a PumpCommandQueue).
 
The following three methods have been added to the PumpCommandQueue:

    PumpCommandQueue() = default;
    PumpCommandQueue(const PumpCommandQueue&) = delete; // This disables copying
    PumpCommandQueue& operator=(const PumpCommandQueue&) = delete; // This disables assignment

In addition the explicitly defined constructor for this class has been 
removed from pump.cpp. This had to be removed, otherwise the following
error would result:

    error: definition of explicitly-defaulted ‘PumpCommandQueue::PumpCommandQueue()’

The final change to the PumpManagementQueue is that called to mtx->lock 
and mtx->unlock have, been commented out. This was done for two reasons:
1) the mutex is no longer a pointer and thus the syntax is wrong, and 
2) I am farily confident that this implementation was wrong anyway. I 
decided that the tbest thing I could do was to get queue refactored in a 
way that could build for now and then worry about implementing the 
thread safe features later.

## Deletion of PumpManager class

I wasn't sure how to solve the problem of initialising the reference in
PumpManager. So, instead, I have removed this class and replace the 
PumpManager with PumpManagementTask, which is just a function:

    void PumpManagementTask(PumpCommandQueue& command_queue, shared_ptr<Pump> pump, shared_ptr<bool> exit_signal)

Using this, and changing all of the testing code to use this function to
run in the pump management thread instead of the PumpManager, allow the
build to complete.
*/
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


// In the below two methods I have commented out the mutex lock and 
// unlock method calls. These are no longer supposed to be method calls,
// but I am not sure that I have implemented them properly. So, instead 
// of changing the "->" to a "." I have just decided to comment them out
// for now.
void PumpCommandQueue::push(shared_ptr<PumpCommand> command) {
    //mtx->lock();
    commands.push(command);
    //mtx->unlock();
}

shared_ptr<PumpCommand> PumpCommandQueue::pop() {
    //mtx->lock();
    shared_ptr<PumpCommand> pump_command = commands.front();
    commands.pop();
    //mtx->unlock();
    return pump_command;
}

bool PumpCommandQueue::is_empty() {
    return commands.empty();
}


void PumpManagementTask(PumpCommandQueue& command_queue, shared_ptr<Pump> pump, shared_ptr<bool> exit_signal){
    /* This PumpManagementTask is designed to run in its own thread, 
     * where it can manage the pump. This thread will be created by 
     * running:
     * 
     *      std::thread manager_thread(std::bind(PumpManagementTask, std::ref(command_queue), pump_ptr, exit_signal));
     * 
     * The exit signal is a pointer to a boolean that ends the task when
     * it is set to true. The task checks the command queue, executes 
     * tasks, and updates the pump twice per second. Thus pump times 
     * should be precise to withing one second of the requests pumping 
     * time.
     */
    shared_ptr<PumpCommand> pump_command; 
    
    while(!*exit_signal) {
        if (!command_queue.is_empty()) {
            pump_command = command_queue.pop();
            pump_command->execute(pump);
        }
        pump->update();
        if (command_queue.is_empty()) {
            this_thread::sleep_for(chrono::milliseconds(500));
        } else {
        }
    }

}
