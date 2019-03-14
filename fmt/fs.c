5250 // File system implementation.  Five layers:
5251 //   + Blocks: allocator for raw disk blocks.
5252 //   + Log: crash recovery for multi-step updates.
5253 //   + Files: inode allocator, reading, writing, metadata.
5254 //   + Directories: inode with special contents (list of other inodes!)
5255 //   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
5256 //
5257 // This file contains the low-level file system manipulation
5258 // routines.  The (higher-level) system call implementations
5259 // are in sysfile.c.
5260 
5261 #include "types.h"
5262 #include "defs.h"
5263 #include "param.h"
5264 #include "stat.h"
5265 #include "mmu.h"
5266 #include "proc.h"
5267 #include "spinlock.h"
5268 #include "sleeplock.h"
5269 #include "fs.h"
5270 #include "buf.h"
5271 #include "file.h"
5272 
5273 #define min(a, b) ((a) < (b) ? (a) : (b))
5274 static void itrunc(struct inode*);
5275 // there should be one superblock per disk device, but we run with
5276 // only one device
5277 struct superblock sb;
5278 
5279 // Read the super block.
5280 void
5281 readsb(int dev, struct superblock *sb)
5282 {
5283   struct buf *bp;
5284 
5285   bp = bread(dev, 1);
5286   memmove(sb, bp->data, sizeof(*sb));
5287   brelse(bp);
5288 }
5289 
5290 
5291 
5292 
5293 
5294 
5295 
5296 
5297 
5298 
5299 
5300 // Zero a block.
5301 static void
5302 bzero(int dev, int bno)
5303 {
5304   struct buf *bp;
5305 
5306   bp = bread(dev, bno);
5307   memset(bp->data, 0, BSIZE);
5308   log_write(bp);
5309   brelse(bp);
5310 }
5311 
5312 // Blocks.
5313 
5314 // Allocate a zeroed disk block.
5315 static uint
5316 balloc(uint dev)
5317 {
5318   int b, bi, m;
5319   struct buf *bp;
5320 
5321   bp = 0;
5322   for(b = 0; b < sb.size; b += BPB){
5323     bp = bread(dev, BBLOCK(b, sb));
5324     for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
5325       m = 1 << (bi % 8);
5326       if((bp->data[bi/8] & m) == 0){  // Is block free?
5327         bp->data[bi/8] |= m;  // Mark block in use.
5328         log_write(bp);
5329         brelse(bp);
5330         bzero(dev, b + bi);
5331         return b + bi;
5332       }
5333     }
5334     brelse(bp);
5335   }
5336   panic("balloc: out of blocks");
5337 }
5338 
5339 
5340 
5341 
5342 
5343 
5344 
5345 
5346 
5347 
5348 
5349 
5350 // Free a disk block.
5351 static void
5352 bfree(int dev, uint b)
5353 {
5354   struct buf *bp;
5355   int bi, m;
5356 
5357   readsb(dev, &sb);
5358   bp = bread(dev, BBLOCK(b, sb));
5359   bi = b % BPB;
5360   m = 1 << (bi % 8);
5361   if((bp->data[bi/8] & m) == 0)
5362     panic("freeing free block");
5363   bp->data[bi/8] &= ~m;
5364   log_write(bp);
5365   brelse(bp);
5366 }
5367 
5368 // Inodes.
5369 //
5370 // An inode describes a single unnamed file.
5371 // The inode disk structure holds metadata: the file's type,
5372 // its size, the number of links referring to it, and the
5373 // list of blocks holding the file's content.
5374 //
5375 // The inodes are laid out sequentially on disk at
5376 // sb.startinode. Each inode has a number, indicating its
5377 // position on the disk.
5378 //
5379 // The kernel keeps a cache of in-use inodes in memory
5380 // to provide a place for synchronizing access
5381 // to inodes used by multiple processes. The cached
5382 // inodes include book-keeping information that is
5383 // not stored on disk: ip->ref and ip->valid.
5384 //
5385 // An inode and its in-memory representation go through a
5386 // sequence of states before they can be used by the
5387 // rest of the file system code.
5388 //
5389 // * Allocation: an inode is allocated if its type (on disk)
5390 //   is non-zero. ialloc() allocates, and iput() frees if
5391 //   the reference and link counts have fallen to zero.
5392 //
5393 // * Referencing in cache: an entry in the inode cache
5394 //   is free if ip->ref is zero. Otherwise ip->ref tracks
5395 //   the number of in-memory pointers to the entry (open
5396 //   files and current directories). iget() finds or
5397 //   creates a cache entry and increments its ref; iput()
5398 //   decrements ref.
5399 //
5400 // * Valid: the information (type, size, &c) in an inode
5401 //   cache entry is only correct when ip->valid is 1.
5402 //   ilock() reads the inode from
5403 //   the disk and sets ip->valid, while iput() clears
5404 //   ip->valid if ip->ref has fallen to zero.
5405 //
5406 // * Locked: file system code may only examine and modify
5407 //   the information in an inode and its content if it
5408 //   has first locked the inode.
5409 //
5410 // Thus a typical sequence is:
5411 //   ip = iget(dev, inum)
5412 //   ilock(ip)
5413 //   ... examine and modify ip->xxx ...
5414 //   iunlock(ip)
5415 //   iput(ip)
5416 //
5417 // ilock() is separate from iget() so that system calls can
5418 // get a long-term reference to an inode (as for an open file)
5419 // and only lock it for short periods (e.g., in read()).
5420 // The separation also helps avoid deadlock and races during
5421 // pathname lookup. iget() increments ip->ref so that the inode
5422 // stays cached and pointers to it remain valid.
5423 //
5424 // Many internal file system functions expect the caller to
5425 // have locked the inodes involved; this lets callers create
5426 // multi-step atomic operations.
5427 //
5428 // The icache.lock spin-lock protects the allocation of icache
5429 // entries. Since ip->ref indicates whether an entry is free,
5430 // and ip->dev and ip->inum indicate which i-node an entry
5431 // holds, one must hold icache.lock while using any of those fields.
5432 //
5433 // An ip->lock sleep-lock protects all ip-> fields other than ref,
5434 // dev, and inum.  One must hold ip->lock in order to
5435 // read or write that inode's ip->valid, ip->size, ip->type, &c.
5436 
5437 struct {
5438   struct spinlock lock;
5439   struct inode inode[NINODE];
5440 } icache;
5441 
5442 void
5443 iinit(int dev)
5444 {
5445   int i = 0;
5446 
5447   initlock(&icache.lock, "icache");
5448   for(i = 0; i < NINODE; i++) {
5449     initsleeplock(&icache.inode[i].lock, "inode");
5450   }
5451 
5452   readsb(dev, &sb);
5453   cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d\
5454  inodestart %d bmap start %d\n", sb.size, sb.nblocks,
5455           sb.ninodes, sb.nlog, sb.logstart, sb.inodestart,
5456           sb.bmapstart);
5457 }
5458 
5459 static struct inode* iget(uint dev, uint inum);
5460 
5461 
5462 
5463 
5464 
5465 
5466 
5467 
5468 
5469 
5470 
5471 
5472 
5473 
5474 
5475 
5476 
5477 
5478 
5479 
5480 
5481 
5482 
5483 
5484 
5485 
5486 
5487 
5488 
5489 
5490 
5491 
5492 
5493 
5494 
5495 
5496 
5497 
5498 
5499 
5500 // Allocate an inode on device dev.
5501 // Mark it as allocated by  giving it type type.
5502 // Returns an unlocked but allocated and referenced inode.
5503 struct inode*
5504 ialloc(uint dev, short type)
5505 {
5506   int inum;
5507   struct buf *bp;
5508   struct dinode *dip;
5509 
5510   for(inum = 1; inum < sb.ninodes; inum++){
5511     bp = bread(dev, IBLOCK(inum, sb));
5512     dip = (struct dinode*)bp->data + inum%IPB;
5513     if(dip->type == 0){  // a free inode
5514       memset(dip, 0, sizeof(*dip));
5515       dip->type = type;
5516       log_write(bp);   // mark it allocated on the disk
5517       brelse(bp);
5518       return iget(dev, inum);
5519     }
5520     brelse(bp);
5521   }
5522   panic("ialloc: no inodes");
5523 }
5524 
5525 // Copy a modified in-memory inode to disk.
5526 // Must be called after every change to an ip->xxx field
5527 // that lives on disk, since i-node cache is write-through.
5528 // Caller must hold ip->lock.
5529 void
5530 iupdate(struct inode *ip)
5531 {
5532   struct buf *bp;
5533   struct dinode *dip;
5534 
5535   bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5536   dip = (struct dinode*)bp->data + ip->inum%IPB;
5537   dip->type = ip->type;
5538   dip->major = ip->major;
5539   dip->minor = ip->minor;
5540   dip->nlink = ip->nlink;
5541   dip->size = ip->size;
5542   memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
5543   log_write(bp);
5544   brelse(bp);
5545 }
5546 
5547 
5548 
5549 
5550 // Find the inode with number inum on device dev
5551 // and return the in-memory copy. Does not lock
5552 // the inode and does not read it from disk.
5553 static struct inode*
5554 iget(uint dev, uint inum)
5555 {
5556   struct inode *ip, *empty;
5557 
5558   acquire(&icache.lock);
5559 
5560   // Is the inode already cached?
5561   empty = 0;
5562   for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
5563     if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
5564       ip->ref++;
5565       release(&icache.lock);
5566       return ip;
5567     }
5568     if(empty == 0 && ip->ref == 0)    // Remember empty slot.
5569       empty = ip;
5570   }
5571 
5572   // Recycle an inode cache entry.
5573   if(empty == 0)
5574     panic("iget: no inodes");
5575 
5576   ip = empty;
5577   ip->dev = dev;
5578   ip->inum = inum;
5579   ip->ref = 1;
5580   ip->valid = 0;
5581   release(&icache.lock);
5582 
5583   return ip;
5584 }
5585 
5586 // Increment reference count for ip.
5587 // Returns ip to enable ip = idup(ip1) idiom.
5588 struct inode*
5589 idup(struct inode *ip)
5590 {
5591   acquire(&icache.lock);
5592   ip->ref++;
5593   release(&icache.lock);
5594   return ip;
5595 }
5596 
5597 
5598 
5599 
5600 // Lock the given inode.
5601 // Reads the inode from disk if necessary.
5602 void
5603 ilock(struct inode *ip)
5604 {
5605   struct buf *bp;
5606   struct dinode *dip;
5607 
5608   if(ip == 0 || ip->ref < 1)
5609     panic("ilock");
5610 
5611   acquiresleep(&ip->lock);
5612 
5613   if(ip->valid == 0){
5614     bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5615     dip = (struct dinode*)bp->data + ip->inum%IPB;
5616     ip->type = dip->type;
5617     ip->major = dip->major;
5618     ip->minor = dip->minor;
5619     ip->nlink = dip->nlink;
5620     ip->size = dip->size;
5621     memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
5622     brelse(bp);
5623     ip->valid = 1;
5624     if(ip->type == 0)
5625       panic("ilock: no type");
5626   }
5627 }
5628 
5629 // Unlock the given inode.
5630 void
5631 iunlock(struct inode *ip)
5632 {
5633   if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
5634     panic("iunlock");
5635 
5636   releasesleep(&ip->lock);
5637 }
5638 
5639 
5640 
5641 
5642 
5643 
5644 
5645 
5646 
5647 
5648 
5649 
5650 // Drop a reference to an in-memory inode.
5651 // If that was the last reference, the inode cache entry can
5652 // be recycled.
5653 // If that was the last reference and the inode has no links
5654 // to it, free the inode (and its content) on disk.
5655 // All calls to iput() must be inside a transaction in
5656 // case it has to free the inode.
5657 void
5658 iput(struct inode *ip)
5659 {
5660   acquiresleep(&ip->lock);
5661   if(ip->valid && ip->nlink == 0){
5662     acquire(&icache.lock);
5663     int r = ip->ref;
5664     release(&icache.lock);
5665     if(r == 1){
5666       // inode has no links and no other references: truncate and free.
5667       itrunc(ip);
5668       ip->type = 0;
5669       iupdate(ip);
5670       ip->valid = 0;
5671     }
5672   }
5673   releasesleep(&ip->lock);
5674 
5675   acquire(&icache.lock);
5676   ip->ref--;
5677   release(&icache.lock);
5678 }
5679 
5680 // Common idiom: unlock, then put.
5681 void
5682 iunlockput(struct inode *ip)
5683 {
5684   iunlock(ip);
5685   iput(ip);
5686 }
5687 
5688 
5689 
5690 
5691 
5692 
5693 
5694 
5695 
5696 
5697 
5698 
5699 
5700 // Inode content
5701 //
5702 // The content (data) associated with each inode is stored
5703 // in blocks on the disk. The first NDIRECT block numbers
5704 // are listed in ip->addrs[].  The next NINDIRECT blocks are
5705 // listed in block ip->addrs[NDIRECT].
5706 
5707 // Return the disk block address of the nth block in inode ip.
5708 // If there is no such block, bmap allocates one.
5709 static uint
5710 bmap(struct inode *ip, uint bn)
5711 {
5712   uint addr, *a;
5713   struct buf *bp;
5714 
5715   if(bn < NDIRECT){
5716     if((addr = ip->addrs[bn]) == 0)
5717       ip->addrs[bn] = addr = balloc(ip->dev);
5718     return addr;
5719   }
5720   bn -= NDIRECT;
5721 
5722   if(bn < NINDIRECT){
5723     // Load indirect block, allocating if necessary.
5724     if((addr = ip->addrs[NDIRECT]) == 0)
5725       ip->addrs[NDIRECT] = addr = balloc(ip->dev);
5726     bp = bread(ip->dev, addr);
5727     a = (uint*)bp->data;
5728     if((addr = a[bn]) == 0){
5729       a[bn] = addr = balloc(ip->dev);
5730       log_write(bp);
5731     }
5732     brelse(bp);
5733     return addr;
5734   }
5735 
5736   panic("bmap: out of range");
5737 }
5738 
5739 
5740 
5741 
5742 
5743 
5744 
5745 
5746 
5747 
5748 
5749 
5750 // Truncate inode (discard contents).
5751 // Only called when the inode has no links
5752 // to it (no directory entries referring to it)
5753 // and has no in-memory reference to it (is
5754 // not an open file or current directory).
5755 static void
5756 itrunc(struct inode *ip)
5757 {
5758   int i, j;
5759   struct buf *bp;
5760   uint *a;
5761 
5762   for(i = 0; i < NDIRECT; i++){
5763     if(ip->addrs[i]){
5764       bfree(ip->dev, ip->addrs[i]);
5765       ip->addrs[i] = 0;
5766     }
5767   }
5768 
5769   if(ip->addrs[NDIRECT]){
5770     bp = bread(ip->dev, ip->addrs[NDIRECT]);
5771     a = (uint*)bp->data;
5772     for(j = 0; j < NINDIRECT; j++){
5773       if(a[j])
5774         bfree(ip->dev, a[j]);
5775     }
5776     brelse(bp);
5777     bfree(ip->dev, ip->addrs[NDIRECT]);
5778     ip->addrs[NDIRECT] = 0;
5779   }
5780 
5781   ip->size = 0;
5782   iupdate(ip);
5783 }
5784 
5785 // Copy stat information from inode.
5786 // Caller must hold ip->lock.
5787 void
5788 stati(struct inode *ip, struct stat *st)
5789 {
5790   st->dev = ip->dev;
5791   st->ino = ip->inum;
5792   st->type = ip->type;
5793   st->nlink = ip->nlink;
5794   st->size = ip->size;
5795 }
5796 
5797 
5798 
5799 
5800 // Read data from inode.
5801 // Caller must hold ip->lock.
5802 int
5803 readi(struct inode *ip, char *dst, uint off, uint n)
5804 {
5805   uint tot, m;
5806   struct buf *bp;
5807 
5808   if(ip->type == T_DEV){
5809     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
5810       return -1;
5811     return devsw[ip->major].read(ip, dst, n);
5812   }
5813 
5814   if(off > ip->size || off + n < off)
5815     return -1;
5816   if(off + n > ip->size)
5817     n = ip->size - off;
5818 
5819   for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
5820     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5821     m = min(n - tot, BSIZE - off%BSIZE);
5822     memmove(dst, bp->data + off%BSIZE, m);
5823     brelse(bp);
5824   }
5825   return n;
5826 }
5827 
5828 
5829 
5830 
5831 
5832 
5833 
5834 
5835 
5836 
5837 
5838 
5839 
5840 
5841 
5842 
5843 
5844 
5845 
5846 
5847 
5848 
5849 
5850 // Write data to inode.
5851 // Caller must hold ip->lock.
5852 int
5853 writei(struct inode *ip, char *src, uint off, uint n)
5854 {
5855   uint tot, m;
5856   struct buf *bp;
5857 
5858   if(ip->type == T_DEV){
5859     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
5860       return -1;
5861     return devsw[ip->major].write(ip, src, n);
5862   }
5863 
5864   if(off > ip->size || off + n < off)
5865     return -1;
5866   if(off + n > MAXFILE*BSIZE)
5867     return -1;
5868 
5869   for(tot=0; tot<n; tot+=m, off+=m, src+=m){
5870     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5871     m = min(n - tot, BSIZE - off%BSIZE);
5872     memmove(bp->data + off%BSIZE, src, m);
5873     log_write(bp);
5874     brelse(bp);
5875   }
5876 
5877   if(n > 0 && off > ip->size){
5878     ip->size = off;
5879     iupdate(ip);
5880   }
5881   return n;
5882 }
5883 
5884 
5885 
5886 
5887 
5888 
5889 
5890 
5891 
5892 
5893 
5894 
5895 
5896 
5897 
5898 
5899 
5900 // Directories
5901 
5902 int
5903 namecmp(const char *s, const char *t)
5904 {
5905   return strncmp(s, t, DIRSIZ);
5906 }
5907 
5908 // Look for a directory entry in a directory.
5909 // If found, set *poff to byte offset of entry.
5910 struct inode*
5911 dirlookup(struct inode *dp, char *name, uint *poff)
5912 {
5913   uint off, inum;
5914   struct dirent de;
5915 
5916   if(dp->type != T_DIR)
5917     panic("dirlookup not DIR");
5918 
5919   for(off = 0; off < dp->size; off += sizeof(de)){
5920     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5921       panic("dirlookup read");
5922     if(de.inum == 0)
5923       continue;
5924     if(namecmp(name, de.name) == 0){
5925       // entry matches path element
5926       if(poff)
5927         *poff = off;
5928       inum = de.inum;
5929       return iget(dp->dev, inum);
5930     }
5931   }
5932 
5933   return 0;
5934 }
5935 
5936 
5937 
5938 
5939 
5940 
5941 
5942 
5943 
5944 
5945 
5946 
5947 
5948 
5949 
5950 // Write a new directory entry (name, inum) into the directory dp.
5951 int
5952 dirlink(struct inode *dp, char *name, uint inum)
5953 {
5954   int off;
5955   struct dirent de;
5956   struct inode *ip;
5957 
5958   // Check that name is not present.
5959   if((ip = dirlookup(dp, name, 0)) != 0){
5960     iput(ip);
5961     return -1;
5962   }
5963 
5964   // Look for an empty dirent.
5965   for(off = 0; off < dp->size; off += sizeof(de)){
5966     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5967       panic("dirlink read");
5968     if(de.inum == 0)
5969       break;
5970   }
5971 
5972   strncpy(de.name, name, DIRSIZ);
5973   de.inum = inum;
5974   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5975     panic("dirlink");
5976 
5977   return 0;
5978 }
5979 
5980 
5981 
5982 
5983 
5984 
5985 
5986 
5987 
5988 
5989 
5990 
5991 
5992 
5993 
5994 
5995 
5996 
5997 
5998 
5999 
6000 // Paths
6001 
6002 // Copy the next path element from path into name.
6003 // Return a pointer to the element following the copied one.
6004 // The returned path has no leading slashes,
6005 // so the caller can check *path=='\0' to see if the name is the last one.
6006 // If no name to remove, return 0.
6007 //
6008 // Examples:
6009 //   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
6010 //   skipelem("///a//bb", name) = "bb", setting name = "a"
6011 //   skipelem("a", name) = "", setting name = "a"
6012 //   skipelem("", name) = skipelem("////", name) = 0
6013 //
6014 static char*
6015 skipelem(char *path, char *name)
6016 {
6017   char *s;
6018   int len;
6019 
6020   while(*path == '/')
6021     path++;
6022   if(*path == 0)
6023     return 0;
6024   s = path;
6025   while(*path != '/' && *path != 0)
6026     path++;
6027   len = path - s;
6028   if(len >= DIRSIZ)
6029     memmove(name, s, DIRSIZ);
6030   else {
6031     memmove(name, s, len);
6032     name[len] = 0;
6033   }
6034   while(*path == '/')
6035     path++;
6036   return path;
6037 }
6038 
6039 
6040 
6041 
6042 
6043 
6044 
6045 
6046 
6047 
6048 
6049 
6050 // Look up and return the inode for a path name.
6051 // If parent != 0, return the inode for the parent and copy the final
6052 // path element into name, which must have room for DIRSIZ bytes.
6053 // Must be called inside a transaction since it calls iput().
6054 static struct inode*
6055 namex(char *path, int nameiparent, char *name)
6056 {
6057   struct inode *ip, *next;
6058 
6059   if(*path == '/')
6060     ip = iget(ROOTDEV, ROOTINO);
6061   else
6062     ip = idup(myproc()->cwd);
6063 
6064   while((path = skipelem(path, name)) != 0){
6065     ilock(ip);
6066     if(ip->type != T_DIR){
6067       iunlockput(ip);
6068       return 0;
6069     }
6070     if(nameiparent && *path == '\0'){
6071       // Stop one level early.
6072       iunlock(ip);
6073       return ip;
6074     }
6075     if((next = dirlookup(ip, name, 0)) == 0){
6076       iunlockput(ip);
6077       return 0;
6078     }
6079     iunlockput(ip);
6080     ip = next;
6081   }
6082   if(nameiparent){
6083     iput(ip);
6084     return 0;
6085   }
6086   return ip;
6087 }
6088 
6089 struct inode*
6090 namei(char *path)
6091 {
6092   char name[DIRSIZ];
6093   return namex(path, 0, name);
6094 }
6095 
6096 
6097 
6098 
6099 
6100 struct inode*
6101 nameiparent(char *path, char *name)
6102 {
6103   return namex(path, 1, name);
6104 }
6105 
6106 
6107 
6108 
6109 
6110 
6111 
6112 
6113 
6114 
6115 
6116 
6117 
6118 
6119 
6120 
6121 
6122 
6123 
6124 
6125 
6126 
6127 
6128 
6129 
6130 
6131 
6132 
6133 
6134 
6135 
6136 
6137 
6138 
6139 
6140 
6141 
6142 
6143 
6144 
6145 
6146 
6147 
6148 
6149 
