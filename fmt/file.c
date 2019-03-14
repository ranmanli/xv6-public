6150 //
6151 // File descriptors
6152 //
6153 
6154 #include "types.h"
6155 #include "defs.h"
6156 #include "param.h"
6157 #include "fs.h"
6158 #include "spinlock.h"
6159 #include "sleeplock.h"
6160 #include "file.h"
6161 
6162 struct devsw devsw[NDEV];
6163 struct {
6164   struct spinlock lock;
6165   struct file file[NFILE];
6166 } ftable;
6167 
6168 void
6169 fileinit(void)
6170 {
6171   initlock(&ftable.lock, "ftable");
6172 }
6173 
6174 // Allocate a file structure.
6175 struct file*
6176 filealloc(void)
6177 {
6178   struct file *f;
6179 
6180   acquire(&ftable.lock);
6181   for(f = ftable.file; f < ftable.file + NFILE; f++){
6182     if(f->ref == 0){
6183       f->ref = 1;
6184       release(&ftable.lock);
6185       return f;
6186     }
6187   }
6188   release(&ftable.lock);
6189   return 0;
6190 }
6191 
6192 
6193 
6194 
6195 
6196 
6197 
6198 
6199 
6200 // Increment ref count for file f.
6201 struct file*
6202 filedup(struct file *f)
6203 {
6204   acquire(&ftable.lock);
6205   if(f->ref < 1)
6206     panic("filedup");
6207   f->ref++;
6208   release(&ftable.lock);
6209   return f;
6210 }
6211 
6212 // Close file f.  (Decrement ref count, close when reaches 0.)
6213 void
6214 fileclose(struct file *f)
6215 {
6216   struct file ff;
6217 
6218   acquire(&ftable.lock);
6219   if(f->ref < 1)
6220     panic("fileclose");
6221   if(--f->ref > 0){
6222     release(&ftable.lock);
6223     return;
6224   }
6225   ff = *f;
6226   f->ref = 0;
6227   f->type = FD_NONE;
6228   release(&ftable.lock);
6229 
6230   if(ff.type == FD_PIPE)
6231     pipeclose(ff.pipe, ff.writable);
6232   else if(ff.type == FD_INODE){
6233     begin_op();
6234     iput(ff.ip);
6235     end_op();
6236   }
6237 }
6238 
6239 
6240 
6241 
6242 
6243 
6244 
6245 
6246 
6247 
6248 
6249 
6250 // Get metadata about file f.
6251 int
6252 filestat(struct file *f, struct stat *st)
6253 {
6254   if(f->type == FD_INODE){
6255     ilock(f->ip);
6256     stati(f->ip, st);
6257     iunlock(f->ip);
6258     return 0;
6259   }
6260   return -1;
6261 }
6262 
6263 // Read from file f.
6264 int
6265 fileread(struct file *f, char *addr, int n)
6266 {
6267   int r;
6268 
6269   if(f->readable == 0)
6270     return -1;
6271   if(f->type == FD_PIPE)
6272     return piperead(f->pipe, addr, n);
6273   if(f->type == FD_INODE){
6274     ilock(f->ip);
6275     if((r = readi(f->ip, addr, f->off, n)) > 0)
6276       f->off += r;
6277     iunlock(f->ip);
6278     return r;
6279   }
6280   panic("fileread");
6281 }
6282 
6283 
6284 
6285 
6286 
6287 
6288 
6289 
6290 
6291 
6292 
6293 
6294 
6295 
6296 
6297 
6298 
6299 
6300 // Write to file f.
6301 int
6302 filewrite(struct file *f, char *addr, int n)
6303 {
6304   int r;
6305 
6306   if(f->writable == 0)
6307     return -1;
6308   if(f->type == FD_PIPE)
6309     return pipewrite(f->pipe, addr, n);
6310   if(f->type == FD_INODE){
6311     // write a few blocks at a time to avoid exceeding
6312     // the maximum log transaction size, including
6313     // i-node, indirect block, allocation blocks,
6314     // and 2 blocks of slop for non-aligned writes.
6315     // this really belongs lower down, since writei()
6316     // might be writing a device like the console.
6317     int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
6318     int i = 0;
6319     while(i < n){
6320       int n1 = n - i;
6321       if(n1 > max)
6322         n1 = max;
6323 
6324       begin_op();
6325       ilock(f->ip);
6326       if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
6327         f->off += r;
6328       iunlock(f->ip);
6329       end_op();
6330 
6331       if(r < 0)
6332         break;
6333       if(r != n1)
6334         panic("short filewrite");
6335       i += r;
6336     }
6337     return i == n ? n : -1;
6338   }
6339   panic("filewrite");
6340 }
6341 
6342 
6343 
6344 
6345 
6346 
6347 
6348 
6349 
