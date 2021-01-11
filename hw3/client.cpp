#include "client.h"

int server_sock = -1; // talk to server
int client_sock = -1; // talk to client
int new_sock = -1; // listen from client
char CLIENT_CERT[MAX] = "a.crt";
char CLIENT_PRI[MAX] = "a.key";
SSL *ssl;

SSL_CTX* initCTX(void)
{
    SSL_CTX *ctx;
    const SSL_METHOD *ssl_method;

    SSL_library_init();
    SSL_load_error_strings();
    ssl_method = SSLv23_method();
    ctx = SSL_CTX_new(ssl_method);
    if(ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void CertifyClient(SSL_CTX* ctx)
{
    if(SSL_CTX_use_certificate_file(ctx, CLIENT_CERT, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, CLIENT_PRI, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    
    if (!SSL_CTX_check_private_key(ctx))
    {
        ERR_print_errors_fp(stderr);
        cout << "--> Private key does not match the public certification." << endl;
        abort();
    }
}

void showCerts(SSL *ssl)
{
    X509 *certification;
    char *certResult;
    
    certification = SSL_get_peer_certificate(ssl);
    if (certification != NULL)
    {
        cout << "Digital certificate information: " << endl;
        certResult = X509_NAME_oneline(X509_get_subject_name(certification), 0, 0);
        cout << "Certification: " << certResult << endl;
        free(certResult);
        certResult = X509_NAME_oneline(X509_get_issuer_name(certification), 0, 0);
        cout << "Issuer: " << certResult << endl;
        free(certResult);
        X509_free(certification);
    }
    else
        cout << "--> No certification!" << endl;
}

Client::Client()
{
    bzero(&server, sizeof(server));                 // initialize server
    bzero(&clientReciever, sizeof(clientReciever)); // initialize clientReciever
    isServerConnected = false;
}

int Client::GetClientSock()
{
    return client_sock;
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

        // initialize SSL
        SSL_CTX *ctx;
        ctx = initCTX();
        CertifyClient(ctx);

        // based on CTX and generate new SSL and connect it
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, server_sock);
        if(SSL_connect(ssl) <= 0)
            ERR_print_errors_fp(stderr);
        else
        {
            showCerts(ssl);
            receive(true);
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

void Client::closeConnection()
{
    cout << "end connection!";
    close(server_sock);
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

        if (SSL_write(ssl, data.c_str(), data.length()) < 0)
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

string Client::receive(bool fromHost)
{
    char buffer[MAX];
    bzero(buffer, MAX);

    if (fromHost)
    {
        if (SSL_read(ssl, buffer, sizeof(buffer)) > 0)
        {
            cout << "SSL Server response: \n" << buffer << endl;
        }
    }
    else
    {
        if (recv(new_sock, buffer, sizeof(buffer), 0) > 0) 
        {
            cout << "response from client : \n" << buffer << "\n";
        }
    }
    
    string response = buffer;
    return response;
}