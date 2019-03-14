8200 // Console input and output.
8201 // Input is from the keyboard or serial port.
8202 // Output is written to the screen and serial port.
8203 
8204 #include "types.h"
8205 #include "defs.h"
8206 #include "param.h"
8207 #include "traps.h"
8208 #include "spinlock.h"
8209 #include "sleeplock.h"
8210 #include "fs.h"
8211 #include "file.h"
8212 #include "memlayout.h"
8213 #include "mmu.h"
8214 #include "proc.h"
8215 #include "x86.h"
8216 
8217 static void consputc(int);
8218 
8219 static int panicked = 0;
8220 
8221 static struct {
8222   struct spinlock lock;
8223   int locking;
8224 } cons;
8225 
8226 static void
8227 printint(int xx, int base, int sign)
8228 {
8229   static char digits[] = "0123456789abcdef";
8230   char buf[16];
8231   int i;
8232   uint x;
8233 
8234   if(sign && (sign = xx < 0))
8235     x = -xx;
8236   else
8237     x = xx;
8238 
8239   i = 0;
8240   do{
8241     buf[i++] = digits[x % base];
8242   }while((x /= base) != 0);
8243 
8244   if(sign)
8245     buf[i++] = '-';
8246 
8247   while(--i >= 0)
8248     consputc(buf[i]);
8249 }
8250 
8251 
8252 
8253 
8254 
8255 
8256 
8257 
8258 
8259 
8260 
8261 
8262 
8263 
8264 
8265 
8266 
8267 
8268 
8269 
8270 
8271 
8272 
8273 
8274 
8275 
8276 
8277 
8278 
8279 
8280 
8281 
8282 
8283 
8284 
8285 
8286 
8287 
8288 
8289 
8290 
8291 
8292 
8293 
8294 
8295 
8296 
8297 
8298 
8299 
8300 // Print to the console. only understands %d, %x, %p, %s.
8301 void
8302 cprintf(char *fmt, ...)
8303 {
8304   int i, c, locking;
8305   uint *argp;
8306   char *s;
8307 
8308   locking = cons.locking;
8309   if(locking)
8310     acquire(&cons.lock);
8311 
8312   if (fmt == 0)
8313     panic("null fmt");
8314 
8315   argp = (uint*)(void*)(&fmt + 1);
8316   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
8317     if(c != '%'){
8318       consputc(c);
8319       continue;
8320     }
8321     c = fmt[++i] & 0xff;
8322     if(c == 0)
8323       break;
8324     switch(c){
8325     case 'd':
8326       printint(*argp++, 10, 1);
8327       break;
8328     case 'x':
8329     case 'p':
8330       printint(*argp++, 16, 0);
8331       break;
8332     case 's':
8333       if((s = (char*)*argp++) == 0)
8334         s = "(null)";
8335       for(; *s; s++)
8336         consputc(*s);
8337       break;
8338     case '%':
8339       consputc('%');
8340       break;
8341     default:
8342       // Print unknown % sequence to draw attention.
8343       consputc('%');
8344       consputc(c);
8345       break;
8346     }
8347   }
8348 
8349 
8350   if(locking)
8351     release(&cons.lock);
8352 }
8353 
8354 void
8355 panic(char *s)
8356 {
8357   int i;
8358   uint pcs[10];
8359 
8360   cli();
8361   cons.locking = 0;
8362   // use lapiccpunum so that we can call panic from mycpu()
8363   cprintf("lapicid %d: panic: ", lapicid());
8364   cprintf(s);
8365   cprintf("\n");
8366   getcallerpcs(&s, pcs);
8367   for(i=0; i<10; i++)
8368     cprintf(" %p", pcs[i]);
8369   panicked = 1; // freeze other CPU
8370   for(;;)
8371     ;
8372 }
8373 
8374 
8375 
8376 
8377 
8378 
8379 
8380 
8381 
8382 
8383 
8384 
8385 
8386 
8387 
8388 
8389 
8390 
8391 
8392 
8393 
8394 
8395 
8396 
8397 
8398 
8399 
8400 #define BACKSPACE 0x100
8401 #define CRTPORT 0x3d4
8402 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
8403 
8404 static void
8405 cgaputc(int c)
8406 {
8407   int pos;
8408 
8409   // Cursor position: col + 80*row.
8410   outb(CRTPORT, 14);
8411   pos = inb(CRTPORT+1) << 8;
8412   outb(CRTPORT, 15);
8413   pos |= inb(CRTPORT+1);
8414 
8415   if(c == '\n')
8416     pos += 80 - pos%80;
8417   else if(c == BACKSPACE){
8418     if(pos > 0) --pos;
8419   } else
8420     crt[pos++] = (c&0xff) | 0x0700;  // black on white
8421 
8422   if(pos < 0 || pos > 25*80)
8423     panic("pos under/overflow");
8424 
8425   if((pos/80) >= 24){  // Scroll up.
8426     memmove(crt, crt+80, sizeof(crt[0])*23*80);
8427     pos -= 80;
8428     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
8429   }
8430 
8431   outb(CRTPORT, 14);
8432   outb(CRTPORT+1, pos>>8);
8433   outb(CRTPORT, 15);
8434   outb(CRTPORT+1, pos);
8435   crt[pos] = ' ' | 0x0700;
8436 }
8437 
8438 
8439 
8440 
8441 
8442 
8443 
8444 
8445 
8446 
8447 
8448 
8449 
8450 void
8451 consputc(int c)
8452 {
8453   if(panicked){
8454     cli();
8455     for(;;)
8456       ;
8457   }
8458 
8459   if(c == BACKSPACE){
8460     uartputc('\b'); uartputc(' '); uartputc('\b');
8461   } else
8462     uartputc(c);
8463   cgaputc(c);
8464 }
8465 
8466 #define INPUT_BUF 128
8467 struct {
8468   char buf[INPUT_BUF];
8469   uint r;  // Read index
8470   uint w;  // Write index
8471   uint e;  // Edit index
8472 } input;
8473 
8474 #define C(x)  ((x)-'@')  // Control-x
8475 
8476 void
8477 consoleintr(int (*getc)(void))
8478 {
8479   int c, doprocdump = 0;
8480 
8481   acquire(&cons.lock);
8482   while((c = getc()) >= 0){
8483     switch(c){
8484     case C('P'):  // Process listing.
8485       // procdump() locks cons.lock indirectly; invoke later
8486       doprocdump = 1;
8487       break;
8488     case C('U'):  // Kill line.
8489       while(input.e != input.w &&
8490             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8491         input.e--;
8492         consputc(BACKSPACE);
8493       }
8494       break;
8495     case C('H'): case '\x7f':  // Backspace
8496       if(input.e != input.w){
8497         input.e--;
8498         consputc(BACKSPACE);
8499       }
8500       break;
8501     default:
8502       if(c != 0 && input.e-input.r < INPUT_BUF){
8503         c = (c == '\r') ? '\n' : c;
8504         input.buf[input.e++ % INPUT_BUF] = c;
8505         consputc(c);
8506         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8507           input.w = input.e;
8508           wakeup(&input.r);
8509         }
8510       }
8511       break;
8512     }
8513   }
8514   release(&cons.lock);
8515   if(doprocdump) {
8516     procdump();  // now call procdump() wo. cons.lock held
8517   }
8518 }
8519 
8520 int
8521 consoleread(struct inode *ip, char *dst, int n)
8522 {
8523   uint target;
8524   int c;
8525 
8526   iunlock(ip);
8527   target = n;
8528   acquire(&cons.lock);
8529   while(n > 0){
8530     while(input.r == input.w){
8531       if(myproc()->killed){
8532         release(&cons.lock);
8533         ilock(ip);
8534         return -1;
8535       }
8536       sleep(&input.r, &cons.lock);
8537     }
8538     c = input.buf[input.r++ % INPUT_BUF];
8539     if(c == C('D')){  // EOF
8540       if(n < target){
8541         // Save ^D for next time, to make sure
8542         // caller gets a 0-byte result.
8543         input.r--;
8544       }
8545       break;
8546     }
8547     *dst++ = c;
8548     --n;
8549     if(c == '\n')
8550       break;
8551   }
8552   release(&cons.lock);
8553   ilock(ip);
8554 
8555   return target - n;
8556 }
8557 
8558 int
8559 consolewrite(struct inode *ip, char *buf, int n)
8560 {
8561   int i;
8562 
8563   iunlock(ip);
8564   acquire(&cons.lock);
8565   for(i = 0; i < n; i++)
8566     consputc(buf[i] & 0xff);
8567   release(&cons.lock);
8568   ilock(ip);
8569 
8570   return n;
8571 }
8572 
8573 void
8574 consoleinit(void)
8575 {
8576   initlock(&cons.lock, "console");
8577 
8578   devsw[CONSOLE].write = consolewrite;
8579   devsw[CONSOLE].read = consoleread;
8580   cons.locking = 1;
8581 
8582   ioapicenable(IRQ_KBD, 0);
8583 }
8584 
8585 
8586 
8587 
8588 
8589 
8590 
8591 
8592 
8593 
8594 
8595 
8596 
8597 
8598 
8599 
