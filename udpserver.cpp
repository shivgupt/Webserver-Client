#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>


int server_fd,client_fd,respLen;
char data[100]; // To read request from client
sockaddr_in saddr,caddr;
int fileLength;
bool sending=false;

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
        b[k]='\0';
}

void parseRequest(char *data)
{
    char *t;
    char *str;

    char a[]= "Host:";
    char c[] = "Accept:";
    //Method
    req.method = strtok(data," ");

    //File Name
    req.file_name = strtok(NULL," /");

    //Protocol
    req.protocol = strtok(NULL,"/");

    //Version
    req.version = strtok(NULL,"\r\n");


    t = strtok(NULL,"\n ");

        while(t != NULL)
        {
        str = a;

        if(memcmp(str,t,5)==0)
        {
            req.host = strtok(NULL," \r");
            t = strtok(NULL,"\n ");
            continue;
        }
        else
        {
            str = c;

            if(memcmp(str,t,7)==0)
            {
                req.type = strtok(NULL," \r");
                t = strtok(NULL,"\n ");
                continue;
            }
            else
                t = strtok(NULL,"\n ");

        }

        }
}

void createResponse(char buffer[])
{

    int len = 0,i=0,k=0,j=0;
    fstream ifile;
    char e[] = "HTTP";
    char f[] = "1.1";
    char c[] ="text/html";
    char d[4];
    char sen[] = "Sending File";
    char err[] = "404 NOT FOUND";
    bool flag;
    res.protocol = e;
    res.version = f;

    ifile.open(req.file_name,ios::in);

        if (!ifile.fail())
        {
            ifile.unsetf(ios_base::skipws);
            ifile.seekg(0, ios::end);
            fileLength = ifile.tellg();
            ifile.seekg(0, ios::beg);
            char status[] = "200 OK";
            res.status = status;
            ifile.close();
            flag = true;
            sending = true;
        }
        else
        {
            char status[] = "404";
            res.status = status;
            flag=false;
            fileLength = strlen(err);
            sending=false;
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
            while(i < 999 && k < strlen(sen))
            {
                cout << sen[k];
                buffer[i++] = sen[k++];

            }
        else
            while(i < 999 && k < strlen(err))
            {
                cout << err[k];
                buffer[i++] = err[k++];

            }
}

void createSocket(char a[])
{
    server_fd = socket(AF_INET,SOCK_DGRAM,0);

    if (server_fd < 0)
    {
        cout << "Invalid Socket";
    }
unsigned int port = atoi(a);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd,(sockaddr *)&saddr,sizeof(saddr))<0)
    {
        cout << "Could not bind!!";
    }

}

void read_write()
{
    char buffer[1000];
    int n,i=0;
    unsigned int fp=0;
    socklen_t len = sizeof(caddr);
    int count =0;
    fstream ifile;
    while(true)
    {
        fp=0;
        bzero(data,100);
        n = recvfrom(server_fd,data,100,0,(struct sockaddr *)&caddr,&len);
        if ( n < 0)
            cout << "\nNothing Received\n";
        else
        	puts(data);

        parseRequest(data);
        bzero(buffer,1000);
        createResponse(buffer);
        count++;

        n = sendto(server_fd,buffer,strlen(buffer),0,(struct sockaddr *)&caddr,len);
        if(sending)
        while(fp < fileLength)
        {
        	count++;
            bzero(buffer,1000);
            ifile.open(req.file_name,ios::in);
            ifile.unsetf(ios_base::skipws);
            if (ifile)
            {
                ifile.seekg(fp);
                while(ifile && i < 1000)
                {
                    ifile >> buffer[i++];
                }
                fp += i;
                ifile.close();
            }
            if(count%10 == 0)
                usleep(200000);
            n = sendto(server_fd,buffer,i,0,(struct sockaddr *)&caddr,len);
            i = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
	cout << "\nInsufficient Arguments\n";
                return 0;
    }
    createSocket(argv[1]);

    read_write();

    close(client_fd);
    close(server_fd);

    return 0;
}
