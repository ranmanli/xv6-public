4900 // Sleeping locks
4901 
4902 #include "types.h"
4903 #include "defs.h"
4904 #include "param.h"
4905 #include "x86.h"
4906 #include "memlayout.h"
4907 #include "mmu.h"
4908 #include "proc.h"
4909 #include "spinlock.h"
4910 #include "sleeplock.h"
4911 
4912 void
4913 initsleeplock(struct sleeplock *lk, char *name)
4914 {
4915   initlock(&lk->lk, "sleep lock");
4916   lk->name = name;
4917   lk->locked = 0;
4918   lk->pid = 0;
4919 }
4920 
4921 void
4922 acquiresleep(struct sleeplock *lk)
4923 {
4924   acquire(&lk->lk);
4925   while (lk->locked) {
4926     sleep(lk, &lk->lk);
4927   }
4928   lk->locked = 1;
4929   lk->pid = myproc()->pid;
4930   release(&lk->lk);
4931 }
4932 
4933 void
4934 releasesleep(struct sleeplock *lk)
4935 {
4936   acquire(&lk->lk);
4937   lk->locked = 0;
4938   lk->pid = 0;
4939   wakeup(lk);
4940   release(&lk->lk);
4941 }
4942 
4943 
4944 
4945 
4946 
4947 
4948 
4949 
4950 int
4951 holdingsleep(struct sleeplock *lk)
4952 {
4953   int r;
4954 
4955   acquire(&lk->lk);
4956   r = lk->locked && (lk->pid == myproc()->pid);
4957   release(&lk->lk);
4958   return r;
4959 }
4960 
4961 
4962 
4963 
4964 
4965 
4966 
4967 
4968 
4969 
4970 
4971 
4972 
4973 
4974 
4975 
4976 
4977 
4978 
4979 
4980 
4981 
4982 
4983 
4984 
4985 
4986 
4987 
4988 
4989 
4990 
4991 
4992 
4993 
4994 
4995 
4996 
4997 
4998 
4999 
