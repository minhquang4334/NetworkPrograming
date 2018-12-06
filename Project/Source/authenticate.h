/*
* api for authenticate
* feature: login, logout
*/

#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
#define BLOCK 0
#define ACTIVE 1
#define MAX_LOGIN_FAILS 3


typedef struct user {
	int id;
	char username[32];
	char password[32];
	int status;
	int isLogin;
	int countFails;
	struct user *next;
} User;

User *head = NULL;
User *current = NULL;

//Create new User from username, password, status
User *createNewUser(char* username, char* password, int status);

// print List users
void printList();

// add user to end of list
void push(User* newUser);

// ad user to beginning of list
void pushToBeginning(User** head, User * newUser);

// // check list empty
// int isEmpty();

// // check user login
// int isLogin(User *user);

// // check user is active
// int isActive(User *user);

// get List User
// readfile and create list users
void getListUser(char* fileName); 

// updateListUser
void updateListUser(char* fileName);

// fucntional
User* search(char *username);

// compare password 
int identifyPassWord(User* user, char* password);

// login
int login(char* username, char* password);

// register
int registerUser(char* username, char* password);

// logout
int logout(User* user);





