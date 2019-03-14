9600 /* Simple linker script for the JOS kernel.
9601    See the GNU ld 'info' manual ("info ld") to learn the syntax. */
9602 
9603 OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
9604 OUTPUT_ARCH(i386)
9605 ENTRY(_start)
9606 
9607 SECTIONS
9608 {
9609 	/* Link the kernel at this address: "." means the current address */
9610         /* Must be equal to KERNLINK */
9611 	. = 0x80100000;
9612 
9613 	.text : AT(0x100000) {
9614 		*(.text .stub .text.* .gnu.linkonce.t.*)
9615 	}
9616 
9617 	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */
9618 
9619 	.rodata : {
9620 		*(.rodata .rodata.* .gnu.linkonce.r.*)
9621 	}
9622 
9623 	/* Include debugging information in kernel memory */
9624 	.stab : {
9625 		PROVIDE(__STAB_BEGIN__ = .);
9626 		*(.stab);
9627 		PROVIDE(__STAB_END__ = .);
9628 		BYTE(0)		/* Force the linker to allocate space
9629 				   for this section */
9630 	}
9631 
9632 	.stabstr : {
9633 		PROVIDE(__STABSTR_BEGIN__ = .);
9634 		*(.stabstr);
9635 		PROVIDE(__STABSTR_END__ = .);
9636 		BYTE(0)		/* Force the linker to allocate space
9637 				   for this section */
9638 	}
9639 
9640 	/* Adjust the address for the data segment to the next page */
9641 	. = ALIGN(0x1000);
9642 
9643 	/* Conventionally, Unix linkers provide pseudo-symbols
9644 	 * etext, edata, and end, at the end of the text, data, and bss.
9645 	 * For the kernel mapping, we need the address at the beginning
9646 	 * of the data section, but that's not one of the conventional
9647 	 * symbols, because the convention started before there was a
9648 	 * read-only rodata section between text and data. */
9649 	PROVIDE(data = .);
9650 	/* The data segment */
9651 	.data : {
9652 		*(.data)
9653 	}
9654 
9655 	PROVIDE(edata = .);
9656 
9657 	.bss : {
9658 		*(.bss)
9659 	}
9660 
9661 	PROVIDE(end = .);
9662 
9663 	/DISCARD/ : {
9664 		*(.eh_frame .note.GNU-stack)
9665 	}
9666 }
9667 
9668 
9669 
9670 
9671 
9672 
9673 
9674 
9675 
9676 
9677 
9678 
9679 
9680 
9681 
9682 
9683 
9684 
9685 
9686 
9687 
9688 
9689 
9690 
9691 
9692 
9693 
9694 
9695 
9696 
9697 
9698 
9699 
