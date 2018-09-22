//=====================================================================================================
// AHRS.h
// S.O.H. Madgwick
// 25th August 2010
//=====================================================================================================
//
// See AHRS.c file for description.
// 
//=====================================================================================================
#ifndef AHRS_h
#define AHRS_h

//----------------------------------------------------------------------------------------------------
// Variable declaration

extern float q0, q1, q2, q3;	// quaternion elements representing the estimated orientation

//---------------------------------------------------------------------------------------------------
// Function declaration

void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

#endif
//=====================================================================================================
// End of file
//=====================================================================================================