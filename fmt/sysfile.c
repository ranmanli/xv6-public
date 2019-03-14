6350 //
6351 // File-system system calls.
6352 // Mostly argument checking, since we don't trust
6353 // user code, and calls into file.c and fs.c.
6354 //
6355 
6356 #include "types.h"
6357 #include "defs.h"
6358 #include "param.h"
6359 #include "stat.h"
6360 #include "mmu.h"
6361 #include "proc.h"
6362 #include "fs.h"
6363 #include "spinlock.h"
6364 #include "sleeplock.h"
6365 #include "file.h"
6366 #include "fcntl.h"
6367 
6368 // Fetch the nth word-sized system call argument as a file descriptor
6369 // and return both the descriptor and the corresponding struct file.
6370 static int
6371 argfd(int n, int *pfd, struct file **pf)
6372 {
6373   int fd;
6374   struct file *f;
6375 
6376   if(argint(n, &fd) < 0)
6377     return -1;
6378   if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
6379     return -1;
6380   if(pfd)
6381     *pfd = fd;
6382   if(pf)
6383     *pf = f;
6384   return 0;
6385 }
6386 
6387 
6388 
6389 
6390 
6391 
6392 
6393 
6394 
6395 
6396 
6397 
6398 
6399 
6400 // Allocate a file descriptor for the given file.
6401 // Takes over file reference from caller on success.
6402 static int
6403 fdalloc(struct file *f)
6404 {
6405   int fd;
6406   struct proc *curproc = myproc();
6407 
6408   for(fd = 0; fd < NOFILE; fd++){
6409     if(curproc->ofile[fd] == 0){
6410       curproc->ofile[fd] = f;
6411       return fd;
6412     }
6413   }
6414   return -1;
6415 }
6416 
6417 int
6418 sys_dup(void)
6419 {
6420   struct file *f;
6421   int fd;
6422 
6423   if(argfd(0, 0, &f) < 0)
6424     return -1;
6425   if((fd=fdalloc(f)) < 0)
6426     return -1;
6427   filedup(f);
6428   return fd;
6429 }
6430 
6431 int
6432 sys_read(void)
6433 {
6434   struct file *f;
6435   int n;
6436   char *p;
6437 
6438   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
6439     return -1;
6440   return fileread(f, p, n);
6441 }
6442 
6443 
6444 
6445 
6446 
6447 
6448 
6449 
6450 int
6451 sys_write(void)
6452 {
6453   struct file *f;
6454   int n;
6455   char *p;
6456 
6457   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
6458     return -1;
6459   return filewrite(f, p, n);
6460 }
6461 
6462 int
6463 sys_close(void)
6464 {
6465   int fd;
6466   struct file *f;
6467 
6468   if(argfd(0, &fd, &f) < 0)
6469     return -1;
6470   myproc()->ofile[fd] = 0;
6471   fileclose(f);
6472   return 0;
6473 }
6474 
6475 int
6476 sys_fstat(void)
6477 {
6478   struct file *f;
6479   struct stat *st;
6480 
6481   if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
6482     return -1;
6483   return filestat(f, st);
6484 }
6485 
6486 
6487 
6488 
6489 
6490 
6491 
6492 
6493 
6494 
6495 
6496 
6497 
6498 
6499 
6500 // Create the path new as a link to the same inode as old.
6501 int
6502 sys_link(void)
6503 {
6504   char name[DIRSIZ], *new, *old;
6505   struct inode *dp, *ip;
6506 
6507   if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
6508     return -1;
6509 
6510   begin_op();
6511   if((ip = namei(old)) == 0){
6512     end_op();
6513     return -1;
6514   }
6515 
6516   ilock(ip);
6517   if(ip->type == T_DIR){
6518     iunlockput(ip);
6519     end_op();
6520     return -1;
6521   }
6522 
6523   ip->nlink++;
6524   iupdate(ip);
6525   iunlock(ip);
6526 
6527   if((dp = nameiparent(new, name)) == 0)
6528     goto bad;
6529   ilock(dp);
6530   if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
6531     iunlockput(dp);
6532     goto bad;
6533   }
6534   iunlockput(dp);
6535   iput(ip);
6536 
6537   end_op();
6538 
6539   return 0;
6540 
6541 bad:
6542   ilock(ip);
6543   ip->nlink--;
6544   iupdate(ip);
6545   iunlockput(ip);
6546   end_op();
6547   return -1;
6548 }
6549 
6550 // Is the directory dp empty except for "." and ".." ?
6551 static int
6552 isdirempty(struct inode *dp)
6553 {
6554   int off;
6555   struct dirent de;
6556 
6557   for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
6558     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6559       panic("isdirempty: readi");
6560     if(de.inum != 0)
6561       return 0;
6562   }
6563   return 1;
6564 }
6565 
6566 
6567 
6568 
6569 
6570 
6571 
6572 
6573 
6574 
6575 
6576 
6577 
6578 
6579 
6580 
6581 
6582 
6583 
6584 
6585 
6586 
6587 
6588 
6589 
6590 
6591 
6592 
6593 
6594 
6595 
6596 
6597 
6598 
6599 
6600 int
6601 sys_unlink(void)
6602 {
6603   struct inode *ip, *dp;
6604   struct dirent de;
6605   char name[DIRSIZ], *path;
6606   uint off;
6607 
6608   if(argstr(0, &path) < 0)
6609     return -1;
6610 
6611   begin_op();
6612   if((dp = nameiparent(path, name)) == 0){
6613     end_op();
6614     return -1;
6615   }
6616 
6617   ilock(dp);
6618 
6619   // Cannot unlink "." or "..".
6620   if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
6621     goto bad;
6622 
6623   if((ip = dirlookup(dp, name, &off)) == 0)
6624     goto bad;
6625   ilock(ip);
6626 
6627   if(ip->nlink < 1)
6628     panic("unlink: nlink < 1");
6629   if(ip->type == T_DIR && !isdirempty(ip)){
6630     iunlockput(ip);
6631     goto bad;
6632   }
6633 
6634   memset(&de, 0, sizeof(de));
6635   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6636     panic("unlink: writei");
6637   if(ip->type == T_DIR){
6638     dp->nlink--;
6639     iupdate(dp);
6640   }
6641   iunlockput(dp);
6642 
6643   ip->nlink--;
6644   iupdate(ip);
6645   iunlockput(ip);
6646 
6647   end_op();
6648 
6649   return 0;
6650 bad:
6651   iunlockput(dp);
6652   end_op();
6653   return -1;
6654 }
6655 
6656 static struct inode*
6657 create(char *path, short type, short major, short minor)
6658 {
6659   uint off;
6660   struct inode *ip, *dp;
6661   char name[DIRSIZ];
6662 
6663   if((dp = nameiparent(path, name)) == 0)
6664     return 0;
6665   ilock(dp);
6666 
6667   if((ip = dirlookup(dp, name, &off)) != 0){
6668     iunlockput(dp);
6669     ilock(ip);
6670     if(type == T_FILE && ip->type == T_FILE)
6671       return ip;
6672     iunlockput(ip);
6673     return 0;
6674   }
6675 
6676   if((ip = ialloc(dp->dev, type)) == 0)
6677     panic("create: ialloc");
6678 
6679   ilock(ip);
6680   ip->major = major;
6681   ip->minor = minor;
6682   ip->nlink = 1;
6683   iupdate(ip);
6684 
6685   if(type == T_DIR){  // Create . and .. entries.
6686     dp->nlink++;  // for ".."
6687     iupdate(dp);
6688     // No ip->nlink++ for ".": avoid cyclic ref count.
6689     if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
6690       panic("create dots");
6691   }
6692 
6693   if(dirlink(dp, name, ip->inum) < 0)
6694     panic("create: dirlink");
6695 
6696   iunlockput(dp);
6697 
6698   return ip;
6699 }
6700 int
6701 sys_open(void)
6702 {
6703   char *path;
6704   int fd, omode;
6705   struct file *f;
6706   struct inode *ip;
6707 
6708   if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
6709     return -1;
6710 
6711   begin_op();
6712 
6713   if(omode & O_CREATE){
6714     ip = create(path, T_FILE, 0, 0);
6715     if(ip == 0){
6716       end_op();
6717       return -1;
6718     }
6719   } else {
6720     if((ip = namei(path)) == 0){
6721       end_op();
6722       return -1;
6723     }
6724     ilock(ip);
6725     if(ip->type == T_DIR && omode != O_RDONLY){
6726       iunlockput(ip);
6727       end_op();
6728       return -1;
6729     }
6730   }
6731 
6732   if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
6733     if(f)
6734       fileclose(f);
6735     iunlockput(ip);
6736     end_op();
6737     return -1;
6738   }
6739   iunlock(ip);
6740   end_op();
6741 
6742   f->type = FD_INODE;
6743   f->ip = ip;
6744   f->off = 0;
6745   f->readable = !(omode & O_WRONLY);
6746   f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
6747   return fd;
6748 }
6749 
6750 int
6751 sys_mkdir(void)
6752 {
6753   char *path;
6754   struct inode *ip;
6755 
6756   begin_op();
6757   if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
6758     end_op();
6759     return -1;
6760   }
6761   iunlockput(ip);
6762   end_op();
6763   return 0;
6764 }
6765 
6766 int
6767 sys_mknod(void)
6768 {
6769   struct inode *ip;
6770   char *path;
6771   int major, minor;
6772 
6773   begin_op();
6774   if((argstr(0, &path)) < 0 ||
6775      argint(1, &major) < 0 ||
6776      argint(2, &minor) < 0 ||
6777      (ip = create(path, T_DEV, major, minor)) == 0){
6778     end_op();
6779     return -1;
6780   }
6781   iunlockput(ip);
6782   end_op();
6783   return 0;
6784 }
6785 
6786 
6787 
6788 
6789 
6790 
6791 
6792 
6793 
6794 
6795 
6796 
6797 
6798 
6799 
6800 int
6801 sys_chdir(void)
6802 {
6803   char *path;
6804   struct inode *ip;
6805   struct proc *curproc = myproc();
6806 
6807   begin_op();
6808   if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
6809     end_op();
6810     return -1;
6811   }
6812   ilock(ip);
6813   if(ip->type != T_DIR){
6814     iunlockput(ip);
6815     end_op();
6816     return -1;
6817   }
6818   iunlock(ip);
6819   iput(curproc->cwd);
6820   end_op();
6821   curproc->cwd = ip;
6822   return 0;
6823 }
6824 
6825 int
6826 sys_exec(void)
6827 {
6828   char *path, *argv[MAXARG];
6829   int i;
6830   uint uargv, uarg;
6831 
6832   if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
6833     return -1;
6834   }
6835   memset(argv, 0, sizeof(argv));
6836   for(i=0;; i++){
6837     if(i >= NELEM(argv))
6838       return -1;
6839     if(fetchint(uargv+4*i, (int*)&uarg) < 0)
6840       return -1;
6841     if(uarg == 0){
6842       argv[i] = 0;
6843       break;
6844     }
6845     if(fetchstr(uarg, &argv[i]) < 0)
6846       return -1;
6847   }
6848   return exec(path, argv);
6849 }
6850 int
6851 sys_pipe(void)
6852 {
6853   int *fd;
6854   struct file *rf, *wf;
6855   int fd0, fd1;
6856 
6857   if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
6858     return -1;
6859   if(pipealloc(&rf, &wf) < 0)
6860     return -1;
6861   fd0 = -1;
6862   if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
6863     if(fd0 >= 0)
6864       myproc()->ofile[fd0] = 0;
6865     fileclose(rf);
6866     fileclose(wf);
6867     return -1;
6868   }
6869   fd[0] = fd0;
6870   fd[1] = fd1;
6871   return 0;
6872 }
6873 
6874 
6875 
6876 
6877 
6878 
6879 
6880 
6881 
6882 
6883 
6884 
6885 
6886 
6887 
6888 
6889 
6890 
6891 
6892 
6893 
6894 
6895 
6896 
6897 
6898 
6899 
