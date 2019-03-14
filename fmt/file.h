4450 struct file {
4451   enum { FD_NONE, FD_PIPE, FD_INODE } type;
4452   int ref; // reference count
4453   char readable;
4454   char writable;
4455   struct pipe *pipe;
4456   struct inode *ip;
4457   uint off;
4458 };
4459 
4460 
4461 // in-memory copy of an inode
4462 struct inode {
4463   uint dev;           // Device number
4464   uint inum;          // Inode number
4465   int ref;            // Reference count
4466   struct sleeplock lock; // protects everything below here
4467   int valid;          // inode has been read from disk?
4468 
4469   short type;         // copy of disk inode
4470   short major;
4471   short minor;
4472   short nlink;
4473   uint size;
4474   uint addrs[NDIRECT+1];
4475 };
4476 
4477 // table mapping major device number to
4478 // device functions
4479 struct devsw {
4480   int (*read)(struct inode*, char*, int);
4481   int (*write)(struct inode*, char*, int);
4482 };
4483 
4484 extern struct devsw devsw[];
4485 
4486 #define CONSOLE 1
4487 
4488 
4489 
4490 
4491 
4492 
4493 
4494 
4495 
4496 
4497 
4498 
4499 
