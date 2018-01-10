
// Sparkfun 9DOF Razor IMU AHRS
// 9 Degree of Freedom Attitude and Heading Reference System
// Firmware v1.1gke
//
// Released under Creative Commons License
// Based on ArduIMU v1.5 by Jordi Munoz and William Premerlani, Jose Julio and Doug Weibel
// Substantially rewritten by Prof. G.K. Egan 2010

#if USING_UAVX == 1

byte TxCheckSum;

#if PRINT_UAVX_READABLE == 1

void TxByte(char b)
{
  Serial.print(b);
  Serial.print(',');
} // TxByte

void TxWord(int w)
{
  Serial.print(w);
  Serial.print(','); 
} // TxWord

#else

void TxByte(char b)
{
  Serial.write(b);
  TxCheckSum ^= b;
} // TxByte

void TxWord(int w)
{
  static byte h,l;

  h = highByte(w);
  l = lowByte(w);

  TxCheckSum ^= h;
  TxCheckSum ^= l;

  Serial.write(highByte(w));
  Serial.write(lowByte(w));
} // TxWord

#endif // PRINT_UAVX_READABLE

void SendAttitude(void)
{
  static byte i;

  TxCheckSum = 0;
  Serial.write('$'); // sentinel not included in checksum

  // angles in milliradian
  TxWord((int)(Roll * 1000.0));
  TxWord((int)(Pitch * 1000.0));
  TxWord((int)(Yaw * 1000.0));
  
  for ( i = 0; i < 3 ; i++ )
  TxWord(Gyro[i]);
  
  TxWord((int)(MagHeading * 1000.0));

  TxByte(TxCheckSum);

#if PRINT_UAVX_READABLE == 1
  Serial.println();
#endif // PRINT_UAVX_READABLE

} // SendAttitude

#else

void SendAttitude(void)
{ 
  static byte i,j;

  Serial.print("!");

#if PRINT_EULER == 1
  Serial.print("ANG:");
  Serial.print(Roll * 57.2957795131);
  Serial.print(",");
  Serial.print(Pitch * 57.2957795131);
  Serial.print(",");
  Serial.print(Yaw * 57.2957795131);
  Serial.print(",");
#endif // PRINT_EULER

#if PRINT_AUX == 1
  Serial.print("AUX:");
  Serial.print(GMagnitude); // total g
  Serial.print(",");
  Serial.print(Acc_V[1] / (float)GRAVITY); //slip (y)
  Serial.print(",");
  Serial.print(Acc_V[2] / (float)GRAVITY); //g (z)
  Serial.print(",");
  Serial.print(Rot * 57.2957795131);
  Serial.print(",");
  Serial.print(Velocity * 1.94);
  Serial.print(",");
#endif // PRINT_AUX

#if PRINT_ANALOGS == 1
  Serial.print("AN:");
  for ( i = 0; i < 3 ; i++ ) {
    Serial.print(GyroADC[Map[i]]); 
    Serial.print(",");
  }
  for ( i = 0; i < 3 ; i++ ) {
    Serial.print(AccADC[i]);
    Serial.print (",");
  }
  for ( i = 0; i < 3 ; i++ ) { 
    Serial.print(Mag[i]); 
    Serial.print (","); 
  }
#endif // PRINT_ANALOGS

#if PRINT_DCM == 1
  Serial.print ("DCM:");
  for ( i = 0; i < 3 ; i++ )
    for ( j = 0; j < 3 ; j++ ) {
      Serial.print(DCM_Matrix[i][j]*10000000);
      Serial.print (",");
    }
#endif // PRINT_DCM

#if PRINT_VECTOR == 1
  Serial.print ("VEC:");
  for ( i = 0; i < 3 ; i++ ) {
    Serial.print(Acc_V[i]);
    Serial.print (",");
  }
  for ( i = 0; i < 3 ; i++ ) {
    Serial.print(Gyro_V[i]);
    Serial.print (",");
  }
  for ( i = 0; i < 3 ; i++ ) {
    Serial.print(Omega_V[i]);
    Serial.print (",");
  }
  for ( i = 0; i < 3 ; i++ ) { 
    Serial.print(Mag[i]); 
    Serial.print (","); 
  }
#endif // PRINT_VECTOR

#if PRINT_DEBUG == 1
#endif // PRINT_DEBUG

  Serial.println();    

} // SendAttitude

#endif // USING_UAVX




//
// Display on the LED Panel
//
void SendLEDpanel()
{
  drawPixel(1, 2);


#if 0
  int x = 16;
  int y = 16;
  int l = 16;

  int a, b;
  float gain = 0.5;

  bool bSHowMirror = true;

   MemClear(0x0);

  //
  // AH
  //
  a = gain * Pitch * icos(Roll + 90);
  b = gain * Pitch * isin(Roll + 90);
  l = SCR_WID;

  if (bSHowMirror) {
     MemRadLine((x-a), (y+b), l, Roll);
     MemRadLine((x-a), (y+b), l, Roll+180);
  }
  else {
     MemRadLine((x+a), (y-b), l, Roll);
     MemRadLine((x+a), (y-b), l, Roll+180);
  }

  // Pitch markers
  l = 3;
  for (int step = -80; step <= 80; step+= 20) {
    if (step == 0) continue;

    a = gain * (Pitch+step) * icos(Roll + 90);
    b = gain * (Pitch+step) * isin(Roll + 90);

    if (bSHowMirror) {
	
       MemRadLine((x-a), (y+b), l, Roll);
       MemRadLine((x-a), (y+b), l, Roll+180);
      if (step < 0)  MemSetBlob((x-a), (y+b), 0x0);
    }
    else {
       MemRadLine((x+a), (y-b), l, Roll);
       MemRadLine((x+a), (y-b), l, Roll+180);
      if (step < 0)  MemSetBlob((x+a), (y-b), 0x0);
    }
  }
  MemBlt();
#endif //0
}




