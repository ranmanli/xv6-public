8850 // Shell.
8851 
8852 #include "types.h"
8853 #include "user.h"
8854 #include "fcntl.h"
8855 
8856 // Parsed command representation
8857 #define EXEC  1
8858 #define REDIR 2
8859 #define PIPE  3
8860 #define LIST  4
8861 #define BACK  5
8862 
8863 #define MAXARGS 10
8864 
8865 struct cmd {
8866   int type;
8867 };
8868 
8869 struct execcmd {
8870   int type;
8871   char *argv[MAXARGS];
8872   char *eargv[MAXARGS];
8873 };
8874 
8875 struct redircmd {
8876   int type;
8877   struct cmd *cmd;
8878   char *file;
8879   char *efile;
8880   int mode;
8881   int fd;
8882 };
8883 
8884 struct pipecmd {
8885   int type;
8886   struct cmd *left;
8887   struct cmd *right;
8888 };
8889 
8890 struct listcmd {
8891   int type;
8892   struct cmd *left;
8893   struct cmd *right;
8894 };
8895 
8896 struct backcmd {
8897   int type;
8898   struct cmd *cmd;
8899 };
8900 int fork1(void);  // Fork but panics on failure.
8901 void panic(char*);
8902 struct cmd *parsecmd(char*);
8903 
8904 // Execute cmd.  Never returns.
8905 void
8906 runcmd(struct cmd *cmd)
8907 {
8908   int p[2];
8909   struct backcmd *bcmd;
8910   struct execcmd *ecmd;
8911   struct listcmd *lcmd;
8912   struct pipecmd *pcmd;
8913   struct redircmd *rcmd;
8914 
8915   if(cmd == 0)
8916     exit();
8917 
8918   switch(cmd->type){
8919   default:
8920     panic("runcmd");
8921 
8922   case EXEC:
8923     ecmd = (struct execcmd*)cmd;
8924     if(ecmd->argv[0] == 0)
8925       exit();
8926     exec(ecmd->argv[0], ecmd->argv);
8927     printf(2, "exec %s failed\n", ecmd->argv[0]);
8928     break;
8929 
8930   case REDIR:
8931     rcmd = (struct redircmd*)cmd;
8932     close(rcmd->fd);
8933     if(open(rcmd->file, rcmd->mode) < 0){
8934       printf(2, "open %s failed\n", rcmd->file);
8935       exit();
8936     }
8937     runcmd(rcmd->cmd);
8938     break;
8939 
8940   case LIST:
8941     lcmd = (struct listcmd*)cmd;
8942     if(fork1() == 0)
8943       runcmd(lcmd->left);
8944     wait();
8945     runcmd(lcmd->right);
8946     break;
8947 
8948 
8949 
8950   case PIPE:
8951     pcmd = (struct pipecmd*)cmd;
8952     if(pipe(p) < 0)
8953       panic("pipe");
8954     if(fork1() == 0){
8955       close(1);
8956       dup(p[1]);
8957       close(p[0]);
8958       close(p[1]);
8959       runcmd(pcmd->left);
8960     }
8961     if(fork1() == 0){
8962       close(0);
8963       dup(p[0]);
8964       close(p[0]);
8965       close(p[1]);
8966       runcmd(pcmd->right);
8967     }
8968     close(p[0]);
8969     close(p[1]);
8970     wait();
8971     wait();
8972     break;
8973 
8974   case BACK:
8975     bcmd = (struct backcmd*)cmd;
8976     if(fork1() == 0)
8977       runcmd(bcmd->cmd);
8978     break;
8979   }
8980   exit();
8981 }
8982 
8983 int
8984 getcmd(char *buf, int nbuf)
8985 {
8986   printf(2, "$ ");
8987   memset(buf, 0, nbuf);
8988   gets(buf, nbuf);
8989   if(buf[0] == 0) // EOF
8990     return -1;
8991   return 0;
8992 }
8993 
8994 
8995 
8996 
8997 
8998 
8999 
9000 int
9001 main(void)
9002 {
9003   static char buf[100];
9004   int fd;
9005 
9006   // Ensure that three file descriptors are open.
9007   while((fd = open("console", O_RDWR)) >= 0){
9008     if(fd >= 3){
9009       close(fd);
9010       break;
9011     }
9012   }
9013 
9014   // Read and run input commands.
9015   while(getcmd(buf, sizeof(buf)) >= 0){
9016     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
9017       // Chdir must be called by the parent, not the child.
9018       buf[strlen(buf)-1] = 0;  // chop \n
9019       if(chdir(buf+3) < 0)
9020         printf(2, "cannot cd %s\n", buf+3);
9021       continue;
9022     }
9023     if(fork1() == 0)
9024       runcmd(parsecmd(buf));
9025     wait();
9026   }
9027   exit();
9028 }
9029 
9030 void
9031 panic(char *s)
9032 {
9033   printf(2, "%s\n", s);
9034   exit();
9035 }
9036 
9037 int
9038 fork1(void)
9039 {
9040   int pid;
9041 
9042   pid = fork();
9043   if(pid == -1)
9044     panic("fork");
9045   return pid;
9046 }
9047 
9048 
9049 
9050 // Constructors
9051 
9052 struct cmd*
9053 execcmd(void)
9054 {
9055   struct execcmd *cmd;
9056 
9057   cmd = malloc(sizeof(*cmd));
9058   memset(cmd, 0, sizeof(*cmd));
9059   cmd->type = EXEC;
9060   return (struct cmd*)cmd;
9061 }
9062 
9063 struct cmd*
9064 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
9065 {
9066   struct redircmd *cmd;
9067 
9068   cmd = malloc(sizeof(*cmd));
9069   memset(cmd, 0, sizeof(*cmd));
9070   cmd->type = REDIR;
9071   cmd->cmd = subcmd;
9072   cmd->file = file;
9073   cmd->efile = efile;
9074   cmd->mode = mode;
9075   cmd->fd = fd;
9076   return (struct cmd*)cmd;
9077 }
9078 
9079 struct cmd*
9080 pipecmd(struct cmd *left, struct cmd *right)
9081 {
9082   struct pipecmd *cmd;
9083 
9084   cmd = malloc(sizeof(*cmd));
9085   memset(cmd, 0, sizeof(*cmd));
9086   cmd->type = PIPE;
9087   cmd->left = left;
9088   cmd->right = right;
9089   return (struct cmd*)cmd;
9090 }
9091 
9092 
9093 
9094 
9095 
9096 
9097 
9098 
9099 
9100 struct cmd*
9101 listcmd(struct cmd *left, struct cmd *right)
9102 {
9103   struct listcmd *cmd;
9104 
9105   cmd = malloc(sizeof(*cmd));
9106   memset(cmd, 0, sizeof(*cmd));
9107   cmd->type = LIST;
9108   cmd->left = left;
9109   cmd->right = right;
9110   return (struct cmd*)cmd;
9111 }
9112 
9113 struct cmd*
9114 backcmd(struct cmd *subcmd)
9115 {
9116   struct backcmd *cmd;
9117 
9118   cmd = malloc(sizeof(*cmd));
9119   memset(cmd, 0, sizeof(*cmd));
9120   cmd->type = BACK;
9121   cmd->cmd = subcmd;
9122   return (struct cmd*)cmd;
9123 }
9124 
9125 
9126 
9127 
9128 
9129 
9130 
9131 
9132 
9133 
9134 
9135 
9136 
9137 
9138 
9139 
9140 
9141 
9142 
9143 
9144 
9145 
9146 
9147 
9148 
9149 
9150 // Parsing
9151 
9152 char whitespace[] = " \t\r\n\v";
9153 char symbols[] = "<|>&;()";
9154 
9155 int
9156 gettoken(char **ps, char *es, char **q, char **eq)
9157 {
9158   char *s;
9159   int ret;
9160 
9161   s = *ps;
9162   while(s < es && strchr(whitespace, *s))
9163     s++;
9164   if(q)
9165     *q = s;
9166   ret = *s;
9167   switch(*s){
9168   case 0:
9169     break;
9170   case '|':
9171   case '(':
9172   case ')':
9173   case ';':
9174   case '&':
9175   case '<':
9176     s++;
9177     break;
9178   case '>':
9179     s++;
9180     if(*s == '>'){
9181       ret = '+';
9182       s++;
9183     }
9184     break;
9185   default:
9186     ret = 'a';
9187     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
9188       s++;
9189     break;
9190   }
9191   if(eq)
9192     *eq = s;
9193 
9194   while(s < es && strchr(whitespace, *s))
9195     s++;
9196   *ps = s;
9197   return ret;
9198 }
9199 
9200 int
9201 peek(char **ps, char *es, char *toks)
9202 {
9203   char *s;
9204 
9205   s = *ps;
9206   while(s < es && strchr(whitespace, *s))
9207     s++;
9208   *ps = s;
9209   return *s && strchr(toks, *s);
9210 }
9211 
9212 struct cmd *parseline(char**, char*);
9213 struct cmd *parsepipe(char**, char*);
9214 struct cmd *parseexec(char**, char*);
9215 struct cmd *nulterminate(struct cmd*);
9216 
9217 struct cmd*
9218 parsecmd(char *s)
9219 {
9220   char *es;
9221   struct cmd *cmd;
9222 
9223   es = s + strlen(s);
9224   cmd = parseline(&s, es);
9225   peek(&s, es, "");
9226   if(s != es){
9227     printf(2, "leftovers: %s\n", s);
9228     panic("syntax");
9229   }
9230   nulterminate(cmd);
9231   return cmd;
9232 }
9233 
9234 struct cmd*
9235 parseline(char **ps, char *es)
9236 {
9237   struct cmd *cmd;
9238 
9239   cmd = parsepipe(ps, es);
9240   while(peek(ps, es, "&")){
9241     gettoken(ps, es, 0, 0);
9242     cmd = backcmd(cmd);
9243   }
9244   if(peek(ps, es, ";")){
9245     gettoken(ps, es, 0, 0);
9246     cmd = listcmd(cmd, parseline(ps, es));
9247   }
9248   return cmd;
9249 }
9250 struct cmd*
9251 parsepipe(char **ps, char *es)
9252 {
9253   struct cmd *cmd;
9254 
9255   cmd = parseexec(ps, es);
9256   if(peek(ps, es, "|")){
9257     gettoken(ps, es, 0, 0);
9258     cmd = pipecmd(cmd, parsepipe(ps, es));
9259   }
9260   return cmd;
9261 }
9262 
9263 struct cmd*
9264 parseredirs(struct cmd *cmd, char **ps, char *es)
9265 {
9266   int tok;
9267   char *q, *eq;
9268 
9269   while(peek(ps, es, "<>")){
9270     tok = gettoken(ps, es, 0, 0);
9271     if(gettoken(ps, es, &q, &eq) != 'a')
9272       panic("missing file for redirection");
9273     switch(tok){
9274     case '<':
9275       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
9276       break;
9277     case '>':
9278       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9279       break;
9280     case '+':  // >>
9281       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9282       break;
9283     }
9284   }
9285   return cmd;
9286 }
9287 
9288 
9289 
9290 
9291 
9292 
9293 
9294 
9295 
9296 
9297 
9298 
9299 
9300 struct cmd*
9301 parseblock(char **ps, char *es)
9302 {
9303   struct cmd *cmd;
9304 
9305   if(!peek(ps, es, "("))
9306     panic("parseblock");
9307   gettoken(ps, es, 0, 0);
9308   cmd = parseline(ps, es);
9309   if(!peek(ps, es, ")"))
9310     panic("syntax - missing )");
9311   gettoken(ps, es, 0, 0);
9312   cmd = parseredirs(cmd, ps, es);
9313   return cmd;
9314 }
9315 
9316 struct cmd*
9317 parseexec(char **ps, char *es)
9318 {
9319   char *q, *eq;
9320   int tok, argc;
9321   struct execcmd *cmd;
9322   struct cmd *ret;
9323 
9324   if(peek(ps, es, "("))
9325     return parseblock(ps, es);
9326 
9327   ret = execcmd();
9328   cmd = (struct execcmd*)ret;
9329 
9330   argc = 0;
9331   ret = parseredirs(ret, ps, es);
9332   while(!peek(ps, es, "|)&;")){
9333     if((tok=gettoken(ps, es, &q, &eq)) == 0)
9334       break;
9335     if(tok != 'a')
9336       panic("syntax");
9337     cmd->argv[argc] = q;
9338     cmd->eargv[argc] = eq;
9339     argc++;
9340     if(argc >= MAXARGS)
9341       panic("too many args");
9342     ret = parseredirs(ret, ps, es);
9343   }
9344   cmd->argv[argc] = 0;
9345   cmd->eargv[argc] = 0;
9346   return ret;
9347 }
9348 
9349 
9350 // NUL-terminate all the counted strings.
9351 struct cmd*
9352 nulterminate(struct cmd *cmd)
9353 {
9354   int i;
9355   struct backcmd *bcmd;
9356   struct execcmd *ecmd;
9357   struct listcmd *lcmd;
9358   struct pipecmd *pcmd;
9359   struct redircmd *rcmd;
9360 
9361   if(cmd == 0)
9362     return 0;
9363 
9364   switch(cmd->type){
9365   case EXEC:
9366     ecmd = (struct execcmd*)cmd;
9367     for(i=0; ecmd->argv[i]; i++)
9368       *ecmd->eargv[i] = 0;
9369     break;
9370 
9371   case REDIR:
9372     rcmd = (struct redircmd*)cmd;
9373     nulterminate(rcmd->cmd);
9374     *rcmd->efile = 0;
9375     break;
9376 
9377   case PIPE:
9378     pcmd = (struct pipecmd*)cmd;
9379     nulterminate(pcmd->left);
9380     nulterminate(pcmd->right);
9381     break;
9382 
9383   case LIST:
9384     lcmd = (struct listcmd*)cmd;
9385     nulterminate(lcmd->left);
9386     nulterminate(lcmd->right);
9387     break;
9388 
9389   case BACK:
9390     bcmd = (struct backcmd*)cmd;
9391     nulterminate(bcmd->cmd);
9392     break;
9393   }
9394   return cmd;
9395 }
9396 
9397 
9398 
9399 
