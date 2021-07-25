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
#define PORT 2040

using namespace std;

vector<string> split(char[]);
void init(int&, struct sockaddr_in&, socklen_t&);

int main()
{
    int socket_fd, client;
    int portnum = 2040;
    int buffersize = 1024;
    char buffer[buffersize];
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
        cout << request << endl;

        //send
        int responsesize = 1024;
        char response[responsesize];
        for(int i = 0; i < responsesize; i++)
        {
            response[i] = request[i];
        }
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
    int i = 0;
    for(int j = 0; j < sizeof(buffer); j++)
    {
        if(buffer[j] == ' ' || j == sizeof(buffer))
        {
            string word;
            for(i; i < j - 1; i++)
            {
                word += buffer[i];
            }
            result.push_back(word);
            i = j + 1;
        }
    }
}
