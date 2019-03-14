7900 // The I/O APIC manages hardware interrupts for an SMP system.
7901 // http://www.intel.com/design/chipsets/datashts/29056601.pdf
7902 // See also picirq.c.
7903 
7904 #include "types.h"
7905 #include "defs.h"
7906 #include "traps.h"
7907 
7908 #define IOAPIC  0xFEC00000   // Default physical address of IO APIC
7909 
7910 #define REG_ID     0x00  // Register index: ID
7911 #define REG_VER    0x01  // Register index: version
7912 #define REG_TABLE  0x10  // Redirection table base
7913 
7914 // The redirection table starts at REG_TABLE and uses
7915 // two registers to configure each interrupt.
7916 // The first (low) register in a pair contains configuration bits.
7917 // The second (high) register contains a bitmask telling which
7918 // CPUs can serve that interrupt.
7919 #define INT_DISABLED   0x00010000  // Interrupt disabled
7920 #define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
7921 #define INT_ACTIVELOW  0x00002000  // Active low (vs high)
7922 #define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
7923 
7924 volatile struct ioapic *ioapic;
7925 
7926 // IO APIC MMIO structure: write reg, then read or write data.
7927 struct ioapic {
7928   uint reg;
7929   uint pad[3];
7930   uint data;
7931 };
7932 
7933 static uint
7934 ioapicread(int reg)
7935 {
7936   ioapic->reg = reg;
7937   return ioapic->data;
7938 }
7939 
7940 static void
7941 ioapicwrite(int reg, uint data)
7942 {
7943   ioapic->reg = reg;
7944   ioapic->data = data;
7945 }
7946 
7947 
7948 
7949 
7950 void
7951 ioapicinit(void)
7952 {
7953   int i, id, maxintr;
7954 
7955   ioapic = (volatile struct ioapic*)IOAPIC;
7956   maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
7957   id = ioapicread(REG_ID) >> 24;
7958   if(id != ioapicid)
7959     cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");
7960 
7961   // Mark all interrupts edge-triggered, active high, disabled,
7962   // and not routed to any CPUs.
7963   for(i = 0; i <= maxintr; i++){
7964     ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
7965     ioapicwrite(REG_TABLE+2*i+1, 0);
7966   }
7967 }
7968 
7969 void
7970 ioapicenable(int irq, int cpunum)
7971 {
7972   // Mark interrupt edge-triggered, active high,
7973   // enabled, and routed to the given cpunum,
7974   // which happens to be that cpu's APIC ID.
7975   ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
7976   ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
7977 }
7978 
7979 
7980 
7981 
7982 
7983 
7984 
7985 
7986 
7987 
7988 
7989 
7990 
7991 
7992 
7993 
7994 
7995 
7996 
7997 
7998 
7999 
