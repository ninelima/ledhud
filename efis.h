

#ifndef EFIS_H
#define EFIS_H

#include <qwidget.h>
#include <qdialog.h>
#include <qdatetime.h>

class GLPFD;
class GLHSI;

class TDSU;

class QextSerialPort;


class EFIS : public QWidget
{
    Q_OBJECT
    
public:
    bool bSHowMirror;
    bool bTDSUAvailable;

    EFIS( QWidget *parent=0, const char *name=0, Qt::WFlags f = 0);    

public slots:

    void initCommPort(QString sPortName);
    QString readCommPort(QextSerialPort *port);
    int writeCommPort(QextSerialPort *port, QString sData);

    // drawing routines
    /*
    void drawPixel(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRadLine(int x, int y, int l, int phi);
    */
    void setLEDBrightness(int nBrt);
    void setTerrain(bool bTerrain);
    void setTape(bool bTape);

    void frameRate();

protected slots:

private:
    GLPFD *PFD;
    GLHSI *HSI;
    TDSU  *LEDPanel;

    QTime       time;
    QString sPortName;
    QextSerialPort *portMEMS;
    // b2 QextSerialPort *portLED;

};

#endif // EFIS_H
