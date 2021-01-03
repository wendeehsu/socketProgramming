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

#define MAX_CLIENT 10

class Host
{
private:
    pthread_t my_thread[MAX_CLIENT];
    struct sockaddr_in server;
    void *client_thread(void *arg);
    static void *client_thread_helper(void *context);

public:
    Host();
    bool createSocket(int port); // create socket
    void listen_port();
    void Start();
    bool send_data(int client_sock, string data);
    void receive(int client_sock);
    string handleEvent(int client_sock, vector<string> tokens);
    string RegisterAccount(vector<string> data);
    string GetOnlineAccounts(int sd);
    string Login(int sd, string name);
    void EndConnection(int sd);
};

class Account
{
public:
    Account(string _name, int _balance);
    string name;
    int balance;
    int sd;
};