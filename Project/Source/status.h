/*
* define status 
* feature: 
*/

#ifndef __AUTHENTICATE_H__
#define __AUTHENTICATE_H__

typedef enum{
	USER_NOT_FOUND = "User is not found! Please try again!",
	USER_IS_BLOCKED = "User is blocked!",
	BLOCKED_USER = "Password is invalid! Account is blocked!",
	PASSWORD_INVALID = "Password is invalid!",
	LOGIN_SUCCESS = "Login is successfully!",
	USER_IS_ONLINE = "Account is signing in on another device!",
	ACCOUNT_IS_EXIST = "Account is existed! Please try again!",
	REGISTER_SUCCESS = "Register is successfully! ",
	LOGOUT_SUCCESS = "Logout successfully! See you again!",
} AuthenticateCode;

