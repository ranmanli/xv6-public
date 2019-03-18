#include "types.h"
#include "user.h"

#define NULL ((void *)0)

struct lock
{
	struct lock *l;
}frisbee;

int main(int argc, char *argv[]){
	int test1 (void);

	test1();

	exit();

	return 0;
}

int test1 (void){

	int i;

	frisbee.l = NULL;
	printf(1, "%d", (int)frisbee.l);
	i=0;

	return i;
	
}