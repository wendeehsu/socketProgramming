#include "server.h"

SSL *ssl[MAX];
SSL_CTX *ctx;
char SERVER_CERT[MAX] = "server.crt";
char SERVER_PRI[MAX] = "server.key";

int server_sock = -1;
int server_port = -1;
const string server_addr = "127.0.0.1";
vector<Account> accounts; // account list

bool contains(string src, string token)
{
    return src.find(token) != string::npos;
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

Account::Account(string _name, int _balance)
{
    name = _name;
    balance = _balance;
    sd = -1;
}

Host::Host()
{
    bzero(&server, sizeof(server));
}

void CertifyServer(SSL_CTX* ctx)
{ 
    if (SSL_CTX_use_certificate_file(ctx, SERVER_CERT, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    
    if (SSL_CTX_use_PrivateKey_file(ctx, SERVER_PRI, SSL_FILETYPE_PEM) <= 0)
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

    // verify client??
}

SSL_CTX* initCTXServer(void)
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

bool Host::createSocket(int port)
{
    // initialize SSL
    ctx = initCTXServer();
    CertifyServer(ctx);

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

    server.sin_addr.s_addr = inet_addr(server_addr.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server_port = port;

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
    if (SSL_write(ssl[client_sock], data.c_str(), data.length()) < 0)
    {
        perror("Send failed : ");
        return false;
    }

    return true;
}

void Host::receive(int client_sock)
{
    char buffer[MAX];
    bzero(buffer, MAX);
    
    if (SSL_read(ssl[client_sock], buffer, sizeof(buffer)) > 0)
    {
        cout << "SSL client: " << buffer << "\n";
        string data = buffer;
        string response = handleEvent(client_sock, split_str(data));
    }
}

void *Host::client_thread_helper(void *context)
{
    return ((Host *)context)->client_thread(context);
}

void *Host::client_thread(void *arg)
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
        cout << "client_sock = " << client_sock << endl;

        ssl[client_sock] = SSL_new(ctx);
        SSL_set_fd(ssl[client_sock], client_sock);
        int sslAcceptRes = SSL_accept(ssl[client_sock]);
        if(sslAcceptRes < 0)
        {
            cout << "--> No SSL acception at server." << endl;
            continue;
        }
        cout << "sslAcceptRes = " << sslAcceptRes << endl;
        
        cout << "Connection accepted from " << inet_ntoa(newSocketAddr.sin_addr) << " " << ntohs(newSocketAddr.sin_port) << endl;

        string msg = "--> Connection accepted.";
        send_data(client_sock, msg);

        while (true)
        {
            receive(client_sock);
        }

        EndConnection(client_sock);
    }

    pthread_exit(NULL);
}

void Host::Start()
{
    for (int i = 0; i < 2; i++)
    {
        int ret = pthread_create(&my_thread[i], NULL, &Host::client_thread_helper, (void *)i);
        if (ret != 0)
        {
            cout << "Error: pthread_create() failed\n";
            exit(EXIT_FAILURE);
        }
    }

    pthread_join(my_thread[0], NULL);
}

string Host::handleEvent(int client_sock, vector<string> tokens)
{
    string response;

    if (contains(tokens[0], "Exit") && tokens.size() == 1)
    {
        response = "Bye \n";
    }
    else if (contains(tokens[0], "REGISTER") && tokens.size() == 3)
    {
        response = RegisterAccount(tokens);
    }
    else if (contains(tokens[0], "List") && tokens.size() == 1)
    {
        response = GetOnlineAccounts(client_sock);
    }
    else if (tokens.size() == 2)
    {
        response = Login(client_sock, tokens[0]);
    }
    else if (contains(tokens[0], "TRANS") && tokens.size() == 4)
    {
        string payer = tokens[1];
        string receiver = tokens[3];
        int num = stoi(tokens[2]);
        string accounts = GetOnlineAccounts(client_sock);
        if (contains(accounts, payer) && contains(accounts, receiver))
        {
            response = Transaction(payer, num, receiver);
        }
        else
        {
            response = "Account is not online. \n";
        }
    }
    else
    {
        response = "please check your command format \n";
    }
    cout << response;
    send_data(client_sock, response);

    return response;
}

string Host::RegisterAccount(vector<string> data)
{
    string name = data[1];
    bool accountExist = false;
    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].name == name)
        {
            accountExist = true;
        }
    }

    if (accountExist)
    {
        return "210 FAIL\n";
    }
    else
    {
        Account newUser(name, stoi(data[2]));
        accounts.push_back(newUser);
        return "100 OK\n";
    }
}

string Host::GetOnlineAccounts(int sd)
{
    int balance = -1;
    int numLoggined = 0;
    string accountsLoggined;
    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].sd != -1) // loggined 
        {
            if (accounts[i].sd == sd)
            {
                balance = accounts[i].balance;
            }

            numLoggined++;
            accountsLoggined += accounts[i].name + "#" + server_addr + "#" + to_string(server_port) + "\n";
        }
    }

    if(balance == -1)
    {
        return "please login first \n";
    }

    string response = to_string(balance) + "\n";
    response += to_string(numLoggined) + "\n" + accountsLoggined;
    return response;
}

string Host::Login(int sd, string name)
{
    int accountIndex = -1;
    bool isLogined = false;
    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].name == name)
        {
            accountIndex = i;

            if (accounts[i].sd != -1)
            {
                isLogined = true;
            }
            else
            {
                accounts[i].sd = sd;
            }
        }
    }

    if (accountIndex != -1) // account exist
    {
        if (isLogined) // handle second-login-error
        {
            return "account has been logged in \n";
        }
        else
        {
            return GetOnlineAccounts(sd);
        }
    }
    else
    {
        return "220 AUTH_FAIL\n";
    }
}

string Host::Transaction(string payer, int num, string receiver)
{
    // check num enough
    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].name == payer)
        {
            bool balanceEnough = accounts[i].balance >= num;
            if (balanceEnough)
            {
                accounts[i].balance -= num;
            }
            else
            {
                return "Balance not enough";
            }
            break;    
        }
    }

    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].name == receiver)
        {
            
            accounts[i].balance += num;
            break;          
        }
    }

    return "100 OK\n"
}


void Host::EndConnection(int sd)
{
    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].sd == sd)
        {
            accounts[i].sd = -1;
        }
    }

    SSL_shutdown(ssl[sd]);
    SSL_free(ssl[sd]);
    close(sd);
}

void Host::EndServerConnection()
{
    close(server_sock);
    SSL_CTX_free(ctx);
}