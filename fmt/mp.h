7350 // See MultiProcessor Specification Version 1.[14]
7351 
7352 struct mp {             // floating pointer
7353   uchar signature[4];           // "_MP_"
7354   void *physaddr;               // phys addr of MP config table
7355   uchar length;                 // 1
7356   uchar specrev;                // [14]
7357   uchar checksum;               // all bytes must add up to 0
7358   uchar type;                   // MP system config type
7359   uchar imcrp;
7360   uchar reserved[3];
7361 };
7362 
7363 struct mpconf {         // configuration table header
7364   uchar signature[4];           // "PCMP"
7365   ushort length;                // total table length
7366   uchar version;                // [14]
7367   uchar checksum;               // all bytes must add up to 0
7368   uchar product[20];            // product id
7369   uint *oemtable;               // OEM table pointer
7370   ushort oemlength;             // OEM table length
7371   ushort entry;                 // entry count
7372   uint *lapicaddr;              // address of local APIC
7373   ushort xlength;               // extended table length
7374   uchar xchecksum;              // extended table checksum
7375   uchar reserved;
7376 };
7377 
7378 struct mpproc {         // processor table entry
7379   uchar type;                   // entry type (0)
7380   uchar apicid;                 // local APIC id
7381   uchar version;                // local APIC verison
7382   uchar flags;                  // CPU flags
7383     #define MPBOOT 0x02           // This proc is the bootstrap processor.
7384   uchar signature[4];           // CPU signature
7385   uint feature;                 // feature flags from CPUID instruction
7386   uchar reserved[8];
7387 };
7388 
7389 struct mpioapic {       // I/O APIC table entry
7390   uchar type;                   // entry type (2)
7391   uchar apicno;                 // I/O APIC id
7392   uchar version;                // I/O APIC version
7393   uchar flags;                  // I/O APIC flags
7394   uint *addr;                  // I/O APIC address
7395 };
7396 
7397 
7398 
7399 
7400 // Table entry types
7401 #define MPPROC    0x00  // One per processor
7402 #define MPBUS     0x01  // One per bus
7403 #define MPIOAPIC  0x02  // One per I/O APIC
7404 #define MPIOINTR  0x03  // One per bus interrupt source
7405 #define MPLINTR   0x04  // One per system interrupt source
7406 
7407 
7408 
7409 
7410 
7411 
7412 
7413 
7414 
7415 
7416 
7417 
7418 
7419 
7420 
7421 
7422 
7423 
7424 
7425 
7426 
7427 
7428 
7429 
7430 
7431 
7432 
7433 
7434 
7435 
7436 
7437 
7438 
7439 
7440 
7441 
7442 
7443 
7444 
7445 
7446 
7447 
7448 
7449 
7450 // Blank page.
7451 
7452 
7453 
7454 
7455 
7456 
7457 
7458 
7459 
7460 
7461 
7462 
7463 
7464 
7465 
7466 
7467 
7468 
7469 
7470 
7471 
7472 
7473 
7474 
7475 
7476 
7477 
7478 
7479 
7480 
7481 
7482 
7483 
7484 
7485 
7486 
7487 
7488 
7489 
7490 
7491 
7492 
7493 
7494 
7495 
7496 
7497 
7498 
7499 
