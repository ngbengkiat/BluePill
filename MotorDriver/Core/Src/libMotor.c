// ---------------------------------------------------------------------------------
// libMotor.c
// Created on: 12-Feb-2010
// Author: nbk
// Desc:
// InitMotor() is used to initalise the IOs required for motor PID.
// EnWheelMotor() is used to enable motor PID on wheel motor
// DisWheelMotor() is used to disable motor PID on wheel motor
// ---------------------------------------------------------------------------------
#include "main.h"
#include <math.h>
/* Data for motor functions */
// PID gain constant
#define NUM_OF_WHEEL 2
#define NUM_OF_SPEED 2
#define R_ENCODER		(int16_t)LL_TIM_GetCounter(TIM2)
#define L_ENCODER		(int16_t)LL_TIM_GetCounter(TIM3)
#define L_PWM(pwm)		LL_TIM_OC_SetCompareCH1(TIM4, pwm)
#define R_PWM(pwm)		LL_TIM_OC_SetCompareCH2(TIM4, pwm)


volatile float gyroSpeedRad;

volatile float gyroAngleRad;

volatile int16_t kp[NUM_OF_SPEED] =	{0, 0};	// {1000, 6}
volatile int16_t ki[NUM_OF_SPEED] =	{2000, 0};	// {30, 6}
volatile int16_t kd[NUM_OF_SPEED] =	{0, 0};	//{0,900}

int8_t bUseGyro=1;

volatile int16_t variable1=0;
volatile int16_t variable2=0;
volatile int16_t variable3=0;

int16_t iTerm, dTerm, pTerm;


int16_t iSetPWMFlag;
int16_t PWMSetData=200;
int8_t bWheelMotorEnable=1;
int8_t bDoMotorPIDFlag=1;
int8_t bFeedForwardEnableFlag;

volatile int8_t bMotorISRFlag;
// These variables relate to individual wheel control
volatile int16_t motorPWM[NUM_OF_WHEEL];		// motorPWM[0] is for left wheel
												// motorPWM[1] is for right wheel
volatile int16_t newEncoderData[NUM_OF_WHEEL];
volatile int16_t encoderDelta[NUM_OF_WHEEL];
volatile int16_t oldEncoderData[NUM_OF_WHEEL];
volatile float encoderSpeed[NUM_OF_WHEEL];	//in mm/s
volatile int16_t motorFeedFwdVolt[NUM_OF_WHEEL];

// PID control variables
volatile int16_t PIDOut[NUM_OF_SPEED];
volatile int16_t PIDOut_1[NUM_OF_SPEED];
volatile float PIDError[NUM_OF_SPEED];
volatile float PIDError_1[NUM_OF_SPEED];
volatile float dPIDError[NUM_OF_SPEED];
volatile float PIDErrorSum[NUM_OF_SPEED];
volatile float PIDErrorSum_1[NUM_OF_SPEED];

volatile float PIDFeedback[NUM_OF_SPEED];
volatile float PIDFeedback_1[NUM_OF_SPEED];
volatile float PIDInput[NUM_OF_SPEED];

int16_t speedVolt[NUM_OF_SPEED], accVolt[NUM_OF_SPEED];

int16_t maxRotationalOut = (MAXPWMVALUE*7/10);

void CalFeedForwardVolt();

void InitMotor(void)
{
	SetPWM0(0);
	SetPWM1(0);

	DisWheelMotor();
}

void EnWheelMotor() {
	ResetSpeedProfileData();
	bWheelMotorEnable = 1;
}

void DisWheelMotor() {
	bWheelMotorEnable = 0;
}

// ---------------------------------------------------------------------------------
// Motor PID for a wheel chair configuration robot
// Robot movement comprised 2 components, translational (forward & backward)
// and rotational movements.
// The PID is implemented according to the 2 movement components. This is because
// the Kp & Kd parameters might be different for both. Hence the performance for
// such a system should be better.
// ---------------------------------------------------------------------------------

void MotorPID(void)
{
	int i;

	// Read in encoder values.
	// The encoder values, encoderSpeed[] should be positive when robot is moving forward
	newEncoderData[0] = (int16_t)LL_TIM_GetCounter(TIM2);		// left wheel motor encoder
	newEncoderData[1] = (int16_t)LL_TIM_GetCounter(TIM3);		// right wheel motor

	// wheel0 - left wheel
	// wheel1 - right wheel
	// speed0 - x speed
	// speed1 - w speed
	// Calculate encoder difference
	for (i=0; i<NUM_OF_WHEEL; i++) {
		encoderDelta[i] = (newEncoderData[i] - oldEncoderData[i]);
		oldEncoderData[i] = newEncoderData[i];
	}

	// X is the sum of left and right wheels speed
	encoderSpeed[X] = Conv_oc_mm(encoderDelta[0] + encoderDelta[1])/(2*dT);

	// W is the difference between left and right wheel
	// SInce antiClockwise is positive, we take right-left wheel speed

	// ---------------------------------------------------------------------------------
	// PID position control
	// ---------------------------------------------------------------------------------
	/////////////////////////////////////////////////////////
	// Translational servo control
	/////////////////////////////////////////////////////////
	{
		PIDInput[X] = curSpeed[X];// + curAcc[X]*dT/2;
		PIDFeedback[X] = encoderSpeed[X];
		PIDError[X] = PIDInput[X]-PIDFeedback[X];
		dPIDError[X] = PIDError[X]-PIDError_1[X];
		PIDErrorSum[X] += PIDError[X]*dT;

		float limit = (2.0f*MAXPWMVALUE)/( ki[X] );
		CLIP(PIDErrorSum[W], -limit, limit);

		// Calculate PWM for each X component
		PIDOut[X] = kp[X]*PIDError[X] + ki[X]*PIDErrorSum[X] + kd[X]*dPIDError[X];

		PIDError_1[X] = PIDError[X];
		PIDErrorSum_1[X] = PIDErrorSum[X];
		PIDOut_1[X] = PIDOut[X];
		PIDFeedback_1[X] = PIDFeedback[X];

	}
	/////////////////////////////////////////////////////////
	// Rotational servo control
	/////////////////////////////////////////////////////////

	{
		if (bUseGyro){	 						// Gyro all the time.
			PIDInput[W] = curSpeed[W];
			PIDFeedback[W] = RAD_DEG(gyroSpeedRad);
		}

		PIDError[W] = PIDInput[W]-PIDFeedback[W];
//		dPIDError[W] = PIDError[W]-PIDError_1[W];
		dPIDError[W] = PIDFeedback_1[W]-PIDFeedback[W];
		PIDErrorSum[W] += PIDError[W]*dT;

		//float limit = 400.0f/( kd[W] );
		//CLIP(dPIDError[W], -limit, limit);

		{
			PIDOut[W] = 0;//( kp[W]*0.001f )*PIDError[W] + ki[W]*PIDErrorSum[W] + ( kd[W] )*dPIDError[W]*TICK_FREQ/1000;
		}
	}

	PIDError_1[W] = PIDError[W];
	PIDErrorSum_1[W] = PIDErrorSum[W];
	PIDOut_1[W] = PIDOut[W];
	PIDFeedback_1[W] = PIDFeedback[W];

	/////////////////////////////////////////////////////////
	// Calculate individual wheels PWM from X & W components
	/////////////////////////////////////////////////////////
	CLIP(PIDOut[W], -maxRotationalOut, maxRotationalOut );
	CalFeedForwardVolt();
	//PIDOut[0] = PIDOut[1] = 0;
	motorPWM[0] = PIDOut[X] - PIDOut[W] + motorFeedFwdVolt[LEFT_WHEEL];
	motorPWM[1] = PIDOut[X] + PIDOut[W] + motorFeedFwdVolt[RIGHT_WHEEL];

	// Limit maximum PWM value
	int16_t pwm0, pwm1;
	pwm0 = ABS(motorPWM[0]);
	pwm1 = ABS(motorPWM[1]);
	if (pwm0<pwm1) pwm0 = pwm1;
	if (pwm0>(MAXPWMVALUE)) {
		motorPWM[0] = ((int32_t)motorPWM[0]*MAXPWMVALUE)/pwm0;
		motorPWM[1] = ((int32_t)motorPWM[1]*MAXPWMVALUE)/pwm0;
	}

	// In the following, the dir and pwm sign depends on the motor connection
	// To check SetPWM0() & SetPWM1() functions, set the values below.
	//motorPWM[0] = motorPWM[1] = 0;
	if (iSetPWMFlag==1) {
		//For forward open-loop characterisation
		SetPWM0(PWMSetData);
		SetPWM1(PWMSetData);
	}
	else if (iSetPWMFlag==2) {
		//For rotational open-loop characterisation
		SetPWM0(PIDOut[X] - PWMSetData);
		SetPWM1(PIDOut[X] + PWMSetData);
	}
	else if (bWheelMotorEnable){
		DI;		// Disable interrupt so that motor pwm and dir pin are set without interruption
		SetPWM0(motorPWM[0]);
		SetPWM1(motorPWM[1]);
		EI;
	}


	bMotorISRFlag = TRUE;

#define LOG
#ifdef LOG
	extern int mSecCnt;
	if (( curSpeed[X] || curSpeed[X] || iSetPWMFlag))// && (elapsedTime>000) &&(!mSecCnt))	//4500
	{ //curXSpeed || curWSpeed) {	//curFnPtr ||
//		if (bOpenLoopLogFlag ) {
//			StartLog(8);
//			LogHeaderStrgFn("PWM0 kmSpd enSpd0 enSpd1 gyro accel enSpdX kmSpd2");
//			LogDataFn(iSetPWMFlag?PWMSetData:0);
//			LogDataFn(KalmanGetSpeed());
//			LogDataFn(encoderDelta[0]);
//			LogDataFn(encoderDelta[1]);
//			LogDataFn(RAD_DEG(gyroSpeedRad));
//			LogDataFn(accelerometerValue/10);
//			LogDataFn(encoderSpeed[X]);
//			LogDataFn(KalmanGetSpeed2());
//		}
//		else
		{

			//set bFeedForwardEnableFlag = false;
			//bAlignLineFlag = false;
			StartLog(4);
            LogHeaderStrgFn("pidOut curSpeed pwm0 pwm1");
            LogDataFn(PIDOut[0]);
            LogDataFn((int16_t)curSpeed[0]);
			LogDataFn((int16_t)PIDErrorSum[X]);//accelerometerValue/10);
            LogDataFn(motorPWM[1]);//PIDOut[X]);
//			LogDataFn(RAD_DEG(gyroSpeedRad));
//			LogDataFn(debugData[5]);//PIDErrorSum[X]*10);
//			LogDataFn(accelerometerValue/10);//debugData[3]);


		}
	}
#endif
}

// ---------------------------------------------------------------------------------
// Functions to set the motor PWM values
// pwm from -254 to +254
// For positive values, the robot should be moving forward
// Depending on motor mounting, motor driver connection etc, these functions need
// to be customised for different robot design.
// Left motor
// ---------------------------------------------------------------------------------
void SetPWM0(int16_t pwm){
	//CLIP(pwm, -MAXPWMVALUE, MAXPWMVALUE);
	GPIO_Write(LMOTORDIR_PIN, pwm>=0?0:1);
	L_PWM(pwm>=0?pwm:MAXPWMVALUE+pwm);
}

//right
void SetPWM1(int16_t pwm){

	//CLIP(pwm, -MAXPWMVALUE, MAXPWMVALUE);
	GPIO_Write(RMOTORDIR_PIN, pwm>=0?0:1);
	R_PWM(pwm>=0?pwm:MAXPWMVALUE+pwm);
}

// Feedforward voltage obtained from datalogging
// Robot-1
// pwm=200-50(50 friction loss), wSpeed = 1300dps, aAcc = 6000dps^2, wDec = 1300/0.17 = 7600dps^2
// pwm=250, xSpeed = 2400mm/s aAcc = 9300mm/s^2, xDec = 7000mm/s^2

void CalFeedForwardVolt() {
	int16_t wAcc, wSpeed;
/*************************************************************/
/* Calculate feedforward voltage						 */
/*************************************************************/
#define wSpeedVoltMplier 	0.14f//0.143f//(0.1f) ?????
#define wAccVoltMplier 		0.018f	//(0.018f)
#define wDecVoltMplier 		0.015f	//(0.015f)

#define xSpeedVoltMplier 	0.1f
#define xAccVoltMplier 		0.019f	//0.02f
#define xDecVoltMplier 		0.016f	//0.018f


	speedVolt[X] = (xSpeedVoltMplier*curSpeed[X]);
	if (curAcc[X]>=0) {
		accVolt[X] = xAccVoltMplier*curAcc[X];

	}
	else {
		accVolt[X] = xDecVoltMplier*curAcc[X];
	}

	wSpeed = curSpeed[W];

	wAcc = curAcc[W];

	if (((wAcc>=0) && (wSpeed>0)) || ((wAcc<0) && (wSpeed<0)) ){
		accVolt[W] = wAccVoltMplier*wAcc;
	}
	else {
		accVolt[W] = wDecVoltMplier*wAcc;
	}
	//CLIP(accVolt[W], -MAXPWMVALUE*2/10, MAXPWMVALUE*2/10);

	speedVolt[W] = (wSpeedVoltMplier*(wSpeed));

	motorFeedFwdVolt[LEFT_WHEEL] = speedVolt[X] + accVolt[X];	//left wheel
	motorFeedFwdVolt[RIGHT_WHEEL] = motorFeedFwdVolt[LEFT_WHEEL];	//right wheel

	speedVolt[W] += accVolt[W];
	CLIP(speedVolt[W], -maxRotationalOut, maxRotationalOut );
	motorFeedFwdVolt[LEFT_WHEEL] -= speedVolt[W];
	motorFeedFwdVolt[RIGHT_WHEEL] += speedVolt[W];

	if (!bFeedForwardEnableFlag)
	{
		motorFeedFwdVolt[LEFT_WHEEL] = 0;
		motorFeedFwdVolt[RIGHT_WHEEL] = 0;
	}
}
