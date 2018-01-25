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

#define PORT 1223
#define PACKAGE_SIZE 1012
#define TIMEOUT 1
#define RECV_BUFFER_SIZE 100

int destination=0;
int getdest(int PATH_SIZE)
{
    return destination=(++destination>=PATH_SIZE)? 0:destination;
}

typedef struct MYTCP
{
    int sequence_num;
    int ack;
    short FIN;
    short length;
    char content[PACKAGE_SIZE];
}mytcp;

int main(int argc, char *argv[])  
{  
    int len,winSize=1,threshold=1,winOffset=0,i,j,hasFIN=0,filenum=0;  
    int addr_len =sizeof(struct sockaddr_in); 
    char buffer[256],str[32];
    mytcp package,ackpackage;  
    char **pptr;
    struct hostent *hptr;
    struct timeval timeout={0,250000};
    if(argc==1)                 //receiver
    {
     /*建立socket*/  
        int recv_buffer_offset=1,recv_buffer_max=0,sockfd;
        struct sockaddr_in addr;  
        mytcp recv_buffer[RECV_BUFFER_SIZE];
        if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){  
            perror ("socket");  
            exit(1);  
        }  
        /*填寫sockaddr_in 結構*/  
        bzero ( &addr, sizeof(addr) );  
        addr.sin_family=AF_INET;  
        addr.sin_port=htons(PORT);  
        addr.sin_addr.s_addr=htonl(INADDR_ANY) ;  
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0){  
            perror("connect");  
            exit(1);  
        }  
        while(1)
        {  
            for(i=0;i<RECV_BUFFER_SIZE;++i)
            {
                memset(recv_buffer[i].content,0,sizeof(recv_buffer[i].content));
                recv_buffer[i].length=0;
            }memset(package.content,0,sizeof(package.content));
            char filename[256];
            sprintf(filename, "file%d", filenum++);
            recv_buffer_offset=1;
            recv_buffer_max=0;
            len = recvfrom(sockfd,&package,sizeof(mytcp), 0 , (struct sockaddr *)&addr ,&addr_len); 
            int fd = open(filename, O_RDWR | O_CREAT,0777); 
            while(package.FIN!=1)
            {
                if(recv_buffer_offset+RECV_BUFFER_SIZE<=package.sequence_num&&recv_buffer_max==RECV_BUFFER_SIZE)
                {
                    printf("drop data #%d\n",package.sequence_num);
                    printf("flush\n");
                    for(i=0;i<RECV_BUFFER_SIZE;++i)
                    {
                        write(fd,recv_buffer[i].content,recv_buffer[i].length);
                        recv_buffer[i].length=0;
                    }
                    recv_buffer_max=0;
                    recv_buffer_offset+=RECV_BUFFER_SIZE;
                }
                else if(recv_buffer_offset+RECV_BUFFER_SIZE<=package.sequence_num);
                else if(recv_buffer_offset>package.sequence_num)
                {
                    printf("ignr data #%d\n",package.sequence_num);
                    package.ack=package.sequence_num;
                    sendto(sockfd,&package,sizeof(mytcp),0,(struct sockaddr *)&addr,addr_len); 
                    printf("send ack  #%d\n",package.ack);
                }
                else
                {
                    if(recv_buffer[package.sequence_num-recv_buffer_offset].length==0)
                    {
                        printf("recv data #%d\n",package.sequence_num);
                        package.ack=package.sequence_num;
                        sendto(sockfd,&package,sizeof(mytcp),0,(struct sockaddr *)&addr,addr_len); 
                        printf("send ack  #%d\n",package.ack);
                        ++recv_buffer_max;
                        recv_buffer[package.sequence_num-recv_buffer_offset].length=package.length;
                        for(i=0;i<PACKAGE_SIZE;++i)
                            recv_buffer[package.sequence_num-recv_buffer_offset].content[i]=package.content[i]; 
                    }
                    else
                    {   
                        printf("ignr data #%d\n",package.sequence_num);
                        package.ack=package.sequence_num;
                        sendto(sockfd,&package,sizeof(mytcp),0,(struct sockaddr *)&addr,addr_len); 
                        printf("send ack  #%d\n",package.ack);                       
                    }
                }
                len = recvfrom(sockfd,&package,sizeof(mytcp), 0 , (struct sockaddr *)&addr ,&addr_len); 
            }
            for(i=0;i<recv_buffer_max;++i)
                write(fd,recv_buffer[i].content,recv_buffer[i].length);
            printf("get  FIN package\n");
            printf("send FIN ack\n");
            printf("complete\n");
            close(fd);
            sendto(sockfd,&package,sizeof(mytcp),0,(struct sockaddr *)&addr,addr_len); 
        }
        return 0;
    }  
    //sender
    //sender's recv
    int sockfd;
    struct sockaddr_in addr_recv;  
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){  
        perror ("socket");  
        exit(1);  
    }  
    /*填寫sockaddr_in 結構*/  
    bzero ( &addr_recv, sizeof(addr_recv) );  
    addr_recv.sin_family=AF_INET;  
    addr_recv.sin_port=htons(PORT);  
    addr_recv.sin_addr.s_addr=htonl(INADDR_ANY) ;  
    if (bind(sockfd, (struct sockaddr *)&addr_recv, sizeof(addr_recv))<0){  
        perror("connect");  
        exit(1);  
    }  
    //sender'send    
    int PATH_SIZE=argc-1;
    destination=PATH_SIZE-1;
    int s[PATH_SIZE];   
    struct sockaddr_in addr[PATH_SIZE];  
    /* 建立socket*/  
    for(int si=0;si<PATH_SIZE;++si)
    {
        if((s[si] = socket(AF_INET,SOCK_DGRAM,0))<0)
        {  
            perror("socket");  
            exit(1);  
        }  
        /* 填寫sockaddr_in*/  
        bzero(&addr[si],sizeof(addr[si]));  
        addr[si].sin_family = AF_INET;  
        addr[si].sin_port = htons(PORT);
    	if( (hptr = gethostbyname(argv[si+1]) ) == NULL )
    	{
    		printf("gethostbyname error for host:%s/n", argv[si+1]);
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
        addr[si].sin_addr.s_addr = inet_addr(str); 
    }
    package.sequence_num=0; 
    package.ack=0; 
    while(1)
    {  
        int has_ack[100000]={0};
        memset(buffer,0,sizeof(buffer));
        memset(package.content,0,sizeof(package.content));
        package.FIN=0;
        hasFIN=0;
        package.ack=0;
        package.sequence_num=0;
        winSize=1;
        threshold=1;
        winOffset=0;
        fgets(buffer,sizeof(buffer),stdin);
        if (buffer[strlen(buffer)-1]== '\n')
            buffer[strlen(buffer)-1] = '\0';
        int fd = open(buffer, O_RDONLY);
        if(fd==-1)
        {
            printf("%s is not exist.\n",buffer);
            continue;
        }
        package.length=read(fd,package.content,PACKAGE_SIZE-1);
        while(1)
        {
            for(i=0;i<winSize&&package.length>0;++i)
            {
                ++package.sequence_num;
                sendto(s[getdest(PATH_SIZE)],&package,sizeof(mytcp),0,(struct sockaddr *)&addr[destination],addr_len); 
                if(has_ack[package.sequence_num]==0)
                    printf("send  data#%d, winSize = %d to %d\n",package.sequence_num,winSize,destination+1);
                else
                    printf("resnd data#%d, winSize = %d to %d\n",package.sequence_num,winSize,destination+1);
                memset(package.content,0,sizeof(package.content)); 
                do{
                    package.length=read(fd,package.content,PACKAGE_SIZE-1);
                }while(package.length==-1);
            }
            if(package.length==0)
                hasFIN=1;
            for(j=0;j<i;j++)
            {
                if(j==0)
                {
                    int ret=setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
                    int recvd=recvfrom(sockfd,&ackpackage,sizeof(mytcp), 0 , (struct sockaddr *)&addr_recv ,&addr_len);
                }
                else
                {
                    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
                    recvfrom(sockfd,&ackpackage,sizeof(mytcp), 0 , (struct sockaddr *)&addr_recv ,&addr_len);          
                }
                if(errno!=EAGAIN)
                {
                    printf("recv  ack #%d\n",ackpackage.ack);
                    has_ack[ackpackage.ack]=1;
                }
                if(ackpackage.ack==winOffset+1)
                    ++winOffset;
            }
            if(errno==EAGAIN||winOffset!=package.sequence_num)
            {
                errno = 0;
                printf("timeout     ,threshold = %d\n",threshold);
                threshold=winSize/2;
                winSize=1;
                package.sequence_num=winOffset;
                lseek(fd,(PACKAGE_SIZE-1)*(winOffset),SEEK_SET);
                hasFIN=0;
                package.length=read(fd,package.content,PACKAGE_SIZE-1);
                continue;
            }
            else if(winSize<threshold)
                winSize*=2;
            else
                ++winSize;
            if(hasFIN==1)
                break;
        }
        package.FIN=1;
        while(1)
        {
            sendto(s[getdest(PATH_SIZE)],&package,sizeof(mytcp),0,(struct sockaddr *)&addr[destination],addr_len); 
            printf("Send FIN package\n");
            int ret=setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
            int recvd=recvfrom(sockfd,&ackpackage,sizeof(mytcp), 0 , (struct sockaddr *)&addr_recv ,&addr_len);
            if(recvd==-1&&errno==EAGAIN)
            {
                printf("timeout   ,threshold = %d\n",threshold);
                continue;
            }
            break;   
        }
        printf("get  FIN ack\n");
        printf("complete\n");
    }  
    return 0;
} 
