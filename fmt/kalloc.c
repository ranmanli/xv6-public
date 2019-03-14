3300 // Physical memory allocator, intended to allocate
3301 // memory for user processes, kernel stacks, page table pages,
3302 // and pipe buffers. Allocates 4096-byte pages.
3303 
3304 #include "types.h"
3305 #include "defs.h"
3306 #include "param.h"
3307 #include "memlayout.h"
3308 #include "mmu.h"
3309 #include "spinlock.h"
3310 
3311 void freerange(void *vstart, void *vend);
3312 extern char end[]; // first address after kernel loaded from ELF file
3313                    // defined by the kernel linker script in kernel.ld
3314 
3315 struct run {
3316   struct run *next;
3317 };
3318 
3319 struct {
3320   struct spinlock lock;
3321   int use_lock;
3322   struct run *freelist;
3323 } kmem;
3324 
3325 // Initialization happens in two phases.
3326 // 1. main() calls kinit1() while still using entrypgdir to place just
3327 // the pages mapped by entrypgdir on free list.
3328 // 2. main() calls kinit2() with the rest of the physical pages
3329 // after installing a full page table that maps them on all cores.
3330 void
3331 kinit1(void *vstart, void *vend)
3332 {
3333   initlock(&kmem.lock, "kmem");
3334   kmem.use_lock = 0;
3335   freerange(vstart, vend);
3336 }
3337 
3338 void
3339 kinit2(void *vstart, void *vend)
3340 {
3341   freerange(vstart, vend);
3342   kmem.use_lock = 1;
3343 }
3344 
3345 
3346 
3347 
3348 
3349 
3350 void
3351 freerange(void *vstart, void *vend)
3352 {
3353   char *p;
3354   p = (char*)PGROUNDUP((uint)vstart);
3355   for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
3356     kfree(p);
3357 }
3358 
3359 // Free the page of physical memory pointed at by v,
3360 // which normally should have been returned by a
3361 // call to kalloc().  (The exception is when
3362 // initializing the allocator; see kinit above.)
3363 void
3364 kfree(char *v)
3365 {
3366   struct run *r;
3367 
3368   if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
3369     panic("kfree");
3370 
3371   // Fill with junk to catch dangling refs.
3372   memset(v, 1, PGSIZE);
3373 
3374   if(kmem.use_lock)
3375     acquire(&kmem.lock);
3376   r = (struct run*)v;
3377   r->next = kmem.freelist;
3378   kmem.freelist = r;
3379   if(kmem.use_lock)
3380     release(&kmem.lock);
3381 
3382 }
3383 
3384 // Allocate one 4096-byte page of physical memory.
3385 // Returns a pointer that the kernel can use.
3386 // Returns 0 if the memory cannot be allocated.
3387 char*
3388 kalloc(void)
3389 {
3390 
3391   struct run *r;
3392 
3393   if(kmem.use_lock)
3394     acquire(&kmem.lock);
3395   r = kmem.freelist;
3396   if(r)
3397     kmem.freelist = r->next;
3398   if(kmem.use_lock)
3399     release(&kmem.lock);
3400   return (char*)r;
3401 }
3402 
3403 
3404 
3405 
3406 
3407 
3408 
3409 
3410 
3411 
3412 
3413 
3414 
3415 
3416 
3417 
3418 
3419 
3420 
3421 
3422 
3423 
3424 
3425 
3426 
3427 
3428 
3429 
3430 
3431 
3432 
3433 
3434 
3435 
3436 
3437 
3438 
3439 
3440 
3441 
3442 
3443 
3444 
3445 
3446 
3447 
3448 
3449 
