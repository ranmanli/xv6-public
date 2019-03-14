7200 #include "types.h"
7201 #include "x86.h"
7202 
7203 void*
7204 memset(void *dst, int c, uint n)
7205 {
7206   if ((int)dst%4 == 0 && n%4 == 0){
7207     c &= 0xFF;
7208     stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
7209   } else
7210     stosb(dst, c, n);
7211   return dst;
7212 }
7213 
7214 int
7215 memcmp(const void *v1, const void *v2, uint n)
7216 {
7217   const uchar *s1, *s2;
7218 
7219   s1 = v1;
7220   s2 = v2;
7221   while(n-- > 0){
7222     if(*s1 != *s2)
7223       return *s1 - *s2;
7224     s1++, s2++;
7225   }
7226 
7227   return 0;
7228 }
7229 
7230 void*
7231 memmove(void *dst, const void *src, uint n)
7232 {
7233   const char *s;
7234   char *d;
7235 
7236   s = src;
7237   d = dst;
7238   if(s < d && s + n > d){
7239     s += n;
7240     d += n;
7241     while(n-- > 0)
7242       *--d = *--s;
7243   } else
7244     while(n-- > 0)
7245       *d++ = *s++;
7246 
7247   return dst;
7248 }
7249 
7250 // memcpy exists to placate GCC.  Use memmove.
7251 void*
7252 memcpy(void *dst, const void *src, uint n)
7253 {
7254   return memmove(dst, src, n);
7255 }
7256 
7257 int
7258 strncmp(const char *p, const char *q, uint n)
7259 {
7260   while(n > 0 && *p && *p == *q)
7261     n--, p++, q++;
7262   if(n == 0)
7263     return 0;
7264   return (uchar)*p - (uchar)*q;
7265 }
7266 
7267 char*
7268 strncpy(char *s, const char *t, int n)
7269 {
7270   char *os;
7271 
7272   os = s;
7273   while(n-- > 0 && (*s++ = *t++) != 0)
7274     ;
7275   while(n-- > 0)
7276     *s++ = 0;
7277   return os;
7278 }
7279 
7280 // Like strncpy but guaranteed to NUL-terminate.
7281 char*
7282 safestrcpy(char *s, const char *t, int n)
7283 {
7284   char *os;
7285 
7286   os = s;
7287   if(n <= 0)
7288     return os;
7289   while(--n > 0 && (*s++ = *t++) != 0)
7290     ;
7291   *s = 0;
7292   return os;
7293 }
7294 
7295 
7296 
7297 
7298 
7299 
7300 int
7301 strlen(const char *s)
7302 {
7303   int n;
7304 
7305   for(n = 0; s[n]; n++)
7306     ;
7307   return n;
7308 }
7309 
7310 
7311 
7312 
7313 
7314 
7315 
7316 
7317 
7318 
7319 
7320 
7321 
7322 
7323 
7324 
7325 
7326 
7327 
7328 
7329 
7330 
7331 
7332 
7333 
7334 
7335 
7336 
7337 
7338 
7339 
7340 
7341 
7342 
7343 
7344 
7345 
7346 
7347 
7348 
7349 
