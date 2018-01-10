// Some useful defines

#define NULL_BYTE 0x00
#define SCN_BYTES 128
#define SCN_COL   4
#define SCN_ROW   32

// putbyte options
#define PUT_INC  0xc0    // write and increment
#define PUT_NIL  0xc4    // write no increment
#define PUT_DEC  0xc2    // write and decrement

// getbyte options
#define GET_INC  0xc1    // write and increment
#define GET_NIL  0xc5    // write no increment
#define GET_DEC  0xc3    // write and decrement

// setmode options
#define MODE_OR  0x80
#define MODE_XOR 0x81
#define MODE_AND 0x82
//#define MODE_AND 0x83

// Primatives
void setmode(char cmd);
void sethome(int addr);
void putbyte(char b, char cmd);
void setpage(int npage);
void setaddr(int addr);



void clrscr(void);
void setpixel(int x1, int y1);
void setchar(int x1, int y1, char c);
void setstring(int x1, int y1, char *ps);

void circle(int x, int y, int l);







