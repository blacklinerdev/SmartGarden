#ifndef PUMP_H_
#define PUMP_H_

#include <queue>
#include <iostream>
#include <wiringPi.h>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <ctime>

using namespace std;

// Pump allows for the control of a single pump. Allows for the pump to 
// be turned on and off. The pump must be connected to one of the GPIO 
// pins of the Raspberry Pi, and the pin number of this pin must be 
// provided to the constructor following the wiringPi pin numbering 
// scheme. Details of this pin numbering scheme can be found here:
//
//      projects.drogan.net/raspberry-pi/wiringpi/pins/ 
//
// Example:
//      
//      // Create a pump object to control a pump that is connected to 
//      // wiringPi pin 0 (GPIO 17):
//      Pump pump = Pump(0);
//
//      // Now set the pump to run for 30 seconds and turn it on:
//      pump.set_run_time(30);
//      pump.on();
//
//      // Now we must call the update() method regularly:
//      while( ... ) {
//          
//          ...
//          
//          pump.update();
//          
//          // sleep for a bit (~500 ms)
//
//      }
//
// The pump will track the ammount of time has been running and turn 
// itself off after the requested 30 seconds, as long as the update() 
// method is called on it regularly.      
class Pump {
    
    private:
        int pin;
        bool on_flag = 0;
        time_t running_since;
        unsigned int time_to_run = 0;
        bool is_on_timed();
        unsigned int get_running_time();
        bool has_reached_or_exceeded_time_to_run();
    
    public:
        Pump(int pumpPin);
        int off();
        int on();
        void set_run_time(unsigned int seconds_to_run);
        bool is_on();
        void update();
};


// PumpCommand follow a modification of the GOF command pattern for 
// sending commands to the pump. Three commands are implemented, these 
// are:
//
//      1) PumpOn() - Turns the pump on indefinitely.
//      2) PumpOff() - Turns the pump off indefinitely.
//      3) PumpOnTimed(unsigned int pump_time) - Turns the pump on for 
//          the number of minutes specified by pump_time (minimum times 
//          is 1 minute, maximum time is 60 minutes).
//
// Unlike the GOF command pattern, the pump on which to execute the 
// command is not bound to the command when it is constructed. Instead, 
// the pump is provided in the arguement to the execute command by the 
// invoker.
//
// Example:
//
//      // Create a pump and then create a command to turn it on for 5 
//      // minutes:
//      std::shared_ptr<Pump> pump = std:make_shared<Pump>(0);
//      PumpOnTimed command_on_five_mins = PumpOnTimed(5);
//      
//      // Now use the command to turn the pump on for five mins:
//      command_on_five_mins.execute(pump);
//
// Remember: in the above example, in order to ensure the pump runs for 
// 5 mins will require constant, regular calling of the pump's `update` 
// method.
class PumpCommand {
    public:
        virtual ~PumpCommand() {};
        virtual void execute(shared_ptr <Pump> pump) = 0;
};

class PumpOn : public PumpCommand {
    public:
        void execute(shared_ptr <Pump> pump) override;
};

class PumpOff : public PumpCommand {
    public:
        void execute(shared_ptr <Pump> pump) override;
};

class PumpOnTimed : public PumpCommand {
    private:
        unsigned int pump_time;

    public:
        PumpOnTimed(unsigned int pump_time);
        void execute(shared_ptr <Pump> pump) override;
};


// PumpCommmandQueue is a thread safe queue for sending commands to the 
// pump when it is running in a separate thread.
//
// Example:
// 
//      // Create a queue (this can be sent, using std::ref, to the 
//      // thread in which the pump is running.)
//      PumpCommandQueue command_queue;
//            
//      // Create a PumpCommand and send it to the queue, for example, 
//      // we can command the pump to turn off:
//      std::shared_ptr<PumpCommand> command = std::make_shared<PumpOff>();
//      command_queue.push(command);
class PumpCommandQueue {
    
    private:
        mutex mtx;
        condition_variable cv;
        queue <shared_ptr<PumpCommand>> commands;

    public:
        PumpCommandQueue() = default;
        PumpCommandQueue(const PumpCommandQueue&) = delete; // This disables copying
        PumpCommandQueue& operator=(const PumpCommandQueue&) = delete; // This disables assignment
        void push(shared_ptr<PumpCommand> command);
        shared_ptr<PumpCommand> pop();
        shared_ptr<PumpCommand> dequeue();
        bool is_empty();
};

void PumpManagementTask(PumpCommandQueue& command_queue, shared_ptr<Pump> pump, shared_ptr<bool> exit_signal);

#endif // PUMP_H_
