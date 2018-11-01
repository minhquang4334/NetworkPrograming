#include <stdio.h>

struct student{
   int id;
   int score; 
};
int main() {
	int i;
	struct student students[5]; 
	for(i=0; i<5; i++){
	   	students[i].id = i;
	   	students[i].score = i;
	}
	for(i=0;i<5;i++){
       	printf("student id:%d, score:%d\n",
		students[i].id, students[i].score);
	}
	return 0; 
}