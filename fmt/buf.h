4150 struct buf {
4151   int flags;
4152   uint dev;
4153   uint blockno;
4154   struct sleeplock lock;
4155   uint refcnt;
4156   struct buf *prev; // LRU cache list
4157   struct buf *next;
4158   struct buf *qnext; // disk queue
4159   uchar data[BSIZE];
4160 };
4161 #define B_VALID 0x2  // buffer has been read from disk
4162 #define B_DIRTY 0x4  // buffer needs to be written to disk
4163 
4164 
4165 
4166 
4167 
4168 
4169 
4170 
4171 
4172 
4173 
4174 
4175 
4176 
4177 
4178 
4179 
4180 
4181 
4182 
4183 
4184 
4185 
4186 
4187 
4188 
4189 
4190 
4191 
4192 
4193 
4194 
4195 
4196 
4197 
4198 
4199 
