#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <pthread.h>

int server_fd,client_fd,respLen;
char datax[1000]; // To read request from client
sockaddr_in saddr,caddr;

using namespace std;

struct HTTP
{
    char *method;
    char *file_name;
    char *protocol;
    char *version;
    char *host;
    char *length;
    char *status;
    char *type;
    char *connection;
}req,res;

void* read_write(void*);
void parseRequest(char *data,char *file_name)
{
    char *t;
    char *str;

    char a[]= "Host:";
    char b[] = "Connection:";
    char c[] = "Accept:";
    //Method
    req.method = strtok(data," ");

    //File Name
    strcpy(file_name,strtok(NULL," /"));

    //Protocol
    req.protocol = strtok(NULL,"/");

    //Version
    req.version = strtok(NULL,"\r\n");


    t = strtok(NULL,"\n ");

        while(t != NULL)
        {
        str = a;
       // str = "Host";
        if(memcmp(str,t,5)==0)
        {
            req.host = strtok(NULL," \r");
            t = strtok(NULL,"\n ");
            continue;
        }
        else
        {
            str = c;
            //str = "Accept";
            if(memcmp(str,t,7)==0)
            {
                req.type = strtok(NULL," \r");
                t = strtok(NULL,"\n ");
                continue;
            }
            else
            {
                str = b;
                //str = "Connection";
                if(memcmp(str,t,11)==0)
                {
                    req.connection = strtok(NULL," \r");
                    t = strtok(NULL,"\n ");
                    continue;
                }

                else
                    t = strtok(NULL,"\n ");
            }
        }



        }
}
unsigned int atoi(char a[])
{
    int len = strlen(a);
    int i=0,result=0;
    while(i < len )
    {
	result = result + ( pow(10,len-i-1)*((int)(a[i]-48)));
	i++;
    }
    return result;
}

void itoa(int a,char b[])
{
	char t[6];
	int i=0,k=0;
	while(a != 0)
	{
		t[i++] = a%10 + 48;
		a /= 10;
	}

	while(i>0)
	{
		b[k++] = t[i-1];
		i--;
	}

	b[k] = '\0';
}
void createResponse(char buffer[],char *file_name)
{
    int len = 0,i=0,k=0,j=0;
    fstream ifile;
    char e[] = "HTTP";
    char f[] = "1.1";
    char c[] ="text/html";
    char d[4];
    char err[] = "404 NOT FOUND";
    char err2[] = "400 BAD REQUEST";
    bool flag;
    bool flag2=false;
    res.protocol = e;
    res.version = f;
    int fileLength;
    char *resource;
    ifile.open(file_name,ios::in);

        if(strlen(file_name) < 2)
        {
            char status[] = "400";
            res.status = status;
            flag=false;
            flag2=true;
            fileLength = strlen(err2);
        }
        else if (!ifile.fail())
        {
            ifile.unsetf(ios_base::skipws);
            ifile.seekg(0, ios::end);
            fileLength = ifile.tellg();
            ifile.seekg(0, ios::beg);
            resource= (char *) malloc(sizeof(char)*fileLength);
            while(i<fileLength)
            {
                ifile >> resource[i++];
            }
            char status[] = "200 OK";
            res.status = status;
            ifile.close();
            flag = true;
        }
        else
        {
            char status[] = "404";
            res.status = status;
            flag=false;
            fileLength = strlen(err);
           //ending=false;
        }
        itoa(fileLength,d);
            res.length = d;
            i=0;
        while(i < strlen(res.protocol))
        {
            buffer[i++] = res.protocol[k++];
        }
        buffer[i++] = '/';
        k=0;len = i;

        while(k < strlen(res.version))
        {
            buffer[i++] = res.version[k++];
        }
        buffer[i++] = ' ';
        k=0;len = i;

        while(k < strlen(res.status) )
        {
            buffer[i++] = res.status[k++];
        }
        buffer[i++] = '\r';
        buffer[i++] = '\n';

        char a[] = "Content-Length: ";
        k = 0; len = i;
        while(i < strlen(a)+len)
        {
            buffer[i++] = a[k++];
        }

        k=0; len = i;
        while(k < strlen(res.length))
        {
            buffer[i++] = res.length[k++];
        }
        buffer[i++] = '\r';
        buffer[i++] = '\n';

        char b[] = "Content-Type: ";
        k = 0; len = i;

        while(k < strlen(b))
        {
            buffer[i++] = b[k++];
        }
        k = 0; len = i;
        while(k < strlen(c))
        {
            buffer[i++] = c[k++];
        }
        buffer[i++] = '\r';
        buffer[i++] = '\n';
        buffer[i++] = '\r';
        buffer[i++] = '\n';

        k = 0; len = i;

        if(flag==true)
            while(k < fileLength)
            {
                buffer[i++] = resource[k++];

            }
        else if (flag2 == true)
            while(i < 999 && k < strlen(err2))
            {
                buffer[i++] = err2[k++];

            }
           else
            while(i < 999 && k < strlen(err))
            {
                buffer[i++] = err[k++];

            }
            buffer[i]='\0';
}

void startServer(char a[])
{
    pthread_t rw;
    server_fd = socket(AF_INET,SOCK_STREAM,0);

    if (server_fd < 0)
    {
        cout << "Invalid Socket";
        exit(0);
    }

    saddr.sin_family = AF_INET;
    unsigned int port = atoi(a);
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd,(sockaddr *)&saddr,sizeof(saddr))<0)
    {
        cout << "Could not bind!!";
        exit(0);
    }

    listen(server_fd,6);
    socklen_t len = sizeof(caddr);

    while (true)
    {
    	client_fd = ::accept(server_fd,(sockaddr *)&caddr,&len);
    	if (client_fd < 0)
    	{
        	cout << "Accept failed!!";
        	exit(0);
    	}
    	else
    	{
		pthread_create(&rw,0,&read_write,(void*) &client_fd);
    	}
    }

}

void* read_write(void* a)
{
    int *fd = (int *) a;
    int cfd = *fd;
    char *file_name= (char *) malloc(sizeof(char)*30);
    int fileLength=0;
    fstream ifile;
    int i=0,n;
    char data[1000];
    while(true)
    {
    bzero(file_name,30);
        bzero(data,1000);
        if(cfd > 0)
        {
            n = recv(cfd,data,1000,0);
            if(n<0)
            {
                cout << "error";
                break;
            }
        }
        else
        {
            cout << "exiting";
            break;
        }

        puts(data);
        int d = strlen(data);
        if(d <=0 )
            break;
        parseRequest(data,file_name);
        ifile.open(file_name,ios::in);
        if(ifile)
        {
            ifile.unsetf(ios_base::skipws);
            ifile.seekg(0, ios::end);
            fileLength = ifile.tellg();
            ifile.seekg(0, ios::beg);
            ifile.close();
        }
        char *buffer= (char *) malloc(sizeof(char)*(fileLength+100));
        bzero(buffer,fileLength);
        createResponse(buffer,file_name);
        send(cfd,buffer,fileLength+100,0);

    }
}
int main(int argc, char *argv[])
{
    if (argc == 1)
    {
	cout << "\nInsufficient Arguments\n";
                return 0;
    }
    startServer(argv[1]);
    return 0;
}
