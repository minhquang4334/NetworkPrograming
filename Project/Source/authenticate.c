
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
void append(User* newUser) {
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
void prepend(User** head, User * newUser) {
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
int identifyPassWord(User* user, char* password){
	return strcmp(user->password, password);
}

int isOnline(char *username){
	User *user = search(username);
	return username->isLogin;
}
// login
int login(char* username, char* password){
	User *user = search(username);
	if(user == NULL) return USER_NOT_FOUND;
	else{
		if(isOnline(username)) return USER_IS_ONLINE;
		else if(user->countLoginFails < MAX_LOGIN_FAILS){
			if(!identifyPassWord(user, password)){
				user->isLogin=ONLINE;
				user->countLoginFails=0;
				return LOGIN_SUCCESS;
			}
			else{
				user->countLoginFails++;
				if(use->countLoginFails == MAX_LOGIN_FAILS)
					return BLOCKED_USER;
				return PASSWORD_INVALID;
			}
		}
		else return USER_IS_BLOCKED;
	}
}

// register
int registerUser(char* username, char* password){
	User *user = search(username);
	if(user == NULL){
		user = createNewUser(username, password, ACTIVE);
		append(user);
		return REGISTER_SUCCESS;
	}
	return ACCOUNT_IS_EXIST;
}

// logout
int logout(User* user){

}

