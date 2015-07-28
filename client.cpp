/*
 Student's name: Van Do Tuan Nguyen
 Student's number: 4752764
 Lab: Thursday: 8h30
 Assignment: 4
 File name: client.cpp
 Purpose of this assigment: Client program
 Last modified: 3/6/2015
 */

#include <iostream>
#include <sstream>
#include "auxfns.h"
using namespace std;

int setUpConnection(char *, int);//set up connection
int parseCommand(char *);// interpret command
void GetFile(int, char *);//get a file
void PutFile(int, char *);//put a file
void Exit(int);//close a connection
void GetDir(int);//get
bool empty(char *); // check if a string is empty
//authorise a user
bool getUserInfo(int);

//list, remove and add users
void AddUser(int, char *);
void RemoveUser(int, char *);
void ListUser(int);



int main(int argc, char * argv[])
{
    char *servHost;
    int sockfd;
    char fullCommand[100];
    
    int servPort;
    if(argc != 3)
    {      // Test for correct number of arguments
        cerr << "Usage client hostname port" << endl;
        exit(1);
    }
    servHost = argv[1];
    servPort = atoi(argv[2]);
    
    //set up the connection
    sockfd = setUpConnection(servHost, servPort);
    //authorise the user
    bool status = getUserInfo(sockfd);
    if (!status)
    {
        cerr << "Fail to verify" << endl;
        exit(0);
    }
    cin.ignore();//ifnore newline bytes
    
    while (1)
    {
        cout << "ftp> ";
        cin.getline(fullCommand, 100);
        //if command is empty
        if (empty(fullCommand))
        {
            continue;
        }
        int opt = parseCommand(fullCommand);
        switch(opt)
        {
            //invalid command
            case INVALID:
                cerr << "Invalid Command" << endl;
                break;
            //get a file
            case GET:
                GetFile(sockfd, fullCommand);
                break;
            //put a file
            case PUT:
                PutFile(sockfd, fullCommand);
                break;
            //close connection
            case EXIT:
                Exit(sockfd);
                break;
            //ls command
            case LS:
                GetDir(sockfd);
                break;
            //add a user
            case ADD:
                AddUser(sockfd, fullCommand);
                break;
            //list a user
            case LIST:
                ListUser(sockfd);
                break;
            //remove a user
            case REMOVE:
                RemoveUser(sockfd, fullCommand);
                break;
            
        }
    }
    return 0;
}


//add a user
void AddUser(int sockfd, char * command)
{
    istringstream is (command);
    char cmd[5];
    char login[10];
    char passwd[10];
    bool status, right;
    char access[15];
    is >> cmd;
    is >> login >> passwd >> access;
    if (strcmp(access, "administrator") && strcmp(access, "user"))
    {
        cerr << "Unknown access rights" << endl;
        return;
    }
    //send the command
    Send(sockfd, command, BuffSize);
    //read the permission
    read(sockfd, &right, sizeof(bool));
    //don't have permission
    if (!right)
    {
        cerr << "You don't have permission to add a user" << endl;
        return;
    }
    else
    {
        //read result of the command
        read(sockfd, &status, sizeof(bool));
        if (status)
        {
            cout << "New " << access << " added" << endl;
        }
        else
        {
            cout << "Cannot add the user" << endl;
        }
    }
}


//list a user
void ListUser(int sockfd)
{
    long size;
    char *list_user;
    bool right;
    char buff[100] = "list";
    //send the command
    Send(sockfd, buff, BuffSize);
    //read permission
    read(sockfd, &right, sizeof(bool));
    //allowed
    if (right)
    {
        //display the list of users
        read(sockfd, &size, sizeof(long));
        list_user = new char[size + 1];
        Receive(sockfd, list_user, size);
        list_user[size] = 0;
        cout << list_user << endl;
        delete [] list_user;
    }
    //not allowed
    else
    {
        cerr << "You don't have permission to list users" << endl;
        return;
    }
}


//remove user
void RemoveUser(int sockfd, char *command)
{
    istringstream is (command);
    char cmd[5];
    bool right;
    char login[10];
    is >> cmd >> login;
    bool status;
    //send the command
    Send(sockfd, command, BuffSize);
    //read permission
    read(sockfd, &right, sizeof(bool));
    //allowed
    if (right)
    {
        read(sockfd, &status, sizeof(bool));
        if(status)
        {
            cout << "User " << login << " has been removed" << endl;
        }
        else
        {
            cout << "Cannot delete the user" << endl;
        }
    }
    else
    {
        cerr << "You don't have permission to remove a user" << endl;
        return;

    }
}


//set up connection
int setUpConnection(char * servHost, int servPort)
{
    int sockfd;// Socket descriptor
    sockaddr_in servAddr;
    hostent *host;
    
    host = Gethostbyname(servHost);
    // Create a reliable, stream socket
    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // set up the server address structure
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = ((in_addr*)host->h_addr_list[0])->s_addr;
    servAddr.sin_port  = htons(servPort);   // Server port
    
    // Establish the connection
    Connect(sockfd, (sockaddr*)&servAddr, sizeof(servAddr));
    return sockfd;
}

bool getUserInfo(int sockfd)
{
    int status = 0;
    int num_trials = 0;
    char login[10];
    char password[10];
    do
    {
        cout << "Enter login: ";
        cin >> login;
        cout << "Enter password: ";
        cin >> password;
        Send(sockfd, login, 10);
        Send(sockfd, password, 10);
        read(sockfd, &status, sizeof(int));
        if (status == ERROR)
        {
            cerr << "Server problem. Try again later" << endl;
        }
        if (status == 0)
        {
            cerr << endl << "ERROR: Incorrect login name or password name. Try again" << endl << endl;
        }
        num_trials ++;
    }
    while (num_trials < 3 && !status);
    return (status != 0);
}

void GetFile(int sockfd, char * command)
{
   
    char filename[50];
    long size;
    char cmd[5];
    istringstream is (command);
    is >> cmd;
    Send(sockfd, command, BuffSize);
    while (is >> filename)
    {
        read(sockfd, &size, sizeof(long));
        if (size == NON_EXIST)
        {
            cerr << "File " << filename << " not found" << endl;
        }
        else if (size == DIRECTORY)
        {
            cerr << "File " << filename << " is a directory" << endl;
        }
        else
        {
            cout << "Retrieve " << filename << " - " << size << " bytes" << endl;
            getFile(sockfd, filename, size);
        }
    }
}

void PutFile(int sockfd, char *command)
{
    char filename[50];
    char cmd[5];
    bool stt;
    long err = NON_EXIST;
    istringstream is (command);
    is >> cmd;
    Send(sockfd, command, BuffSize);
    while (is >> filename)
    {
        long res = sendFile(sockfd, filename);
        if (res == -1)
        {
            cerr << "File " << filename << " not found" << endl;
            send(sockfd, &err, sizeof(long), 0);
        }
        else if (res == -2)
        {
            cerr << "File " << filename << " is a directory" << endl;
            send(sockfd, &err, sizeof(long), 0);
        }
        else
        {
            cout << "Send " << filename << " - " << res << " bytes ";
            read(sockfd, &stt, 1);
            if (stt)
            {
                cout << " : Success " << endl;
            }
        }
    }
}

void Exit(int sockfd)
{
    char buff[BuffSize] = "exit";
    send(sockfd, buff, BuffSize, 0);
    exit(0);
}

void GetDir(int sockfd)
{
    long size;
    char *list;
    char buff[BuffSize] = "ls";
    send(sockfd, buff, BuffSize, 0);
    read(sockfd, &size, sizeof(long));
    list = new char[size + 1];
    Receive(sockfd, list, size);
    list[size] = 0;
    cout << list << endl;
    delete [] list;
}

int parseCommand (char * command)
{
    istringstream is(command);
    char cmd[5];
    char arg[50];
    is >> cmd;
    if (strcmp(cmd, "get") == 0)
    {
        if (!(is >> arg))
        {
            cerr << "Usage: get filename1 filename2 ..." << endl;
            return MISUSE;
        }
        return GET;
    }
    if (strcmp(cmd, "put") == 0)
    {
        if (!(is >> arg))
        {
            cerr << "Usage: put filename1 filename2 ..." << endl;
            return MISUSE;
        }
        return PUT;
    }
    if (strcmp(cmd, "exit") == 0)
    {
        if (is >> arg)
        {
            cerr << "Usage: exit" << endl;
            return MISUSE;
        }
        return EXIT;
    }
    if (strcmp(cmd, "ls") == 0)
    {
        if (is >> arg)
        {
            cerr << "Usage: ls" << endl;
            return MISUSE;
        }
        return LS;
    }
    if (strcmp(cmd, "add") == 0)
    {
        char login[10], passwd[10], access[15];
        if (!(is >> login >> passwd >> access))
        {
            cerr << "Usage: add login passwd access-right" << endl;
            return MISUSE;
        }
        if (is >> arg)
        {
            cerr << "Usage: add login passwd access-right" << endl;
            return MISUSE;
        }
        return ADD;
    }
    if (strcmp(cmd, "list") == 0)
    {
        if (is >> arg)
        {
            cerr << "Usage: list" << endl;
            return MISUSE;
        }
        return LIST;
    }
    if (strcmp(cmd, "remove") == 0)
    {
        char login[10];
        if (!(is >> login))
        {
            cerr << "Usage: remove user" << endl;
            return MISUSE;
        }
        if (is >> arg)
        {
            cerr << "Usage: remove user" << endl;
            return MISUSE;
        }
        return REMOVE;
    }
    return INVALID;
}
// check if a string is empty
bool empty(char * str)
{
    while (*str != '\0' && *str == ' ')
    {
        str ++;
    }
    return (*str == '\0');
}
