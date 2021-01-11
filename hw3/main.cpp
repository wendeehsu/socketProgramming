#include "server.h" 

int main(int argc, char *argv[])
{
    Host myserver;
    int port;

    cout << "type in your port: ";
    cin >> port;
    myserver.createSocket(port);
    myserver.listen_port();
    myserver.Start();
    myserver.EndServerConnection();
    return 0;
}