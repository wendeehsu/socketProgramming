#include <iostream>     //cout
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
    int port;
    struct sockaddr_in server;

public:
    Client();
    bool isConnected = false;
    bool connection(string, int); // create connection with server
    bool send_data(string data);
    void receive();
};

Client::Client()
{
    bzero(&server, sizeof(server)); // initialize
    sock = -1;
    port = 0;
    address = "";
}

bool Client::connection(string address, int port)
{
    // create socket if it is not already created
    if (sock == -1)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0); // Create socket
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

void Client::receive()
{
    char buffer[2000] = {0};

    memset(buffer, '\0', sizeof(buffer));
    int result = recv(sock, buffer, sizeof(buffer), 0);
    cout << "response from server : \n" << buffer << "\n";
}

int main(int argc, char *argv[])
{
    Client client;
    string host;
    int port;

    cout << "type in your host ip and port: ";
    cin >> host >> port;
    client.isConnected = client.connection(host, port); //connect to host
    client.receive();
    
    while(client.isConnected)
    {
        string command;
        cin >> command;
        client.send_data(command);
        client.receive();
    }

    return 0;
}