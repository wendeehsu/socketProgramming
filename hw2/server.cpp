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
#include <pthread.h>
#include <vector>
using namespace std;

#define MAX_CLIENT 5

int server_sock = -1;

bool contains(string src, string token){
    return src.rfind(token, 0) == 0;
}

vector<string> split_str(string s)
{
    vector<string> tokens;
    string delimiter = "#";
    string token;
    int pos = 0;
    while ((pos = s.find(delimiter)) != string::npos)
    {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

class Host
{
private:
    pthread_t my_thread[MAX_CLIENT];
    struct sockaddr_in server;
    void* client_thread(void* arg);
    static void *client_thread_helper(void *context);

public:
    Host();
    bool createSocket(int port); // create socket
    void listen_port();
    void Start();
    string handleEvent(vector<string> tokens);
    bool send_data(int client_sock, string data);
    void receive(int client_sock);
};

Host::Host()
{
    bzero(&server, sizeof(server));
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

    listen(server_sock, MAX_CLIENT);
}

bool Host::send_data(int client_sock, string data)
{
    cout << "Sending data..." << data << "\n";
    if (send(client_sock, data.c_str(), strlen(data.c_str()), 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }

    return true;
}

void Host::receive(int client_sock)
{
    char buffer[2000] = {0};
    memset(buffer, '\0', sizeof(buffer));

    if (recv(client_sock, buffer, sizeof(buffer), 0) > 0)
    {
        cout << "client :" << buffer << "\n";
        string data = buffer;
        string response = handleEvent(split_str(data));
        send_data(client_sock, response);
    }
}

void* Host::client_thread_helper(void *context)
{
    return ((Host *)context)->client_thread(context);
}

void* Host::client_thread(void* arg)
{
    int threadID = (int)(size_t)(arg);
    cout << "This is thread : " << threadID << "\n";
    sockaddr_in newSocketAddr;
    socklen_t newSocketSize = sizeof(newSocketAddr);

    while (true)
    {
        int client_sock;
        client_sock = accept(server_sock, (sockaddr *)&newSocketAddr, &newSocketSize);
        if (client_sock < 0)
        {
            cout << "client_sock fail: " << client_sock << endl;
            cerr << "Can't accepting the request from client!" << endl;
            exit(0);
        }
        cout << "Connection accepted from " << inet_ntoa(newSocketAddr.sin_addr) << " " << ntohs(newSocketAddr.sin_port) << endl;
        send_data(client_sock, "Connection accepted!");

        while(true)
        {
            receive(client_sock);
        }
    }

    pthread_exit(NULL);
}

void Host::Start()
{
    for(int i = 0; i < MAX_CLIENT; i++) {
        int ret =  pthread_create(&my_thread[i], NULL, &Host::client_thread_helper, (void*)i);
        if(ret != 0) {
            cout << "Error: pthread_create() failed\n";
            exit(EXIT_FAILURE);
        }
    }

    pthread_join(my_thread[0], NULL);
}

string Host::handleEvent(vector<string> tokens)
{
    string response;
    if (contains(tokens[0], "Exit") && tokens.size() == 1)
    {
        response = "Bye \n";
    }
    else if (contains(tokens[0], "REGISTER") && tokens.size() == 3)
    {
        response = "REGISTER: " + tokens[1] + ", balance = " + tokens[2] + "\n";
    }
    else if (contains(tokens[0], "List") && tokens.size() == 1)
    {
        response = "List my balance \n";
    }
    else if (tokens.size() == 2)
    {
        response = "Log in \n";
    }
    else
    {
        response = "please check your command format \n";
    }
    cout << response;

    return response;
}

int main(int argc, char *argv[])
{
    Host myserver;
    int port;

    cout << "type in your port: ";
    cin >> port;
    myserver.createSocket(port);
    myserver.listen_port();
    myserver.Start();

    return 0;
}