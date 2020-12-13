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
    int server_sock; // socket description
    int client_sock;
    int new_sock;
    struct sockaddr_in server;
    struct sockaddr_in clientReciever;

public:
    Client();
    bool isServerConnected;
    bool getClientConnectStatus();
    bool createSocket(bool withHost, string address, int port); // create socket
    bool connection(bool withHost); // create connection
    void listen_port();
    void close_client_connection();
    bool send_data(bool toHost, string data);
    void receive(bool fromHost);
};

Client::Client()
{
    bzero(&server, sizeof(server));                 // initialize server
    bzero(&clientReciever, sizeof(clientReciever)); // initialize clientReciever
    server_sock = -1;
    client_sock = -1;
    new_sock = -1;
    isServerConnected = false;
}

bool Client::createSocket(bool withHost, string address, int port)
{
    if (withHost)
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

        server.sin_addr.s_addr = inet_addr(address.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
    }
    else
    {        
        /* connect with client */
        if (client_sock == -1)
        {
            client_sock = socket(AF_INET, SOCK_STREAM, 0); // Create socket
            if (client_sock == -1)
            {
                perror("Could not create socket with client");
            }
            cout << "Client socket created\n";
        }

        clientReciever.sin_addr.s_addr = inet_addr(address.c_str());
        clientReciever.sin_family = AF_INET;
        clientReciever.sin_port = htons(port);
    }

    return true;
}

bool Client::connection(bool withHost)
{
    if (withHost)
    {
        // Connect to remote server
        if (connect(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("server connect failed.");
            return false;
        }
    }
    else
    {
        // Connect to client server
        if (connect(client_sock, (struct sockaddr *)&clientReciever, sizeof(clientReciever)) < 0)
        {
            perror("client connect failed.");
            return false;
        }
    }

    return true;
}

void Client::listen_port()
{
    int bindStatus = bind(client_sock, (struct sockaddr *)&clientReciever, sizeof(clientReciever));
    if (bindStatus < 0)
    {
        cout << "Binding port result: " << bindStatus << endl;
        cerr << "Can't binding socket to local address!" << endl;
        exit(0);
    }

    listen(client_sock, 5);

    sockaddr_in newSocketAddr;
    socklen_t newSocketSize = sizeof(newSocketAddr);

    while(true)
    {
        new_sock = accept(client_sock, (sockaddr *)&newSocketAddr, &newSocketSize);
        if (new_sock < 0)
        {
            cout << "new_sock fail: " << new_sock << endl;
            cerr << "Can't accepting the request from client!" << endl;
            exit(0);
        }

        cout << "Connection accepted from " << inet_ntoa(newSocketAddr.sin_addr) << " " << ntohs(newSocketAddr.sin_port) << endl;
        while (true)
        {
            receive(false);
        }
    }
}

bool Client::getClientConnectStatus()
{
    cout << "new_sock: " << new_sock << ", client_sock: " << client_sock << endl;
    if (new_sock == -1 || client_sock == -1)
    {
        return false;
    }
    return true;
}

void Client::close_client_connection()
{
    close(new_sock);
    client_sock = -1;
    new_sock = -1;
}

bool Client::send_data(bool toHost, string data)
{
    cout << "Sending data to ";

    if (toHost)
    {
        cout << "host..." << data << "\n";
        if (send(server_sock, data.c_str(), strlen(data.c_str()), 0) < 0)
        {
            perror("Send failed : ");
            return false;
        }
    }
    else
    {
        cout << "client..." << data << "\n";
        if (send(client_sock, data.c_str(), strlen(data.c_str()), 0) < 0)
        {
            perror("Send failed : ");
            return false;
        }
    }

    return true;
}

void Client::receive(bool fromHost)
{
    char buffer[2000] = {0};
    memset(buffer, '\0', sizeof(buffer));

    if (fromHost)
    {
        if (recv(server_sock, buffer, sizeof(buffer), 0) > 0)
        {
            cout << "response from server : \n" << buffer << "\n";
        }
    }
    else
    {
        if (recv(new_sock, buffer, sizeof(buffer), 0) > 0) 
        {
            cout << "response from client : \n" << buffer << "\n";
        }
    }
}

int main(int argc, char *argv[])
{
    Client client;
    string host;
    int server_port;

    cout << "type in your host ip and port: ";
    cin >> host >> server_port;

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork fail");
    }
    else if (pid == 0) // child id, listen and print
    {
        string client_add = "127.0.0.1";
        int client_port = 8888;

        client.createSocket(false, client_add, client_port);
        client.listen_port();
    }
    else
    {
        client.createSocket(true, host, server_port); //connect to host
        client.isServerConnected = client.connection(true);
        client.receive(true);

        while (client.isServerConnected)
        {
            string receiver;
            cout << "send command to host or client (h/c) ? ";
            cin >> receiver;

            if (receiver != "h" and receiver != "c")
            {
                cout << "please type `h` or `c` \n";
                continue;
            }

            bool withHost = receiver == "h";
            if (!withHost && !client.getClientConnectStatus())
            {
                string client_ip;
                int client_port;
                cout << "type in client receiver's ip and host: ";
                cin >> client_ip >> client_port;
                client.createSocket(withHost, client_ip, client_port);
                client.connection(withHost);
            }

            // send message
            string command;
            cout << "command: ";
            cin >> command;
            client.send_data(withHost, command);
            client.receive(withHost);
        }
    }

    return 0;
}