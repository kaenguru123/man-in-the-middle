#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <thread>
#include <netdb.h>	//hostent
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <vector>
#include <list>



void sslreadnwrite(SSL* dest, SSL* source, const char* sourceName)
{
    char buff[1024*100] = {0};
    
    while(1)
    {      
        int res = SSL_read ( source, buff, sizeof(buff)-1);
        //std::cout << "read server " << res << std::endl;
        if (res > 0)
        {
            buff[res] = 0;
            char bufcopy[1024*100];
            memcpy(bufcopy, buff, sizeof(bufcopy));
            for (int i=0; i<res; i++)
            {
                if (bufcopy[i] < 0x20)
                {
                    bufcopy[i] = '.';
                }
            }
            std::cout << bufcopy << std::endl;
            int resWrite = SSL_write (dest, buff, res);
            if (resWrite != res)
            {
                std::cout << std::endl<< std::endl << "WRITE PROBLEM " << sourceName << " "<<resWrite << " " << res << " " << errno << std::endl<< std::endl << std::endl;
                
                ERR_print_errors_fp(stderr);
                exit(-1);

            }
        }
        else if (res == 0)
        {
            // disconnect
            std::cout << "sock server disconnect" << std::endl;
            break;
        }
        else
        {
            // socket invalid
            std::cout << "sock server invalid" << std::endl;
            break;
        }
    }
}





    
    
void readnwrite(int& dest, int& source)
{
    char buff[1024*100] = {0};
    
    while(1)
    {      
        int res = read ( source, buff, sizeof(buff)-1);
        //std::cout << "read server " << res << std::endl;
        if (res > 0)
        {
            buff[res] = 0;
            char bufcopy[1024*100];
            memcpy(bufcopy, buff, sizeof(bufcopy));
            for (int i=0; i<res; i++)
            {
                if (bufcopy[i] < 0x20)
                {
                    bufcopy[i] = '.';
                }
            }
            std::cout << bufcopy << std::endl;
            write (dest, buff, res);
        }
        else if (res == 0)
        {
            // disconnect
            std::cout << "sock server disconnect" << std::endl;
            break;
        }
        else
        {
            // socket invalid
            std::cout << "sock server invalid" << std::endl;
            break;
        }
    }
}


class Connection
{
public:
    Connection()
        : m_sock_server(-1)
        , m_sock_web(-1)
        , m_sslServer(nullptr)
        , m_sslClient(nullptr)
        , m_t1(nullptr)
        , m_t2(nullptr)
    {
    }
    
    ~Connection()
    {
        terminate();
        if (m_t1)
        {
            m_t1->join();
        }
        if (m_t2)
        {
            m_t2->join();
        }
        delete m_t1;
        delete m_t2;
    }
    
    void init(int sock_server, SSL* sslServer, const char* hostname, int port, SSL_CTX* ctxClient)
    {
        std::cout << "in init" << std::endl;
        m_sock_server = sock_server;
        m_sslServer = sslServer;
        
        struct sockaddr_in serv_addr;
            
        m_sock_web = socket(AF_INET, SOCK_STREAM, 0);
        m_sslClient = SSL_new(ctxClient);
        
        std::cout << "HOSTBYNAME " << std::endl;

        struct hostent *host;
        struct sockaddr_in addr = {0};
        if ((host = gethostbyname(hostname)) == NULL)
        {
            perror("hostname");
            abort();
        }
	        
        std::cout << "HOSTBYNAME done " << std::endl;

        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_addr.s_addr = *(long *)(host->h_addr_list[0]);
        serv_addr.sin_port = htons(port);  
        
        std::cout << "CONNECT... " << std::endl;
        int resConnect = connect( m_sock_web, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        
        SSL_set_fd(m_sslClient, m_sock_web);
        int status = SSL_connect(m_sslClient);
        if (status != 1)
        {
            SSL_get_error(m_sslClient, status);
            fprintf(stderr, "SSL_connect failed with SSL_get_error code %d\n", status);
        }
        
        std::cout << "CONNECTED " << resConnect << std::endl;
        
        m_t1 = new std::thread([this] () {
            //readnwrite(m_sock_server, m_sock_web);
            sslreadnwrite(m_sslServer, m_sslClient, "Client");
            terminate();
        });
        m_t2 = new std::thread([this] () {
            //readnwrite(m_sock_web, m_sock_server);
            sslreadnwrite(m_sslClient, m_sslServer, "Server");
            terminate();
        });
    }
    
    void terminate()
    {
        if (m_sock_server != -1)
        {
            //SSL_free(m_sslServer);
            ::close(m_sock_server);
            m_sock_server = -1;
        }
        if (m_sock_web != -1)
        {
            //SSL_free(m_sslClient);
            ::close(m_sock_web);
            m_sock_web = -1;
        }
    }
    
private:
    int m_sock_server;
    int m_sock_web;
    SSL* m_sslServer;
    SSL* m_sslClient;
    std::thread* m_t1;
    std::thread* m_t2;
};




SSL_CTX* createContextServer()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) 
    {
	    perror("Unable to create SSL context");
	    exit(EXIT_FAILURE);
    }

    return ctx;
}

SSL_CTX* createContextClient()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_client_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) 
    {
	    perror("Unable to create SSL context");
	    exit(EXIT_FAILURE);
    }

    return ctx;
}


void configureServerContext(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "error cert");
	    exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "privkey.pem", SSL_FILETYPE_PEM) <= 0 ) {
        fprintf(stderr, "error key");
	    exit(EXIT_FAILURE);
    }
}



int main()
{
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
    SSL_CTX* ctxServer = createContextServer();
    SSL_CTX* ctxClient = createContextClient();
    configureServerContext(ctxServer);


    int server_fd = -1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // server
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct linger sl;
    sl.l_onoff = 1;		/* non-zero value enables linger option in kernel */
    sl.l_linger = 0;	/* timeout interval in seconds */
    setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( 443 );

    bind( server_fd, (struct sockaddr *)&address, sizeof(address));
    listen( server_fd, 300);
    
    std::list<Connection> connections;
    
    while(1)
    {
        std::cout << "WAIT..." << std::endl;
        int sock_server = accept ( server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        std::cout << "ACCEPT" << std::endl;
        
        SSL* sslServer = SSL_new(ctxServer);
        SSL_set_fd(sslServer,sock_server);
        if (SSL_accept(sslServer) <= 0) 
        {
            fprintf(stderr, "Error at SSL_accept\n");
            ERR_print_errors_fp(stderr);
            exit(-1);
        }    
        
        connections.resize(connections.size() + 1);
        std::cout << "\nnew connection created" << std::endl;
        Connection& connection = connections.back();
        std::cout << "\ninit" << std::endl;
        connection.init(sock_server, sslServer, "195.200.33.6", 443, ctxClient);
    }
    
    std::cout << "end" << std::endl;

    return 0;
}

