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

class Client
{
private:
    struct sockaddr_in server;
    struct sockaddr_in clientReciever;

public:
    Client();
    int GetClientSock();
    bool isServerConnected;
    bool getClientConnectStatus();
    bool createSocket(bool withHost, string address, int port); // create socket
    bool connection(bool withHost); // create connection
    void listen_port();
    void closeConnection();
    void close_client_connection();
    bool send_data(bool toHost, string data);
    string receive(bool fromHost);
};