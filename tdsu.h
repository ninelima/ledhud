

#ifndef TDSU_H
#define TDSU_H

#include <qwidget.h>
//#include <qdialog.h>
#include <qdatetime.h>

#define SCR_WID 32
#define SCR_HGT 32


class QextSerialPort;

class TDSU : public QObject //QWidget
{
    Q_OBJECT
    
public:
    int nBrightness;

    TDSU( QObject *parent=0);
    
public slots:

    //void initCommPort(QString sPortName);
    bool initCommPort();
    QString readCommPort(QextSerialPort *port);
    int writeCommPort(QextSerialPort *port, QString sData);

    int setBrightness(int bright);

    // Direct drawing routines
    void drawPixel(int x, int y);
    void clearPixel(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRadLine(int x, int y, int l, int phi);
    void drawText(int x, int y, QString msg);
    void clearScreen();

    // Buffered uvect functions
    void MemSetPixel(int x1, int y1, unsigned char color);
    long MemGetPixel(int x1, int y1);
    void MemClear(unsigned char color);
    void MemSetSpot(int x1, int y1, unsigned char color);
    void MemSetBlob(int x1, int y1, unsigned char color);
    void MemLine(int x1, int y1, int x2, int y2, unsigned char color);
    void MemLineTo(int x1, int y1, unsigned char color);
    void MemMoveTo(int x1, int y1);
    void MemCircle(int x, int y, int l);

    void MemRadLine(int x, int y, int l, int phi);
    void MemRadLineOff(int x, int y, int l, int off, int phi);

    void MemDispGrid();
    void MemBlt();

    void frameRate();

protected slots:

private:

    unsigned char  grid[SCR_WID*SCR_HGT];       // The bitmap
    long cx, cy;                                // The current graphic pointer

    QTime       time;
    QString sPortName;
    QextSerialPort *portLED;

    // private functions
    // Buffered uvect functions
    void Bresenham(int x1, int y1, int x2, int y2, unsigned char color);
    long GetOffset(int x1, int y1);
  };

#endif // TDSU_H
