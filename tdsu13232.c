#include <iogp.h>
#include "def.h"
#include "mgls128128.h"
#include "utrig.h"
#include "umath.h"

int g_page; //0x0000 or 0x1000

//-----------------------------------------------------------------------------
//     D3  D2  D1  D0
//     __   _  __  __
//     CE C/D  WR  RD
//
//      7   8   5   6
//

void wrdata(char b)
{
  DDRB = 0xff;                   // all bits output
  DDRD = 0xff;                   // all bits output

  PORTB = b;
  PORTD = 0x09;  // 1001
  PORTD = 0x01;  // 0001
  PORTD = 0x09;  // 1001
}

void wrcmd(char b)
{
  DDRB = 0xff;                   // all bits output
  DDRD = 0xff;                   // all bits output

  PORTB = b;
  PORTD = 0x0d;  // 1101
  PORTD = 0x05;  // 0101
  PORTD = 0x0d;  // 1101
}

void rddata(char *b)
{
  DDRB = 0x00;                   // all bits input
  DDRD = 0xff;                   // all bits output

  PORTD = 0x02;  // 0010
  //delay();
  *b = PORTB;
  PORTD = 0x10;  // 1010

}

void rdstat(char *b)
{
  DDRB = 0x00;                   // all bits input
  DDRD = 0xff;                   // all bits output

  PORTD = 0x06;  // 0110
  //delay();
  *b = PORTB;
  PORTD = 0x0e;  // 1110
}

void chkstat(void)
{
  char b;

  b = 1;
  while (b & 0x03 != 0x03)
    rdstat(&b);
}

void sethome(int addr)
{
  chkstat();
  wrdata(LOBYTE(addr));
  chkstat();
  wrdata(HIBYTE(addr));
  chkstat();
  wrcmd(0x42);
}

BOOL initdsp(void)
{
  //graphics home address 0x0000
  chkstat();
  wrdata(0x00);
  chkstat();
  wrdata(0x00);
  chkstat();
  wrcmd(0x42);

  //graphics area set
  chkstat();
  wrdata(16);  //128 / 8 = 16
  chkstat();
  wrdata(0x00);
  chkstat();
  wrcmd(0x43);

  //text home address 0x0800
  chkstat();
  wrdata(0x00);
  chkstat();
  wrdata(0x08);
  chkstat();
  wrcmd(0x40);

  //text area set
  chkstat();
  wrdata(16);  //128 / 8 = 16
  chkstat();
  wrdata(0x00);
  chkstat();
  wrcmd(0x41);

  //mode set - This does not appear to have any effect ??
  chkstat();
  wrcmd(0x82); // 82 AND mode 0x80 OR mode 0x81 XOR

  //display mode
  chkstat();
  wrcmd(0x9e); //0x98 = only GRPH is on 0x9e = GRPH,TXT,CUR

  g_page = 0;
}

void setmode(char cmd)
{
  //mode set
  chkstat();
  wrcmd(cmd); // 82 AND mode 0x80 OR mode
}

void setaddr(int addr)
{
  chkstat();
  wrdata(LOBYTE(addr));
  chkstat();
  wrdata(HIBYTE(addr));
  chkstat();
  wrcmd(0x24);  // adress pointer
}

void putbyte(char b, char cmd)
{
  chkstat();
  wrdata(b);
  chkstat();
  wrcmd(cmd);
}

void getbyte(char *b, char cmd)
{
  chkstat();
  wrcmd(cmd);
  chkstat();
  rddata(b);
}

void clrscr(void)
{
  unsigned int i;

  setaddr(g_page);
  for (i = 0; i < SCN_BYTES + 0x100; i++) {
    putbyte(0x00, PUT_INC);
  }
}

void setpage(int npage)
{
  g_page = npage;
}

#define SCR_WIDTH  128
#define SCR_HEIGHT 128

void getoffset(BYTE x1, BYTE y1, WORD *offset, BYTE *offbit)
{

  *offset = y1*SCR_WIDTH/8 + x1 / 8;
  *offbit  = x1 % 8;

  //if (*offset >= SCR_WIDTH*SCR_HEIGHT/8) {
  //  *offset = -1;
  //  *offset  = -1;
  //}
  *offset += g_page;
}

void setpixel(int x1, int y1)
{
  WORD offset;
  BYTE offbit;
  BYTE b;

  if ((x1 >= SCR_WIDTH) || (y1 >= SCR_HEIGHT)) return;
  if ((x1 < 0) || (y1 < 0)) return;
  getoffset(x1, y1, &offset, &offbit);

  setaddr(offset);
  //getbyte(&b, GET_NIL);
  //b = b | (0x80 >> offbit);
  //putbyte(b, PUT_NIL);

  chkstat();
  wrcmd(0xf8 | (7-offbit));  // Bit set/reset command
}

//charmap
//  !"#$%&'()*+,-./
// 0123456789:;<=>?
// @ABCDEFGHIJKLMNO
// PQRSTUVWXYZ[\]^_
// `abcdefghijklmno
// pqrstuvwxyz{|}~
// ÇüéâäàåçêëèïîìÄÂ
// ÉæÆôöòûùÿöü¢£¥þf

void setchar(int x1, int y1, char c)
{
  WORD offset;

  // Text addr start at 0x0800
  offset = 0x0800 + y1*SCN_COL + x1;
  setaddr(offset);
  putbyte(c - 0x20, PUT_INC);
}

void setstring(int x1, int y1, char *ps)
{
  WORD offset;

  // Text addr start at 0x0800
  offset = 0x0800 + y1*SCN_COL + x1;
  setaddr(offset);

  while (*ps) {
    putbyte(*ps - 0x20, PUT_INC);
    ps++;
  }
}

/*
WORD MemGetPixel(long x1, long y1)
{
  int offbyte, offbit;
  BYTE b;

  if ((x1 >= SCR_WIDTH) || (y1 >= SCR_HEIGHT)) return 0;
  if ((x1 < 0) || (y1 < 0)) return 0;
  GetOffset(x1, y1, &offbyte, &offbit);

  b = Grid[offbyte] >> offbit;
  b = b & 1;
  return b;
}

#define ASPECT 1.0

void circle(int x, int y, int l)
{
  int a, b;
  int phi;

  for (phi = 0; phi <= 90; phi++) {
    a = round(l*icos(phi));
    b = round(l*isin(phi));

    setpixel(ASPECT*(x+a), y-b);
    setpixel(ASPECT*(x-a), y-b);
    setpixel(ASPECT*(x+a), y+b);
    setpixel(ASPECT*(x-a), y+b);

    COPCTL = 0;
  }
}
*/

