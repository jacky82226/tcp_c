#include <sys/types.h> 
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <stdio.h>  
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define PORT 1223
#define PACKAGE_SIZE 1018
#define TIMEOUT 3
#define RECV_BUFFER_SIZE 4

typedef struct MYTCP
{
    int sequence_num;
    int ack;
    short FIN;
    short length;
    char content[PACKAGE_SIZE];
}mytcp;

int decideloss(double lossrate)
{
	int chance=(int)(lossrate*100000);
	if(rand()%100000<chance)
		return 1;
	else
		return 0;
}
int main(int argc, char *argv[])  
{  
    srand (time(NULL));
    double lossrate;
    int s,r,len,sockfd,first_time=1;  
    struct sockaddr_in addr,addr_recv,addr_return;  
    int addr_len =sizeof(struct sockaddr_in); 
    char buffer[256],str[32];
    mytcp package;  
    char **pptr;
    struct hostent *hptr;
    fd_set fds;
    FD_ZERO(&fds);
    sscanf(argv[3],"%lf",&lossrate);
    /*return to receiver*/
    if((r = socket(AF_INET,SOCK_DGRAM,0))<0){  
        perror("socket");  
        exit(1);  
    }  
    /* 填寫sockaddr_in*/  
    bzero(&addr_return,sizeof(addr));  
    addr_return.sin_family = AF_INET;  
    addr_return.sin_port = htons(PORT);
    if( (hptr = gethostbyname(argv[1]) ) == NULL )
    {
        printf("gethostbyname error for host:%s/n", argv[1]);
        return 0;
    }
    switch(hptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            pptr=hptr->h_addr_list;
            for(;*pptr!=NULL;pptr++)
                inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
        break;
        default:
            printf("unknown address type/n");
        break;
    }
    addr_return.sin_addr.s_addr = inet_addr(str); 
    FD_SET(r,&fds);
    /*receiver*/
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0)
    {  
        perror ("socket");  
        exit(1);  
    }  
    /*填寫sockaddr_in 結構*/  
    bzero ( &addr_recv, sizeof(addr_recv) );  
    addr_recv.sin_family=AF_INET;  
    addr_recv.sin_port=htons(PORT);  
    addr_recv.sin_addr.s_addr=htonl(INADDR_ANY) ;  
    if (bind(sockfd, (struct sockaddr *)&addr_recv, sizeof(addr_recv))<0)
    {  
        perror("connect");  
        exit(1);  
    }  
    FD_SET(sockfd,&fds);
    /*sender*/
    if((s = socket(AF_INET,SOCK_DGRAM,0))<0){  
        perror("socket");  
        exit(1);  
    }  
    /* 填寫sockaddr_in*/  
    bzero(&addr,sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(PORT);
	if( (hptr = gethostbyname(argv[2]) ) == NULL )
	{
		printf("gethostbyname error for host:%s/n", argv[1]);
		return 0;
	}
	switch(hptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
			pptr=hptr->h_addr_list;
			for(;*pptr!=NULL;pptr++)
				inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
		break;
		default:
			printf("unknown address type/n");
		break;
	}
    addr.sin_addr.s_addr = inet_addr(str); 
    FD_SET(s,&fds);
    int maxfdp=sockfd>s?sockfd+1:s+1;
    while(1)
    {
        if(select(maxfdp,&fds,NULL,NULL,NULL)>=0)
        {
            if(FD_ISSET(sockfd, &fds)) //package send
            {
                recvfrom(sockfd,&package,sizeof(mytcp), 0 , (struct sockaddr *)&addr_recv ,&addr_len);
                printf("get  data #%d\n",package.sequence_num);
                if(package.FIN==1)
                {
                    printf("get  FIN package\n");
                    sendto(s,&package,sizeof(mytcp),0,(struct sockaddr *)&addr,addr_len); 
                    printf("fwd  FIN package\n");
                }
                else if(decideloss(lossrate))
                {
                   printf("get  data #%d\n",package.sequence_num);
                   printf("drop data #%d, loss rate = %.4lf\n",package.sequence_num,lossrate);
                }
                else
                {
                    printf("get  data #%d\n",package.sequence_num);
                    sendto(s,&package,sizeof(mytcp),0,(struct sockaddr *)&addr,addr_len); 
                    printf("fwd  data #%d, loss rate = %.4lf\n",package.sequence_num,lossrate);
                }
            }
            if(FD_ISSET(s, &fds))   //ack return 
            {
                recvfrom(s,&package,sizeof(mytcp), 0 , (struct sockaddr *)&addr ,&addr_len);
                if(package.ack==0)
                {
                    printf("get  FIN ack\n");
                    sendto(r,&package,sizeof(mytcp),0,(struct sockaddr *)&addr_return,addr_len); 
                    printf("fwd  FIN ack\n");
                }
                else
                {
                    printf("get  ack  #%d\n",package.ack);
                    sendto(r,&package,sizeof(mytcp),0,(struct sockaddr *)&addr_return,addr_len); 
                    printf("fwd  ack  #%d\n",package.ack);
                }
            }
            FD_SET(s,&fds);
            FD_SET(sockfd,&fds);
        }
    }
    return 0;
}