// ---------------------------------------------------------------------------------
// libTestMotor.c
// Date : 3/04/2007
// Name : nbk
// Desc : PrintMotorMenu() is for testing the various motors and speed profile functions.
// ---------------------------------------------------------------------------------
#include "main.h"
#include "project.h"
#include <ctype.h>


void PrintMotorMenu();

void PrintMotorMenu() {
	clrscr();
    printf("\n ? - Print this menu");
    printf("\n 1 - Drive motor %s",(bWheelMotorEnable)?"enabled":"Disable");
    printf("\n 2 - fwd Straighht run");
    printf("\n 3 - RWD Straight  run");
    printf("\n 4 - Turn on spot");
    printf("\n 5 - Move forward 20cm");
    printf("\n 6 - Move backward 20cm");
    printf("\n 7 - Move 90 deg CCW");
    printf("\n 8 - Move 90 deg CW");
    printf("\n 9 - Move forward, curve turn and forward");
    printf("\n P - Test motor pwm");
    printf("\n Q - Quit");
}

#define START_ROW		17
void PrintMotorStatus() {
	    gotoxy(12,START_ROW);
		printf("encoder0=%-6d  1=%-6d",
		oldEncoderData[0], oldEncoderData[1]);

	    gotoxy(12,START_ROW+2);
		printf("PIDErrorSum0=%-6.2f 1=%-6.2f 2=%-6.2f", PIDErrorSum[0], PIDErrorSum[1], PIDErrorSum[2]);
	    gotoxy(12,START_ROW+3);
		printf("PIDError0=%-6.1f 1=%-6.1f 2=%-6.1f", PIDError[0], PIDError[1], PIDError[2]);

	    gotoxy(12,START_ROW+4);
		printf("encDiff0=%-6d  1=%-6d ", encoderDelta[0], encoderDelta[1]);

	    gotoxy(12,START_ROW+6);
		printf("motorPWM0=%-6d  1=%-6d ", motorPWM[0], motorPWM[1]);

	    gotoxy(12,START_ROW+7);
		printf("curPos0=%-6.1f  1=%-6.1fd 2=%-6.1fd", curPos[0], curPos[1], curPos[2]);
	    gotoxy(12,START_ROW+8);
		printf("finalPos0=%-6.1f  1=%-6.1f 2=%-6.1f", finalPos[0], finalPos[1], finalPos[2]);
	    gotoxy(12,START_ROW+9);
		printf("curSpeed0=%-6.1f  1=%-6.1f 2=%-6.1f", curSpeed[0], curSpeed[1], curSpeed[2]);

	    gotoxy(12,START_ROW+10);
		printf("targetSpeed0=%-6.1f  1=%-6.1f 2=%-6.1f", targetSpeed[0], targetSpeed[1], targetSpeed[2]);

}
void TestMotorPWM(){
	int16_t	pwm = 0;
	int8_t bFlag = 1;
	clrscr();
	printf("\n+ : Increase PWM");
	printf("\n- : Decrease PWM");
	printf("\nq : quit");

	while(bFlag){
		gotoxy(5,5);
		printf("pwm = %5d", pwm);
		switch(GetChar()) {
		case '+':pwm+=1;
				break;
		case '-':pwm-=1;
				break;
		case 'q':
		case 'Q':bFlag = FALSE;
				break;
		}
		SetPWM0(pwm);
		SetPWM1(pwm);
	}
}
// Menu for testing various speed profile functions
void TestMotorMenu() {
	char c;

	EnWheelMotor();
	HAL_Delay(5);
	SetRobotSpeedX(0);
	SetRobotSpeedW(0);

	PrintMotorMenu();
	while(1)  {

		if (kbhit())
		{

			c = toupper(GetChar());

			switch(c) {
			case 'Z':PrintLog( );
					break;
			case 'Q':
					DisWheelMotor();
					return;

			case  'P':
					bDoMotorPIDFlag=FALSE;
					TestMotorPWM();
					bDoMotorPIDFlag=TRUE;
					break;

			case  '1':
					if (bWheelMotorEnable)
						DisWheelMotor();
					else{
						//CalibrateGyroDC();
						EnWheelMotor();
					}
					PrintMotorMenu();
					break;
			case '2':
					// No auto speed profile
					SetRobotSpeedX(500);
					HAL_Delay(500);
					SetRobotSpeedX(0);
					HAL_Delay(500);
					break;
			case '3':
					// No auto speed profile
					SetRobotSpeedX(-500);
					HAL_Delay(500);
					SetRobotSpeedX(0);
					HAL_Delay(500);
					break;
			case '4':
					// No auto speed profile
					SetRobotAccW(1000);
					SetRobotSpeedW(360);
					HAL_Delay(500);
					SetRobotSpeedW(0);
					HAL_Delay(500);
					break;
			case '5':
					// Auto speed profile
					// Robot will travel a designated distance using the
					// speed and acceleration parameters given.
					MoveRobot(X, 100, 0, 200, 10, 1000);
					StopRobot();
					break;
			case '6':
					MoveRobot(X, -100, 0, 500, 10, 1000);
					StopRobot();
					break;
			case '7':
					MoveRobot(W, 90, 0, 360, 10, 1800);
					StopRobot();
					break;
			case '8':
					MoveRobot(W, -90, 0, 360, 10, 1800);
					StopRobot();
					break;
			case '9':
					// Move 2cm
					MoveRobot(X, 30, 0, 300, 300, 500);
					// Curve turn 90 degrees
					MoveRobot(W, -45, 0, 1800, 10, 1800);
					// Move 2cm
					MoveRobot(X, 30, 0, 300, 0, 500);
					StopRobot();
					break;
			case '0':
					finalPos[W] = 360;
					curPos[W] = 0;
					targetSpeed[W] = 2000;
					targetEndSpeed[W] = 0;
					bDistDirFlag[W]=1;
					moveState[W]=0;
					break;
			}
		}
	PrintMotorStatus();

	}
}


