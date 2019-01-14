/*
* define status 
* feature: 
*/

#ifndef __STATUS_H__
#define __STATUS_H__

typedef enum {
	USER_NOT_FOUND,
	USER_IS_BLOCKED,
	BLOCKED_USER,
	PASSWORD_INVALID,
	LOGIN_SUCCESS,
	USER_IS_ONLINE,
	ACCOUNT_IS_EXIST,
	REGISTER_SUCCESS,
	LOGOUT_SUCCESS,
	COMMAND_INVALID,
	SERVER_ERROR
} StatusCode;

typedef enum{
	UNDER_SOCK,
	SOCK
} SocketType;

typedef enum{
	FILE_NOT_FOUND
} FileCode;

#endif