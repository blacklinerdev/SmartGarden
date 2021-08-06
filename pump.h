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

class PumpCommandQueue {
    // PumpCommmandQueue is intended to be a thread safe queue for 
    // sending commands from the querry handler to the pump. It is a 
    // wrapper around a std::queue.
    private:
        shared_ptr<mutex> mtx;
        shared_ptr<condition_variable> cv;
        queue <shared_ptr<PumpCommand>> commands;

    public:
        PumpCommandQueue();
        void push(shared_ptr<PumpCommand> command);
        shared_ptr<PumpCommand> pop();
        shared_ptr<PumpCommand> dequeue();
        bool is_empty();
};

class PumpManager {

    private:
        shared_ptr<PumpCommandQueue> command_queue;
        shared_ptr<Pump> pump;

    public:
        PumpManager(shared_ptr<PumpCommandQueue> commandQueue, shared_ptr<Pump> pump_to_manage);
        void task();
        void cont_task(shared_ptr<bool> exit_signal);
};

#endif // PUMP_H_
