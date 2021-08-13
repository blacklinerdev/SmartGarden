#ifndef PUMP_H_
#define PUMP_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "wiringPi.h"

using namespace std;

class Pump {

    private:
        int pin = -1;
        int off();
        int on();
        int on(int sec);

    public:
        Pump(int pin);
        int method(vector<string> querry);
        bool is_running();
};

#endif // PUMP_H_
