9500 // Boot loader.
9501 //
9502 // Part of the boot block, along with bootasm.S, which calls bootmain().
9503 // bootasm.S has put the processor into protected 32-bit mode.
9504 // bootmain() loads an ELF kernel image from the disk starting at
9505 // sector 1 and then jumps to the kernel entry routine.
9506 
9507 #include "types.h"
9508 #include "elf.h"
9509 #include "x86.h"
9510 #include "memlayout.h"
9511 
9512 #define SECTSIZE  512
9513 
9514 void readseg(uchar*, uint, uint);
9515 
9516 void
9517 bootmain(void)
9518 {
9519   struct elfhdr *elf;
9520   struct proghdr *ph, *eph;
9521   void (*entry)(void);
9522   uchar* pa;
9523 
9524   elf = (struct elfhdr*)0x10000;  // scratch space
9525 
9526   // Read 1st page off disk
9527   readseg((uchar*)elf, 4096, 0);
9528 
9529   // Is this an ELF executable?
9530   if(elf->magic != ELF_MAGIC)
9531     return;  // let bootasm.S handle error
9532 
9533   // Load each program segment (ignores ph flags).
9534   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9535   eph = ph + elf->phnum;
9536   for(; ph < eph; ph++){
9537     pa = (uchar*)ph->paddr;
9538     readseg(pa, ph->filesz, ph->off);
9539     if(ph->memsz > ph->filesz)
9540       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9541   }
9542 
9543   // Call the entry point from the ELF header.
9544   // Does not return!
9545   entry = (void(*)(void))(elf->entry);
9546   entry();
9547 }
9548 
9549 
9550 void
9551 waitdisk(void)
9552 {
9553   // Wait for disk ready.
9554   while((inb(0x1F7) & 0xC0) != 0x40)
9555     ;
9556 }
9557 
9558 // Read a single sector at offset into dst.
9559 void
9560 readsect(void *dst, uint offset)
9561 {
9562   // Issue command.
9563   waitdisk();
9564   outb(0x1F2, 1);   // count = 1
9565   outb(0x1F3, offset);
9566   outb(0x1F4, offset >> 8);
9567   outb(0x1F5, offset >> 16);
9568   outb(0x1F6, (offset >> 24) | 0xE0);
9569   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9570 
9571   // Read data.
9572   waitdisk();
9573   insl(0x1F0, dst, SECTSIZE/4);
9574 }
9575 
9576 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9577 // Might copy more than asked.
9578 void
9579 readseg(uchar* pa, uint count, uint offset)
9580 {
9581   uchar* epa;
9582 
9583   epa = pa + count;
9584 
9585   // Round down to sector boundary.
9586   pa -= offset % SECTSIZE;
9587 
9588   // Translate from bytes to sectors; kernel starts at sector 1.
9589   offset = (offset / SECTSIZE) + 1;
9590 
9591   // If this is too slow, we could read lots of sectors at a time.
9592   // We'd write more to memory than asked, but it doesn't matter --
9593   // we load in increasing order.
9594   for(; pa < epa; pa += SECTSIZE, offset++)
9595     readsect(pa, offset);
9596 }
9597 
9598 
9599 
