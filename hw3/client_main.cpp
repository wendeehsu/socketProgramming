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

                SSL_CTX *ctx1;
                SSL *ssl1;
                char CLIENT_B_CERT[MAX] = "b.crt";
                char CLIENT_B_PRI[MAX] = "b.key";
                
                ctx1 = client.initCTX();
                client.Certify(ctx1, CLIENT_B_CERT, CLIENT_B_PRI);
                ssl1 = SSL_new(ctx1);
                SSL_set_fd(ssl1, socket_transfer);
                
                if(SSL_connect(ssl1) <= 0)
                    ERR_print_errors_fp(stderr);
                else
                {
                    client.receive(ssl1);

                    string command;
                    cout << "command: ";
                    cin >> command;
                    
                    client.send_data(ssl1, command);
                }

                SSL_free(ssl1);
                close(socket_transfer);
            }
            else // talk to server
            {
                string command;
                cout << "command: ";
                cin >> command;
                client.send_data(client.GetHostSSL(), command);
                string response = client.receive(client.GetHostSSL());

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