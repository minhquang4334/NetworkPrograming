/*
* api for handle for action related file
* feature: search, original_path
*/

#ifndef __FILE_H__
#define __FILE_H__

int searchFileInFolder(char* fileName, char* folderPath);

typedef struct requestFile{
	char fileName[50];
	int requestID;
	struct requestFile *next;
} RequestFile;

void push();

void remove();



