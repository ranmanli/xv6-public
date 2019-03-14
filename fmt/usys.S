8750 #include "syscall.h"
8751 #include "traps.h"
8752 
8753 #define SYSCALL(name) \
8754   .globl name; \
8755   name: \
8756     movl $SYS_ ## name, %eax; \
8757     int $T_SYSCALL; \
8758     ret
8759 
8760 SYSCALL(fork)
8761 SYSCALL(exit)
8762 SYSCALL(wait)
8763 SYSCALL(pipe)
8764 SYSCALL(read)
8765 SYSCALL(write)
8766 SYSCALL(close)
8767 SYSCALL(kill)
8768 SYSCALL(exec)
8769 SYSCALL(open)
8770 SYSCALL(mknod)
8771 SYSCALL(unlink)
8772 SYSCALL(fstat)
8773 SYSCALL(link)
8774 SYSCALL(mkdir)
8775 SYSCALL(chdir)
8776 SYSCALL(dup)
8777 SYSCALL(getpid)
8778 SYSCALL(sbrk)
8779 SYSCALL(sleep)
8780 SYSCALL(uptime)
8781 SYSCALL(info) //cs202
8782 SYSCALL(settickets) //cs202
8783 
8784 
8785 
8786 
8787 
8788 
8789 
8790 
8791 
8792 
8793 
8794 
8795 
8796 
8797 
8798 
8799 
