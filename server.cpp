//
//  server.cpp
//  Network
//
//  Created by Tuan Nguyen on 15/05/2015.
//  Copyright (c) 2015 Tuan. All rights reserved.
//

#include "auxfns.h"
#include <sstream>
#include <pthread.h>
using namespace std;


void* handleClient(void *);
void sendfile(char *);
void handleCommand(int, char *, int);
int verifyUser(int) ;
int lookUpPassword(const char *, const char *);
bool updateUser(const char [], const char[], const char[]);
bool removeUser(const char []);

pthread_mutex_t sharedlock;

int main(int argc, char *argv[])
{
    int ServerPort;
    
    int Sockfd;
    sockaddr_in ServAddr;
    sockaddr_in ClientAddr;
    char *opt;
    if (argc != 2)
    {
        cerr << "Usage: server port";
    }
    ServerPort = atoi(argv[1]);
    memset(&ServAddr, 0, sizeof(ServAddr) );
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServAddr.sin_port = htons(ServerPort);
    Sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(Sockfd,SOL_SOCKET,SO_REUSEADDR,(void *)&opt, sizeof(opt));
    Bind(Sockfd, (sockaddr*) &ServAddr, sizeof(ServAddr));
    Listen(Sockfd, 5);
    
    
    while (1)
    {
        unsigned int len = sizeof(ClientAddr);
        int Newfd = Accept(Sockfd, (sockaddr *)&ClientAddr, &len);
        pthread_t thread;
        pthread_create(&thread, NULL, handleClient, &Newfd);
        pthread_detach(thread);
    }
    return 0;
}


void* handleClient(void * arg)
{
    int sockfd = *((int *)arg);
    char command[BuffSize];
    int status = verifyUser(sockfd);
    if (!status)
    {
        close(sockfd);
        pthread_exit(NULL);
    }
    while (1)
    {
        recv(sockfd, command, BuffSize, 0);
        
        if (strcmp(command, "exit") == 0)
        {
            close(sockfd);
            pthread_exit(NULL);
        }
        handleCommand(sockfd, command, status);
    }
    return NULL;
    
}

void handleCommand(int sockfd, char * command, int access_rights)
{
    istringstream is(command);
    char cmd[5];
    is >> cmd;
    long status;
    long size;
    char filename[50];
    if (strcmp(cmd, "get") == 0)
    {
        while (is >> filename)
        {
            long res = sendFile(sockfd, filename);
            if (res == NON_EXIST)
            {
                status = NON_EXIST;
                write(sockfd, &status, sizeof(long));
            }
            else if (res == DIRECTORY)
            {
                status = DIRECTORY;
                write(sockfd, &status, sizeof(long));
            }
        }
    }
    else if (strcmp(cmd, "put") == 0)
    {
        while (is >> filename)
        {
            bool success = 1;
            recv(sockfd, &size, sizeof(long), 0);
            if (size == NON_EXIST)
                continue;
            getFile(sockfd, filename, size);
            write(sockfd, &success, 1);
        }
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        system("ls >temps.txt");
        sendFile(sockfd, "temps.txt");
    }
    else if (strcmp(cmd, "add") == 0)
    {
        bool right = 1;
        if (access_rights == USER)
        {
            right = 0;
            write(sockfd, &right, sizeof(bool));
            return;
        }
        write(sockfd, &right, sizeof(bool));
        char login[10], password[10], access[15];
        bool status;
        is >> login >> password >> access;
        status = updateUser(login, password, access);
        write(sockfd, &status, sizeof(bool));
    }
    else if (strcmp(cmd, "list") == 0)
    {
        bool right = 1;
        if (access_rights == USER)
        {
            right = 0;
            write(sockfd, &right, sizeof(bool));
            return;
        }
        write(sockfd, &right, sizeof(bool));
        sendFile(sockfd, "password.txt");
    }
    else if (strcmp(cmd, "remove") == 0)
    {
        bool right = 1;
        if (access_rights == USER)
        {
            bool right = 0;
            write(sockfd, &right, sizeof(bool));
            return;
        }
        write(sockfd, &right, sizeof(bool));
        char login[10];
        is >> login;
        bool status = removeUser(login);
        write(sockfd, &status, sizeof(bool));
    }
}


int verifyUser(int sockfd)
{
    int status = 0;
    int num_trials = 0;
    char login[10];
    char password[10];
    do
    {
        Receive(sockfd, login, 10);
        Receive(sockfd, password, 10);
        status = lookUpPassword(login, password);
        if (status == ERROR)
        {
            return status;
        }
        write(sockfd, &status, sizeof(int));
        num_trials ++;
    }
    while (num_trials < 3 && !status);
    return status;

}

bool updateUser(const char login[], const char password[], const char access[])
{
    pthread_mutex_lock(&sharedlock);
    ifstream is("password.txt");
    ofstream os("temps.txt");
    char lname[10], passwd[10];
    char access_level[15];
    bool found = false;
    bool exists = false;
    while (is >> lname >> passwd >> access_level)
    {
        if (strcmp(login, lname))
        {
            os << lname << " " <<  passwd << " " <<  access_level << endl;
        }
        else if (strcmp(login, lname) == 0 && strcmp(password, passwd) == 0 && strcmp(access_level, access) == 0)
        {
            found = true;
            exists = true;
        }
        else
        {
            os << login << " " <<  password << " " << access << endl;
            found = true;
        }
    }
    if (!found)
    {
        os << login << " " <<  password << " " <<  access << endl;
    }
    os.close();
    is.close();
    remove("password.txt");
    rename("temps.txt", "password.txt");
    pthread_mutex_unlock(&sharedlock);
    return !exists;
}

bool removeUser(const char login[])
{
    pthread_mutex_lock(&sharedlock);
    ifstream is("password.txt");
    ofstream os("temps.txt");
    char lname[10], passwd[10];
    char access_level[15];
    bool found = false;
    while (is >> lname >> passwd >> access_level)
    {
        if (strcmp(login, lname))
        {
            os << lname << " " <<  passwd << " " <<  access_level << endl;
        }
        else
        {
            found = true;
        }
    }
    remove("password.txt");
    rename("temps.txt", "password.txt");
    os.close();
    is.close();
    pthread_mutex_unlock(&sharedlock);
    return found;
}

int lookUpPassword(const char login[], const char password[])
{
    ifstream is("password.txt");
    char lname[10], passwd[10];
    char access[15];
    if (!is)
    {
        return ERROR;
    }
    while (is >> lname >> passwd >> access)
    {
        if (strcmp(login, lname) == 0 && strcmp(password, passwd) == 0)
        {
            if (strcmp(access, "administrator") == 0)
            {
                return ADMIN;
            }
            else if (strcmp(access,"user") == 0)
            {
                return USER;
            }
        }
    }
    is.close();
    return UNAUTHORISED;
}