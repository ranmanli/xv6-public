4200 // Long-term locks for processes
4201 struct sleeplock {
4202   uint locked;       // Is the lock held?
4203   struct spinlock lk; // spinlock protecting this sleep lock
4204 
4205   // For debugging:
4206   char *name;        // Name of lock.
4207   int pid;           // Process holding lock
4208 };
4209 
4210 
4211 
4212 
4213 
4214 
4215 
4216 
4217 
4218 
4219 
4220 
4221 
4222 
4223 
4224 
4225 
4226 
4227 
4228 
4229 
4230 
4231 
4232 
4233 
4234 
4235 
4236 
4237 
4238 
4239 
4240 
4241 
4242 
4243 
4244 
4245 
4246 
4247 
4248 
4249 
