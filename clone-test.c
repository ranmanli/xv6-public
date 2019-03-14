/*
-use the parent's address space
-use the same file descriptor but not duplicates
parameter stack is the pointer passed in 
which works as the user stack 
must be allocated before calling clone

*/

int clone(void *stack, int size)
{
	int i, pid;
	struct proc *np;
	struct proc *curproc = myproc();

	
}