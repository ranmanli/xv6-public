3450 // x86 trap and interrupt constants.
3451 
3452 // Processor-defined:
3453 #define T_DIVIDE         0      // divide error
3454 #define T_DEBUG          1      // debug exception
3455 #define T_NMI            2      // non-maskable interrupt
3456 #define T_BRKPT          3      // breakpoint
3457 #define T_OFLOW          4      // overflow
3458 #define T_BOUND          5      // bounds check
3459 #define T_ILLOP          6      // illegal opcode
3460 #define T_DEVICE         7      // device not available
3461 #define T_DBLFLT         8      // double fault
3462 // #define T_COPROC      9      // reserved (not used since 486)
3463 #define T_TSS           10      // invalid task switch segment
3464 #define T_SEGNP         11      // segment not present
3465 #define T_STACK         12      // stack exception
3466 #define T_GPFLT         13      // general protection fault
3467 #define T_PGFLT         14      // page fault
3468 // #define T_RES        15      // reserved
3469 #define T_FPERR         16      // floating point error
3470 #define T_ALIGN         17      // aligment check
3471 #define T_MCHK          18      // machine check
3472 #define T_SIMDERR       19      // SIMD floating point error
3473 
3474 // These are arbitrarily chosen, but with care not to overlap
3475 // processor defined exceptions or interrupt vectors.
3476 #define T_SYSCALL       64      // system call
3477 #define T_DEFAULT      500      // catchall
3478 
3479 #define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ
3480 
3481 #define IRQ_TIMER        0
3482 #define IRQ_KBD          1
3483 #define IRQ_COM1         4
3484 #define IRQ_IDE         14
3485 #define IRQ_ERROR       19
3486 #define IRQ_SPURIOUS    31
3487 
3488 
3489 
3490 
3491 
3492 
3493 
3494 
3495 
3496 
3497 
3498 
3499 
