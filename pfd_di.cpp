/*
		pfd_alt.cpp - Altimeter Members for an OpenGL PFD Avionics Widget
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

/*
 ToDo List
		- much ...
*/

#include <math.h>

#include "pfd.h"
#include <qstring.h>
#include <qfont.h>
#include <qgl.h>

#include "utrig.h"


#if defined(Q_CC_MSVC)
#pragma warning(disable:4305)											// init: truncation from const double to float
#endif

void GLPFD::renderFixedDIMarkers()
{
	GLfloat z;

	QString t;

	glLineWidth( 2 );
	z = zfloat;

	// The tapes are positioned left & right of the roll circle, occupying the space based
	// on the vertical dimension, from .6 to 1.0 pixH2.  This makes the basic display
	// square, leaving extra space outside the edges for terrain which can be clipped if reqd.

  qglColor( QColor( "white" ) );
	font = QFont("Fixed", 12, QFont::Bold);
	QFontMetrics fm = fontMetrics();

  // Mask over the for the value display box
	qglColor( QColor( "black" ) );
  glBegin(GL_QUADS);
    glVertex3f(  0.175 * pixH2, 0.9*pixH2 -fm.height(), z+.1);
    glVertex3f(  0.175 * pixH2, 0.9*pixH2 +fm.height(), z+.1);
    glVertex3f( -0.175 * pixH2, 0.9*pixH2 +fm.height(), z+.1);
    glVertex3f( -0.175 * pixH2, 0.9*pixH2 -fm.height(), z+.1);
  glEnd();

	// The value display box line
	glBegin(GL_LINE_STRIP);
    qglColor( QColor( "white" ) );
    glVertex3f(  0.175 * pixH2, 0.9*pixH2 -fm.height(), z+.2);
    glVertex3f(  0.175 * pixH2, 0.9*pixH2 +fm.height(), z+.2);
    glVertex3f( -0.175 * pixH2, 0.9*pixH2 +fm.height(), z+.2);
    glVertex3f( -0.175 * pixH2, 0.9*pixH2 +fm.height()/2, z+.2);
    glVertex3f( -0.175 * pixH2, 0.9*pixH2 -fm.height()/2, z+.2);
    glVertex3f( -0.175 * pixH2, 0.9*pixH2 -fm.height(), z+.2);
    glVertex3f(  0.175 * pixH2, 0.9*pixH2 -fm.height(), z+.2);
  glEnd();


  // slip box
  glBegin(GL_LINE_STRIP);
    qglColor( QColor( "lightgrey" ) );
    glVertex3f(  -0.07 * pixH2, -0.9*pixH2 - 0.8*fm.height(), z+.2);
    glVertex3f(  -0.07 * pixH2, -0.9*pixH2 + 0.8*fm.height(), z+.2);
  glEnd();

  glBegin(GL_LINE_STRIP);
    qglColor( QColor( "lightgrey" ) );
    glVertex3f(   0.07 * pixH2, -0.9*pixH2 + 0.8*fm.height(), z+.2);
    glVertex3f(   0.07 * pixH2, -0.9*pixH2 - 0.8*fm.height(), z+.2);
  glEnd();

  // turn indicator
  GLfloat x, y;
  GLfloat i;


  qglColor( QColor( "lightgrey" ) );
    for (i = -30; i <= 0; i+= 30)  {
      //right
      glBegin(GL_LINE_STRIP);
        x = 0.15* icos (i);
        y = 0.15* isin (i);

        glVertex3f( +x * pixH2, -0.7*pixH2 + y* pixH2, z+.2);
        x = 0.20* icos (i);
        y = 0.20* isin (i);
        glVertex3f( +x * pixH2, -0.7*pixH2 + y* pixH2, z+.2);
      glEnd();

      //left
      glBegin(GL_LINE_STRIP);
        x = 0.15* icos (i);
        y = 0.15* isin (i);

        glVertex3f( -x * pixH2, -0.7*pixH2 + y* pixH2, z+.2);
        x = 0.20* icos (i);
        y = 0.20* isin (i);
        glVertex3f( -x * pixH2, -0.7*pixH2 + y* pixH2, z+.2);
      glEnd();

    }


  glLineWidth( 4 );
  qglColor( QColor( "white" ) );
  glBegin(GL_LINE_STRIP);
    x = 0.15* icos (ROTValue);
    y = 0.15* isin (ROTValue);

    //x = 0.4; y = 0.4;
    glVertex3f( -x * pixH2, -0.7*pixH2 - y* pixH2 , z+.2);
    glVertex3f( +x * pixH2, -0.7*pixH2 + y* pixH2, z+.2);

    glVertex3f( 0 * pixH2, -0.7*pixH2, z+.2);
    glVertex3f( -0.5*y * pixH2, -0.7*pixH2 + 0.5*x* pixH2, z+.2);

  glEnd();



}


void GLPFD::renderHDGValue()
{
  GLfloat z, pixPerUnit;

  pixPerUnit = pixH2/DIInView;
  z = zfloat;

  qglColor( QColor( "white" ) );
  font = QFont("Fixed", 12, QFont::Bold);
  QFontMetrics fm = fontMetrics();

  // Heading Display
  qglColor( QColor( "white" ) );
  t = (QString( "%1" ).arg(5 * ((2*DIValue + 5) / 10 ))); // round to the nearest 5
  QGLWidget::renderText ( - fm.width(t), 0.9*pixH2 -fm.height()/2 + 1, z+.1, t, font, 2000 ) ;
}


void GLPFD::renderGForceValue()
{
  GLfloat z, pixPerUnit;

  pixPerUnit = pixH2/DIInView;
  z = zfloat;

  qglColor( QColor( "white" ) );
  font = QFont("Fixed", 10, QFont::Normal);
  QFontMetrics fm = fontMetrics();

  // G Display
  qglColor( QColor( "white" ) );
  t = (QString( "%1" ).sprintf("G %03.1f", GForceValue));
  QGLWidget::renderText (-0.97*pixW2, -0.9*pixH2 -fm.height()/2 + 1, z+.1, t, font, 2000 ) ;


  //-------------------------------------------------
  // todo move to individual and populate
  t = (QString( "%1" ).sprintf("WPT: %s", "YABA")); // todo ... demo
  QGLWidget::renderText (-0.97*pixW2, 0.90*pixH2, z+.1, t, font, 2000 ) ;

  t = (QString( "%1" ).sprintf("BRG: %03d", 123)); // todo ... demo
  QGLWidget::renderText (-0.97*pixW2, 0.80*pixH2, z+.1, t, font, 2000 ) ;


  t = (QString( "%1" ).sprintf("GS: %03.1f", GSValue )); // todo ... demo
  QGLWidget::renderText (-0.97*pixW2, 0.60*pixH2, z+.1, t, font, 2000 ) ;
}




void GLPFD::renderSlipBall()
{
  //GLint i, j;
  GLfloat z, pixPerUnit;

  glLineWidth( 2 );
  pixPerUnit = pixH2/DIInView;
  z = zfloat;

  GLfloat radius = 10;
  GLfloat x1= SlipValue;
  GLfloat y1= -0.9*pixH2;

  qglColor( QColor( "white" ) );
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x1, y1, z );
    for (int angle = 0; angle < 360; angle += 5)
      //glVertex3f(x1 + sin(angle) * radius, y1 + cos(angle) * radius, z);
      glVertex2f(x1 + sin(angle) * radius, y1 + cos(angle) * radius);
    glEnd();

}


void GLPFD::renderDIMarkers()
{
	GLint i, j;
	GLfloat innerTic, midTic, outerTic, z, pixPerUnit, iPix;

	glLineWidth( 2 );
  pixPerUnit = pixH2/DIInView;
  z = zfloat;

  font = QFont("Fixed", 10, QFont::Normal);
	QFontMetrics fm = fontMetrics();

  innerTic = 0.80 * pixH2;												// inner & outer are relative to the vertical scale line
  outerTic = 0.90 * pixH2;
  midTic = 0.87 * pixH2;

	// The numbers & tics for the tape
	qglColor( QColor( "white" ) );

  for (i = 0; i < 360; i=i+30) {
    iPix = (float) i * pixPerUnit;
    t = QString( "%1" ).arg( i );

    glBegin(GL_LINE_STRIP);
      glVertex3f( iPix, innerTic, z);
      glVertex3f( iPix, outerTic, z);
    glEnd();
    QGLWidget::renderText (iPix - fm.width(t)/2 , outerTic  + fm.ascent() / 2 , z, t, font, 2000 ) ;

    //for (j = i + 20; j < i+90; j=j+20) {
    for (j = i + 10; j < i+90; j=j+10) {
      iPix = (float) j * pixPerUnit;
			glBegin(GL_LINE_STRIP);
        glVertex3f( iPix, innerTic,z);
        glVertex3f( iPix, midTic,z);
			glEnd();
    }


    iPix = (float) (360-i) * pixPerUnit;
    t = QString( "%1" ).arg( i );
    glBegin(GL_LINE_STRIP);
      glVertex3f( -iPix, innerTic, z);
      glVertex3f( -iPix, outerTic, z);
    glEnd();
    QGLWidget::renderText (-iPix - fm.width(t)/2 , outerTic  + fm.ascent() / 2 , z, t, font, 2000 ) ;

    //for (j = i + 20; j < i+90; j=j+20) {
    for (j = i + 10; j < i+90; j=j+10) {
      iPix = (float) j * pixPerUnit;
      glBegin(GL_LINE_STRIP);
        glVertex3f( -iPix, innerTic,z);
        glVertex3f( -iPix, midTic,z);
      glEnd();
    }
	}

  // The horizontal scale bar
	glBegin(GL_LINE_STRIP);
    glVertex3f(-180 * pixPerUnit , innerTic, z);
    glVertex3f(180 * pixPerUnit, innerTic, z);
  glEnd();
}


/*!
	Set the indicator
*/

void GLPFD::setHeading(int degrees)
{
  DIValue = degrees;
  while (DIValue < 0) DIValue += 360;
  DITranslation = DIValue / DIInView  * pixH2;
	updateGL();
}

void GLPFD::setSlip(int slip)
{
  SlipValue = slip;
  updateGL();
}


void GLPFD::setGForce(float gforce)
{
  GForceValue = gforce;
  updateGL();
}

void GLPFD::setGS(float gs)
{
  GSValue = gs;
  updateGL();
}

void GLPFD::setROT(float rot)
{
  ROTValue = rot;
  updateGL();
}


/*!
	Set the bearing (to steer) 
*/

void GLPFD::setBearing(int degrees)
{
  baroPressure = degrees;
	updateGL();
}
