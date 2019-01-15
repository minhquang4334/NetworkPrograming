/*

*/
#include <stdio.h>
#include <string.h>
#include "validate.h"

int isHasBlankSpace(char* str) {
	int i = 0;
	for(i; i < strlen(str); i++) {
		if(str[i] == ' ') {
			return 1;
		}
	}
	return 0;
}

int inValidRange(int num, int min, int max) {
	return (num >= min) && (num <= max)
}
int validateUsername(char* username) {
	int length = strlen(username);
	if(inValidRange(length, MIN_LENGTH_OF_USERNAME, MAX_LENGTH_OF_USERNAME) && !isHasBlankSpace(username)) {
		return 1;
	}
	return 0;
}

int validatePassword(char* password) {
	int length = strlen(password);
	if(inValidRange(length, MIN_LENGTH_OF_PASSWORD, MAX_LENGTH_OF_PASSWORD) && !isHasBlankSpace(password)) {
		return 1;
	}
	return 0;
}