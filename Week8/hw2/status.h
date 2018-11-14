#include<stdio.h>

enum StatusCode {
	//success code
	LoginSuccess = 200,
	UserNameSucess = 201,
	PasswordSuccess = 202,
	LogoutSuccess = 203,
	//error code
	PhaseError = 300,
	Logged = 301,
	NotLogin = 302,
	// invalid code
	PasswordIncorrect = 400,
	CommandInvalid = 401,
	UserNotExist = 402,
	UserIsBlocked = 403,
	//server error
	ServerError = 500
};