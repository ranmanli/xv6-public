#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int i,j;
   	for(i=0;i<1000;i++)
       for(j=0;j<1000;j++);
               asm("nop");
	info(1);
	info(2);
	info(3);
	// info(4);

	exit();

}