#include "pump.h"

Pump::Pump(int pin)
{
    Pump::pin = pin;
    //wiringPiSetup();
}
int Pump::off()
{
    try
    {
        //pinMode(pin, OUTPUT);
        //digitalWrite(pin, LOW);
        return 0;
    } catch(...){return -1;}
}
int Pump::on()
{
    try
    {
        //pinMode(pin, OUTPUT);
        //digitalWrite(pin, HIGH);
        return 0;
    } catch (...) {return -1;}
}
int Pump::on(int sec)
{
    try
    {
        //pinMode(pin, OUTPUT);
        //digitalWrite(pin, HIGH);
        //shut off at time
        return 0;
    } catch (...) {return -1;}
}
int Pump::method(vector<string> querry)
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
        } catch(...)
        {
            return -1;
        }
    }
    return 0;
}
bool Pump::is_running()
{
    int state;
    try
    {
        state = 0;//state = digitalRead(pin);
    } catch(...) { return false;}
    return state;
}
