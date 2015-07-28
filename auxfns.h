/*
 Student's name: Van Do Tuan Nguyen
 Student's number: 4752764
 Lab: Thursday: 8h30
 Assignment: 4
 File name: auxfns.h
 Purpose of this assigment: declaration wrapper functions
 Last modified: 3/6/2015
 */

#ifndef __Network__auxfns__
#define __Network__auxfns__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <netdb.h>
#include <sys/stat.h>


const int BuffSize = 100;

//Server state
const int ERROR = -1;


//File status
const int NON_EXIST = -1;
const int DIRECTORY = -2;


//User status
const int UNAUTHORISED = 0;
const int USER = 1;
const int ADMIN = 2;


//Commands
const int INVALID = 0;
const int GET = 1;
const int PUT = 2;
const int EXIT = 3;
const int LS = 4;
const int ADD = 5;
const int LIST = 6;
const int REMOVE = 7;
const int MISUSE = 8;


int Socket( int, int, int);
void Bind(int, const struct sockaddr *, int  );
void Listen( int , int );
void Connect(int , sockaddr *, int );
int Accept( int , sockaddr *, unsigned int *);
hostent *Gethostbyname(char *);
long sendFile(int, const char *);
void Send(int fd, char *, long);
void Receive(int fd,  char *, long);
void getFile(int fd, char *, long);


#endif /* defined(__Network__auxfns__) */
