#include "client.h"

int server_sock = -1; // talk to server
int client_sock = -1; // talk to client
int new_sock = -1; // listen from client
char CLIENT_CERT[MAX] = "a.crt";
char CLIENT_PRI[MAX] = "a.key";
SSL *ssl;
SSL *ssl_cs; // client-side server

SSL_CTX* Client::initCTX(void)
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

void Client::Certify(SSL_CTX* ctx, char* cert, char* key)
{
    if(SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0)
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

SSL* Client::GetHostSSL()
{
    return ssl;
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
        Certify(ctx, CLIENT_CERT, CLIENT_PRI);

        // based on CTX and generate new SSL and connect it
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, server_sock);
        if(SSL_connect(ssl) <= 0)
            ERR_print_errors_fp(stderr);
        else
        {
            showCerts(ssl);
            receive(ssl);
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
        
        SSL_CTX *ctx2;
        ctx2 = initCTX();
        Certify(ctx2, CLIENT_CERT, CLIENT_PRI);
        ssl_cs = SSL_new(ctx2);
        SSL_set_fd(ssl_cs, new_sock);
        SSL_accept(ssl_cs);

        cout << "Connection accepted from " << inet_ntoa(newSocketAddr.sin_addr) << " " << ntohs(newSocketAddr.sin_port) << endl;
        send_data(ssl_cs, "--> Client connection accepted!");
        
        while (true)
        {
            receive(ssl_cs);
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

bool Client::send_data(SSL *receiverSSL, string data)
{
    cout << "Sending data..." << data << endl;

    if (SSL_write(receiverSSL, data.c_str(), data.length()) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    return true;
}

string Client::receive(SSL *receiverSSL)
{
    char buffer[MAX];
    bzero(buffer, MAX);

    if (SSL_read(receiverSSL, buffer, sizeof(buffer)) > 0)
    {
        cout << "SSL response: \n" << buffer << endl;
    }
    
    string response = buffer;
    return response;
}