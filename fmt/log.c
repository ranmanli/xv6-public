5000 #include "types.h"
5001 #include "defs.h"
5002 #include "param.h"
5003 #include "spinlock.h"
5004 #include "sleeplock.h"
5005 #include "fs.h"
5006 #include "buf.h"
5007 
5008 // Simple logging that allows concurrent FS system calls.
5009 //
5010 // A log transaction contains the updates of multiple FS system
5011 // calls. The logging system only commits when there are
5012 // no FS system calls active. Thus there is never
5013 // any reasoning required about whether a commit might
5014 // write an uncommitted system call's updates to disk.
5015 //
5016 // A system call should call begin_op()/end_op() to mark
5017 // its start and end. Usually begin_op() just increments
5018 // the count of in-progress FS system calls and returns.
5019 // But if it thinks the log is close to running out, it
5020 // sleeps until the last outstanding end_op() commits.
5021 //
5022 // The log is a physical re-do log containing disk blocks.
5023 // The on-disk log format:
5024 //   header block, containing block #s for block A, B, C, ...
5025 //   block A
5026 //   block B
5027 //   block C
5028 //   ...
5029 // Log appends are synchronous.
5030 
5031 // Contents of the header block, used for both the on-disk header block
5032 // and to keep track in memory of logged block# before commit.
5033 struct logheader {
5034   int n;
5035   int block[LOGSIZE];
5036 };
5037 
5038 struct log {
5039   struct spinlock lock;
5040   int start;
5041   int size;
5042   int outstanding; // how many FS sys calls are executing.
5043   int committing;  // in commit(), please wait.
5044   int dev;
5045   struct logheader lh;
5046 };
5047 
5048 
5049 
5050 struct log log;
5051 
5052 static void recover_from_log(void);
5053 static void commit();
5054 
5055 void
5056 initlog(int dev)
5057 {
5058   if (sizeof(struct logheader) >= BSIZE)
5059     panic("initlog: too big logheader");
5060 
5061   struct superblock sb;
5062   initlock(&log.lock, "log");
5063   readsb(dev, &sb);
5064   log.start = sb.logstart;
5065   log.size = sb.nlog;
5066   log.dev = dev;
5067   recover_from_log();
5068 }
5069 
5070 // Copy committed blocks from log to their home location
5071 static void
5072 install_trans(void)
5073 {
5074   int tail;
5075 
5076   for (tail = 0; tail < log.lh.n; tail++) {
5077     struct buf *lbuf = bread(log.dev, log.start+tail+1); // read log block
5078     struct buf *dbuf = bread(log.dev, log.lh.block[tail]); // read dst
5079     memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
5080     bwrite(dbuf);  // write dst to disk
5081     brelse(lbuf);
5082     brelse(dbuf);
5083   }
5084 }
5085 
5086 // Read the log header from disk into the in-memory log header
5087 static void
5088 read_head(void)
5089 {
5090   struct buf *buf = bread(log.dev, log.start);
5091   struct logheader *lh = (struct logheader *) (buf->data);
5092   int i;
5093   log.lh.n = lh->n;
5094   for (i = 0; i < log.lh.n; i++) {
5095     log.lh.block[i] = lh->block[i];
5096   }
5097   brelse(buf);
5098 }
5099 
5100 // Write in-memory log header to disk.
5101 // This is the true point at which the
5102 // current transaction commits.
5103 static void
5104 write_head(void)
5105 {
5106   struct buf *buf = bread(log.dev, log.start);
5107   struct logheader *hb = (struct logheader *) (buf->data);
5108   int i;
5109   hb->n = log.lh.n;
5110   for (i = 0; i < log.lh.n; i++) {
5111     hb->block[i] = log.lh.block[i];
5112   }
5113   bwrite(buf);
5114   brelse(buf);
5115 }
5116 
5117 static void
5118 recover_from_log(void)
5119 {
5120   read_head();
5121   install_trans(); // if committed, copy from log to disk
5122   log.lh.n = 0;
5123   write_head(); // clear the log
5124 }
5125 
5126 // called at the start of each FS system call.
5127 void
5128 begin_op(void)
5129 {
5130   acquire(&log.lock);
5131   while(1){
5132     if(log.committing){
5133       sleep(&log, &log.lock);
5134     } else if(log.lh.n + (log.outstanding+1)*MAXOPBLOCKS > LOGSIZE){
5135       // this op might exhaust log space; wait for commit.
5136       sleep(&log, &log.lock);
5137     } else {
5138       log.outstanding += 1;
5139       release(&log.lock);
5140       break;
5141     }
5142   }
5143 }
5144 
5145 
5146 
5147 
5148 
5149 
5150 // called at the end of each FS system call.
5151 // commits if this was the last outstanding operation.
5152 void
5153 end_op(void)
5154 {
5155   int do_commit = 0;
5156 
5157   acquire(&log.lock);
5158   log.outstanding -= 1;
5159   if(log.committing)
5160     panic("log.committing");
5161   if(log.outstanding == 0){
5162     do_commit = 1;
5163     log.committing = 1;
5164   } else {
5165     // begin_op() may be waiting for log space,
5166     // and decrementing log.outstanding has decreased
5167     // the amount of reserved space.
5168     wakeup(&log);
5169   }
5170   release(&log.lock);
5171 
5172   if(do_commit){
5173     // call commit w/o holding locks, since not allowed
5174     // to sleep with locks.
5175     commit();
5176     acquire(&log.lock);
5177     log.committing = 0;
5178     wakeup(&log);
5179     release(&log.lock);
5180   }
5181 }
5182 
5183 // Copy modified blocks from cache to log.
5184 static void
5185 write_log(void)
5186 {
5187   int tail;
5188 
5189   for (tail = 0; tail < log.lh.n; tail++) {
5190     struct buf *to = bread(log.dev, log.start+tail+1); // log block
5191     struct buf *from = bread(log.dev, log.lh.block[tail]); // cache block
5192     memmove(to->data, from->data, BSIZE);
5193     bwrite(to);  // write the log
5194     brelse(from);
5195     brelse(to);
5196   }
5197 }
5198 
5199 
5200 static void
5201 commit()
5202 {
5203   if (log.lh.n > 0) {
5204     write_log();     // Write modified blocks from cache to log
5205     write_head();    // Write header to disk -- the real commit
5206     install_trans(); // Now install writes to home locations
5207     log.lh.n = 0;
5208     write_head();    // Erase the transaction from the log
5209   }
5210 }
5211 
5212 // Caller has modified b->data and is done with the buffer.
5213 // Record the block number and pin in the cache with B_DIRTY.
5214 // commit()/write_log() will do the disk write.
5215 //
5216 // log_write() replaces bwrite(); a typical use is:
5217 //   bp = bread(...)
5218 //   modify bp->data[]
5219 //   log_write(bp)
5220 //   brelse(bp)
5221 void
5222 log_write(struct buf *b)
5223 {
5224   int i;
5225 
5226   if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)
5227     panic("too big a transaction");
5228   if (log.outstanding < 1)
5229     panic("log_write outside of trans");
5230 
5231   acquire(&log.lock);
5232   for (i = 0; i < log.lh.n; i++) {
5233     if (log.lh.block[i] == b->blockno)   // log absorbtion
5234       break;
5235   }
5236   log.lh.block[i] = b->blockno;
5237   if (i == log.lh.n)
5238     log.lh.n++;
5239   b->flags |= B_DIRTY; // prevent eviction
5240   release(&log.lock);
5241 }
5242 
5243 
5244 
5245 
5246 
5247 
5248 
5249 
