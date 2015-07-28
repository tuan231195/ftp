/*
 Student's name: Van Do Tuan Nguyen
 Student's number: 4752764
 Lab: Thursday: 8h30
 Assignment: 4
 File name: auxfns.cpp
 Purpose of this assigment: implementation of wrapper functions
 Last modified: 3/6/2015
 */

#include "auxfns.h"

//create a socket

int Socket(int domain, int type, int protocol ) {
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        perror( "creation" );
        exit(ERROR);
    }
    return sockfd;
}


//bind to a port
void Bind( int sockfd, const struct sockaddr *servAddr, int addrLen )
{
    if (bind(sockfd, servAddr, addrLen) < 0)
    {
        perror("bind error");
        exit(ERROR);
    }
}


//listen for connections
void Listen( int sockfd, int backlog )
{
    if (listen(sockfd, backlog ) < 0)
    {
        perror( "listen" );
        exit(ERROR);
    }
}

//connect to a server
void Connect(int sockfd, sockaddr *servAddr, int size)
{
    if(connect(sockfd, servAddr, size) < 0)
    {
        perror( "Can't connect to server!");
        exit(ERROR);
    }
}

//accept a connection
int Accept( int sockfd, sockaddr *clientAddr, unsigned int *addrLen )
{
    int Connfd = accept(sockfd, clientAddr, addrLen);
    if (Connfd < 0)
    {
        perror ("Accept");
        exit(ERROR);
    }
    return Connfd;
}


//resolve a host name
hostent * Gethostbyname(char *servHost)
{
    // get the address of the host
    hostent *host = gethostbyname(servHost);
    if(host == NULL)
    {
        perror( "gethostbyname error");
        exit(ERROR);
    }
    return host;
}


//send a file over a network, return the number of bytes in the file
long sendFile(int sockfd, const char *filename)
{
    using namespace std;
    struct stat info;
    ifstream is(filename);
    char buff[BuffSize];
    long size;
    //if the file does not exists
    if (!is)
    {
        return NON_EXIST;
    }
    stat(filename, &info);
    //if it is a directoty
    if ((info.st_mode & S_IFMT) == S_IFDIR)
    {
        return DIRECTORY;
    }
    //get file's size
    is.seekg(0, ios::end);
    size = is.tellg();
    is.seekg(0, ios::beg);
    write(sockfd, &size, sizeof(long));
    long bytesRemaining = size;
    
    //read content of the file
    while (bytesRemaining > 0)
    {
        int bytes;
        if (bytesRemaining < BuffSize)
        {
            bytes = (int)bytesRemaining;
        }
        else
        {
            bytes = BuffSize;
        }
        is.read(buff, bytes);
        Send(sockfd, buff, bytes);
        bytesRemaining -= bytes;
    }
    is.close();
    return size;
}


//Send a char array over the network
void Send(int sockfd, char *buffer, long len)
{
    do
    {
        long sentBytes = write(sockfd, buffer, len);
        if (sentBytes < 0)
        {
            perror("Send :");
            exit(1);
        }
        len -= sentBytes;
        buffer += sentBytes;
    }
    while (len > 0);
}


//get a file
void getFile(int sockfd, char *filename, long filesize)
{
    using namespace std;
    
    char buff[BuffSize];
    long bytesRemaining = filesize;
    ofstream os(filename);
    do
    {
        int bytes;
        if (bytesRemaining < BuffSize )
        {
            bytes = (int)bytesRemaining;
        }
        else
        {
            bytes = BuffSize;
        }
        
        //read into a buffer
        long receiveBytes = read(sockfd, buff, bytes);
        if (receiveBytes < 0)
        {
            perror("Receive : ");
            exit(1);
        }
        
        
        //write the buffer into the file.
        os.write(buff, receiveBytes);
        bytesRemaining -= receiveBytes;
    }
    while (bytesRemaining > 0);
    os.close();
}


//read exactly len bytes and save in buffer
void Receive(int sockfd, char *buffer, long len)
{
    do
    {
        long receiveBytes = read(sockfd, buffer, len);
        if (receiveBytes < 0)
        {
            perror("Receive : ");
            exit(1);
        }
        len -= receiveBytes;
        buffer += receiveBytes;
    }
    while (len > 0);
}