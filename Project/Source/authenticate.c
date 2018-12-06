
// create new user from username, password, status
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

// get List User
// readfile and create list users
void getListUser(char* fileName); 

// updateListUser
void updateListUser(char* fileName);

// fucntional
User* search(char *username) {
	User *ptr = head;

    while (ptr != NULL) {
    	if(!strcmp(ptr->username, username)) break;
        ptr = ptr->next;
    }
    
	return ptr;
}


// compare password 
int identifyPassWord(User* user, char* password);

// login
int login(char* username, char* password);

// register
int registerUser(char* username, char* password);

// logout
int logout(User* user);

