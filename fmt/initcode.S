8700 # Initial process execs /init.
8701 # This code runs in user space.
8702 
8703 #include "syscall.h"
8704 #include "traps.h"
8705 
8706 
8707 # exec(init, argv)
8708 .globl start
8709 start:
8710   pushl $argv
8711   pushl $init
8712   pushl $0  // where caller pc would be
8713   movl $SYS_exec, %eax
8714   int $T_SYSCALL
8715 
8716 # for(;;) exit();
8717 exit:
8718   movl $SYS_exit, %eax
8719   int $T_SYSCALL
8720   jmp exit
8721 
8722 # char init[] = "/init\0";
8723 init:
8724   .string "/init\0"
8725 
8726 # char *argv[] = { init, 0 };
8727 .p2align 2
8728 argv:
8729   .long init
8730   .long 0
8731 
8732 
8733 
8734 
8735 
8736 
8737 
8738 
8739 
8740 
8741 
8742 
8743 
8744 
8745 
8746 
8747 
8748 
8749 
