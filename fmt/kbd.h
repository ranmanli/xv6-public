8000 // PC keyboard interface constants
8001 
8002 #define KBSTATP         0x64    // kbd controller status port(I)
8003 #define KBS_DIB         0x01    // kbd data in buffer
8004 #define KBDATAP         0x60    // kbd data port(I)
8005 
8006 #define NO              0
8007 
8008 #define SHIFT           (1<<0)
8009 #define CTL             (1<<1)
8010 #define ALT             (1<<2)
8011 
8012 #define CAPSLOCK        (1<<3)
8013 #define NUMLOCK         (1<<4)
8014 #define SCROLLLOCK      (1<<5)
8015 
8016 #define E0ESC           (1<<6)
8017 
8018 // Special keycodes
8019 #define KEY_HOME        0xE0
8020 #define KEY_END         0xE1
8021 #define KEY_UP          0xE2
8022 #define KEY_DN          0xE3
8023 #define KEY_LF          0xE4
8024 #define KEY_RT          0xE5
8025 #define KEY_PGUP        0xE6
8026 #define KEY_PGDN        0xE7
8027 #define KEY_INS         0xE8
8028 #define KEY_DEL         0xE9
8029 
8030 // C('A') == Control-A
8031 #define C(x) (x - '@')
8032 
8033 static uchar shiftcode[256] =
8034 {
8035   [0x1D] CTL,
8036   [0x2A] SHIFT,
8037   [0x36] SHIFT,
8038   [0x38] ALT,
8039   [0x9D] CTL,
8040   [0xB8] ALT
8041 };
8042 
8043 static uchar togglecode[256] =
8044 {
8045   [0x3A] CAPSLOCK,
8046   [0x45] NUMLOCK,
8047   [0x46] SCROLLLOCK
8048 };
8049 
8050 static uchar normalmap[256] =
8051 {
8052   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
8053   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
8054   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
8055   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
8056   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
8057   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
8058   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
8059   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
8060   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
8061   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
8062   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
8063   [0x9C] '\n',      // KP_Enter
8064   [0xB5] '/',       // KP_Div
8065   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8066   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8067   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8068   [0x97] KEY_HOME,  [0xCF] KEY_END,
8069   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8070 };
8071 
8072 static uchar shiftmap[256] =
8073 {
8074   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
8075   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
8076   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
8077   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
8078   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
8079   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
8080   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
8081   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
8082   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
8083   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
8084   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
8085   [0x9C] '\n',      // KP_Enter
8086   [0xB5] '/',       // KP_Div
8087   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8088   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8089   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8090   [0x97] KEY_HOME,  [0xCF] KEY_END,
8091   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8092 };
8093 
8094 
8095 
8096 
8097 
8098 
8099 
8100 static uchar ctlmap[256] =
8101 {
8102   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
8103   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
8104   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
8105   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
8106   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
8107   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
8108   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
8109   [0x9C] '\r',      // KP_Enter
8110   [0xB5] C('/'),    // KP_Div
8111   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8112   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8113   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8114   [0x97] KEY_HOME,  [0xCF] KEY_END,
8115   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8116 };
8117 
8118 
8119 
8120 
8121 
8122 
8123 
8124 
8125 
8126 
8127 
8128 
8129 
8130 
8131 
8132 
8133 
8134 
8135 
8136 
8137 
8138 
8139 
8140 
8141 
8142 
8143 
8144 
8145 
8146 
8147 
8148 
8149 
