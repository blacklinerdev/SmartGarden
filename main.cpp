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
#include "pump.h"
#define PORT 2040

using namespace std;

// Create a global Pump object using wiringPi pin 0 (GPIO 17).
// This shouldn't stay as a global, its just the only way I could think of
// to implement it now.
Pump pump = Pump(0);
vector<string> split(char[]);
void init(int&, struct sockaddr_in&, socklen_t&);
int querry_handler(vector<string>);

int main()
{
    int socket_fd, client;
    bool running = true;
    struct sockaddr_in server_address;
    socklen_t size;

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
        int result = querry_handler(querry);

        //send
        int responsesize = 1024;
        char response[responsesize];
        sprintf(response, "%d", result);
        send(client, response, responsesize, 0);
    }
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
int querry_handler(vector<string> querry)
{
    string device = querry[0];
    if(device == "pump")
    {
        return pump.method(querry);
    }
    if(device == "server")
    {
        return 0;//server::method(querry);
    }
    cout << "No Device found: " << device << endl;
    return -1;
}
