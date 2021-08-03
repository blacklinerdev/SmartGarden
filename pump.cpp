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

int Pump::method(vector<string> values) {
    
    if(values[1] == "off") {
	cout << "Method: off" << endl;
	return off();
    }
    
    if(values[1] == "on") {
	cout << "Method: on" << endl;
        stringstream ss(values[2]);
        int val;
        ss >> val;
        return on(val);
    }
    
    cout << "No Method found: " << values[1] << endl;
    return -1;
}

int Pump::off() {
    try {
	digitalWrite(pin, LOW);
    } catch(...) {
        cout << "Pump may not shut off" << endl;
        return -1;
    }
    return 0;
}

int Pump::on(int time) {

    if(time < 0) return -1;
    if(time > 60) time = 60;

    else if(time == 0) {
	try {
	    digitalWrite(pin, HIGH);
        }catch(...) {
            return -1;
        }
        return 0;
    }
    unsigned int sec = time * 60;
    try {
        digitalWrite(pin, HIGH);
        // Using the thread sleep_for function should allow for the
        // sleep to be compatible with threads.
        this_thread::sleep_for(std::chrono::seconds(sec));
        off();
    } catch (...) {
        off();      //safety shut off if exception is thrown in try block
	return -1;
    }
    return 0;
}
