4000 #include "types.h"
4001 #include "x86.h"
4002 #include "defs.h"
4003 #include "date.h"
4004 #include "param.h"
4005 #include "memlayout.h"
4006 #include "mmu.h"
4007 #include "proc.h"
4008 
4009 //cs202
4010 int
4011 sys_info(void)
4012 {
4013   int type;
4014 
4015   if (argint(0, &type) < 0)
4016     return -1;
4017 
4018   return info(type);
4019 }
4020 
4021 int
4022 sys_settickets(void)
4023 {
4024   int tickets;
4025 
4026   if (argint(0, &tickets) < 0)
4027     return -1;
4028 
4029   return settickets(tickets);
4030 }
4031 //cs202
4032 
4033 int
4034 sys_fork(void)
4035 {
4036   return fork();
4037 }
4038 
4039 int
4040 sys_exit(void)
4041 {
4042   exit();
4043   return 0;  // not reached
4044 }
4045 
4046 
4047 
4048 
4049 
4050 int
4051 sys_wait(void)
4052 {
4053   return wait();
4054 }
4055 
4056 int
4057 sys_kill(void)
4058 {
4059   int pid;
4060 
4061   if(argint(0, &pid) < 0)
4062     return -1;
4063   return kill(pid);
4064 }
4065 
4066 int
4067 sys_getpid(void)
4068 {
4069   return myproc()->pid;
4070 }
4071 
4072 int
4073 sys_sbrk(void)
4074 {
4075   int addr;
4076   int n;
4077 
4078   if(argint(0, &n) < 0)
4079     return -1;
4080   addr = myproc()->sz;
4081   if(growproc(n) < 0)
4082     return -1;
4083   return addr;
4084 }
4085 
4086 
4087 
4088 
4089 
4090 
4091 
4092 
4093 
4094 
4095 
4096 
4097 
4098 
4099 
4100 int
4101 sys_sleep(void)
4102 {
4103   int n;
4104   uint ticks0;
4105 
4106   if(argint(0, &n) < 0)
4107     return -1;
4108   acquire(&tickslock);
4109   ticks0 = ticks;
4110   while(ticks - ticks0 < n){
4111     if(myproc()->killed){
4112       release(&tickslock);
4113       return -1;
4114     }
4115     sleep(&ticks, &tickslock);
4116   }
4117   release(&tickslock);
4118   return 0;
4119 }
4120 
4121 // return how many clock tick interrupts have occurred
4122 // since start.
4123 int
4124 sys_uptime(void)
4125 {
4126   uint xticks;
4127 
4128   acquire(&tickslock);
4129   xticks = ticks;
4130   release(&tickslock);
4131   return xticks;
4132 }
4133 
4134 
4135 
4136 
4137 
4138 
4139 
4140 
4141 
4142 
4143 
4144 
4145 
4146 
4147 
4148 
4149 
