2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 #include "rand.h"  //cs202
2409 
2410 struct {
2411   struct spinlock lock;
2412   struct proc proc[NPROC];
2413 } ptable;
2414 
2415 static struct proc *initproc;
2416 
2417 int nextpid = 1;
2418 //  cs202
2419 const int stride1 = (1<<20);
2420 extern void forkret(void);
2421 extern void trapret(void);
2422 
2423 // int pagecount = 0;
2424 
2425 static void wakeup1(void *chan);
2426 
2427 void
2428 pinit(void)
2429 {
2430   initlock(&ptable.lock, "ptable");
2431 }
2432 
2433 // Must be called with interrupts disabled
2434 int
2435 cpuid() {
2436   return mycpu()-cpus;
2437 }
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Must be called with interrupts disabled to avoid the caller being
2451 // rescheduled between reading lapicid and running through the loop.
2452 struct cpu*
2453 mycpu(void)
2454 {
2455   int apicid, i;
2456 
2457   if(readeflags()&FL_IF)
2458     panic("mycpu called with interrupts enabled\n");
2459 
2460   apicid = lapicid();
2461   // APIC IDs are not guaranteed to be contiguous. Maybe we should have
2462   // a reverse map, or reserve a register to store &cpus[i].
2463   for (i = 0; i < ncpu; ++i) {
2464     if (cpus[i].apicid == apicid)
2465       return &cpus[i];
2466   }
2467   panic("unknown apicid\n");
2468 }
2469 
2470 // Disable interrupts so that we are not rescheduled
2471 // while reading proc from the cpu structure
2472 struct proc*
2473 myproc(void) {
2474   struct cpu *c;
2475   struct proc *p;
2476   pushcli();
2477   c = mycpu();
2478   p = c->proc;
2479   popcli();
2480   return p;
2481 }
2482 
2483 
2484 
2485 
2486 
2487 
2488 
2489 
2490 
2491 
2492 
2493 
2494 
2495 
2496 
2497 
2498 
2499 
2500 // Look in the process table for an UNUSED proc.
2501 // If found, change state to EMBRYO and initialize
2502 // state required to run in the kernel.
2503 // Otherwise return 0.
2504 static struct proc*
2505 allocproc(void)
2506 {
2507   struct proc *p;
2508   char *sp;
2509 
2510   acquire(&ptable.lock);
2511 
2512   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2513     if(p->state == UNUSED)
2514       goto found;
2515 
2516   release(&ptable.lock);
2517   return 0;
2518 
2519 found:
2520   p->state = EMBRYO;
2521   p->pid = nextpid++;
2522   // cs202
2523   p->tickets = 10;
2524   p->syscallcount = 0;
2525   p->stride = stride1 / p->tickets;
2526   p->pass = p->stride;
2527   p->runtimes = 0;
2528   // cs202
2529 
2530 
2531   release(&ptable.lock);
2532 
2533   // Allocate kernel stack.
2534   if((p->kstack = kalloc()) == 0){
2535     p->state = UNUSED;
2536     return 0;
2537   }
2538   sp = p->kstack + KSTACKSIZE;
2539 
2540   // Leave room for trap frame.
2541   sp -= sizeof *p->tf;
2542   p->tf = (struct trapframe*)sp;
2543 
2544   // Set up new context to start executing at forkret,
2545   // which returns to trapret.
2546   sp -= 4;
2547   *(uint*)sp = (uint)trapret;
2548 
2549 
2550   sp -= sizeof *p->context;
2551   p->context = (struct context*)sp;
2552   memset(p->context, 0, sizeof *p->context);
2553   p->context->eip = (uint)forkret;
2554 
2555   return p;
2556 }
2557 
2558 
2559 // Set up first user process.
2560 void
2561 userinit(void)
2562 {
2563   struct proc *p;
2564   extern char _binary_initcode_start[], _binary_initcode_size[];
2565 
2566   p = allocproc();
2567 
2568   initproc = p;
2569   if((p->pgdir = setupkvm()) == 0)
2570     panic("userinit: out of memory?");
2571   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2572   p->sz = PGSIZE;
2573   memset(p->tf, 0, sizeof(*p->tf));
2574   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2575   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2576   p->tf->es = p->tf->ds;
2577   p->tf->ss = p->tf->ds;
2578   p->tf->eflags = FL_IF;
2579   p->tf->esp = PGSIZE;
2580   p->tf->eip = 0;  // beginning of initcode.S
2581 
2582   safestrcpy(p->name, "initcode", sizeof(p->name));
2583   p->cwd = namei("/");
2584 
2585   // this assignment to p->state lets other cores
2586   // run this process. the acquire forces the above
2587   // writes to be visible, and the lock is also needed
2588   // because the assignment might not be atomic.
2589   acquire(&ptable.lock);
2590 
2591   p->state = RUNNABLE;
2592 
2593   release(&ptable.lock);
2594 }
2595 
2596 
2597 
2598 
2599 
2600 // Grow current process's memory by n bytes.
2601 // Return 0 on success, -1 on failure.
2602 int
2603 growproc(int n)
2604 {
2605   uint sz;
2606   struct proc *curproc = myproc();
2607 
2608   sz = curproc->sz;
2609   if(n > 0){
2610     if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
2611       return -1;
2612   } else if(n < 0){
2613     if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
2614       return -1;
2615   }
2616   curproc->sz = sz;
2617   switchuvm(curproc);
2618   return 0;
2619 }
2620 
2621 // Create a new process copying p as the parent.
2622 // Sets up stack to return as if from system call.
2623 // Caller must set state of returned proc to RUNNABLE.
2624 int
2625 fork(void)
2626 {
2627   int i, pid;
2628   struct proc *np;
2629   struct proc *curproc = myproc();
2630 
2631   // Allocate process.
2632   if((np = allocproc()) == 0){
2633     return -1;
2634   }
2635 
2636   // Copy process state from proc.
2637   if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
2638     kfree(np->kstack);
2639     np->kstack = 0;
2640     np->state = UNUSED;
2641     return -1;
2642   }
2643   np->sz = curproc->sz;
2644   np->parent = curproc;
2645   *np->tf = *curproc->tf;
2646 
2647   // Clear %eax so that fork returns 0 in the child.
2648   np->tf->eax = 0;
2649 
2650   for(i = 0; i < NOFILE; i++)
2651     if(curproc->ofile[i])
2652       np->ofile[i] = filedup(curproc->ofile[i]);
2653   np->cwd = idup(curproc->cwd);
2654 
2655   safestrcpy(np->name, curproc->name, sizeof(curproc->name));
2656 
2657   pid = np->pid;
2658 
2659   acquire(&ptable.lock);
2660 
2661   np->state = RUNNABLE;
2662 
2663   release(&ptable.lock);
2664 
2665   return pid;
2666 }
2667 
2668 // Exit the current process.  Does not return.
2669 // An exited process remains in the zombie state
2670 // until its parent calls wait() to find out it exited.
2671 void
2672 exit(void)
2673 {
2674   struct proc *curproc = myproc();
2675   struct proc *p;
2676   int fd;
2677 
2678   if(curproc == initproc)
2679     panic("init exiting");
2680 
2681   // Close all open files.
2682   for(fd = 0; fd < NOFILE; fd++){
2683     if(curproc->ofile[fd]){
2684       fileclose(curproc->ofile[fd]);
2685       curproc->ofile[fd] = 0;
2686     }
2687   }
2688 
2689   begin_op();
2690   iput(curproc->cwd);
2691   end_op();
2692   curproc->cwd = 0;
2693 
2694   acquire(&ptable.lock);
2695 
2696   // Parent might be sleeping in wait().
2697   wakeup1(curproc->parent);
2698 
2699 
2700   // Pass abandoned children to init.
2701   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2702     if(p->parent == curproc){
2703       p->parent = initproc;
2704       if(p->state == ZOMBIE)
2705         wakeup1(initproc);
2706     }
2707   }
2708 
2709   // Jump into the scheduler, never to return.
2710   curproc->state = ZOMBIE;
2711   sched();
2712   panic("zombie exit");
2713 }
2714 
2715 // Wait for a child process to exit and return its pid.
2716 // Return -1 if this process has no children.
2717 int
2718 wait(void)
2719 {
2720   struct proc *p;
2721   int havekids, pid;
2722   struct proc *curproc = myproc();
2723 
2724   acquire(&ptable.lock);
2725   for(;;){
2726     // Scan through table looking for exited children.
2727     havekids = 0;
2728     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2729       if(p->parent != curproc)
2730         continue;
2731       havekids = 1;
2732       if(p->state == ZOMBIE){
2733         // Found one.
2734         pid = p->pid;
2735         kfree(p->kstack);
2736         p->kstack = 0;
2737         freevm(p->pgdir);
2738         p->pid = 0;
2739         p->parent = 0;
2740         p->name[0] = 0;
2741         p->killed = 0;
2742         p->state = UNUSED;
2743         release(&ptable.lock);
2744         return pid;
2745       }
2746     }
2747 
2748 
2749 
2750     // No point waiting if we don't have any children.
2751     if(!havekids || curproc->killed){
2752       release(&ptable.lock);
2753       return -1;
2754     }
2755 
2756     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2757     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
2758   }
2759 }
2760 
2761 
2762 
2763 
2764 
2765 
2766 
2767 
2768 
2769 
2770 
2771 
2772 
2773 
2774 
2775 
2776 
2777 
2778 
2779 
2780 
2781 
2782 
2783 
2784 
2785 
2786 
2787 
2788 
2789 
2790 
2791 
2792 
2793 
2794 
2795 
2796 
2797 
2798 
2799 
2800 // Per-CPU process scheduler.
2801 // Each CPU calls scheduler() after setting itself up.
2802 // Scheduler never returns.  It loops, doing:
2803 //  - choose a process to run
2804 //  - swtch to start running that process
2805 //  - eventually that process transfers control
2806 //      via swtch back to the scheduler.
2807 void
2808 scheduler(void)
2809 {
2810   struct proc *p = initproc;
2811   struct cpu *c = mycpu();
2812   c->proc = 0;
2813 
2814   // stride scheduling
2815   // struct proc *tempp; // cs202
2816 
2817   for(;;){
2818     // Enable interrupts on this processor.
2819     sti();
2820 
2821     // Loop over process table looking for process to run.
2822     acquire(&ptable.lock);
2823 
2824     // cs202
2825 
2826     // stride scheduling
2827     /*
2828     int minpass = -1;
2829 
2830     for(tempp = ptable.proc; tempp < &ptable.proc[NPROC]; tempp++){
2831 
2832       if(tempp->state != RUNNABLE){
2833         continue;
2834       }
2835 
2836       if( (minpass < 0) | (tempp->pass < minpass)){
2837         minpass = tempp->pass;
2838         p = tempp;
2839       }
2840 
2841     }
2842     // Switch to chosen process.  It is the process's job
2843     // to release ptable.lock and then reacquire it
2844     // before jumping back to us.
2845     c->proc = p;
2846 
2847 
2848 
2849 
2850     switchuvm(p);
2851     p->state = RUNNING;
2852     if (p->pid > 2)
2853       p->runtimes ++;
2854 
2855     swtch(&(c->scheduler), p->context);
2856     switchkvm();
2857 
2858     // Process is done running for now.
2859     // It should have changed its p->state before coming back.
2860     c->proc = 0;
2861     */
2862 
2863 
2864     // lottery scheduling
2865 
2866     int tickcount = 0;  //the count of all ticket of processes
2867     int ticksum = 0;  //the sum of tickets of checked processes
2868 
2869 
2870 
2871     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2872       if (p->state == RUNNABLE){
2873         tickcount += p->tickets;
2874       }
2875     }
2876 
2877     long winner = random_at_most(tickcount);
2878     // lottery scheduling
2879 
2880     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2881 
2882       if(p->state != RUNNABLE)
2883         continue;
2884 
2885       // cs202
2886 
2887 
2888       ticksum += p->tickets;
2889 
2890       if(ticksum < winner)
2891         continue;
2892 
2893 
2894 
2895 
2896       // Switch to chosen process.  It is the process's job
2897       // to release ptable.lock and then reacquire it
2898       // before jumping back to us.
2899       c->proc = p;
2900       switchuvm(p);
2901       p->state = RUNNING;
2902       if (p->pid > 2)
2903         p->runtimes ++;
2904 
2905 
2906       swtch(&(c->scheduler), p->context);
2907       switchkvm();
2908 
2909       // Process is done running for now.
2910       // It should have changed its p->state before coming back.
2911       c->proc = 0;
2912       break;
2913     }
2914 
2915 
2916     release(&ptable.lock);
2917 
2918   }
2919 }
2920 
2921 // Enter scheduler.  Must hold only ptable.lock
2922 // and have changed proc->state. Saves and restores
2923 // intena because intena is a property of this
2924 // kernel thread, not this CPU. It should
2925 // be proc->intena and proc->ncli, but that would
2926 // break in the few places where a lock is held but
2927 // there's no process.
2928 void
2929 sched(void)
2930 {
2931   int intena;
2932   struct proc *p = myproc();
2933 
2934   if(!holding(&ptable.lock))
2935     panic("sched ptable.lock");
2936   if(mycpu()->ncli != 1)
2937     panic("sched locks");
2938   if(p->state == RUNNING)
2939     panic("sched running");
2940   if(readeflags()&FL_IF)
2941     panic("sched interruptible");
2942   intena = mycpu()->intena;
2943   swtch(&p->context, mycpu()->scheduler);
2944   mycpu()->intena = intena;
2945 }
2946 
2947 
2948 
2949 
2950 // Give up the CPU for one scheduling round.
2951 void
2952 yield(void)
2953 {
2954   acquire(&ptable.lock);  //DOC: yieldlock
2955   myproc()->state = RUNNABLE;
2956   myproc()->pass += myproc()->stride; // cs202
2957   sched();
2958   release(&ptable.lock);
2959 }
2960 
2961 // A fork child's very first scheduling by scheduler()
2962 // will swtch here.  "Return" to user space.
2963 void
2964 forkret(void)
2965 {
2966   static int first = 1;
2967   // Still holding ptable.lock from scheduler.
2968   release(&ptable.lock);
2969 
2970   if (first) {
2971     // Some initialization functions must be run in the context
2972     // of a regular process (e.g., they call sleep), and thus cannot
2973     // be run from main().
2974     first = 0;
2975     iinit(ROOTDEV);
2976     initlog(ROOTDEV);
2977   }
2978 
2979   // Return to "caller", actually trapret (see allocproc).
2980 }
2981 
2982 // Atomically release lock and sleep on chan.
2983 // Reacquires lock when awakened.
2984 void
2985 sleep(void *chan, struct spinlock *lk)
2986 {
2987   struct proc *p = myproc();
2988 
2989   if(p == 0)
2990     panic("sleep");
2991 
2992   if(lk == 0)
2993     panic("sleep without lk");
2994 
2995   // Must acquire ptable.lock in order to
2996   // change p->state and then call sched.
2997   // Once we hold ptable.lock, we can be
2998   // guaranteed that we won't miss any wakeup
2999   // (wakeup runs with ptable.lock locked),
3000   // so it's okay to release lk.
3001   if(lk != &ptable.lock){  //DOC: sleeplock0
3002     acquire(&ptable.lock);  //DOC: sleeplock1
3003     release(lk);
3004   }
3005   // Go to sleep.
3006   p->chan = chan;
3007   p->state = SLEEPING;
3008 
3009   sched();
3010 
3011   // Tidy up.
3012   p->chan = 0;
3013 
3014   // Reacquire original lock.
3015   if(lk != &ptable.lock){  //DOC: sleeplock2
3016     release(&ptable.lock);
3017     acquire(lk);
3018   }
3019 }
3020 
3021 
3022 
3023 
3024 
3025 
3026 
3027 
3028 
3029 
3030 
3031 
3032 
3033 
3034 
3035 
3036 
3037 
3038 
3039 
3040 
3041 
3042 
3043 
3044 
3045 
3046 
3047 
3048 
3049 
3050 // Wake up all processes sleeping on chan.
3051 // The ptable lock must be held.
3052 static void
3053 wakeup1(void *chan)
3054 {
3055   struct proc *p;
3056 
3057   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
3058     if(p->state == SLEEPING && p->chan == chan)
3059       p->state = RUNNABLE;
3060 }
3061 
3062 // Wake up all processes sleeping on chan.
3063 void
3064 wakeup(void *chan)
3065 {
3066   acquire(&ptable.lock);
3067   wakeup1(chan);
3068   release(&ptable.lock);
3069 }
3070 
3071 // Kill the process with the given pid.
3072 // Process won't exit until it returns
3073 // to user space (see trap in trap.c).
3074 int
3075 kill(int pid)
3076 {
3077   struct proc *p;
3078 
3079   acquire(&ptable.lock);
3080   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3081     if(p->pid == pid){
3082       p->killed = 1;
3083       // Wake process from sleep if necessary.
3084       if(p->state == SLEEPING)
3085         p->state = RUNNABLE;
3086       release(&ptable.lock);
3087       return 0;
3088     }
3089   }
3090   release(&ptable.lock);
3091   return -1;
3092 }
3093 
3094 
3095 
3096 
3097 
3098 
3099 
3100 // Print a process listing to console.  For debugging.
3101 // Runs when user types ^P on console.
3102 // No lock to avoid wedging a stuck machine further.
3103 void
3104 procdump(void)
3105 {
3106   static char *states[] = {
3107   [UNUSED]    "unused",
3108   [EMBRYO]    "embryo",
3109   [SLEEPING]  "sleep ",
3110   [RUNNABLE]  "runble",
3111   [RUNNING]   "run   ",
3112   [ZOMBIE]    "zombie"
3113   };
3114   int i;
3115   struct proc *p;
3116   char *state;
3117   uint pc[10];
3118 
3119   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3120     if(p->state == UNUSED)
3121       continue;
3122     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
3123       state = states[p->state];
3124     else
3125       state = "???";
3126     cprintf("%d %s %s", p->pid, state, p->name);
3127     if(p->state == SLEEPING){
3128       getcallerpcs((uint*)p->context->ebp+2, pc);
3129       for(i=0; i<10 && pc[i] != 0; i++)
3130         cprintf(" %p", pc[i]);
3131     }
3132     cprintf("\n");
3133   }
3134 }
3135 
3136 
3137 
3138 
3139 
3140 
3141 
3142 
3143 
3144 
3145 
3146 
3147 
3148 
3149 
3150 // cs202
3151 int
3152 info(int infotype)
3153 {
3154 
3155   int count = 0;
3156   struct proc *p;
3157   struct proc *curproc = myproc();
3158 
3159   if(infotype == 1){
3160     acquire(&ptable.lock);
3161 
3162     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
3163       if((p->state == RUNNING) | (p->state == RUNNABLE))
3164         count++;
3165 
3166     release(&ptable.lock);
3167 
3168     cprintf("\n  processes count: %d\n\n", count);
3169 
3170     return 0;
3171   }
3172 
3173   if(infotype == 2){
3174 
3175 
3176     cprintf("\n  system calls count: %d\n\n", curproc->syscallcount);
3177 
3178     return 0;
3179   }
3180 
3181   if(infotype == 3){
3182 
3183     int pagecount = curproc->sz / PGSIZE;
3184 
3185     // cprintf("\n\n Welcome to the %dth kernel space! \n\n", infotype);
3186     cprintf("\n  pages count: %d\n\n", pagecount);
3187 
3188     return 0;
3189   }
3190 
3191   if(infotype == 4){
3192 
3193     cprintf("\n--------------------------------------------------\n");
3194     acquire(&ptable.lock);
3195 
3196 
3197 
3198 
3199 
3200     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3201       if((p->state == UNUSED) | (p->pid < 3))
3202         continue;
3203       cprintf("\n#%dth process with %d tickets has run for %d times\n", p->pid, p->tickets, p->runtimes);
3204     }
3205 
3206     release(&ptable.lock);
3207     cprintf("\n--------------------------------------------------\n");
3208 
3209     return -1;
3210 
3211   }
3212 
3213   return -1;
3214 }
3215 
3216 int
3217 settickets(int tickets)
3218 {
3219 
3220   struct proc *curproc = myproc();
3221 
3222   curproc->tickets = tickets;
3223   curproc->stride = stride1 / tickets;
3224 
3225   cprintf("\n  assign %d tickets to current process\n", curproc->tickets);
3226 
3227   return 0;
3228 }
3229 // cs202
3230 
3231 
3232 
3233 
3234 
3235 
3236 
3237 
3238 
3239 
3240 
3241 
3242 
3243 
3244 
3245 
3246 
3247 
3248 
3249 
