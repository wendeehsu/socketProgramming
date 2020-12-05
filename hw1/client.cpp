// reference: 
// https://www.binarytides.com/code-a-simple-socket-client-class-in-c/

#include <iostream>     //cout
// #include <stdio.h>      //printf
#include <string.h>     //strlen
#include <string>       //string
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <netdb.h>      //hostent

using namespace std;

class Client
{
private:
    int sock; // socket description
    string address;
    string response_data = "";
    int port;
    struct sockaddr_in server;

public:
    Client();
    bool connection(string, int); // create connection with server
    bool send_data(string data);
    bool register_user(string name, int num);
    string receive(int);
};

Client::Client()
{
    sock = -1;
    port = 0;
    address = "";
}

bool Client::connection(string address, int port)
{
    // create socket if it is not already created
    if (sock == -1)
    {
        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            perror("Could not create socket");
        }

        cout << "Socket created\n";
    }

    server.sin_addr.s_addr = inet_addr(address.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return false;
    }

    cout << "Connected\n";
    return true;
}

bool Client::send_data(string data)
{
    cout << "Sending data...";
    cout << data << "\n";

    if (send(sock, data.c_str(), strlen(data.c_str()), 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }

    cout << "Data send\n";
    return true;
}

bool Client::register_user(string name, int num)
{
    string msg = "REGISTER#<"+ name +">#<" + to_string(num) + "><CRLF>";
    send_data(msg);
}

string Client::receive(int size = 512)
{
    char buffer[size];
    string reply;

    //Receive a reply from the server
    if (recv(sock, buffer, sizeof(buffer), 0) < 0)
    {
        puts("recv failed");
        return NULL;
    }

    reply = buffer;
    response_data = reply;

    return reply;
}

int main(int argc, char *argv[])
{
    Client client;
    string host = "127.0.0.1";

    //connect to host
    client.connection(host, 5000);

    client.register_user("wendee",20);

    return 0;
}