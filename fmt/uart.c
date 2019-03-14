8600 // Intel 8250 serial port (UART).
8601 
8602 #include "types.h"
8603 #include "defs.h"
8604 #include "param.h"
8605 #include "traps.h"
8606 #include "spinlock.h"
8607 #include "sleeplock.h"
8608 #include "fs.h"
8609 #include "file.h"
8610 #include "mmu.h"
8611 #include "proc.h"
8612 #include "x86.h"
8613 
8614 #define COM1    0x3f8
8615 
8616 static int uart;    // is there a uart?
8617 
8618 void
8619 uartinit(void)
8620 {
8621   char *p;
8622 
8623   // Turn off the FIFO
8624   outb(COM1+2, 0);
8625 
8626   // 9600 baud, 8 data bits, 1 stop bit, parity off.
8627   outb(COM1+3, 0x80);    // Unlock divisor
8628   outb(COM1+0, 115200/9600);
8629   outb(COM1+1, 0);
8630   outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
8631   outb(COM1+4, 0);
8632   outb(COM1+1, 0x01);    // Enable receive interrupts.
8633 
8634   // If status is 0xFF, no serial port.
8635   if(inb(COM1+5) == 0xFF)
8636     return;
8637   uart = 1;
8638 
8639   // Acknowledge pre-existing interrupt conditions;
8640   // enable interrupts.
8641   inb(COM1+2);
8642   inb(COM1+0);
8643   ioapicenable(IRQ_COM1, 0);
8644 
8645   // Announce that we're here.
8646   for(p="xv6...\n"; *p; p++)
8647     uartputc(*p);
8648 }
8649 
8650 void
8651 uartputc(int c)
8652 {
8653   int i;
8654 
8655   if(!uart)
8656     return;
8657   for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
8658     microdelay(10);
8659   outb(COM1+0, c);
8660 }
8661 
8662 static int
8663 uartgetc(void)
8664 {
8665   if(!uart)
8666     return -1;
8667   if(!(inb(COM1+5) & 0x01))
8668     return -1;
8669   return inb(COM1+0);
8670 }
8671 
8672 void
8673 uartintr(void)
8674 {
8675   consoleintr(uartgetc);
8676 }
8677 
8678 
8679 
8680 
8681 
8682 
8683 
8684 
8685 
8686 
8687 
8688 
8689 
8690 
8691 
8692 
8693 
8694 
8695 
8696 
8697 
8698 
8699 
