#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <thread>
#include <netdb.h>	//hostent
#include <vector>
#include <list>


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
    
    void init(int &sock_server, const char* hostname, int port)
    {
        std::cout << "in init" << std::endl;
        m_sock_server = sock_server;
        
        struct sockaddr_in serv_addr;
            
        m_sock_web = socket(AF_INET, SOCK_STREAM, 0);
        
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
        std::cout << "CONNECTED " << resConnect << std::endl;
        
        m_t1 = new std::thread([this] () {
            readnwrite(m_sock_server, m_sock_web);
            terminate();
        });
        m_t2 = new std::thread([this] () {
            readnwrite(m_sock_web, m_sock_server);
            terminate();
        });
    }
    
    void terminate()
    {
        if (m_sock_server != -1)
        {
            ::close(m_sock_server);
            m_sock_server = -1;
        }
        if (m_sock_web != -1)
        {
            ::close(m_sock_web);
            m_sock_web = -1;
        }
    }
    
private:
    int m_sock_server;
    int m_sock_web;
    std::thread* m_t1;
    std::thread* m_t2;
};


int main()
{
    int sock_web = 0;
    int sock_server = 0;

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
    listen( server_fd, 30);
    
    std::list<Connection> connections;
    
    while(1)
    {
        std::cout << "WAIT..." << std::endl;
        sock_server = accept ( server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        std::cout << "ACCEPT" << std::endl;
        connections.resize(connections.size() + 1);
        std::cout << "\nnew connection created" << std::endl;
        Connection& connection = connections.back();
        std::cout << "\ninit" << std::endl;
        connection.init(sock_server, "62.181.152.213", 443);
    }
    
    std::cout << "end" << std::endl;

    return 0;
}

