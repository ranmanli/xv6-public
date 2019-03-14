4700 // Buffer cache.
4701 //
4702 // The buffer cache is a linked list of buf structures holding
4703 // cached copies of disk block contents.  Caching disk blocks
4704 // in memory reduces the number of disk reads and also provides
4705 // a synchronization point for disk blocks used by multiple processes.
4706 //
4707 // Interface:
4708 // * To get a buffer for a particular disk block, call bread.
4709 // * After changing buffer data, call bwrite to write it to disk.
4710 // * When done with the buffer, call brelse.
4711 // * Do not use the buffer after calling brelse.
4712 // * Only one process at a time can use a buffer,
4713 //     so do not keep them longer than necessary.
4714 //
4715 // The implementation uses two state flags internally:
4716 // * B_VALID: the buffer data has been read from the disk.
4717 // * B_DIRTY: the buffer data has been modified
4718 //     and needs to be written to disk.
4719 
4720 #include "types.h"
4721 #include "defs.h"
4722 #include "param.h"
4723 #include "spinlock.h"
4724 #include "sleeplock.h"
4725 #include "fs.h"
4726 #include "buf.h"
4727 
4728 struct {
4729   struct spinlock lock;
4730   struct buf buf[NBUF];
4731 
4732   // Linked list of all buffers, through prev/next.
4733   // head.next is most recently used.
4734   struct buf head;
4735 } bcache;
4736 
4737 void
4738 binit(void)
4739 {
4740   struct buf *b;
4741 
4742   initlock(&bcache.lock, "bcache");
4743 
4744 
4745 
4746 
4747 
4748 
4749 
4750   // Create linked list of buffers
4751   bcache.head.prev = &bcache.head;
4752   bcache.head.next = &bcache.head;
4753   for(b = bcache.buf; b < bcache.buf+NBUF; b++){
4754     b->next = bcache.head.next;
4755     b->prev = &bcache.head;
4756     initsleeplock(&b->lock, "buffer");
4757     bcache.head.next->prev = b;
4758     bcache.head.next = b;
4759   }
4760 }
4761 
4762 // Look through buffer cache for block on device dev.
4763 // If not found, allocate a buffer.
4764 // In either case, return locked buffer.
4765 static struct buf*
4766 bget(uint dev, uint blockno)
4767 {
4768   struct buf *b;
4769 
4770   acquire(&bcache.lock);
4771 
4772   // Is the block already cached?
4773   for(b = bcache.head.next; b != &bcache.head; b = b->next){
4774     if(b->dev == dev && b->blockno == blockno){
4775       b->refcnt++;
4776       release(&bcache.lock);
4777       acquiresleep(&b->lock);
4778       return b;
4779     }
4780   }
4781 
4782   // Not cached; recycle an unused buffer.
4783   // Even if refcnt==0, B_DIRTY indicates a buffer is in use
4784   // because log.c has modified it but not yet committed it.
4785   for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
4786     if(b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
4787       b->dev = dev;
4788       b->blockno = blockno;
4789       b->flags = 0;
4790       b->refcnt = 1;
4791       release(&bcache.lock);
4792       acquiresleep(&b->lock);
4793       return b;
4794     }
4795   }
4796   panic("bget: no buffers");
4797 }
4798 
4799 
4800 // Return a locked buf with the contents of the indicated block.
4801 struct buf*
4802 bread(uint dev, uint blockno)
4803 {
4804   struct buf *b;
4805 
4806   b = bget(dev, blockno);
4807   if((b->flags & B_VALID) == 0) {
4808     iderw(b);
4809   }
4810   return b;
4811 }
4812 
4813 // Write b's contents to disk.  Must be locked.
4814 void
4815 bwrite(struct buf *b)
4816 {
4817   if(!holdingsleep(&b->lock))
4818     panic("bwrite");
4819   b->flags |= B_DIRTY;
4820   iderw(b);
4821 }
4822 
4823 // Release a locked buffer.
4824 // Move to the head of the MRU list.
4825 void
4826 brelse(struct buf *b)
4827 {
4828   if(!holdingsleep(&b->lock))
4829     panic("brelse");
4830 
4831   releasesleep(&b->lock);
4832 
4833   acquire(&bcache.lock);
4834   b->refcnt--;
4835   if (b->refcnt == 0) {
4836     // no one is waiting for it.
4837     b->next->prev = b->prev;
4838     b->prev->next = b->next;
4839     b->next = bcache.head.next;
4840     b->prev = &bcache.head;
4841     bcache.head.next->prev = b;
4842     bcache.head.next = b;
4843   }
4844 
4845   release(&bcache.lock);
4846 }
4847 
4848 
4849 
4850 // Blank page.
4851 
4852 
4853 
4854 
4855 
4856 
4857 
4858 
4859 
4860 
4861 
4862 
4863 
4864 
4865 
4866 
4867 
4868 
4869 
4870 
4871 
4872 
4873 
4874 
4875 
4876 
4877 
4878 
4879 
4880 
4881 
4882 
4883 
4884 
4885 
4886 
4887 
4888 
4889 
4890 
4891 
4892 
4893 
4894 
4895 
4896 
4897 
4898 
4899 
