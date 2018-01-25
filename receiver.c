#include <sys/types.h> 
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <stdio.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PORT 1223 /*使用的port*/  
#define PACKAGE_SIZE 1016
typedef struct MYTCP
{
    int sequence_num;
    int FIN;
    char content[PACKAGE_SIZE];
}mytcp;
int main()
{  
    int sockfd,len,filelength=0;  
    struct sockaddr_in addr;  
    int addr_len = sizeof(struct sockaddr_in);  
    char buffer[256]; 
    mytcp package; 
    /*建立socket*/  
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){  
        perror ("socket");  
        exit(1);  
    }  
    /*填写sockaddr_in 结构*/  
    bzero ( &addr, sizeof(addr) );  
    addr.sin_family=AF_INET;  
    addr.sin_port=htons(PORT);  
    addr.sin_addr.s_addr=htonl(INADDR_ANY) ;  
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0){  
        perror("connect");  
        exit(1);  
    }  
    while(1){  
        memset(package.content,0,sizeof(package.content));
        int fd = open("package", O_RDWR | O_CREAT,0777);
        len = recvfrom(sockfd,&package,sizeof(mytcp), 0 , (struct sockaddr *)&addr ,&addr_len);  
        while(package.FIN==0)
        {
            write(fd,package.content,strlen(package.content));
            len = recvfrom(sockfd,&package,sizeof(mytcp), 0 , (struct sockaddr *)&addr ,&addr_len);  
        }
        close(fd);
        /*
        //显示client端的网络地址
        printf("receive from %s\n" , inet_ntoa( addr.sin_addr));  
        //将字串返回给client端  
        sendto(sockfd,buffer,len,0,(struct sockaddr *)&addr,addr_len); */ 
    }  
} 