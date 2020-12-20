#include <iostream>     //cout
#include <string.h>     //strlen
#include <string>       //string
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <netdb.h>      //hostent
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
using namespace std;

class Host
{
private:
    int server_sock; // socket description
    int new_sock;
    struct sockaddr_in server;

public:
    Host();
    bool createSocket(int port); // create socket
    void listen_port();
    bool send_data(string data);
    void receive();
};

Host::Host()
{
    bzero(&server, sizeof(server));
    server_sock = -1;
    new_sock = -1;
}

bool Host::createSocket(int port)
{
    // create socket if it is not already created
    if (server_sock == -1)
    {
        server_sock = socket(AF_INET, SOCK_STREAM, 0); // Create socket
        if (server_sock == -1)
        {
            perror("Could not create socket");
        }
        cout << "Socket created\n";
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    return true;
}

void Host::listen_port()
{
    int bindStatus = bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    if (bindStatus < 0)
    {
        cout << "Binding port result: " << bindStatus << endl;
        cerr << "Can't binding socket to local address!" << endl;
        exit(0);
    }

    listen(server_sock, 5);

    sockaddr_in newSocketAddr;
    socklen_t newSocketSize = sizeof(newSocketAddr);

    while (true)
    {
        new_sock = accept(server_sock, (sockaddr *)&newSocketAddr, &newSocketSize);
        if (new_sock < 0)
        {
            cout << "new_sock fail: " << new_sock << endl;
            cerr << "Can't accepting the request from client!" << endl;
            exit(0);
        }
        cout << "Connection accepted from " << inet_ntoa(newSocketAddr.sin_addr) << " " << ntohs(newSocketAddr.sin_port) << endl;
        while (true)
        {
            receive();
        }
    }
}

bool Host::send_data(string data)
{
    cout << "Sending data..." << data << "\n";
    if (send(server_sock, data.c_str(), strlen(data.c_str()), 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
}

void Host::receive()
{
    char buffer[2000] = {0};
    memset(buffer, '\0', sizeof(buffer));

    if (recv(new_sock, buffer, sizeof(buffer), 0) > 0)
    {
        cout << "client : \n"
             << buffer << "\n";
    }
}

int main(int argc, char *argv[])
{
    Host myserver;
    int port;

    cout << "type in your port: ";
    cin >> port;
    myserver.createSocket(port);
    myserver.listen_port();

    return 0;
}