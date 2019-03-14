8150 #include "types.h"
8151 #include "x86.h"
8152 #include "defs.h"
8153 #include "kbd.h"
8154 
8155 int
8156 kbdgetc(void)
8157 {
8158   static uint shift;
8159   static uchar *charcode[4] = {
8160     normalmap, shiftmap, ctlmap, ctlmap
8161   };
8162   uint st, data, c;
8163 
8164   st = inb(KBSTATP);
8165   if((st & KBS_DIB) == 0)
8166     return -1;
8167   data = inb(KBDATAP);
8168 
8169   if(data == 0xE0){
8170     shift |= E0ESC;
8171     return 0;
8172   } else if(data & 0x80){
8173     // Key released
8174     data = (shift & E0ESC ? data : data & 0x7F);
8175     shift &= ~(shiftcode[data] | E0ESC);
8176     return 0;
8177   } else if(shift & E0ESC){
8178     // Last character was an E0 escape; or with 0x80
8179     data |= 0x80;
8180     shift &= ~E0ESC;
8181   }
8182 
8183   shift |= shiftcode[data];
8184   shift ^= togglecode[data];
8185   c = charcode[shift & (CTL | SHIFT)][data];
8186   if(shift & CAPSLOCK){
8187     if('a' <= c && c <= 'z')
8188       c += 'A' - 'a';
8189     else if('A' <= c && c <= 'Z')
8190       c += 'a' - 'A';
8191   }
8192   return c;
8193 }
8194 
8195 void
8196 kbdintr(void)
8197 {
8198   consoleintr(kbdgetc);
8199 }
