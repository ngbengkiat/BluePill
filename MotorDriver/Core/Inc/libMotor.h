// ---------------------------------------------------------------------------------
// file : libmotor.h
// date : 05/7/2007
// name : nbk
// desc : header file for libmotor.c
// This is to serve as an example on how motor control is done.
// This is meant for 2 wheel robot.
// A simple speed profile controller and motor PID is implmented.
// Students must modify the various macros and c- source code to suit their own robot.
// ---------------------------------------------------------------------------------

#ifndef _LIBMOTOR_H_
#define _LIBMOTOR_H_


// ---------------------------------------------------------------------------------
//	Robot data
//  Robot speed and distance control are done in real unit (mm and mm/sec)
//	The following macros allow us to easily convert real unit in mm & mm/sec into encoder counts
//  Change these constants to match that of your robot
#define ENCODER		(256*4)			//encoder resolution per revolution

#define W_DIAMETER	(24.2f)			// Wheel diameter
#define W_DISTANCE	(101.5)			// Wheel to wheel distance
#define GEAR_RATIO	(40/12.0f)		// Wheel to motor gear ratio


#define fCNT_PER_MM	(float)((ENCODER)*(GEAR_RATIO)/(W_DIAMETER*M_PI))

#define Conv_mm_oc(dist)	(float)(((fCNT_PER_MM)*(dist)))	// Convert distance to optical count
#define Conv_oc_mm(cnt)		(float)(((cnt))/(fCNT_PER_MM))	// Convert optical count to distance
#define ANGLE_ADJUST		0							// adjust this value to make 360 accurate if necessary

////////////////////////////////////////////////////////////////////////////////////
// There are 2 wheels. It is convenient to store wheels' related data into array
////////////////////////////////////////////////////////////////////////////////////
#define NUM_OF_WHEEL	2		// 0 - left
								// 1 - right
#define LEFT_WHEEL		0
#define RIGHT_WHEEL		1

// There are 2 kind of speeds and distance, linear x-speed and rotational w-speed.
// It is also convenient to theis 2 kinds of related data into array
#define NUM_OF_SPEED	2		// 0 - xSpeed
								// 1 - wSpeed (angular)
#define 	X		0
#define 	W		1

#define MAXPWMVALUE			999
////////////////////////////////////////////////////////////////////////////////////
// Global data
////////////////////////////////////////////////////////////////////////////////////
extern volatile float gyroSpeedRad;

extern volatile float gyroAngleRad;

extern volatile int8_t bMotorISRFlag;
extern int16_t iSetPWMFlag;
extern int16_t PWMSetData;
extern int16_t xControlType;
extern int16_t wControlType;
extern int8_t bWheelMotorEnable;
extern int8_t bDoMotorPIDFlag;
extern int8_t bUseGyro;
extern int8_t bFeedForwardEnableFlag;

volatile extern int16_t kp[NUM_OF_SPEED];		// kp[1] is for W-component
volatile extern int16_t kd[NUM_OF_SPEED];		// kd[1] is for W-component
volatile extern int16_t ki[NUM_OF_SPEED];		// kd[1] is for W-component

extern int16_t volatile variable1, variable2, variable3;
extern volatile int16_t PIDOut_1[NUM_OF_SPEED];
extern volatile float PIDError[NUM_OF_SPEED];
extern volatile float PIDError_1[NUM_OF_SPEED];
extern volatile float dPIDError[NUM_OF_SPEED];
extern volatile float PIDErrorSum[NUM_OF_SPEED];
extern volatile float PIDErrorSum_1[NUM_OF_SPEED];

extern volatile float PIDFeedback[NUM_OF_SPEED];
extern volatile float PIDInput[NUM_OF_SPEED];
extern volatile int16_t oldEncoderData[NUM_OF_WHEEL];
extern volatile int16_t newEncoderData[NUM_OF_WHEEL];
extern volatile int16_t encoderDelta[NUM_OF_WHEEL];
extern volatile float encoderSpeed[NUM_OF_WHEEL];	//in mm/s
extern volatile int16_t motorPWM[NUM_OF_WHEEL];
extern volatile float curPos[NUM_OF_SPEED];
extern volatile float finalPos[NUM_OF_SPEED];
extern volatile float curSpeed[NUM_OF_SPEED];
extern volatile float targetSpeed[NUM_OF_SPEED];
extern volatile float targetEndSpeed[NUM_OF_SPEED];
extern volatile int16_t moveState[NUM_OF_SPEED];

extern float curAcc[NUM_OF_SPEED];
extern float desiredAcc[NUM_OF_SPEED];
extern float desiredDec[NUM_OF_SPEED];
extern float jerk[];
extern float afterBrakeDist[NUM_OF_SPEED];	// Speed profile use
											// after brake(deceleration) distance
											// In case there is not enough distance to brake
extern int8_t bDistDirFlag[NUM_OF_SPEED];	// To indicate direction of move in speed profile
// ---------------------------------------------------------------------------------
// Function prototypes
// Some functions are used internally by the libraries.
// You should never have to call them directly.
// For your convenience, the functions you are likely to use are placed in front.
// The internal functions are place behind.
// ---------------------------------------------------------------------------------

void InitMotor();		// To initialise the 2 drive motor hardware.
						// You need to invoke this function once in main()
						// before enabling the interrupt

void DisWheelMotor();	// Disable the 2 drive wheels
void EnWheelMotor() ;	// Enable the 2 drive wheels

void StopRobot(void);	// Stop robot movement immediately
						// May result in robot skidding if current speed is high.

// ---------------------------------------------------------------------------------
// The following functions are used to set the current target speed of the robot.
// Not to be used with the MoveRobot() or SetMoveCommand()
// ---------------------------------------------------------------------------------
void SetRobotSpeedX( float xSpeed);
void SetRobotSpeedW( float wSpeed);
void SetRobotAccX( float acc);
void SetRobotAccW( float acc);

// ---------------------------------------------------------------------------------
// Set the speed profile parameters. It makes sure the robot travel the dist according
// to the speed and acceleration parameters.
// ---------------------------------------------------------------------------------
void MoveRobot(int16_t speedType, float dist, float brakeDist, float topSpeed, float endSpeed, float acc);

// ---------------------------------------------------------------------------------
// Wait for the robot to travel certain distance. Must make sure the distance is less than
// final target distance
// ---------------------------------------------------------------------------------
void WaitDist(int16_t speedType, int16_t dist);

// ---------------------------------------------------------------------------------
// SetMoveCommand() is the same as MoveRobot() except that it doesn't wait for the
// robot to travel the distance required.
// ---------------------------------------------------------------------------------
void SetMoveCommand(int16_t speedType, float dist, float brakeDist, float topSpeed, float endSpeed, float acc);

// ---------------------------------------------------------------------------------
// Wait for end of speed profile. Basically, the functions just check if the distance to
// travel is reached.
// ---------------------------------------------------------------------------------
int EndOfMove(int moveCmd);

// ---------------------------------------------------------------------------------
// Move robot to position x,y
// Based on cartesian x,y coordinates
// ---------------------------------------------------------------------------------
void GotoPos( );
void TestGotoPos();			// function to test GotoPos()
void PrintMotorStatus();	// Print motor internal status. For debugging
							// You can use this to observe the motor encoder values


// ---------------------------------------------------------------------------------
// For casual users of this library, you should never have to call the functions below.
// ---------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------
// Control the speed profile of robot based on distance left to travel, current speed,
// deceleration and end speed. It has 2 different speed control, the translational and
// rotational speed control
// User should never have to call this function directly
// ---------------------------------------------------------------------------------
void DoMoveCommand();


void MotorPID(void);		// Does the motor PID and robot move speed profile
							// Called from a regular interrupt routine

// ---------------------------------------------------------------------------------
// This function will keep adjusting the curSpeed of the robot until it reaches the
// targetSpeed. The main function would have to set the targetSpeed, and the curAcc[] in
// order to control the speed profile.
// Called from MotorPID().
// ---------------------------------------------------------------------------------
void UpdateCurSpeed();
void UpdateWheelPos();
void UpdateRobotPos();


// Calculate the deceleration required based on input parameters.
float GetDecRequired(float dist, float brakeDist, float curSpeed, float endSpeed);

// ---------------------------------------------------------------------------------
// Set the PWM output for the 2 drive motors.
// Functions to set the motor PWM values
// pwm from -999 to +999
// For positive values, the robot should be moving forward
//
// Depending on motor mounting, motor driver connection etc, these functions need
// to be customised for different robot design.
//
// This function is used by the motor PID controller
// Do not call these functions unless you disable the motor PID
// ---------------------------------------------------------------------------------
void SetPWM0(int16_t pwm);	// left motor
void SetPWM1(int16_t pwm);	// right motor

// ---------------------------------------------------------------------------------
// This function implements the speed profile generator.
// Should be called only from the motor ISR
// ---------------------------------------------------------------------------------
void DoSpeedProfile();

// ---------------------------------------------------------------------------------
// Reset the drive motor speed profile internal data
// This should be called once only after the motor drive wheels are just enabled
// User should never have to call this function directly.
// ---------------------------------------------------------------------------------
void ResetSpeedProfileData();

#endif
