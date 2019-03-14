7500 // Multiprocessor support
7501 // Search memory for MP description structures.
7502 // http://developer.intel.com/design/pentium/datashts/24201606.pdf
7503 
7504 #include "types.h"
7505 #include "defs.h"
7506 #include "param.h"
7507 #include "memlayout.h"
7508 #include "mp.h"
7509 #include "x86.h"
7510 #include "mmu.h"
7511 #include "proc.h"
7512 
7513 struct cpu cpus[NCPU];
7514 int ncpu;
7515 uchar ioapicid;
7516 
7517 static uchar
7518 sum(uchar *addr, int len)
7519 {
7520   int i, sum;
7521 
7522   sum = 0;
7523   for(i=0; i<len; i++)
7524     sum += addr[i];
7525   return sum;
7526 }
7527 
7528 // Look for an MP structure in the len bytes at addr.
7529 static struct mp*
7530 mpsearch1(uint a, int len)
7531 {
7532   uchar *e, *p, *addr;
7533 
7534   addr = P2V(a);
7535   e = addr+len;
7536   for(p = addr; p < e; p += sizeof(struct mp))
7537     if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
7538       return (struct mp*)p;
7539   return 0;
7540 }
7541 
7542 
7543 
7544 
7545 
7546 
7547 
7548 
7549 
7550 // Search for the MP Floating Pointer Structure, which according to the
7551 // spec is in one of the following three locations:
7552 // 1) in the first KB of the EBDA;
7553 // 2) in the last KB of system base memory;
7554 // 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
7555 static struct mp*
7556 mpsearch(void)
7557 {
7558   uchar *bda;
7559   uint p;
7560   struct mp *mp;
7561 
7562   bda = (uchar *) P2V(0x400);
7563   if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
7564     if((mp = mpsearch1(p, 1024)))
7565       return mp;
7566   } else {
7567     p = ((bda[0x14]<<8)|bda[0x13])*1024;
7568     if((mp = mpsearch1(p-1024, 1024)))
7569       return mp;
7570   }
7571   return mpsearch1(0xF0000, 0x10000);
7572 }
7573 
7574 // Search for an MP configuration table.  For now,
7575 // don't accept the default configurations (physaddr == 0).
7576 // Check for correct signature, calculate the checksum and,
7577 // if correct, check the version.
7578 // To do: check extended table checksum.
7579 static struct mpconf*
7580 mpconfig(struct mp **pmp)
7581 {
7582   struct mpconf *conf;
7583   struct mp *mp;
7584 
7585   if((mp = mpsearch()) == 0 || mp->physaddr == 0)
7586     return 0;
7587   conf = (struct mpconf*) P2V((uint) mp->physaddr);
7588   if(memcmp(conf, "PCMP", 4) != 0)
7589     return 0;
7590   if(conf->version != 1 && conf->version != 4)
7591     return 0;
7592   if(sum((uchar*)conf, conf->length) != 0)
7593     return 0;
7594   *pmp = mp;
7595   return conf;
7596 }
7597 
7598 
7599 
7600 void
7601 mpinit(void)
7602 {
7603   uchar *p, *e;
7604   int ismp;
7605   struct mp *mp;
7606   struct mpconf *conf;
7607   struct mpproc *proc;
7608   struct mpioapic *ioapic;
7609 
7610   if((conf = mpconfig(&mp)) == 0)
7611     panic("Expect to run on an SMP");
7612   ismp = 1;
7613   lapic = (uint*)conf->lapicaddr;
7614   for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
7615     switch(*p){
7616     case MPPROC:
7617       proc = (struct mpproc*)p;
7618       if(ncpu < NCPU) {
7619         cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
7620         ncpu++;
7621       }
7622       p += sizeof(struct mpproc);
7623       continue;
7624     case MPIOAPIC:
7625       ioapic = (struct mpioapic*)p;
7626       ioapicid = ioapic->apicno;
7627       p += sizeof(struct mpioapic);
7628       continue;
7629     case MPBUS:
7630     case MPIOINTR:
7631     case MPLINTR:
7632       p += 8;
7633       continue;
7634     default:
7635       ismp = 0;
7636       break;
7637     }
7638   }
7639   if(!ismp)
7640     panic("Didn't find a suitable machine");
7641 
7642   if(mp->imcrp){
7643     // Bochs doesn't support IMCR, so this doesn't run on Bochs.
7644     // But it would on real hardware.
7645     outb(0x22, 0x70);   // Select IMCR
7646     outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
7647   }
7648 }
7649 
