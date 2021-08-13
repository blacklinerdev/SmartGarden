#ifndef DEVICES_H
#define DEVICES_H
#include "pump.h"

class Devices
{
    public:
        Devices();
        void set_pump();
        Pump get_pump();

    private:
        Pump pump;

};

#endif // DEVICES_H
