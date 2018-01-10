/*
		efis.cpp - an application window launch program for OpenGL Avionics Widgets
		Copyright (C)  2003 Tom Dollmeyer (tom@dollmeyer.com)

		This program is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*#include <qslider.h>
#include <qlayout.h>
#include <q3frame.h>
#include <qapplication.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qlabel.h>*/
#include <qtimer.h>
#include <qdatetime.h>


#include "tdsu.h"

#include <math.h>
#include <stdlib.h>

#include "..\ulib\umisc.h"
#include "..\ulib\umath.h"
#include "utrig.h"


#include <qextserialport.h>
#include <qextserialenumerator.h>
#include <QList>

#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qstring.h>

#include <QTDebug>


TDSU::TDSU( QObject* parent) : QObject( parent )
{

  /*
   Maybe we can use this later?

  // 50 Hz main exec loop
  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(frameRate()) );
  timer->start(20);  // 1/50 = 20 ms
*/
}


bool TDSU::initCommPort()
{
  QString sPortName;
  bool success = false;

  QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

  for (int i = 0; i < ports.size(); i++) {
    qDebug() << "port name:" << ports.at(i).portName;
    qDebug() << "friendly name:" << ports.at(i).friendName;
    qDebug() << "physical name:" << ports.at(i).physName;
    qDebug() << "enumerator name:" << ports.at(i).enumName;
    qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
    qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
    qDebug() << "===================================";

    sPortName = ports.at(i).portName;

    // for the TierTex display gyro
    if (ports.at(i).enumName.contains("USB")) {
      sPortName = ports.at(i).portName;
      // create the serial port
      #ifdef _TTY_POSIX_
      port = new QextSerialPort("/dev/ttyS0", QextSerialPort::Polling);
      #else
      portLED = new QextSerialPort(sPortName, QextSerialPort::Polling);
      #endif																					/*_TTY_POSIX*/
      portLED->setBaudRate(BAUD115200); //BAUD57600 BAUD38400
      portLED->setFlowControl(FLOW_OFF);
      portLED->setParity(PAR_NONE);
      portLED->setDataBits(DATA_8);
      portLED->setStopBits(STOP_2);
      //set timeouts to 500 ms
      portLED->setTimeout(500);

      if (!portLED->isOpen()) {
        portLED->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        qDebug("port->isOpen(): %d", portLED->isOpen());
        if (portLED->isOpen()) success = true;
        else success = false;
        break;
      }
    }
  }

  if (!success) {
    return false;
  }
  return true;
}


QString TDSU::readCommPort(QextSerialPort *port)
{
  int i;
  char buff[1024];
  int numBytes;

  //LABEL_1:
  numBytes = port->bytesAvailable();

  if (numBytes > 1023) numBytes = 1023;
  i = port->read(buff, numBytes);

  // did we get some data?
  if (i <= 0) return QString::null;
    //goto LABEL_1;

  // make sure the string is null terminated
  buff[i-1] = '\0';

  // return the data
  return QString(buff);
}



int TDSU::writeCommPort(QextSerialPort *port, QString sData)
{
  return port->write(sData);
}

int TDSU::setBrightness(int bright)
{
  QString s;

  if (bright < 0) bright = 0;
  if (bright > 3) bright = 3;

  s.sprintf("#v=%02d\n", bright);
  return writeCommPort(portLED, s);
}


void TDSU::drawPixel(int x, int y)
{
  QString s;

  if (x < 0) return;
  if (y < 0) return;
  if (x >= SCR_WID) return;
  if (y >= SCR_HGT) return;

  s.sprintf("#p%02d%02d\n", x, y);
  writeCommPort(portLED, s);
}

void TDSU::clearPixel(int x, int y)
{
  QString s;

  if (x < 0) return;
  if (y < 0) return;
  if (x >= SCR_WID) return;
  if (y >= SCR_HGT) return;

  s.sprintf("#P%02d%02d\n", x, y);
  writeCommPort(portLED, s);
}

void TDSU::drawLine(int x1, int y1, int x2, int y2)
{
  int x, y;
  float m;


  if (x2 < x1) {
    x  = x1;   y = y1;
    x1 = x2;  y1 = y2;
    x2 = x;   y2 = y;
  }

  m = float (y2 - y1) / float (x2 - x1);

  if (fabs(m)  < 1) {
    for (x = x1; x < x2; x++) {
      y = m * (x-x1);
    drawPixel(x, y + y1);
    }
  }
  else {
    if (y2 < y1) {
      x  = x1;   y = y1;
      x1 = x2;  y1 = y2;
      x2 = x;   y2 = y;
    }

    for (y = y1; y < y2; y++) {
      x = (y-y1) / m;
      drawPixel(x + x1, y);
    }
  }
}

void TDSU::drawRadLine(int x, int y, int l, int phi)
{
  int a,b;

  a = round(l*icos(phi));
  b = round(l*isin(phi));

  drawLine(x, y, x+a, y-b);
}

void TDSU::drawText(int x, int y, QString msg)
{
  QString s;

  if (x < 0) return;
  if (y < 0) return;
  if (x >= SCR_WID) return;
  if (y >= SCR_HGT) return;

  s.sprintf("#x%02d#y%02d%s\n", x, y, msg.latin1());
  writeCommPort(portLED, s);
}



void TDSU::clearScreen()
{
  writeCommPort(portLED, "#w\n");
}


//
// uvect functions
//
void Sort(int *x1, int *y1, int *x2, int *y2)
{
  int x,y;

  if (*x1 > *x2) {
    x = *x1;
    *x1 = *x2;
    *x2 = x;

    y = *y1;
    *y1 = *y2;
    *y2 = y;
  }
}

long TDSU::GetOffset(int x1, int y1)
{
  long offs;

  offs = y1*SCR_WID + x1;

  if (offs >= SCR_WID*SCR_HGT) {
    return 0;
  }
  else return offs;
}

void TDSU::MemSetPixel(int x1, int y1, unsigned char color)
{
  long offs;

  if ((x1 >= SCR_WID) || (y1 >= SCR_HGT)) return;
  if ((x1 < 0) || (y1 < 0)) return;
  offs = GetOffset(x1, y1);
  grid[offs] = color;
}

long TDSU::MemGetPixel(int x1, int y1)
{
  long offs;

  if ((x1 >= SCR_WID) || (y1 >= SCR_HGT)) return 0;
  if ((x1 < 0) || (y1 < 0)) return 0;
  offs = GetOffset(x1, y1);
  return grid[offs];
}


void TDSU::MemSetSpot(int x1, int y1, unsigned char color)
{
  MemSetPixel(x1,   y1,   color);
  //MemSetPixel(x1+1, y1,   color);
  MemSetPixel(x1-1, y1,   color);
  //MemSetPixel(x1,   y1+1, color);
  MemSetPixel(x1,   y1-1, color);
}

void TDSU::MemSetBlob(int x1, int y1, unsigned char color)
{
  MemSetPixel(x1,   y1,   color);
  MemSetPixel(x1+1, y1,   color);
  MemSetPixel(x1-1, y1,   color);
  MemSetPixel(x1,   y1+1, color);
  MemSetPixel(x1,   y1-1, color);
}


void TDSU::MemClear(unsigned char color)
{
  memset(grid, color, sizeof(grid));
}


void TDSU::Bresenham(int x1, int y1, int x2, int y2, unsigned char color)
{
  int i;
  int x,y;
  int e;
  int dx, dy, sx, sy;

  Sort(&x1, &y1, &x2, &y2);
  x = x1;
  y = y1;
  sx = (x2-x1);
  sy = (y2-y1);
  dx = abs(sx);
  dy = abs(sy);

  if (dx >= dy) {
    e = 2*dy - dx;
    for (i = 1; i <= dx; i++) {
      MemSetPixel(x, y, color);
      // if (LineStyle) MemSetBlob(x, y, color);
      if (e > 0) {
        if (sy > 0) y++; else y--;
        e = e + (2*dy - 2*dx);
      }
      else {
        e = e + 2*dy;
      }
      x++;
    }
  }
  else {
    if (sy < 0) {
      x = x2;
      y = y2;
    }
    e = 2*dx - dy;
    for (i = 1; i <= dy; i++) {
      MemSetPixel(x, y, color);
      //if (LineStyle) MemSetBlob(x, y, color);
      if (e > 0) {
        if (sy > 0) x++; else x--;
        e = e + (2*dx - 2*dy);
      }
      else {
        e = e + 2*dx;
      }
      y++;
    }
  }
}


void TDSU::MemLine(int x1, int y1, int x2, int y2, unsigned char color)
{
  Sort(&x1, &y1, &x2, &y2);
  MemSetPixel(x1, y1, color);
  MemSetPixel(x2, y2, color);
  Bresenham(x1, y1, x2, y2, color);
}

void TDSU::MemLineTo(int x1, int y1, unsigned char color)
{
  MemLine(cx, cy, x1, y1, color);
  cx = x1;
  cy = y1;
}

void TDSU::MemMoveTo(int x1, int y1)
{
  cx = x1;
  cy = y1;
}

void TDSU::MemCircle(int x, int y, int l)
{
  int a,b;
  int phi;

  a = l; //round(l*icos(0));
  b = 0; //round(l*isin(0));
  MemMoveTo((x+a), y-b);

  for (phi = 0; phi < 360; phi++) {
    a = round(l*icos(phi));
    b = round(l*isin(phi));

    //MemSetPixel(ASPECT*(x+a), y-b, TRUE);
    MemLineTo((x+a), y-b, true);
  }
}

void TDSU::MemRadLine(int x, int y, int l, int phi)
{
  int a,b;

  a = round(l*icos(phi));
  b = round(l*isin(phi));

  MemMoveTo(x, y);
  MemLineTo(x+a, y-b, TRUE);
}

void TDSU::MemRadLineOff(int x, int y, int l, int off, int phi)
{
  int a,b;

  a = round(off*icos(phi+90));
  b = round(off*isin(phi+90));
  MemRadLine(x+a, y-b, l, phi);
}



/*
 Example code of binary / hexadecimal conversions using QString


bool ok = false;
QString hexadecimalNumber = "0xDEADBEEF";
QString binaryNumber = QString::number(hexadecimalNumber.toLongLong(&ok, 16), 2);
qDebug() << binaryNumber;   // "11011110101011011011111011101111"


bool ok = false;
QString binaryNumber  = "11011110101011011011111011101111";
QString hexadecimalNumber = QString::number(binaryNumber.toLongLong(&ok, 2), 16).toUpper();
qDebug() << hexadecimalNumber;   // "0xDEADBEEF"

*/


// This is only useful for debugging
void TDSU::MemDispGrid()
{
  int x, y; // , x1, y1;
  int rv;


  for (y = 0; y < SCR_HGT; y++) {
    for (x = 0; x < SCR_WID; x++) {
      if ((rv = MemGetPixel(x, y)) != 0) {
        drawPixel(x, y);
      }
      else {
        clearPixel(x, y);
      }
    }
  }
}


#if 1
// Qt version of MemBlt
void TDSU::MemBlt()
{
  int x, y;
  QString s, row;
  QString bitmap;

  for (y = 0; y < 32; y++) {
    bool ok = false;
    s.clear();

    for (x = 0; x < 32; x++) {
      if (grid[x + 32*y]) s.append("1");
      else s.append("0");
    }

    qlonglong ll  = s.toLongLong(&ok, 2);
    row = QString( "%1" ).arg(ll, 8, 16, QLatin1Char('0')).toUpper();
    bitmap.append(row);
  }

  // int len = bitmap.length();
  // qDebug() << "   len:" << len;  // len *must* be exactly 256 ( 8 chars   x 32 rows)

  s = QString("#w#x00#y00#b0432%1\n").arg(bitmap);

  writeCommPort(portLED, s);
}

#else

// Standad C version of MemBlt
void TDSU::MemBlt()
{
  int x, y;

  char row[64], s[64];
  char bitmap[512];

  memset(bitmap, 0x0, sizeof(bitmap));

  for (y = 0; y < 32; y++) {

    unsigned int ll;
    memset(s, 0x0, sizeof(s));

    for (x = 0; x < 32; x++) {
      if (grid[x + 32*y]) strcat(s, "1");
      else strcat(s, "0");
    }

    ll  = strtoul(s, NULL, 2);
    sprintf(row, "%08X", ll);
    strcat(bitmap, row);
  }

  //sprintf(s, "#w#x00#y00#b0432%s\n", bitmap);
  //writeCommPort(portLED, s);
   writeCommPort(portLED, "#w#x00#y00#b0432");
   writeCommPort(portLED, bitmap);
   writeCommPort(portLED, "\n");
}
#endif //1

// end uvect



void TDSU::frameRate()
{
}



