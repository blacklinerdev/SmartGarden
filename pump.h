#ifndef HELP_H_INCLUDED
#define HELP_H_INCLUDED

#include <sstream>
#include <queue>

namespace pump
{
    int off();
    int on(int&);

    int method(char* values[])
    {
        if(strcmp(values[1], "off"))
        {
            return off();
        }
        if(strcmp(values[1], "on"))
        {
            std::stringstream ss(values[2]);
            int val;
            ss >> val;
            return on(val);
        }
    }

    int off()
    {
        try
        {
            //set gpio off
        } catch(...)
        {
            return -1;
        }
        return 0;
    }
    int on(int& time)
    {
        if(time < 0) return -1;
        if(time > 60) time = 60;
        else if(time == 0)
        {
            try
            {
                //set gpio on
            }catch(...)
            {
                return -1;
            }
            return 0;
        }
        int sec = time * 60;
        try
        {
            //set gpio on
        } catch (...)
        {
            return -1;
        }
        return 0;
    }
}
#endif // HELP_H_INCLUDED
