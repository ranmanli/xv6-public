9400 #include "asm.h"
9401 #include "memlayout.h"
9402 #include "mmu.h"
9403 
9404 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9405 # The BIOS loads this code from the first sector of the hard disk into
9406 # memory at physical address 0x7c00 and starts executing in real mode
9407 # with %cs=0 %ip=7c00.
9408 
9409 .code16                       # Assemble for 16-bit mode
9410 .globl start
9411 start:
9412   cli                         # BIOS enabled interrupts; disable
9413 
9414   # Zero data segment registers DS, ES, and SS.
9415   xorw    %ax,%ax             # Set %ax to zero
9416   movw    %ax,%ds             # -> Data Segment
9417   movw    %ax,%es             # -> Extra Segment
9418   movw    %ax,%ss             # -> Stack Segment
9419 
9420   # Physical address line A20 is tied to zero so that the first PCs
9421   # with 2 MB would run software that assumed 1 MB.  Undo that.
9422 seta20.1:
9423   inb     $0x64,%al               # Wait for not busy
9424   testb   $0x2,%al
9425   jnz     seta20.1
9426 
9427   movb    $0xd1,%al               # 0xd1 -> port 0x64
9428   outb    %al,$0x64
9429 
9430 seta20.2:
9431   inb     $0x64,%al               # Wait for not busy
9432   testb   $0x2,%al
9433   jnz     seta20.2
9434 
9435   movb    $0xdf,%al               # 0xdf -> port 0x60
9436   outb    %al,$0x60
9437 
9438   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9439   # virtual addresses map directly to physical addresses so that the
9440   # effective memory map doesn't change during the transition.
9441   lgdt    gdtdesc
9442   movl    %cr0, %eax
9443   orl     $CR0_PE, %eax
9444   movl    %eax, %cr0
9445 
9446 
9447 
9448 
9449 
9450   # Complete the transition to 32-bit protected mode by using a long jmp
9451   # to reload %cs and %eip.  The segment descriptors are set up with no
9452   # translation, so that the mapping is still the identity mapping.
9453   ljmp    $(SEG_KCODE<<3), $start32
9454 
9455 .code32  # Tell assembler to generate 32-bit code now.
9456 start32:
9457   # Set up the protected-mode data segment registers
9458   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9459   movw    %ax, %ds                # -> DS: Data Segment
9460   movw    %ax, %es                # -> ES: Extra Segment
9461   movw    %ax, %ss                # -> SS: Stack Segment
9462   movw    $0, %ax                 # Zero segments not ready for use
9463   movw    %ax, %fs                # -> FS
9464   movw    %ax, %gs                # -> GS
9465 
9466   # Set up the stack pointer and call into C.
9467   movl    $start, %esp
9468   call    bootmain
9469 
9470   # If bootmain returns (it shouldn't), trigger a Bochs
9471   # breakpoint if running under Bochs, then loop.
9472   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9473   movw    %ax, %dx
9474   outw    %ax, %dx
9475   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9476   outw    %ax, %dx
9477 spin:
9478   jmp     spin
9479 
9480 # Bootstrap GDT
9481 .p2align 2                                # force 4 byte alignment
9482 gdt:
9483   SEG_NULLASM                             # null seg
9484   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9485   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9486 
9487 gdtdesc:
9488   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9489   .long   gdt                             # address gdt
9490 
9491 
9492 
9493 
9494 
9495 
9496 
9497 
9498 
9499 
