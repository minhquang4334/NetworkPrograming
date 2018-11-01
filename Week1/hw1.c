/*
@name HoangMinhQuang
@id   20152945
@class ASK60

BÀI TẬP TUẦN 1 – IT4062
Chương trình quản lý tài khoản người dùng

Yêu cầu nộp bài:
- Tạo Makefile để biên dịch
- Đóng gói tất cả file mã nguồn và Makefile vào một file nén có tên theo định dạnh
HotenSV_MSSV_HW1.zip. Ví dụ: BuiTrongTung_20161234_HW1.zip
- Nộp bài theo quy định
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// define const 
#define LOGIN 1
#define OFFLINE 0
#define BLOCK 0
#define ACTIVE 1

// struct User
typedef struct user{
	char username[254];
	char password[32];
	int isLogin;
	int status;
	int countLoginFails;
	struct user *next;
} User;

User *head = NULL;
User *current = NULL;

//Create new User from username, password, status
User *createNewUser(char* username, char* password, int status) {
	User *newUser = (User*)malloc(sizeof(User));
	strcpy(newUser->username, username);
	strcpy(newUser->password, password);
	newUser->status = status;
	newUser->isLogin = OFFLINE;
	newUser->countLoginFails = 0;
	newUser->next = NULL;
	return newUser;
}
// print List users
void printList() {
    User *ptr = head;

    while (ptr != NULL) {
        printf("Username: %s\n", ptr->username);
        printf("Status: %s\n", ((ptr->status == ACTIVE) ? "Active" : "Blocked"));
        printf("----------------------------------\n");

        ptr = ptr->next;
    }
    return;
}
// add user to end of list
void push(User* newUser) {
	if(head == NULL) {
		head = newUser;
		current = newUser;
	}
 	else {
 		current->next = (User*)malloc(sizeof(User));
		current->next = newUser;
		current = newUser;
 	}
	return;
}
// ad user to beginning of list
void pushToBeginning(User** head, User * newUser) {
	User * new_node;
    new_node = (User*)malloc(sizeof(User));
    new_node = newUser;
    new_node->next = *head;
    *head = new_node;
    return;
}

// check list empty
int isEmpty() {
	return head == NULL ? 1 : 0;
}
// check user login
int isLogin(User *user) {
	return !!user->isLogin;
}
// check user is active
int isActive(User *user) {
	return !!user->status;
}
// validation user name
int checkUsername(char *username) {
	return (strlen(username) <= 254);
}
// validation password
int checkPassword(char *password) {
	return (strlen(password) <= 32);
}
// fucntional
User* search(char *username) {
	User *ptr = head;

    while (ptr != NULL) {
    	if(!strcmp(ptr->username, username)) break;
        ptr = ptr->next;
    }
    
	return ptr;
}
// exit program
void exitProgram() {
	printf("End Program :))\n");
	exit(0);
}
// update file after data users change
void updateFile() {
	FILE* fOut;
	fOut = fopen("account.txt", "w");
	if(!fOut) {
		printf("File not exist??\n");
		exitProgram();
	}

	User *ptr = head;

    while (ptr != NULL) {
        fprintf(fOut, "%s %s %d\n", ptr->username, ptr->password, ptr->status);    	
    	ptr = ptr->next;
    }

    fclose(fOut);
    return;
}
// sign in 
int signIn() {
	char username[254];
	char password[32];
	printf("\n ----------------- Sign In ------------------\n");
	do {
		printf("Username: ");
		scanf("%s", username);
		User* user = search(username);
		if(user == NULL) {
			printf("Username %s not exist!! \n", username);
			continue;
		}
		if(!isActive(user)) {
			printf("User %s was blocked!! You can't sign in with this account!! \n", username);
			continue;
		}
		printf("Password: ");
		scanf("%s", password);
		if(strcmp(user->password, password)) {
			user->countLoginFails++;
			printf("password in correct \n");
			if(user->countLoginFails >= 3) {
				user->status = BLOCK;
				printf("3 times incorrect !! \n");
				printf("User '%s' is blocked !! \n", user->username);
			}
			continue;
		} else {
			user->countLoginFails = 0;
			user->isLogin = LOGIN;
			printf("Sign in with user %s succes!! \n", user->username);
			break;
		}
	} while(1);
	return 1;
}
// readfile and create list users
void readFile() {
	char username[254];
	char password[32];
	int status;
	char c;
	FILE* fIn;
	fIn = fopen("account.txt", "r");
	if(!fIn) {
		printf("File not exist??\n");
		exitProgram();
	}

	while(!feof(fIn)) {
		if(fscanf(fIn, "%s %s %d%c", username, password, &status, &c) != EOF) {
			push(createNewUser(username, password, status));
		}
		if(feof(fIn)) break;
	}
	fclose(fIn);
	return;
}
// register new user
int regis() {
	char username[254];
	char password[32];
	printf("---------- Register Your Account -----------");
	do {
		printf("\nUsername (<254): ");
		scanf("%s", username);
		if(!checkUsername(username)) {
			printf("Username in correct!!");
			continue;
		}
		if(search(username) == NULL) break;
		printf("\n Account existed!!\n");
	} while(1);
	
	do {
		printf("\nPassword (<32): ");
		scanf("%s", password);
		if(!checkPassword(password)) {
			printf("Password in correct!!");
			continue;
		}
		printf("\nConfirm Password: ");
		char confirmPassword[32];
		scanf("%s", confirmPassword);
		if(!strcmp(password, confirmPassword)) {
			break;
		}
		printf("\n Password in correct!!\n");
	} while(1);

	push(createNewUser(username, password, ACTIVE));
	printf("\n REIGISTER SUCCESS: %s WAS ACTIVATE\n\n", username);
	return 1;
}
// check user sign in with any account
int checkLoginWithAnyAccount() {
	User *ptr = head;

    while (ptr != NULL) {
    	if(ptr->isLogin == LOGIN) {
    		return 1;
    	}
    	ptr = ptr->next;
    }
    return 0;
}
// print info a user 
void printUserInfo(User* user) {
	printf("--------------------- Infomation of user %s -------------\n", user->username);
	printf("Username: %s\n", user->username);
    printf("Status: %s\n", ((user->status == ACTIVE) ? "Active" : "Blocked"));
    printf("----------------------------------\n");
}
// find user by userName
void searchByName() {

	char username[254];
	if(!checkLoginWithAnyAccount()) {
		printf("You are not sign in with any account !!\n");
		return;
	}
	printf("\n Username: ");
	scanf("%s", username);
	User *user = search(username);
	if(user == NULL) {
		printf("Username in correct!!");
		return;
	} else {
		printUserInfo(user);
	}

}
// signout with username
int signOut() {
	char username[254];
	printf("---------- Sign Out Your Account -----------\n");
	if(!checkLoginWithAnyAccount()) {
		printf("You are not sign in with any account !!\n");
		return 1;
	}
	printf("\n Username: ");
	scanf("%s", username);
	User *user = search(username);
	if(user == NULL) {
		printf("Username not existed!!");
		return 1;
	} else if(user->isLogin != LOGIN) {
		printf("This user is not sign in!! \n");
	} else {
		user->isLogin = OFFLINE;
		printf("----------SUCCESS-------------\n");
		printf("Goodbye %s!! \n", user->username);
		printf("-----------------------\n");
	}
	return 1;
}
// functional
void doChoiced(choice) {
	switch(choice) {
		case 1: 
			regis(); 
			updateFile();
			break;
		case 2: 
			signIn();
			updateFile();
			break;
		case 3: 
			searchByName();
			break;
		case 4: 
			signOut();
			break;
		default: exitProgram();
	}
}
//show menu
void menu() {
	int choice;
	printf("\n----------------------------------------------------\n");
	printf("USER MANAGEMENT PROGRAM\n");
	printf("----------------------------------------------------\n");
	printf("	1.Register\n");
	printf("	2.Sign In\n");
	printf("	3.Search\n");
	printf("	4.Sign Out\n");
	printf("You choice (1 to 4, other to quit): ");
	scanf("%d", &choice);
	doChoiced(choice);
	menu();
	return;
}
int main() {
	readFile();
	menu();
	return 1;
}