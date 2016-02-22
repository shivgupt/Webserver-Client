#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>

using namespace std;

int fd,reqLen;
int fileLength;
sockaddr_in saddr,caddr;
struct hostent *server;
char data[1000];
char request[1000];

struct HTTP
{
    char *method;
    char *file_name;
    char *protocol;
    char *version;
    char *host;
    char *port;
    char *length;
    char *status;
    char *type;
    char *connection;
}req,res;
unsigned int atoi (char a[]);
char* parseResponse(char *data)
{
    char *t;

    char *temp = (char*) malloc(sizeof(char)*strlen(data));

    for(int i = 0; i < strlen(data); i++)
    {
	temp[i] = data[i];
    }
    char a[] = "Content-Length:";
    t = strtok(temp,"\n ");
    while(t != NULL)
    {
	if(memcmp(a,t,14)==0)
	{
		t = strtok(NULL,"\r\n");
		break;
	}
	t = strtok(NULL,"\n ");
    }

    fileLength = atoi(t);

    char b[]= "\r\n\r\n";
    t = strstr(data,b);

    return &t[3];

}

void createRequest(char file_name[],char host[], char port[],char request[])
{

    int len = 0,i=0,k=0,j=0;

    char *resource;
    char protocol[] = "HTTP";
    char version[] = "1.1";
    char method[] = "GET";
    char a[] = "Host: ";
    char b[] = "Accept: text/html\r\n";
    char d[4];

    while(k < strlen(method))
    {
        request[i++] = method[k++];
    }
    k=0;
    request[i++] = ' ' ;
    request[i++] = '/';
    while(k < strlen(file_name))
    {
        request[i++] = file_name[k++];
    }
    k=0;
    request[i++] = ' ';
    while(k < strlen(protocol))
    {
        request[i++] = protocol[k++];
    }
    k=0;
    request[i++] = '/';
    while(k < strlen(version))
    {
        request[i++] = version[k++];
    }
    k=0;
    request[i++] = '\r';
    request[i++] = '\n';
    while(k < strlen(a))
    {
        request[i++] = a[k++];
    }
    k=0;
    while(k < strlen(host))
    {
        request[i++] = host[k++];
    }
    k=0;
    request[i++] = ':';
    while(k < strlen(port))
    {
        request[i++] = port[k++];
    }
    k=0;
    request[i++] = '\r';
    request[i++] = '\n';
    while(k < strlen(b))
    {
        request[i++] = b[k++];
    }
    k=0;
    request[i++] = '\r';
    request[i++] = '\n';
    request[i++] = '\r';
    request[i++] = '\n';

    reqLen = i;

        while(k < reqLen )
        {
           cout << request[k++];
        }

}

void read_write()
{
struct timeval s,e;
    char ch;
    int n;
    bzero(data,1000);
    socklen_t len = sizeof(struct sockaddr_in);
    gettimeofday(&s,NULL);
    n = sendto(fd,request,reqLen,0,(const struct sockaddr *)&saddr, len);
    recvfrom(fd,data,1000,0,(struct sockaddr *)&caddr,&len);

    int l = strlen(data);
    //puts(data);
    char*  d = parseResponse(data);

    cout << "\nFile Received : \n";
    puts(d);

    if(1000-fileLength < 0)
    {

	int dataLeft = fileLength - strlen(data);
	while(dataLeft > 0)
	{
        	bzero(data,1000);
		n=recvfrom(fd,data,1000,0,(struct sockaddr *)&saddr,&len);
		dataLeft -= strlen(data);
		puts(data);
	}
    }
    else
	{
		n=recvfrom(fd,data,1000,0,(struct sockaddr *)&saddr,&len);
		puts(data);
	}
    gettimeofday(&e,NULL);

cout <<"\nRetrieval Time : " << (e.tv_sec*1000000 + e.tv_usec)-(s.tv_sec*1000000 + s.tv_usec);
}

unsigned int atoi(char a[])
{
    int len = strlen(a);
    int i=0,res=0;
    while(i < len )
    {
	res = res + ( pow(10,len-i-1)*((int)(a[i]-48)));
	i++;
    }
    return res;
}

void startClient(char a[],char b[])
{
    fd = socket(AF_INET,SOCK_DGRAM,0);

    if(fd < 0)
    {
        cout << "Invalid Socket";
    }

    server = gethostbyname(a);

    saddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&saddr.sin_addr.s_addr,server->h_length);



    unsigned int k = atoi(b);
    saddr.sin_port = htons(k);

}
int main(int argc, char *argv[])
{
 if (argc < 2)
    {
        cout << "\nInsufficient Arguments\n";
                return 0;
    }
    for (int i =0; i < argc; i++)
    {
        if(argv[i] == NULL)
        {
                cout << "\nInsufficient Arguments\n";
                return 0;
        }
    }
    req.host = argv[1];
    req.port = argv[2];
    startClient(req.host,req.port);
    createRequest(argv[3],req.host,req.port,request);
    read_write();

    return 0;
}
