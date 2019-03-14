7650 // The local APIC manages internal (non-I/O) interrupts.
7651 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7652 
7653 #include "param.h"
7654 #include "types.h"
7655 #include "defs.h"
7656 #include "date.h"
7657 #include "memlayout.h"
7658 #include "traps.h"
7659 #include "mmu.h"
7660 #include "x86.h"
7661 
7662 // Local APIC registers, divided by 4 for use as uint[] indices.
7663 #define ID      (0x0020/4)   // ID
7664 #define VER     (0x0030/4)   // Version
7665 #define TPR     (0x0080/4)   // Task Priority
7666 #define EOI     (0x00B0/4)   // EOI
7667 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7668   #define ENABLE     0x00000100   // Unit Enable
7669 #define ESR     (0x0280/4)   // Error Status
7670 #define ICRLO   (0x0300/4)   // Interrupt Command
7671   #define INIT       0x00000500   // INIT/RESET
7672   #define STARTUP    0x00000600   // Startup IPI
7673   #define DELIVS     0x00001000   // Delivery status
7674   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7675   #define DEASSERT   0x00000000
7676   #define LEVEL      0x00008000   // Level triggered
7677   #define BCAST      0x00080000   // Send to all APICs, including self.
7678   #define BUSY       0x00001000
7679   #define FIXED      0x00000000
7680 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7681 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7682   #define X1         0x0000000B   // divide counts by 1
7683   #define PERIODIC   0x00020000   // Periodic
7684 #define PCINT   (0x0340/4)   // Performance Counter LVT
7685 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7686 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7687 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7688   #define MASKED     0x00010000   // Interrupt masked
7689 #define TICR    (0x0380/4)   // Timer Initial Count
7690 #define TCCR    (0x0390/4)   // Timer Current Count
7691 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7692 
7693 volatile uint *lapic;  // Initialized in mp.c
7694 
7695 
7696 
7697 
7698 
7699 
7700 static void
7701 lapicw(int index, int value)
7702 {
7703   lapic[index] = value;
7704   lapic[ID];  // wait for write to finish, by reading
7705 }
7706 
7707 void
7708 lapicinit(void)
7709 {
7710   if(!lapic)
7711     return;
7712 
7713   // Enable local APIC; set spurious interrupt vector.
7714   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7715 
7716   // The timer repeatedly counts down at bus frequency
7717   // from lapic[TICR] and then issues an interrupt.
7718   // If xv6 cared more about precise timekeeping,
7719   // TICR would be calibrated using an external time source.
7720   lapicw(TDCR, X1);
7721   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7722   lapicw(TICR, 10000000);
7723 
7724   // Disable logical interrupt lines.
7725   lapicw(LINT0, MASKED);
7726   lapicw(LINT1, MASKED);
7727 
7728   // Disable performance counter overflow interrupts
7729   // on machines that provide that interrupt entry.
7730   if(((lapic[VER]>>16) & 0xFF) >= 4)
7731     lapicw(PCINT, MASKED);
7732 
7733   // Map error interrupt to IRQ_ERROR.
7734   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7735 
7736   // Clear error status register (requires back-to-back writes).
7737   lapicw(ESR, 0);
7738   lapicw(ESR, 0);
7739 
7740   // Ack any outstanding interrupts.
7741   lapicw(EOI, 0);
7742 
7743   // Send an Init Level De-Assert to synchronise arbitration ID's.
7744   lapicw(ICRHI, 0);
7745   lapicw(ICRLO, BCAST | INIT | LEVEL);
7746   while(lapic[ICRLO] & DELIVS)
7747     ;
7748 
7749 
7750   // Enable interrupts on the APIC (but not on the processor).
7751   lapicw(TPR, 0);
7752 }
7753 
7754 int
7755 lapicid(void)
7756 {
7757   if (!lapic)
7758     return 0;
7759   return lapic[ID] >> 24;
7760 }
7761 
7762 // Acknowledge interrupt.
7763 void
7764 lapiceoi(void)
7765 {
7766   if(lapic)
7767     lapicw(EOI, 0);
7768 }
7769 
7770 // Spin for a given number of microseconds.
7771 // On real hardware would want to tune this dynamically.
7772 void
7773 microdelay(int us)
7774 {
7775 }
7776 
7777 #define CMOS_PORT    0x70
7778 #define CMOS_RETURN  0x71
7779 
7780 // Start additional processor running entry code at addr.
7781 // See Appendix B of MultiProcessor Specification.
7782 void
7783 lapicstartap(uchar apicid, uint addr)
7784 {
7785   int i;
7786   ushort *wrv;
7787 
7788   // "The BSP must initialize CMOS shutdown code to 0AH
7789   // and the warm reset vector (DWORD based at 40:67) to point at
7790   // the AP startup code prior to the [universal startup algorithm]."
7791   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7792   outb(CMOS_PORT+1, 0x0A);
7793   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7794   wrv[0] = 0;
7795   wrv[1] = addr >> 4;
7796 
7797 
7798 
7799 
7800   // "Universal startup algorithm."
7801   // Send INIT (level-triggered) interrupt to reset other CPU.
7802   lapicw(ICRHI, apicid<<24);
7803   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7804   microdelay(200);
7805   lapicw(ICRLO, INIT | LEVEL);
7806   microdelay(100);    // should be 10ms, but too slow in Bochs!
7807 
7808   // Send startup IPI (twice!) to enter code.
7809   // Regular hardware is supposed to only accept a STARTUP
7810   // when it is in the halted state due to an INIT.  So the second
7811   // should be ignored, but it is part of the official Intel algorithm.
7812   // Bochs complains about the second one.  Too bad for Bochs.
7813   for(i = 0; i < 2; i++){
7814     lapicw(ICRHI, apicid<<24);
7815     lapicw(ICRLO, STARTUP | (addr>>12));
7816     microdelay(200);
7817   }
7818 }
7819 
7820 #define CMOS_STATA   0x0a
7821 #define CMOS_STATB   0x0b
7822 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7823 
7824 #define SECS    0x00
7825 #define MINS    0x02
7826 #define HOURS   0x04
7827 #define DAY     0x07
7828 #define MONTH   0x08
7829 #define YEAR    0x09
7830 
7831 static uint
7832 cmos_read(uint reg)
7833 {
7834   outb(CMOS_PORT,  reg);
7835   microdelay(200);
7836 
7837   return inb(CMOS_RETURN);
7838 }
7839 
7840 static void
7841 fill_rtcdate(struct rtcdate *r)
7842 {
7843   r->second = cmos_read(SECS);
7844   r->minute = cmos_read(MINS);
7845   r->hour   = cmos_read(HOURS);
7846   r->day    = cmos_read(DAY);
7847   r->month  = cmos_read(MONTH);
7848   r->year   = cmos_read(YEAR);
7849 }
7850 // qemu seems to use 24-hour GWT and the values are BCD encoded
7851 void
7852 cmostime(struct rtcdate *r)
7853 {
7854   struct rtcdate t1, t2;
7855   int sb, bcd;
7856 
7857   sb = cmos_read(CMOS_STATB);
7858 
7859   bcd = (sb & (1 << 2)) == 0;
7860 
7861   // make sure CMOS doesn't modify time while we read it
7862   for(;;) {
7863     fill_rtcdate(&t1);
7864     if(cmos_read(CMOS_STATA) & CMOS_UIP)
7865         continue;
7866     fill_rtcdate(&t2);
7867     if(memcmp(&t1, &t2, sizeof(t1)) == 0)
7868       break;
7869   }
7870 
7871   // convert
7872   if(bcd) {
7873 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7874     CONV(second);
7875     CONV(minute);
7876     CONV(hour  );
7877     CONV(day   );
7878     CONV(month );
7879     CONV(year  );
7880 #undef     CONV
7881   }
7882 
7883   *r = t1;
7884   r->year += 2000;
7885 }
7886 
7887 
7888 
7889 
7890 
7891 
7892 
7893 
7894 
7895 
7896 
7897 
7898 
7899 
