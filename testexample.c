#include   "types.h"
#include   "user.h"

int   main (int  argc,  char  * argv[])
{
  
   int Scheduler ( void );

   Scheduler ();
   return   0 ;
 }


int   Scheduler ( void ){

  int  pid;
  int  i,j,k,tick;

  settickets (60);
  for  (i  =   2 ; i  >=  0 ; i-- ) {
    pid  =   fork ();
  
    if  (pid  >  0) {
       continue ;
    }
    else if (pid == 0) {
      tick = 200 - 30 * i;
      settickets (tick); 
      for(j = 0 ;j < 50000 ;j ++ ) {
        for(k = 0 ;k < 50000 ;k ++ ) {
          asm ( "nop" );
        }
      }
      info(1);
      printf (1, " \n  child# %d with %d tickets has finished!  \n ", getpid(), tick);   
      exit ();
    }
    else  {
      printf (2, "  \n  Error  \n ");  
    }
  }
  if (pid  >  0) {
    for  (i = 0;i < 3;i ++){
       wait (  );
    }
  }
  exit ();   
  return   0 ;
}