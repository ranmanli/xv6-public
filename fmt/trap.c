3600 #include "types.h"
3601 #include "defs.h"
3602 #include "param.h"
3603 #include "memlayout.h"
3604 #include "mmu.h"
3605 #include "proc.h"
3606 #include "x86.h"
3607 #include "traps.h"
3608 #include "spinlock.h"
3609 
3610 // Interrupt descriptor table (shared by all CPUs).
3611 struct gatedesc idt[256];
3612 extern uint vectors[];  // in vectors.S: array of 256 entry pointers
3613 struct spinlock tickslock;
3614 uint ticks;
3615 
3616 void
3617 tvinit(void)
3618 {
3619   int i;
3620 
3621   for(i = 0; i < 256; i++)
3622     SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
3623   SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
3624 
3625   initlock(&tickslock, "time");
3626 }
3627 
3628 void
3629 idtinit(void)
3630 {
3631   lidt(idt, sizeof(idt));
3632 }
3633 
3634 
3635 
3636 
3637 
3638 
3639 
3640 
3641 
3642 
3643 
3644 
3645 
3646 
3647 
3648 
3649 
3650 void
3651 trap(struct trapframe *tf)
3652 {
3653   if(tf->trapno == T_SYSCALL){
3654     if(myproc()->killed)
3655       exit();
3656     myproc()->tf = tf;
3657     syscall();
3658     if(myproc()->killed)
3659       exit();
3660     return;
3661   }
3662 
3663   switch(tf->trapno){
3664   case T_IRQ0 + IRQ_TIMER:
3665     if(cpuid() == 0){
3666       acquire(&tickslock);
3667       ticks++;
3668       wakeup(&ticks);
3669       release(&tickslock);
3670     }
3671     lapiceoi();
3672     break;
3673   case T_IRQ0 + IRQ_IDE:
3674     ideintr();
3675     lapiceoi();
3676     break;
3677   case T_IRQ0 + IRQ_IDE+1:
3678     // Bochs generates spurious IDE1 interrupts.
3679     break;
3680   case T_IRQ0 + IRQ_KBD:
3681     kbdintr();
3682     lapiceoi();
3683     break;
3684   case T_IRQ0 + IRQ_COM1:
3685     uartintr();
3686     lapiceoi();
3687     break;
3688   case T_IRQ0 + 7:
3689   case T_IRQ0 + IRQ_SPURIOUS:
3690     cprintf("cpu%d: spurious interrupt at %x:%x\n",
3691             cpuid(), tf->cs, tf->eip);
3692     lapiceoi();
3693     break;
3694 
3695 
3696 
3697 
3698 
3699 
3700   default:
3701     if(myproc() == 0 || (tf->cs&3) == 0){
3702       // In kernel, it must be our mistake.
3703       cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
3704               tf->trapno, cpuid(), tf->eip, rcr2());
3705       panic("trap");
3706     }
3707     // In user space, assume process misbehaved.
3708     cprintf("pid %d %s: trap %d err %d on cpu %d "
3709             "eip 0x%x addr 0x%x--kill proc\n",
3710             myproc()->pid, myproc()->name, tf->trapno,
3711             tf->err, cpuid(), tf->eip, rcr2());
3712     myproc()->killed = 1;
3713   }
3714 
3715   // Force process exit if it has been killed and is in user space.
3716   // (If it is still executing in the kernel, let it keep running
3717   // until it gets to the regular system call return.)
3718   if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
3719     exit();
3720 
3721   // Force process to give up CPU on clock tick.
3722   // If interrupts were on while locks held, would need to check nlock.
3723   if(myproc() && myproc()->state == RUNNING &&
3724      tf->trapno == T_IRQ0+IRQ_TIMER)
3725     yield();
3726 
3727   // Check if the process has been killed since we yielded
3728   if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
3729     exit();
3730 }
3731 
3732 
3733 
3734 
3735 
3736 
3737 
3738 
3739 
3740 
3741 
3742 
3743 
3744 
3745 
3746 
3747 
3748 
3749 
