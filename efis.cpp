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

#include <qslider.h>
#include <qlayout.h>
#include <q3frame.h>
#include <qapplication.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <Q3HBoxLayout> //Added by qt3to4:
#include <Q3VBoxLayout> //Added by qt3to4:
#include "efis.h"
#include "pfd.h"
#include "hsi.h"
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
//#include <QTDebug>

#define CALIB_COUNT 100


EFIS::EFIS( QWidget* parent, const char* name, Qt::WFlags f)
: QWidget( parent, name, f )
{
	//b2 QAction * actionQuit = new QAction( this, "actionQuit", FALSE );
																									//, FALSE );
	QAction * actionQuit = new QAction( this, "actionQuit");
	actionQuit->setAccel( Qt::Key_Escape );
	connect( actionQuit, SIGNAL( activated() ), qApp, SLOT(quit() ) );

	this->setPaletteForegroundColor( "white" );
	this->setPaletteBackgroundColor( "black" );

	Q3Frame* framePFD = new Q3Frame( this, "framePFD" );
	framePFD->setMinimumSize( 550, 350 );						// Force a specific size for inspection
	framePFD->setMaximumSize( 550, 350 );
	framePFD->setPaletteBackgroundColor( "black" );
	framePFD->setPaletteForegroundColor( "white" );

	// Box up the frames
	Q3VBoxLayout* mainLayout = new Q3VBoxLayout( this );
	Q3HBoxLayout* centerLayout = new Q3HBoxLayout( mainLayout );
	centerLayout->addWidget( framePFD );

	//
	// Create the PFD
	//
	PFD = new GLPFD( framePFD, "PFD");
  PFD->displayTape = false;
  PFD->displayTerrain = false;

	// Load into the frame
	Q3HBoxLayout* boxPFD = new Q3HBoxLayout( framePFD, 5, 0, "cvbox");
	boxPFD->addWidget( PFD );

  // Create the LED panel
  LEDPanel = new TDSU (this);
  bTDSUAvailable = LEDPanel->initCommPort();

  // Initialise the FTDIBUS commport
  initCommPort(sPortName);

  // 50 Hz main exec loop
  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(frameRate()) );
  timer->start(10);  // 1/50 = 20 ms
}


void EFIS::initCommPort(QString sPortName)
{
  bool success = false;

  QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

  for (int i = 0; i < ports.size(); i++) {
    /*qDebug() << "port name:" << ports.at(i).portName;
    qDebug() << "friendly name:" << ports.at(i).friendName;
    qDebug() << "physical name:" << ports.at(i).physName;
    qDebug() << "enumerator name:" << ports.at(i).enumName;
    qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
    qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
    qDebug() << "===================================";*/

    sPortName = ports.at(i).portName;

    // For the Sparkfun Razor IMU
    if (ports.at(i).enumName.contains("FTDIBUS")) {
      sPortName = ports.at(i).portName;
      // create the serial port
      #ifdef _TTY_POSIX_
      portMEMS = new QextSerialPort("/dev/ttyS0", QextSerialPort::Polling);
      #else
      portMEMS = new QextSerialPort(sPortName, QextSerialPort::Polling);
      #endif																					/*_TTY_POSIX*/
      portMEMS->setBaudRate(BAUD115200); //BAUD57600 BAUD38400
      portMEMS->setFlowControl(FLOW_OFF);
      portMEMS->setParity(PAR_NONE);
      portMEMS->setDataBits(DATA_8);
      portMEMS->setStopBits(STOP_2);
      //set timeouts to 500 ms
      portMEMS->setTimeout(500);

      if (!portMEMS->isOpen()) {
        portMEMS->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        qDebug("port->isOpen(): %d", portMEMS->isOpen());
        if (portMEMS->isOpen()) success = true;
        else success = false;
        break;
      }
    }
  }

  if (!success) {
    printf("Cannot open COM port FTDIBUS\n");
    exit(-1);
    QMessageBox::warning(this, "tit","msg");
  }
}


QString EFIS::readCommPort(QextSerialPort *port)
{
  int i;
  char buff[1024];
  char *sp;
  int numBytes = 0;
  int numRead = 0;

  numBytes = port->bytesAvailable();

  if (numBytes < 50) return QString::null;                  // is there at least enough data to do anythibng with?
  if (numBytes >=  sizeof(buff)) numBytes = sizeof(buff);   // make sure we dont overrun our buffer

  numRead = port->read(buff, numBytes);
  i = numRead;

  // did we actually get some data?
  if (numRead <= 0) return QString::null;

  // work back from the last \n to the '!' and return that string
  bool foundEnd = false;
  for (i = numRead; i > 0; i--) {
    if (buff[i] == '\n') {
      buff[i] = '\0';   // null terminate the string
      foundEnd = true;  // and set the flag to indicate we found a valid end
    }
    if (foundEnd && (buff[i] == '!'))  {
      sp = &buff[i];         // we found a valid start marker with a valid end
      //qDebug(sp+1);
      return QString(sp+1);  // this string should be OK so return it
      break;
    }
  }
  return QString::null;  // no valid string was found
}


int runavg_p(int a, int nr)
{
  static int abuff[1024];
  int i;
  int isum = 0;

  if (nr > 1000) nr = 1000;

  for (i = nr; i > 0; i--) abuff[i] = abuff[i-1];
  abuff[0] = a;
  for (i = 0; i < nr; i++) isum += abuff[i];
  return isum / nr;
}


int EFIS::writeCommPort(QextSerialPort *port, QString sData)
{
  return port->write(sData);
}



/*
57600 baud com11

!ANG:-0.16, 0.31, -8.55
      roll, pitch, yaw
*/


// todo move to some umath.h
/*
Constrains a number to be within a range.
Parameters
x: the number to constrain, all data types
a: the lower end of the range, all data types
b: the upper end of the range, all data types
Returns
x: if x is between a and b
a: if x is less than a
b: if x is greater than b
   */
float contrain(float x, float a, float b)
{
  if (x < a) return a;
  if (x > b) return b;
  return x;
}


void EFIS::setLEDBrightness(int nBrt)
{
  if (bTDSUAvailable)
    LEDPanel->setBrightness(nBrt);
}

void EFIS::setTerrain(bool bTerrain)
{
  PFD->displayTerrain = bTerrain;
}

void EFIS::setTape(bool bTape)
{
  PFD->displayTape = bTape;
}


void EFIS::frameRate()
{
  float roll;
  float pitch;
  float yaw;
  float GMagnitude;
  float gforce;
  float slip;
  float gs;
  float rot;

  // for the MEMS gyro
  QString line = readCommPort(portMEMS);
  if (line.isNull()) return;

  line.replace("ANG:",",");
  line.replace("AUX:",",");
  //qDebug(line);

  QStringList list = QStringList::split(",", line);
  if (list.count() < 8)
    return;

  PFD->calibrated = true;

  roll =  list[0].toFloat();
  pitch = list[1].toFloat();
  yaw =   list[2].toFloat();

  GMagnitude = list[3].toFloat();
  slip       = list[4].toFloat();
  gforce     = list[5].toFloat();
  rot        = list[6].toFloat();
  gs         = list[7].toFloat();

  // qDebug("roll: %f pitch: %f yaw: %f g %f  rot %f slip %f gs %f gm %f", roll, pitch, yaw, gforce, rot, slip, gs, GMagnitude);

  //
  // Display on the PC PFD
  //

  PFD->setRoll(-roll);
  PFD->setPitch(pitch);
  PFD->setHeading(yaw+180);
  PFD->setGForce(gforce);
  PFD->setGS(gs);
  //PFD->setGS(GMagnitude);

  PFD->setROT(contrain(rot, -60, 60));         // in 0.1 deg / second. Eg 30 = rate one turn (3 deg/second)
  PFD->setSlip(contrain(-slip*100, -50, 50));

  // fur spass
  int vsi = runavg_p(-50*pitch, 100); //0
	static double alt = 1634;
  PFD->setIAS(150-0.2*vsi);
  PFD->setMSL(alt);
  PFD->setVSI(vsi);


  //
  // Display on the Tiertex TDSU LED Panel
  //
  if (bTDSUAvailable) {
    int x = 16;
    int y = 16;
    int l = 16;
    int a, b;
    float gain = 0.5;
    bSHowMirror = true;

    LEDPanel->MemClear(0x0);

    //
    // AH
    //
    a = gain * pitch * icos(roll + 90);
    b = gain * pitch * isin(roll + 90);
    l = SCR_WID;

    // horizon
    if (bSHowMirror) {
      LEDPanel->MemRadLine((x-a), (y+b), l, roll);
      LEDPanel->MemRadLine((x-a), (y+b), l, roll+180);
      //LEDPanel->MemSetBlob((x-a), (y+b), 0x0);
      LEDPanel->MemSetSpot((x-a), (y+b), 0x0);
    }
    else {
      LEDPanel->MemRadLine((x+a), (y-b), l, roll);
      LEDPanel->MemRadLine((x+a), (y-b), l, roll+180);
      LEDPanel->MemSetSpot((x+a), (y-b), 0x0);
    }

    // pitch markers
    l = 3;
    for (int step = -80; step <= 80; step+= 20) {
      if (step == 0) continue;

      a = gain * (pitch+step) * icos(roll + 90);
      b = gain * (pitch+step) * isin(roll + 90);

      if (bSHowMirror) {
        LEDPanel->MemRadLine((x-a), (y+b), l, roll);
        LEDPanel->MemRadLine((x-a), (y+b), l, roll+180);
        if (step < 0) LEDPanel->MemSetBlob((x-a), (y+b), 0x0);
      }
      else {
        LEDPanel->MemRadLine((x+a), (y-b), l, roll);
        LEDPanel->MemRadLine((x+a), (y-b), l, roll+180);
        if (step < 0) LEDPanel->MemSetBlob((x+a), (y-b), 0x0);
      }
    }
    LEDPanel->MemBlt();
  }
}



