////////////////////////////////////////////////////////////////////////////////////
// file : libLogData.c
// name : nbk
// desc :
////////////////////////////////////////////////////////////////////////////////////
#include "main.h"

#define LOG
#ifdef LOG
#define LOGINDEXSIZE			4000
#define LOGINDEXSIZEFLOAT		LOGINDEXSIZE/2
union {
	int16_t i[LOGINDEXSIZE];
	float   f[LOGINDEXSIZE/2];
}logData;


int32_t logIndex, logWidth=4;
int8_t logDataFlag = 0;
char *headerStr=NULL;

void LogHeaderStrgFn(char *header){
	headerStr = header;
}

void LogFloatDataFn(float d)  {
	if (logIndex>=LOGINDEXSIZEFLOAT) {
		//logIndex=0;
	}
	else {
		logData.f[logIndex] = d;
		logIndex++;
		//logDataFlag = 1;
	}
}

void LogDataFn(int16_t d0) {
		if (logIndex>=LOGINDEXSIZE) {
			return;
			logIndex=0;
		}

		{
    			logData.i[logIndex] = d0;
				logIndex++;
				logDataFlag = 1;
		}
}

void PrintLog( ) {
	int32_t i, j;
	printf("%s\n", headerStr);

	for (i=0; i<logIndex;) {
//	for (i=0; i<LOGINDEXSIZE;) {
		for (j=0; j<logWidth; j++, i++) {
//			if (i==logIndex)
//				printf("-1 ");
			printf("%5d ", logData.i[i]);
		}
		printf("\n");

	}

}

void PrintLogFloat( ) {
	int32_t i, j;
	for (i=0; i<LOGINDEXSIZEFLOAT;) {
		for (j=0; j<logWidth; j++) {
			if (j==logIndex)
				printf("-1 ");
			else
				printf("%8.3f ", logData.f[i++]);
		}
		printf("\n");

	}
}

void StartLog(int16_t num){
	//logDataFlag = 1;
	logWidth = num;
}

#else
void PrintLog() {
}

void ClearLog() {
}

#endif

float logDelay;

void ClearLog() {

	logIndex = 0;
	logDataFlag = 0;
	logDelay = dT/2;

}
