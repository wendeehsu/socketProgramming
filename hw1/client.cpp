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
using namespace std;

class Client
{
private:
    int server_sock; // socket description
    int client_sock;
    struct sockaddr_in server;
    struct sockaddr_in clientReciever;

public:
    Client();
    bool isServerConnected;
    bool connection(bool withHost, string address, int port); // create connection with server
    bool send_data(bool toHost, string data);
    void receive(bool fromHost);
};

Client::Client()
{
    bzero(&server, sizeof(server));                 // initialize server
    bzero(&clientReciever, sizeof(clientReciever)); // initialize clientReciever
    server_sock = -1;
    client_sock = -1;
    isServerConnected = false;
}

bool Client::connection(bool withHost, string address, int port)
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

        // Connect to remote server
        if (connect(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("connect failed. Error");
            return false;
        }
    }
    else
    {
        /* connect with client */
    }

    cout << "Connected\n";
    return true;
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
    }

    return true;
}

void Client::receive(bool fromHost)
{
    char buffer[2000] = {0};
    memset(buffer, '\0', sizeof(buffer));

    if (fromHost)
    {
        int result = recv(server_sock, buffer, sizeof(buffer), 0);
        cout << "response from server : \n" << buffer << "\n";
    }
    else
    {
        /* Recieve from client */
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
    else if (pid == 0)
    { // child id, listen and print
        cout << "pid = 0, i am child \n";
        /* TODO: listen to client port */
    }
    else
    {
        cout << "pid " << pid << ", i am parent, connecting to server \n";
        client.isServerConnected = client.connection(true, host, server_port); //connect to host
        client.receive(true);

        while (client.isServerConnected)
        {
            string receiver;
            cout << "send command to host or client (h/c) ? ";
            cin >> receiver;
            bool withHost = receiver == "h";
            if (!withHost)
            {
                string client_ip;
                int client_port;
                cout << "type in client receiver's ip and host: ";
                cin >> client_ip >> client_port;

                // connect with client
            }

            // send message
            string command;
            cin >> command;
            client.send_data(withHost, command);
            client.receive(withHost);
        }
    }

    return 0;
}