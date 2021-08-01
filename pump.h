#ifndef HELP_H_INCLUDED
#define HELP_H_INCLUDED

#include <sstream>
#include <queue>

using namespace std;
namespace pump
{
    int off();
    int on(int&);

    int method(vector<string> values)
    {
        if(values[1] == "off")
        {
            cout << "Method: off" << endl;
            return off();
        }
        if(values[1] == "on")
        {
            cout << "Method: on" << endl;
            std::stringstream ss(values[2]);
            int val;
            ss >> val;
            return on(val);
        }
        cout << "No Method found: " << values[1] << endl;
        return -1;
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
        unsigned int sec = time * 60;
        try
        {
            //set gpio on
            sleep(sec);
        } catch (...)
        {
            return -1;
        }
        return 0;
    }
}
#endif // HELP_H_INCLUDED
