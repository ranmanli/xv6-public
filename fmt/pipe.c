7050 #include "types.h"
7051 #include "defs.h"
7052 #include "param.h"
7053 #include "mmu.h"
7054 #include "proc.h"
7055 #include "fs.h"
7056 #include "spinlock.h"
7057 #include "sleeplock.h"
7058 #include "file.h"
7059 
7060 #define PIPESIZE 512
7061 
7062 struct pipe {
7063   struct spinlock lock;
7064   char data[PIPESIZE];
7065   uint nread;     // number of bytes read
7066   uint nwrite;    // number of bytes written
7067   int readopen;   // read fd is still open
7068   int writeopen;  // write fd is still open
7069 };
7070 
7071 int
7072 pipealloc(struct file **f0, struct file **f1)
7073 {
7074   struct pipe *p;
7075 
7076   p = 0;
7077   *f0 = *f1 = 0;
7078   if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
7079     goto bad;
7080   if((p = (struct pipe*)kalloc()) == 0)
7081     goto bad;
7082   p->readopen = 1;
7083   p->writeopen = 1;
7084   p->nwrite = 0;
7085   p->nread = 0;
7086   initlock(&p->lock, "pipe");
7087   (*f0)->type = FD_PIPE;
7088   (*f0)->readable = 1;
7089   (*f0)->writable = 0;
7090   (*f0)->pipe = p;
7091   (*f1)->type = FD_PIPE;
7092   (*f1)->readable = 0;
7093   (*f1)->writable = 1;
7094   (*f1)->pipe = p;
7095   return 0;
7096 
7097 
7098 
7099 
7100  bad:
7101   if(p)
7102     kfree((char*)p);
7103   if(*f0)
7104     fileclose(*f0);
7105   if(*f1)
7106     fileclose(*f1);
7107   return -1;
7108 }
7109 
7110 void
7111 pipeclose(struct pipe *p, int writable)
7112 {
7113   acquire(&p->lock);
7114   if(writable){
7115     p->writeopen = 0;
7116     wakeup(&p->nread);
7117   } else {
7118     p->readopen = 0;
7119     wakeup(&p->nwrite);
7120   }
7121   if(p->readopen == 0 && p->writeopen == 0){
7122     release(&p->lock);
7123     kfree((char*)p);
7124   } else
7125     release(&p->lock);
7126 }
7127 
7128 
7129 int
7130 pipewrite(struct pipe *p, char *addr, int n)
7131 {
7132   int i;
7133 
7134   acquire(&p->lock);
7135   for(i = 0; i < n; i++){
7136     while(p->nwrite == p->nread + PIPESIZE){  //DOC: pipewrite-full
7137       if(p->readopen == 0 || myproc()->killed){
7138         release(&p->lock);
7139         return -1;
7140       }
7141       wakeup(&p->nread);
7142       sleep(&p->nwrite, &p->lock);  //DOC: pipewrite-sleep
7143     }
7144     p->data[p->nwrite++ % PIPESIZE] = addr[i];
7145   }
7146   wakeup(&p->nread);  //DOC: pipewrite-wakeup1
7147   release(&p->lock);
7148   return n;
7149 }
7150 int
7151 piperead(struct pipe *p, char *addr, int n)
7152 {
7153   int i;
7154 
7155   acquire(&p->lock);
7156   while(p->nread == p->nwrite && p->writeopen){  //DOC: pipe-empty
7157     if(myproc()->killed){
7158       release(&p->lock);
7159       return -1;
7160     }
7161     sleep(&p->nread, &p->lock); //DOC: piperead-sleep
7162   }
7163   for(i = 0; i < n; i++){  //DOC: piperead-copy
7164     if(p->nread == p->nwrite)
7165       break;
7166     addr[i] = p->data[p->nread++ % PIPESIZE];
7167   }
7168   wakeup(&p->nwrite);  //DOC: piperead-wakeup
7169   release(&p->lock);
7170   return i;
7171 }
7172 
7173 
7174 
7175 
7176 
7177 
7178 
7179 
7180 
7181 
7182 
7183 
7184 
7185 
7186 
7187 
7188 
7189 
7190 
7191 
7192 
7193 
7194 
7195 
7196 
7197 
7198 
7199 
