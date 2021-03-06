// ---------------------------------------------------------------------------------
// file : liProfile.c
// name : nbk
// date : 12 Nov 2008
// desc : Speed profile source code for a DC motor 2-wheel drive robot
// SetRobotSpeedX() & SetRobotSpeedW() - Set robot target translational(X) & rotational(W) speeds
// SetRobotAccX() & SetRobotAccW() - Set robot target translational(X) & rotational(W) accelerations
// EndOfMove() - Check if current speed profile generator has finished
// SetMoveCommand() - Set a new speed profile for the profile generator
// MoveRobot- Set a new speed profile for the profile generator & wait for the robot to finish
//				running that profile
// ---------------------------------------------------------------------------------
#include "main.h"


volatile float robotPosInc[NUM_OF_SPEED];

volatile float curPos[NUM_OF_SPEED];
volatile float finalPos[NUM_OF_SPEED];

float curAcc[NUM_OF_SPEED] = {1000,		// curAcc[0] is for X-component
							  1000};	// curAcc[1] is for W-component

volatile float targetSpeed[NUM_OF_SPEED];
volatile float targetEndSpeed[NUM_OF_SPEED];
volatile float curSpeed[NUM_OF_SPEED];
volatile int16_t moveState[2];

int8_t bDistDirFlag[NUM_OF_SPEED];
float afterBrakeDist[NUM_OF_SPEED];
volatile int uncertainty;

void ResetSpeedProfileData() {
	int i;
	DI;
	for (i=0; i<NUM_OF_SPEED; i++) {
		curSpeed[i]=0;
		targetSpeed[i]=0;
		curPos[i]=0;
		moveState[i]=-1;

	}
	EI;
}

// Robot speed is specified in term of translational speed and rotational speed
// The translational speed is effectively the sum of the left and right wheel speed.
// The rotaional speed is effectively the difference of the left and right wheel speed.
// xSpeed is in cm/sec
// wSpeed is in 10xDeg/sec
//
// ---------------------------------------------------------------------------------
// Set the robot targetSpeed
// The UpdateCurSpeed() function will update the curSpeed[] to the new targetSpeed[]
// according to the acceleration value in curAcc[]
// ---------------------------------------------------------------------------------

// @brief : To move in x-direction
// @param : xSpeed in mm/s
void SetRobotSpeedX( float xSpeed)
{
	targetSpeed[0] = (xSpeed);	// mm to optical count conversion
}

// @brief : To rotate robot
// @param : wSpeed in deg/s
void SetRobotSpeedW( float wSpeed)
{
	targetSpeed[1] = (wSpeed);
}

// @brief : To set acceleration for x-movement
// @param : acc in mm/s/s
void SetRobotAccX( float acc)
{
	curAcc[0] = (acc);
}

// @brief : To set acceleration for rotational-movement
// @param : acc in deg/s/s
void SetRobotAccW( float acc)
{
	curAcc[1] = (acc);
}

// ---------------------------------------------------------------------------------
// This function updates the curPos[] and PIDInput[] based on the curSpeed[].
// It is similar to UpdateRobotPos().
// This function is mainly used for the speed profile generator, whereas UpdateRobotPos()
// is used for keeping track of the robotX,Y,dir in real unit.
//
// Should combine the two functions in future.
// ---------------------------------------------------------------------------------
void UpdateWheelPos() {
	int16_t i;
	float inc;

	for (i=0; i<NUM_OF_SPEED; i++) {
		inc = (curSpeed[i]*dT + curAcc[i]*dT*dT/2);

		curPos[i] += inc;	// add the whole number

		PIDInput[i] = inc;

	}

}



// ---------------------------------------------------------------------------------
// This function will keep adjusting the curSpeed[] of the robot until it reaches the
// targetSpeed. The main function would have to set the targetSpeed, and the curAcc[] in
// order to control the speed profile.
// ---------------------------------------------------------------------------------
void UpdateCurSpeed() {
	register int i;

	for (i=0; i<NUM_OF_SPEED; i++) {

		if (curSpeed[i] < targetSpeed[i]) {
			curSpeed[i] += curAcc[i]*dT;
			if (curSpeed[i] > targetSpeed[i])
				curSpeed[i] = targetSpeed[i];
		}
		else if (curSpeed[i] > targetSpeed[i]) {
			curSpeed[i] -= curAcc[i]*dT;
			if (curSpeed[i] < targetSpeed[i])
				curSpeed[i] = targetSpeed[i];
		}
	}

}

// ---------------------------------------------------------------------------------
// Get the deceleration required
// Deceleration = (curSpeed*curSpeed - endSpeed*endSpeed)/(2*distToDec);
// Note that speed and acceleration are both fixed pt represented.
// Hence when speed*speed, the decimal pt position is doubled and needs to be
// shifted back (i.e. the result of speed*speed is 12 bit fixed point).
// ---------------------------------------------------------------------------------
float GetDecRequired(float dist, float afterBrakeDist, float curSpeed, float endSpeed) {

	if (dist<0 && curSpeed>=0) return 0;
	if (dist>0 && curSpeed<=0) return 0;
	if (curSpeed<0) curSpeed = -curSpeed;
	if (endSpeed<0) endSpeed = -endSpeed;
	if (dist<0) dist = -dist;
	if (endSpeed>=curSpeed) return 0;
	if (dist<=afterBrakeDist) return (9800);


	//(curXSpeed*curXSpeed - endXSpeed*endXSpeed)/(2*(finalPosX-curPosX-brakeAllowanceX))
	// Note that both CS() and CA() multiply the actual speed and acc
	// by 128 due to fix point notation. Hence squaring the speed increases
	// the result to 14-bit fixed point.
	// Hence, the result is increased to 14-bit fixed pt.
	// To reduce result back to 7-bit fix-point, the numerator needs to be shifted right
	// 7 bits. Including the div-2 in the equation, it's 8-bits.

	dist-=afterBrakeDist;

	return ((curSpeed*curSpeed - endSpeed*endSpeed)/dist);
}


// ---------------------------------------------------------------------------------
// This function checks for end of current move
// A move ends when the distance set for the speed profile is reached.
// ---------------------------------------------------------------------------------
int EndOfMove(int moveCmd) {
	return moveState[moveCmd] == -1;
}

// ---------------------------------------------------------------------------------
// It set the targetSpeed[] of the robot according to the speed profile parameters.
// This is called from a regular interrupt.
// Both the Xspeed and Wspeed profile run in parallel
//
// The profile generator keeps checking the distance left to travel with the
// curAcc[]. When it's time to decelerate, it will set the targetSpeed[]to the
// targetEndSpeed[]
// ---------------------------------------------------------------------------------
void DoMoveCommand( ) {
	int i;
	unsigned decelerationRequired;

	for (i=0; i<NUM_OF_SPEED; i++) {

		if (moveState[i] == -1) continue;	// No active profile

		decelerationRequired = GetDecRequired(finalPos[i]-curPos[i], afterBrakeDist[i], curSpeed[i], targetEndSpeed[i] );

		if (decelerationRequired>=ABS(curAcc[i]) ) {
			// Time to decelerate
#ifndef xxx
			if (moveState[i]==0) {
				moveState[i] = 1;
				targetSpeed[i] = targetEndSpeed[i];
				if (targetSpeed[i] == 0) {
					// when targetSpeed[i] == 0, the robot might not
					// be able to reach the final position due to
					// truncation error here and there.
					// solution is to temporarily set the speed slightly
					// more than zero.
					targetSpeed[i]=bDistDirFlag[i]?curAcc[i]:-curAcc[i];
				}
			}
#else
			moveState[i] = 1;
			targetSpeed[i] = targetEndSpeed[i];
#endif
			// set curAcc[i] to decelerationRequired else the robot
			// might not be able to reach targetSpeed[i] when the
			// target distance is reached. The result is that the robot
			// might need to stop at a high speed which caused it to
			// jerk.
			// Changing the curAcc[i] here means that we do not know what's
			// the curAcc[i] after a speed profile is completed and
			// we need to set curAcc[i] again. If you do not want
			// curAcc[i] to change, comment off the following line.
			curAcc[i] = decelerationRequired;
		}

		if (bDistDirFlag[i]) {
			// distance is towards positive direction
			if (finalPos[i]<=curPos[i]) {
				moveState[i] = -1;
				targetSpeed[i] = targetEndSpeed[i];
			}
		}
		else {
			if (finalPos[i]>=curPos[i]) {
				moveState[i] = -1;
				targetSpeed[i] = targetEndSpeed[i];
			}
		}


	}

}

void StopRobot(void)
{
	ResetSpeedProfileData();
}

// ---------------------------------------------------------------------------------
// @brief : To move Robot a certain distance. Distance can be translational (in mm) or
// 			rotational(in degree)
// @param  see SetMoveCommand()
void MoveRobot(int16_t speedType, float dist, float brakeDist, float topSpeed, float endSpeed, float acc) {

	SetMoveCommand(speedType, dist, brakeDist,  topSpeed, endSpeed, acc);

	while(!EndOfMove(speedType)) {
		// Do other stuff here!!!
		// like checking for sensors to detect object etc
//		if (bSWFlag) {	// user switch break!
//			break;
//		}
	}
}

// ---------------------------------------------------------------------------------
// Wait for the robot to travel certain distance. Must make sure the distance is less than
// final target distance
// ---------------------------------------------------------------------------------
void WaitDist(int16_t speedType, int16_t dist) {
	float dist32;
	if (speedType == X) {
		dist32 = (dist)+curPos[X];
	}
	else {
		dist32 = (dist)+curPos[W];
	}
	if (bDistDirFlag[speedType]) {
		// positive direction
		while (dist32>curPos[speedType]);
	}
	else {
		while (dist32<curPos[speedType]);

	}

}

// ---------------------------------------------------------------------------------
// @brief : To set up the speed profile for movement.
// @param : speedType == 0 : XSPEED (forward/backward) movement
// 			speedType == 1 : WSPEED (rotational) movement
// @param : dist(mm or deg) is the distance(translational or angular) the robot will move
// @param :	topSpeed is the maxSpeed of the speed profile
// @param :	endSpeed is the ending speed of the profile (best not set to zero!!!!)
//			(else the robot may not reach the final distance due to truncation error)
// @param :	brakeDist is a parameter to get the robot to decelerate(brake) earlier so that
// 			there is a certain safety margin
//
// Note that both XSPEED and WSPEED profiles can be active at the same time.
// In that case, the robot will be moving in a curve.
//
// The profile generator runs regularly in the background through a ISR.
// It ends when the distance to move is reached or exceeded.
// ---------------------------------------------------------------------------------
void SetMoveCommand(int16_t speedType, float dist, float brakeDist, float topSpeed, float endSpeed, float acc) {

	float dist32;
	if (speedType == X) {
		dist32 = (dist);
		brakeDist = (brakeDist);
		topSpeed = (topSpeed);
		endSpeed = (endSpeed);
		acc 	 = (acc);
	}
	else {
		dist32 = (dist);
		brakeDist = (brakeDist);
		topSpeed = (topSpeed);
		endSpeed = (endSpeed);
		acc 	 = (acc);
	}
	// Init speed profile parameters
	if (dist==0) {
		moveState[speedType]=-1;
		targetSpeed[speedType] = endSpeed;
		return;
	}

	DI;
	finalPos[speedType] = dist32;
	curPos[speedType] = 0;
	afterBrakeDist[speedType]=brakeDist;

	if (dist<0) {
		topSpeed = -topSpeed;
		endSpeed = -endSpeed;
		bDistDirFlag[speedType]=FALSE;		// reverse direction
	}
	else {
		bDistDirFlag[speedType]=TRUE;		// forward (+ve) distance direction
	}

	targetSpeed[speedType] = topSpeed;
	targetEndSpeed[speedType] = endSpeed;
	curAcc[speedType] = acc;
	moveState[speedType] = 0;

	EI;
}

// ---------------------------------------------------------------------------------
// Use this routine only if you need to keep track of robot position.
// To update the robot position based on the curSpeed[].
// This allows the robot to keep track of it's position based on odometry (motor encoder).
// Output in robotPos.
// roboPos.x & robotPos.y contain the x&y coordinates in mm.
// robotPos.dir contains the direction in 0.1 deg.
//
// Following universal mathematical convention,
// positive angle is anticlockwise & negative is clockwise
// Similarly, Y-axis is positive in North or 90 degree direction &
// X-axis is positive in East or 0-degree direction
//
// It is important to follow this convention as we are using the standard
// trigo functions like sine() & cosine() to update the robot position
//
// Use compass or curSpeed[WSPEED]? Combined???
// ---------------------------------------------------------------------------------
int robotPosIncX, robotPosIncY, robotPosIncT;
int robotPosIncW;

void UpdateRobotPos() {

}

void DoSpeedProfile() {
	if (bWheelMotorEnable)
	{
		DoMoveCommand();		// speed profile

		UpdateCurSpeed();		// update curSpeed[] according to targetSpeed[]

		UpdateWheelPos();

		UpdateRobotPos();		// To update the robot's current position based on the robot
		if (curSpeed[1] || curSpeed[0]) {
	//		LogDataFn(debugData0, debugData1, curSpeed[0], curSpeed[1]);
	//		LogDataFn(curSpeed[0], curSpeed[1], curTargetPos[0], finalPos[0]);
		}
	}
	else
		ResetSpeedProfileData();
}
