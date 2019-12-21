#include <iostream>
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <thread>
#include <netdb.h>	//hostent

int sock_web = 0;
int sock_server = 0;

int server_fd = -1;
struct sockaddr_in address;
int addrlen = sizeof(address);
        
void readnwrite()
{
    char buff[1024*10] = {0};
    
    while(1)
    {      
        int res = read ( sock_server, buff, sizeof(buff)-1);
        //std::cout << "read server " << res << std::endl;
        if (res > 0)
        {
            buff[res] = 0;
            char bufcopy[1024*10];
            memcpy(bufcopy, buff, sizeof(bufcopy));
            for (int i=0; i<res; i++)
            {
                if (bufcopy[i] < 0x20)
                {
                    bufcopy[i] += 128;
                }
            }
            std::cout << bufcopy << std::endl;
            write (sock_web, buff, res);
        }
        else if (res == 0)
        {
            // disconnect
            std::cout << "sock server disconnect" << std::endl;
            exit(-1);
        }
        else
        {
            // socket invalid
            std::cout << "sock server invalid" << std::endl;
            exit(-1);
        }
    }
}



int main()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct linger sl;
    sl.l_onoff = 1;		/* non-zero value enables linger option in kernel */
    sl.l_linger = 0;	/* timeout interval in seconds */
    setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));

    struct sockaddr_in serv_addr;
        
    sock_web = socket(AF_INET, SOCK_STREAM, 0);

    struct hostent *host;
    struct sockaddr_in addr = {0};
    if ((host = gethostbyname("195.200.33.6")) == NULL)
    {
        perror("hostname");
        abort();
    }
	    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = *(long *)(host->h_addr_list[0]);
    serv_addr.sin_port = htons(443); 
    
    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( 443 );

    bind( server_fd, (struct sockaddr *)&address, sizeof(address));
    listen( server_fd, 30);
    
    sock_server = accept ( server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    
    int resConnect = connect( sock_web, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //write ( sock_, "Good Morning", 12);
    
    std::cout << "CONNECT " << resConnect << std::endl;
    
    std::thread t1(readnwrite);
    
   
    while(1)
    {      
    
        char buff[1024*10] = {0};
        
        int res = read (sock_web, buff, sizeof(buff)-1);
        //std::cout << "read web " << res << std::endl;
        if (res > 0)
        {
            buff[res] = 0;
            char bufcopy[1024*10];
            memcpy(bufcopy, buff, sizeof(bufcopy));
            for (int i=0; i<res; i++)
            {
                if (bufcopy[i] < 0x20)
                {
                    bufcopy[i] += 128;
                }
            }
            std::cout << bufcopy << std::endl;
            write ( sock_server, buff, res);
        }
        else if (res == 0)
        {
            // disconnect
            std::cout << "sock web disconnect" << std::endl;
            exit(-1);
        }
        else
        {
            // socket invalid
            std::cout << "sock web invalid" << std::endl;
            exit(-1);
        }
    }
    
    std::cout << "end" << std::endl;

    return 0;
}

