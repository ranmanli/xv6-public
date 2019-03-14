6900 #include "types.h"
6901 #include "param.h"
6902 #include "memlayout.h"
6903 #include "mmu.h"
6904 #include "proc.h"
6905 #include "defs.h"
6906 #include "x86.h"
6907 #include "elf.h"
6908 
6909 int
6910 exec(char *path, char **argv)
6911 {
6912   char *s, *last;
6913   int i, off;
6914   uint argc, sz, sp, ustack[3+MAXARG+1];
6915   struct elfhdr elf;
6916   struct inode *ip;
6917   struct proghdr ph;
6918   pde_t *pgdir, *oldpgdir;
6919   struct proc *curproc = myproc();
6920 
6921   begin_op();
6922 
6923   if((ip = namei(path)) == 0){
6924     end_op();
6925     cprintf("exec: fail\n");
6926     return -1;
6927   }
6928   ilock(ip);
6929   pgdir = 0;
6930 
6931   // Check ELF header
6932   if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
6933     goto bad;
6934   if(elf.magic != ELF_MAGIC)
6935     goto bad;
6936 
6937   if((pgdir = setupkvm()) == 0)
6938     goto bad;
6939 
6940   // Load program into memory.
6941   sz = 0;
6942   for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
6943     if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
6944       goto bad;
6945     if(ph.type != ELF_PROG_LOAD)
6946       continue;
6947     if(ph.memsz < ph.filesz)
6948       goto bad;
6949     if(ph.vaddr + ph.memsz < ph.vaddr)
6950       goto bad;
6951     if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
6952       goto bad;
6953     if(ph.vaddr % PGSIZE != 0)
6954       goto bad;
6955     if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
6956       goto bad;
6957   }
6958   iunlockput(ip);
6959   end_op();
6960   ip = 0;
6961 
6962   // Allocate two pages at the next page boundary.
6963   // Make the first inaccessible.  Use the second as the user stack.
6964   sz = PGROUNDUP(sz);
6965   if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
6966     goto bad;
6967   clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
6968   sp = sz;
6969 
6970   // Push argument strings, prepare rest of stack in ustack.
6971   for(argc = 0; argv[argc]; argc++) {
6972     if(argc >= MAXARG)
6973       goto bad;
6974     sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
6975     if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
6976       goto bad;
6977     ustack[3+argc] = sp;
6978   }
6979   ustack[3+argc] = 0;
6980 
6981   ustack[0] = 0xffffffff;  // fake return PC
6982   ustack[1] = argc;
6983   ustack[2] = sp - (argc+1)*4;  // argv pointer
6984 
6985   sp -= (3+argc+1) * 4;
6986   if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
6987     goto bad;
6988 
6989   // Save program name for debugging.
6990   for(last=s=path; *s; s++)
6991     if(*s == '/')
6992       last = s+1;
6993   safestrcpy(curproc->name, last, sizeof(curproc->name));
6994 
6995   // Commit to the user image.
6996   oldpgdir = curproc->pgdir;
6997   curproc->pgdir = pgdir;
6998   curproc->sz = sz;
6999   curproc->tf->eip = elf.entry;  // main
7000   curproc->tf->esp = sp;
7001   switchuvm(curproc);
7002   freevm(oldpgdir);
7003   return 0;
7004 
7005  bad:
7006   if(pgdir)
7007     freevm(pgdir);
7008   if(ip){
7009     iunlockput(ip);
7010     end_op();
7011   }
7012   return -1;
7013 }
7014 
7015 
7016 
7017 
7018 
7019 
7020 
7021 
7022 
7023 
7024 
7025 
7026 
7027 
7028 
7029 
7030 
7031 
7032 
7033 
7034 
7035 
7036 
7037 
7038 
7039 
7040 
7041 
7042 
7043 
7044 
7045 
7046 
7047 
7048 
7049 
