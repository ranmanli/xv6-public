3550 #include "mmu.h"
3551 
3552   # vectors.S sends all traps here.
3553 .globl alltraps
3554 alltraps:
3555   # Build trap frame.
3556   pushl %ds
3557   pushl %es
3558   pushl %fs
3559   pushl %gs
3560   pushal
3561 
3562   # Set up data segments.
3563   movw $(SEG_KDATA<<3), %ax
3564   movw %ax, %ds
3565   movw %ax, %es
3566 
3567   # Call trap(tf), where tf=%esp
3568   pushl %esp
3569   call trap
3570   addl $4, %esp
3571 
3572   # Return falls through to trapret...
3573 .globl trapret
3574 trapret:
3575   popal
3576   popl %gs
3577   popl %fs
3578   popl %es
3579   popl %ds
3580   addl $0x8, %esp  # trapno and errcode
3581   iret
3582 
3583 
3584 
3585 
3586 
3587 
3588 
3589 
3590 
3591 
3592 
3593 
3594 
3595 
3596 
3597 
3598 
3599 
