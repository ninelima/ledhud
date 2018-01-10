/*   
    main.cpp - a generic launch program for OpenGL Avionics Widgets
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

#include "efis.h"
#include <stdio.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qgl.h>
#include <qstring.h>



int main( int argc, char **argv )
{
  QString sPortName, sDisplayName;
  QString sBrt;
  bool bTerrain = false;
  bool bTape = false;

  // Parse the command line for any useful
  // tidbits
  for (int i = 0; i < argc; i++) {
    puts(argv[i]);


    if (strcmp(argv[i], "-bright") == 0) {
      sBrt = argv[i+1];
    }
    if (strcmp(argv[i], "-terrain") == 0) {
      bTerrain = true;
    }
    if (strcmp(argv[i], "-tape") == 0) {
      bTape = true;
    }


    if ((strcmp(argv[i], "-h") == 0) ||
        (strcmp(argv[i], "-?") == 0) ||
        (strcmp(argv[i], "--h") == 0) ||
        (strcmp(argv[i], "--?") == 0)
       ) {

      printf("\n\nLED HUD - (c) 2012 Player One\n\n");
      printf("usage:\n\n");
      printf("ledhud.exe <-option [arg]>\n\n");
      printf("with option:\n");
      printf("  -bright [0-3]\n");
      printf("  -terrain\n");
      printf("  -tape\n");

      printf("  -h help\n");
      printf("  \n");
      printf("Example:\n");
      printf("  ledhud.exe -bright 2\n");
      getchar();
      return 0;
    }
  }


  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication a(argc,argv);


  if ( !QGLFormat::hasOpenGL() ) {
    qWarning( "This system has no OpenGL support." );
    // return -1;
  }


  //    EFIS mainWindow(0,0,Qt::WStyle_Customize|Qt::WStyle_NoBorder);
  EFIS mainWindow;

  mainWindow.setLEDBrightness(sBrt.toInt());
  mainWindow.setTerrain(bTerrain);
  mainWindow.setTape(bTape);
  mainWindow.show();

//    QMessageBox::information( &mainWindow, "EFIS 0.1.0",
//    "This application demonstrates the intended functionality of the EFIS.\n\n"
//    "Function Keystrokes have been provided to show the functions of the EFIS keys.\n"
//    "Other keystrokes as shown in the checklist area may be used to launch override\n"
//    "dialogs, allowing you to control the inputs which normally come from sensors.\n\n"
//    "After closing any dialogs, press Esc to quit.\n\n"
//    "Please send comments to pastoril@users.sourceforge.net\n" );    

  a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
  return a.exec();
}
