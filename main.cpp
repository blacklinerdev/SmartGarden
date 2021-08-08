#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <memory>
#include <functional>
#include <thread>
#include "pump.h"
#define PORT 2040

using namespace std;

vector<string> split(char[]);
void init(int&, struct sockaddr_in&, socklen_t&);
int querry_handler(vector<string>, PumpCommandQueue& pump_command_queue);

int main()
{
    int socket_fd, client;
    int buffersize = 1024;
    char buffer[buffersize];
    bool running = true;
    struct sockaddr_in server_address;
    socklen_t size;
    PumpCommandQueue pump_command_queue;
    
    // Create Pump object using wiringPi pin 0 (GPIO 17).
    std:shared_ptr<Pump> pump_ptr = std::make_shared<Pump>(0); 
    std::shared_ptr<bool> exit_signal = std::make_shared<bool>(0);
    
    thread pump_management_thread(std::bind(PumpManagementTask, std::ref(pump_command_queue), pump_ptr, exit_signal));
    
    // init socket
    init(socket_fd, server_address, size);

    //listening
    while(running)
    {
        cout << "Listening..." << endl;
        listen(socket_fd, 1);

        //accept client
        client = accept(socket_fd, (struct sockaddr*)&server_address, &size);
        if(client < 0)
        {
            cout << "Error accepting clients" << endl;
            exit(3);
        }
        //recieve
        int requestsize = 1024;
        char request[requestsize];
        recv(client, request, requestsize, 0);

        //parse input
        vector<string> querry;
        querry = split(request);
        //handle
        int result = querry_handler(querry, std::ref(pump_command_queue));
    
        //send
        int responsesize = 1024;
        char response[responsesize];
        sprintf(response, "%d", result);
        send(client, response, responsesize, 0);
    }
    *exit_signal = 1;
    pump_management_thread.join();
}

    
    
void init(int& socket_fd, struct sockaddr_in& server_address, socklen_t& size)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        cout << "\nSocket creation failed" << endl;
        exit(-1);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    size = sizeof(server_address);

    int binder = bind(socket_fd, (struct sockaddr*)&server_address, size);
    if(binder < 0)
    {
        cout << "\nSocket binding failed" << endl;
        exit(-2);
    }
}
vector<string> split(char buffer[])
{
    vector<string> result;
    stringstream ss(buffer);
    string token;
    while(getline(ss, token, ' '))
    {
        result.push_back(token);
    }
    return result;
}

int querry_handler(vector<string> querry, PumpCommandQueue& pump_command_queue)
{
    string device = querry[0];
    if(device == "pump")      //fehler
    {
        cout << "Device: " << device << endl;
        
        if (querry[1] == "on") {
            
            stringstream ss(querry[2]);
            int time_to_run_pump;
            ss >> time_to_run_pump;
            
            if (time_to_run_pump == 0) {
                pump_command_queue.push(make_shared<PumpOn>());
            } else if (time_to_run_pump > 0) {
                pump_command_queue.push(make_shared<PumpOnTimed>(time_to_run_pump));
            }
        } else if (querry[1] == "off") {
            pump_command_queue.push(make_shared<PumpOff>());
        } else {
            return -1;
        }
        return 0;
    }
    cout << "No Device found: " << device << endl;
    return -1;
}
