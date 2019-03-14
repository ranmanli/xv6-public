8800 // init: The initial user-level program
8801 
8802 #include "types.h"
8803 #include "stat.h"
8804 #include "user.h"
8805 #include "fcntl.h"
8806 
8807 char *argv[] = { "sh", 0 };
8808 
8809 int
8810 main(void)
8811 {
8812   int pid, wpid;
8813 
8814   if(open("console", O_RDWR) < 0){
8815     mknod("console", 1, 1);
8816     open("console", O_RDWR);
8817   }
8818   dup(0);  // stdout
8819   dup(0);  // stderr
8820 
8821   for(;;){
8822     printf(1, "init: starting sh\n");
8823     pid = fork();
8824     if(pid < 0){
8825       printf(1, "init: fork failed\n");
8826       exit();
8827     }
8828     if(pid == 0){
8829       exec("sh", argv);
8830       printf(1, "init: exec sh failed\n");
8831       exit();
8832     }
8833     while((wpid=wait()) >= 0 && wpid != pid)
8834       printf(1, "zombie!\n");
8835   }
8836 }
8837 
8838 
8839 
8840 
8841 
8842 
8843 
8844 
8845 
8846 
8847 
8848 
8849 
