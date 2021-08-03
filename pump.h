#ifndef PUMP_H_
#define PUMP_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <wiringPi.h>
#include <thread>
#include <chrono>

using namespace std;

class Pump {
    
    private:
        int pin;
    
    public:
        Pump(int pumpPin);
        int off();
        int on(int time);
        int method(vector<string> values);
};

#endif // PUMP_H_