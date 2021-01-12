#include "client.h"

const string local_addr = "127.0.0.1";

bool contains(string src, string token){
    return src.rfind(token, 0) == 0;
}

int main(int argc, char *argv[])
{
    Client client;
    int server_port; // the port to connect with the server
    int client_port; // the port listening for connection

    cout << "type in your host port: ";
    cin >> server_port;
    cout << "type in client communication port: ";
    cin >> client_port;

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork fail");
    }
    else if (pid == 0) // child id, listen and print
    {
        client.createSocket(false, local_addr, client_port);
        client.listen_port();
    }
    else // connect to host
    {
        client.createSocket(true, local_addr, server_port); 
        client.isServerConnected = client.connection(true);

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
            if (!withHost) // talk to client
            {
                int port;
                cout << "type in client receiver's port: ";
                cin >> port;
                // client.createSocket(withHost, local_addr, port);
                // client.connection(withHost);
                
                // string command;
                // cout << "command: ";
                // cin >> command;
                // client.send_data(withHost, command);
                // string response = client.receive(withHost);

                int socket_transfer = 0;
                struct sockaddr_in conn_addr;

                socket_transfer = socket(AF_INET, SOCK_STREAM, 0);
                if(socket_transfer < 0)
                {
                    cout << "--> Error in subjectively connectng socket.";
                    continue;
                }

                bzero((char *)&conn_addr, sizeof(conn_addr));
                conn_addr.sin_family = AF_INET;
                conn_addr.sin_addr.s_addr = inet_addr(local_addr.c_str());
                conn_addr.sin_port = htons(port);

                // Connectng
                if(connect(socket_transfer, (struct sockaddr*)&conn_addr, sizeof(conn_addr)) < 0)
                {
                    cout << "--> Error when connecting another client." << endl;
                    continue;
                }

                string command;
                cout << "command: ";
                cin >> command;

                if (send(socket_transfer, command.c_str(), strlen(command.c_str()), 0) < 0)
                {
                    perror("Send failed : ");
                    return false;
                }

                close(socket_transfer);
            }
            else // talk to server
            {
                string command;
                cout << "command: ";
                cin >> command;
                client.send_data(withHost, command);
                string response = client.receive(withHost);

                if (contains(response, "Bye"))
                {
                    client.closeConnection();
                    return 0;
                }
            }
        }
    }

    return 0;
}