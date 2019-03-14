4350 // On-disk file system format.
4351 // Both the kernel and user programs use this header file.
4352 
4353 
4354 #define ROOTINO 1  // root i-number
4355 #define BSIZE 512  // block size
4356 
4357 // Disk layout:
4358 // [ boot block | super block | log | inode blocks |
4359 //                                          free bit map | data blocks]
4360 //
4361 // mkfs computes the super block and builds an initial file system. The
4362 // super block describes the disk layout:
4363 struct superblock {
4364   uint size;         // Size of file system image (blocks)
4365   uint nblocks;      // Number of data blocks
4366   uint ninodes;      // Number of inodes.
4367   uint nlog;         // Number of log blocks
4368   uint logstart;     // Block number of first log block
4369   uint inodestart;   // Block number of first inode block
4370   uint bmapstart;    // Block number of first free map block
4371 };
4372 
4373 #define NDIRECT 12
4374 #define NINDIRECT (BSIZE / sizeof(uint))
4375 #define MAXFILE (NDIRECT + NINDIRECT)
4376 
4377 // On-disk inode structure
4378 struct dinode {
4379   short type;           // File type
4380   short major;          // Major device number (T_DEV only)
4381   short minor;          // Minor device number (T_DEV only)
4382   short nlink;          // Number of links to inode in file system
4383   uint size;            // Size of file (bytes)
4384   uint addrs[NDIRECT+1];   // Data block addresses
4385 };
4386 
4387 
4388 
4389 
4390 
4391 
4392 
4393 
4394 
4395 
4396 
4397 
4398 
4399 
4400 // Inodes per block.
4401 #define IPB           (BSIZE / sizeof(struct dinode))
4402 
4403 // Block containing inode i
4404 #define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
4405 
4406 // Bitmap bits per block
4407 #define BPB           (BSIZE*8)
4408 
4409 // Block of free map containing bit for block b
4410 #define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
4411 
4412 // Directory is a file containing a sequence of dirent structures.
4413 #define DIRSIZ 14
4414 
4415 struct dirent {
4416   ushort inum;
4417   char name[DIRSIZ];
4418 };
4419 
4420 
4421 
4422 
4423 
4424 
4425 
4426 
4427 
4428 
4429 
4430 
4431 
4432 
4433 
4434 
4435 
4436 
4437 
4438 
4439 
4440 
4441 
4442 
4443 
4444 
4445 
4446 
4447 
4448 
4449 
