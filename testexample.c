#include   "types.h"
#include   "user.h"

uint token;

int   main (int  argc,  char  * argv[])
{
  
   int Scheduler ( void );

   Scheduler ();
   return   0 ;
 }


int   Scheduler ( void ){

  int  pid;
  int  i,j,k,tick;
  int tickets[] = {100, 50, 250};
  token = 111;

  settickets (400);
  for  (i=0;i<3;i++) {
    pid  =   fork ();
  
    if  (pid  >  0) {
       continue ;
    }
    else if (pid == 0) {
      // tick = 30 - 10 * i;
      tick = tickets[i];
      settickets (tick); 
      for(j = 0 ;j < 50000 ;j ++ ) {
        for(k = 0 ;k < 1000 ;k ++ ) {
          asm ( "nop" );
        }
      }
      token++;
      printf(1, "%d : %d\n", getpid(), token);
      // info(1);
      // info(2);
      // info(3);
      info(4);
      printf (1, "\n  child# %d with %d tickets has finished!\n", getpid(), tick);   
      exit ();
    }
    else  {
      printf (2, "  \n  Error  \n ");  
    }
  }
  if (pid  >  0) {
    printf(1, "%d : %d\n", getpid(), token);
    for  (i = 0;i < 3;i ++){
       wait (  );
    }
  }
  exit ();   
  return   0 ;
}