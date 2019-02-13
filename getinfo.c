#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
	int i;
	int type;
	if(!(argc == 2)){
		printf(2, "usafe: getinfo [1, 2, 3]\n");
		exit();
	}
	for(i=1; i<argc; i++){
		type = atoi(argv[i]);
		if (type > 3 || type < 1){
			printf(2, "usafe: getinfo [1, 2, 3]\n");
			exit();
		}
		info(type);
	}
	exit();

}