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
        int off()
        {
            try
            {
                //pinMode(pin, OUTPUT);
                //digitalWrite(pin, LOW);
                return 0;
            } catch(exception e){return -1;}
        }
        int on()
        {
            try{
                //pinMode(pin, OUTPUT);
                //digitalWrite(pin, HIGH);
                return 0;
            } catch (exception e) {return -1;}
        }
        int on(int sec)
        {
            try{
                //pinMode(pin, OUTPUT);
                //digitalWrite(pin, HIGH);
                //shut off at time
                return 0;
            } catch (exception e) {return -1;}
        }

    public:
        Pump(int pin_num)
        {
            pin = pin_num;
            //wiringPiSetup();
        }
        int method(vector<string> querry)
        {
            string method = querry[1];
            if(method == "off")
            {
                off();
                return 0;
            }
            if(method == "on")
            {
                try
                {
                    int value;
                    istringstream(querry[2]) >> value;
                    if(value < 0) return -1;
                    if(value > 60 ) value = 60;
                    if(value == 0) on();
                    else
                    {
                        int seconds = value * 60;
                        on(seconds);
                    }
                } catch(exception e)
                {
                    return -1;
                }
            }
            return 0;
        }
        bool is_running()
        {
            int state = digitalRead(pin);
            return state;
        }
};

#endif // PUMP_H_
