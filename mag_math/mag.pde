//code by Arne
//the acclerometer it set up the way that the z acceleration looks
//to the sky, and x y is flat --> like most cartesian coordinate systems
//I use an A7270 as an Accelerometer -> they are on a breakout board for approx $20
//and a HMC5843 as a triple axis magnetic Sensor for approx $50
//feel free to comment on the code -> improvement is always appreciated
//code to read from HMC5843 was influenced by the gus from sf9domahrs
//you can ask questions in English or German

//required for calculations
#include <math.h>
//required for HMC5843 communication
#include <Wire.h>

#include <Servo.h>

//accelerometer connecteded to pins 0 through 2
#define xAxis 0
#define yAxis 1
#define zAxis 2



//by increasing alphaAccel the response will become faster
//but the noise will increae [alpha must be between 0 and 1]
//values for digital lowpass
//(filtering the Mag. Data does not improve the Noise Level!!)
float alphaAccel = 0.35;
float alphaYaw = 0.35;

unsigned int xOffset=0;
unsigned int yOffset=0;
unsigned int zOffset=0;

float Pitch=0;
float Roll=0;
float Yaw=0;

int xRaw=0;
int yRaw=0;
int zRaw=0;

float xFiltered=0;
float yFiltered=0;
float zFiltered=0;

float xFilteredOld=0;
float yFilteredOld=0;
float zFilteredOld=0;

float xAccel=0;
float yAccel=0;
float zAccel=0;

int xMagnet=0;
int yMagnet=0;
int zMagnet=0;

float norm;

float yawRaw=0;
float yawFiltered=0;
float yawFilteredOld=0;

//X:-770 612 Y:-664 697 Z:-592 656
//after measurements
int xMagnetMax=612;
int yMagnetMax=697;
int zMagnetMax=656;

int xMagnetMin=-770;
int yMagnetMin=-664;
int zMagnetMin=-592;

float xMagnetMap=0;
float yMagnetMap=0;
float zMagnetMap=0;

float YawU;

Servo ServoPitch;
  Servo ServoYaw;

int CompassAddress = 0x1E;  //0x3C //0x3D;  //(0x42>>1);

//initialize µController
void setup()
{
  Serial.begin(115200);       //initialize serial port
  analogReference(EXTERNAL);  //use external reference voltage (3,3V)
  delay(2000);  //calibrate sensor after a short delay
  HMC_Init();  //initializes Mag Compass
  //delay(2000);
  getAccelOffset();           //keep it flat and non moving on the table

  //there are other ways to calibrate the offset, each has some advantes
  //and disadvantes..
  //compare application note AN3447
  //http://www.freescale.com/files/sensors/doc/app_note/AN3447.pdf
  
  
  ServoPitch.attach(9);
  ServoYaw.attach(10);
}





void loop()
{
  getMagnet();
  FilterAD();
  AD2Degree();
  getAzimuth();
  
  
  ServoPitch.write(map(Pitch*180/PI,-90,90,0,180));
  ServoYaw.write(map(Yaw,-180,180,0,180));
  Send2Com();
}


void HMC_Init()
{
  Wire.begin();
  Wire.beginTransmission(CompassAddress);
  Wire.send(0x02);
  Wire.send(0x00);         //Set to continous streaming mode
  Wire.endTransmission();  
  delay(5);                //HMC5843 needs some ms before communication
}


void FilterAD()
{
  // read from AD and subtract the offset
  xRaw=analogRead(xAxis)-xOffset;
  yRaw=analogRead(yAxis)-yOffset;
  zRaw=analogRead(zAxis)-zOffset;
  
  //Digital Low Pass - compare: (for accelerometer)
  //http://en.wikipedia.org/wiki/Low-pass_filter
  xFiltered= xFilteredOld + alphaAccel * (xRaw - xFilteredOld);
  yFiltered= yFilteredOld + alphaAccel * (yRaw - yFilteredOld);
  zFiltered= zFilteredOld + alphaAccel * (zRaw - zFilteredOld);
  
  xFilteredOld = xFiltered;
  yFilteredOld = yFiltered;
  zFilteredOld = zFiltered;
  
}
  

void AD2Degree()
{
  // 3.3 = Vref; 1023 = 10Bit AD; 0.8 = Sensivity Accelerometer
  // (compare datasheet of your accelerometer)
  // the *Accel must be between -1 and 1; you may have to
  // to add/subtract +1 depending on the orientation of the accelerometer
  // (like me on the zAccel)
  // they are not necessary, but are useful for debugging
  xAccel=xFiltered *3.3 / (1023.0*0.8);      
  yAccel=yFiltered *3.3 / (1023.0*0.8);      
  zAccel=zFiltered *3.3 / (1023.0*0.8)+1.0;
  
  // Calculate Pitch and Roll (compare Application Note AN3461 from Freescale
  // http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
  // Microsoft Excel switches the values for atan2
  // -> this info can make your life easier :-D
  //angled are radian, for degree (* 180/3.14159)
  Roll   = -atan2(  yAccel ,  sqrt(sq(xAccel)+sq(zAccel)));  
  Pitch  =  atan2(  xAccel ,  sqrt(sq(yAccel)+sq(zAccel)));
}

void getAzimuth()
{
  //this part is required to normalize the magnetic vector
  //get Min and Max Reading for Magnetic Axis
  if (xMagnet>xMagnetMax) {xMagnetMax = xMagnet;}
  if (yMagnet>yMagnetMax) {yMagnetMax = yMagnet;}
  if (zMagnet>zMagnetMax) {zMagnetMax = zMagnet;}
  
  if (xMagnet<xMagnetMin) {xMagnetMin = xMagnet;}
  if (yMagnet<yMagnetMin) {yMagnetMin = yMagnet;}
  if (zMagnet<zMagnetMin) {zMagnetMin = zMagnet;}
  
  //Map the incoming Data from -1 to 1
  xMagnetMap = float(map(xMagnet, xMagnetMin, xMagnetMax, -30000, 30000))/30000.0;
  yMagnetMap = float(map(yMagnet, yMagnetMin, yMagnetMax, -30000, 30000))/30000.0;
  zMagnetMap = float(map(zMagnet, zMagnetMin, zMagnetMax, -30000, 30000))/30000.0;
  

 //normalize the magnetic vector
 norm= sqrt( sq(xMagnetMap) + sq(yMagnetMap) + sq(zMagnetMap));
 xMagnetMap /=norm;
 yMagnetMap /=norm;
 zMagnetMap /=norm;
  
 //compare "Applications of Magnetic Sensors for Low Cost Compass Systems" by Michael J. Caruso
 //for the compensated Yaw equations...
 //http://www.ssec.honeywell.com/magnetic/datasheets/lowcost.pdf
 yawRaw=atan2( (-yMagnetMap*cos(Roll) + zMagnetMap*sin(Roll) ) , (xMagnetMap*cos(Pitch) + yMagnetMap*sin(Pitch)*sin(Roll)+ zMagnetMap*sin(Pitch)*cos(Roll)) ) *180/PI;
 YawU=atan2(-yMagnetMap, xMagnetMap) *180/PI;


 //apply Low Pass to Yaw
 //Digital Low Pass - compare: (for accelerometer)
  //http://en.wikipedia.org/wiki/Low-pass_filter
  Yaw= yawFilteredOld + alphaYaw * (yawRaw - yawFilteredOld);
  yawFilteredOld=Yaw;
 }



void getMagnet()
{
  
  int i = 0;
  byte buff[6];

  Wire.beginTransmission(CompassAddress);
  Wire.send(0x03);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  //Wire.beginTransmission(CompassAddress);
  Wire.requestFrom(CompassAddress, 6);    // request 6 bytes from device
  while(Wire.available())   // ((Wire.available())&&(i<6))
  {
    buff[i] = Wire.receive();  // receive one byte
    i++;
  }
  Wire.endTransmission(); //end transmission
  
  if (i==6)  // All bytes received?
    {
    // MSB byte first, then LSB, X,Y,Z
    xMagnet = ((((int)buff[2]) << 8) | buff[3]);    // X axis (internal sensor y axis)
    yMagnet = ((((int)buff[0]) << 8) | buff[1]);    // Y axis (internal sensor x axis)
    zMagnet = ((((int)buff[4]) << 8) | buff[5]);    // Z axis
   }
    
  else
   Serial.println("!ERR: Mag data");

}



void Send2Com()
{
  Serial.print("Pitch: ");
  Serial.print(int(Pitch*180/PI));
  Serial.print(" Roll: ");
  Serial.print(int(Roll*180/PI));

  Serial.print(" Yaw: ");
  Serial.print(int(Yaw));
    
  Serial.print(" YawU: ");
  Serial.print(int(YawU));
  
  Serial.print(" MagnetX: ");
  Serial.print(xMagnet);
  Serial.print(" MagnetY: ");
  Serial.print(yMagnet);
  Serial.print(" MagnetZ: ");
  Serial.print(zMagnet);
  
  Serial.print(" X:");
  Serial.print(xMagnetMin);
  Serial.print(" ");
  Serial.print(xMagnetMax);
  Serial.print(" Y:");
  Serial.print(yMagnetMin);
  Serial.print(" ");
  Serial.print(yMagnetMax);
  Serial.print(" Z:");
  Serial.print(zMagnetMin);
  Serial.print(" ");
  Serial.println(zMagnetMax);
      
  delay(50);
}
  



void getAccelOffset()
{ //you can make approx 60 iterations because we use an unsigned int
  //otherwise you get an overflow. But 60 iterations should be fine
   for (int i=1; i <= 60; i++){        
     xOffset += analogRead(xAxis);    
     yOffset += analogRead(yAxis);
     zOffset += analogRead(zAxis);
     }
   xOffset /=60;  
   yOffset /=60;
   zOffset /=60;

   Serial.print("xOffset: ");
   Serial.print(xOffset);
   Serial.print("   yOffset: ");
   Serial.print(yOffset);
   Serial.print("   zOffset: ");
   Serial.println(zOffset);
}



 