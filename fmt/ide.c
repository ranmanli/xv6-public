4500 // Simple PIO-based (non-DMA) IDE driver code.
4501 
4502 #include "types.h"
4503 #include "defs.h"
4504 #include "param.h"
4505 #include "memlayout.h"
4506 #include "mmu.h"
4507 #include "proc.h"
4508 #include "x86.h"
4509 #include "traps.h"
4510 #include "spinlock.h"
4511 #include "sleeplock.h"
4512 #include "fs.h"
4513 #include "buf.h"
4514 
4515 #define SECTOR_SIZE   512
4516 #define IDE_BSY       0x80
4517 #define IDE_DRDY      0x40
4518 #define IDE_DF        0x20
4519 #define IDE_ERR       0x01
4520 
4521 #define IDE_CMD_READ  0x20
4522 #define IDE_CMD_WRITE 0x30
4523 #define IDE_CMD_RDMUL 0xc4
4524 #define IDE_CMD_WRMUL 0xc5
4525 
4526 // idequeue points to the buf now being read/written to the disk.
4527 // idequeue->qnext points to the next buf to be processed.
4528 // You must hold idelock while manipulating queue.
4529 
4530 static struct spinlock idelock;
4531 static struct buf *idequeue;
4532 
4533 static int havedisk1;
4534 static void idestart(struct buf*);
4535 
4536 // Wait for IDE disk to become ready.
4537 static int
4538 idewait(int checkerr)
4539 {
4540   int r;
4541 
4542   while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
4543     ;
4544   if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
4545     return -1;
4546   return 0;
4547 }
4548 
4549 
4550 void
4551 ideinit(void)
4552 {
4553   int i;
4554 
4555   initlock(&idelock, "ide");
4556   ioapicenable(IRQ_IDE, ncpu - 1);
4557   idewait(0);
4558 
4559   // Check if disk 1 is present
4560   outb(0x1f6, 0xe0 | (1<<4));
4561   for(i=0; i<1000; i++){
4562     if(inb(0x1f7) != 0){
4563       havedisk1 = 1;
4564       break;
4565     }
4566   }
4567 
4568   // Switch back to disk 0.
4569   outb(0x1f6, 0xe0 | (0<<4));
4570 }
4571 
4572 // Start the request for b.  Caller must hold idelock.
4573 static void
4574 idestart(struct buf *b)
4575 {
4576   if(b == 0)
4577     panic("idestart");
4578   if(b->blockno >= FSSIZE)
4579     panic("incorrect blockno");
4580   int sector_per_block =  BSIZE/SECTOR_SIZE;
4581   int sector = b->blockno * sector_per_block;
4582   int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
4583   int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;
4584 
4585   if (sector_per_block > 7) panic("idestart");
4586 
4587   idewait(0);
4588   outb(0x3f6, 0);  // generate interrupt
4589   outb(0x1f2, sector_per_block);  // number of sectors
4590   outb(0x1f3, sector & 0xff);
4591   outb(0x1f4, (sector >> 8) & 0xff);
4592   outb(0x1f5, (sector >> 16) & 0xff);
4593   outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
4594   if(b->flags & B_DIRTY){
4595     outb(0x1f7, write_cmd);
4596     outsl(0x1f0, b->data, BSIZE/4);
4597   } else {
4598     outb(0x1f7, read_cmd);
4599   }
4600 }
4601 
4602 // Interrupt handler.
4603 void
4604 ideintr(void)
4605 {
4606   struct buf *b;
4607 
4608   // First queued buffer is the active request.
4609   acquire(&idelock);
4610 
4611   if((b = idequeue) == 0){
4612     release(&idelock);
4613     return;
4614   }
4615   idequeue = b->qnext;
4616 
4617   // Read data if needed.
4618   if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
4619     insl(0x1f0, b->data, BSIZE/4);
4620 
4621   // Wake process waiting for this buf.
4622   b->flags |= B_VALID;
4623   b->flags &= ~B_DIRTY;
4624   wakeup(b);
4625 
4626   // Start disk on next buf in queue.
4627   if(idequeue != 0)
4628     idestart(idequeue);
4629 
4630   release(&idelock);
4631 }
4632 
4633 
4634 
4635 
4636 
4637 
4638 
4639 
4640 
4641 
4642 
4643 
4644 
4645 
4646 
4647 
4648 
4649 
4650 // Sync buf with disk.
4651 // If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
4652 // Else if B_VALID is not set, read buf from disk, set B_VALID.
4653 void
4654 iderw(struct buf *b)
4655 {
4656   struct buf **pp;
4657 
4658   if(!holdingsleep(&b->lock))
4659     panic("iderw: buf not locked");
4660   if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
4661     panic("iderw: nothing to do");
4662   if(b->dev != 0 && !havedisk1)
4663     panic("iderw: ide disk 1 not present");
4664 
4665   acquire(&idelock);  //DOC:acquire-lock
4666 
4667   // Append b to idequeue.
4668   b->qnext = 0;
4669   for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
4670     ;
4671   *pp = b;
4672 
4673   // Start disk if necessary.
4674   if(idequeue == b)
4675     idestart(b);
4676 
4677   // Wait for request to finish.
4678   while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
4679     sleep(b, &idelock);
4680   }
4681 
4682 
4683   release(&idelock);
4684 }
4685 
4686 
4687 
4688 
4689 
4690 
4691 
4692 
4693 
4694 
4695 
4696 
4697 
4698 
4699 
