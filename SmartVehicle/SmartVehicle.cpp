// Smart Vehicle

#include <stdio.h>
#include <signal.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <assert.h>
#include <string.h>

#include "ZWaitObj.h"
#include "ZSyncObj.h"

#define PIN_BR1	1
#define PIN_BR2	4
#define PIN_BL1	5
#define PIN_BL2	6
#define PIN_FR1	26
#define PIN_FR2	27
#define PIN_FL1	28
#define PIN_FL2	29

#define DIR_FORWARD  1
#define DIR_BACKWARD 2

struct termios org_opts, new_opts;
char g_chCommand = '\0';

//#define CONTROL_BY_KEYBORD
#define CONTROL_BY_IR
#if defined CONTROL_BY_IR
	#define PIN_IR_A	25
	#define PIN_IR_B	24
	#define PIN_IR_C	23
	#define PIN_IR_D	22
#endif

unsigned int GetTickCount()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec*1000 + time.tv_nsec/1000000;
}


class LockObj
{
public:
	LockObj() {}
	~LockObj() {}
	ZWaitObj zWaitObj;
	ZSyncObj zSyncObj;
private:
	LockObj(const LockObj& src);
	LockObj& operator = (const LockObj& src);
};

// delay ms
void delayMS(float x)
{
	if (x < 0.001)
	{
		return;
	}
	usleep(x * 1000);
}

// move base
void MoveBase(int PINA, int PINB, int dir)
{
	if (DIR_FORWARD == dir)
	{
		digitalWrite(PINA, 1);
		digitalWrite(PINB, 0);
	}
	else if (DIR_BACKWARD == dir)
	{
		digitalWrite(PINA, 0);
		digitalWrite(PINB, 1);
	}
}

// stop base
void StopBase(int PINA, int PINB)
{
	digitalWrite(PINA, 0);
	digitalWrite(PINB, 0);
}

// move BR
void MoveBR(int dir)
{
	MoveBase(PIN_BR1, PIN_BR2, dir);
}
// move BL
void MoveBL(int dir)
{
	MoveBase(PIN_BL1, PIN_BL2, dir);
}
// move FR
void MoveFR(int dir)
{
	MoveBase(PIN_FR1, PIN_FR2, dir);
}
// move FL
void MoveFL(int dir)
{
	MoveBase(PIN_FL1, PIN_FL2, dir);
}

// stop BR
void StopBR()
{
	StopBase(PIN_BR1, PIN_BR2);
}
// stop BL
void StopBL()
{
	StopBase(PIN_BL1, PIN_BL2);
}
// stop FR
void StopFR()
{
	StopBase(PIN_FR1, PIN_FR2);
}
// stop FL
void StopFL()
{
	StopBase(PIN_FL1, PIN_FL2);
}


// move Forward
void MoveForward(int speedlvl)
{
	if (speedlvl < 4 || speedlvl > 100)
	{
		return;
	}
	MoveBR(DIR_FORWARD);
	MoveBL(DIR_FORWARD);
	MoveFR(DIR_FORWARD);
	MoveFL(DIR_FORWARD);
	delayMS(speedlvl);

	StopBR();
	StopBL();
	StopFR();
	StopFL();
	delayMS(100-speedlvl);
}

// move Backward
void MoveBackward(int speedlvl)
{
	if (speedlvl < 4 || speedlvl > 100)
	{
		return;
	}
	MoveBR(DIR_BACKWARD);
	MoveBL(DIR_BACKWARD);
	MoveFR(DIR_BACKWARD);
	MoveFL(DIR_BACKWARD);
	delayMS(speedlvl);

	StopBR();
	StopBL();
	StopFR();
	StopFL();
	delayMS(100-speedlvl);
}

// turn Left
void TurnLeft(int speedlvl)
{
	if (speedlvl < 4 || speedlvl > 100)
	{
		return;
	}
	MoveBR(DIR_FORWARD);
	MoveBL(DIR_BACKWARD);
	MoveFR(DIR_FORWARD);
	MoveFL(DIR_BACKWARD);
	delayMS(speedlvl);

	StopBR();
	StopBL();
	StopFR();
	StopFL();
	delayMS(100-speedlvl);
}

// turn Right
void TurnRight(int speedlvl)
{
	if (speedlvl < 4 || speedlvl > 100)
	{
		return;
	}
	MoveBR(DIR_BACKWARD);
	MoveBL(DIR_FORWARD);
	MoveFR(DIR_BACKWARD);
	MoveFL(DIR_FORWARD);
	delayMS(speedlvl);

	StopBR();
	StopBL();
	StopFR();
	StopFL();
	delayMS(100-speedlvl);
}


// MoveStop
void MoveStop()
{
	StopBR();
	StopBL();
	StopFR();
	StopFL();
}


void CatchSignal(int n,struct siginfo *siginfo,void *myact)  
{
	switch( n )
	{
	case SIGHUP:
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
		//do nothing
		break;
	default:
		break;
	}
	printf("signal number:%d\n",n);// print signal number
	printf("siginfo.signo:%d\n",siginfo->si_signo); // siginfo value
	printf("siginfo.errno:%d\n",siginfo->si_errno); // error no
	printf("siginfo.code:%d\n",siginfo->si_code);   // signal code

	// clear data
	digitalWrite(PIN_BR1, 0);
	digitalWrite(PIN_BR2, 0);
	digitalWrite(PIN_BL1, 0);
	digitalWrite(PIN_BL2, 0);
	digitalWrite(PIN_FR1, 0);
	digitalWrite(PIN_FR2, 0);
	digitalWrite(PIN_FL1, 0);
	digitalWrite(PIN_FL2, 0);

	//restore old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);

	exit(0);
}

int initSignalCatch()
{
	// install signal use sigaction
	struct sigaction act;  
	sigemptyset(&act.sa_mask);   // clear  
	act.sa_flags=SA_SIGINFO;     // set SA_SIGINFO to trigger function
	act.sa_sigaction=CatchSignal;  
	if(sigaction(SIGINT,&act,NULL) < 0)  
	{  
		printf("install signal error\n");  
		return -1;
	}

	int res = -1;	
	//stroe old setting
	res = tcgetattr(STDIN_FILENO, &org_opts);
	assert(res == 0);

	//set new terminal params  
	new_opts = org_opts;
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE );
	//new_opts.c_lflag &= ~(ICANON | ECHO); //is also ok
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);

	return 0;
}

int initVehicle()
{
	if (-1 == wiringPiSetup()) {
		printf("Setup wiringPi failed!\n");
		return -1;
	}

	// set pin mode
	pinMode(PIN_BR1, OUTPUT);
	pinMode(PIN_BR2, OUTPUT);
	pinMode(PIN_BL1, OUTPUT);
	pinMode(PIN_BL2, OUTPUT);
	pinMode(PIN_FR1, OUTPUT);
	pinMode(PIN_FR2, OUTPUT);
	pinMode(PIN_FL1, OUTPUT);
	pinMode(PIN_FL2, OUTPUT);
	// clear data first
	digitalWrite(PIN_BR1, 0);
	digitalWrite(PIN_BR2, 0);
	digitalWrite(PIN_BL1, 0);
	digitalWrite(PIN_BL2, 0);
	digitalWrite(PIN_FR1, 0);
	digitalWrite(PIN_FR2, 0);
	digitalWrite(PIN_FL1, 0);
	digitalWrite(PIN_FL2, 0);

	delayMS(50);

	return 0;
}

int DoVehicleCommand(int command, int speedlvl)
{
	switch (command)
	{
	case 1:
		MoveForward(speedlvl);
		break;
	case 2:
		MoveBackward(speedlvl);
		break;
	case 3:
		TurnLeft(speedlvl);
		break;
	case 4:
		TurnRight(speedlvl);
		break;
	default:
		return -1;
	}

	return 0;
}

int getCommandFromArgv(int argc,char* argv[], int *pCommand, int *pSpeedlvl)
{
	if(pCommand == NULL || pSpeedlvl == NULL)
	{
		return -1;
	}

	if (argc == 1) 
	{
		// do nothing
	}
	else if (argc == 2) 
	{
		*pSpeedlvl = atoi(argv[1]);
		if (*pSpeedlvl < 4 || *pSpeedlvl > 100)
		{
			printf("input argv[1] error! input 4-100\n");
			return -1;
		}
		return 0;
	}
	else if (argc == 3) 
	{
		*pSpeedlvl = atoi(argv[1]);
		if (*pSpeedlvl < 4 || *pSpeedlvl > 100)
		{
			printf("input argv[1] error! input 4-100\n");
			return -1;
		}
		*pCommand = atoi(argv[2]);
		if (*pCommand < 1 || *pCommand > 4)
		{
			printf("input argv[2] error! input 1-4\n");
			return -1;
		}

		return 0;
	}
	else {
		printf("input argc error\n");
		return -1;
	}

	return 0;
}


void *GetCharFunc(void *arg)
{
	LockObj *pLockObj = (LockObj *)arg;
	if (NULL == pLockObj)
	{
		return NULL;
	}


	const char VK_UP    = 0x41; // 0x1B 0x5B 0x41
	const char VK_DOWN  = 0x42; // 0x1B 0x5B 0x42
	const char VK_RIGHT = 0x43; // 0x1B 0x5B 0x43
	const char VK_LEFT  = 0x44;	// 0x1B 0x5B 0x44

	char ch = '\0';
	while(1)
	{
		ch = getchar();
		//printf("Snd 0x:%02X, char:%c\n", ch, ch);
		if(ch>='A'&& ch<='Z') 
		{
			ch += 32;
		}
		else if (ch == 0x5B)
		{
			ch = getchar();
			//printf("Snd 0x:%02X, char:%c\n", ch, ch);
			switch(ch)
			{
			case VK_UP:
				ch = 'w';
				break;
			case VK_DOWN:
				ch = 's';
				break;
			case VK_LEFT:
				ch = 'a';
				break;
			case VK_RIGHT:
				ch = 'd';
				break;
			default:
				continue;
			}
		}

		if (ch != 'w' && ch != 's' && ch != 'a' && ch != 'd')
			continue;

		pLockObj->zSyncObj.SyncStart();
		g_chCommand = ch;
		pLockObj->zSyncObj.SyncEnd();
		pLockObj->zWaitObj.Notify();

		//printf("Snd 0x:%02X, char:%c\n", ch, ch);

		//usleep(200*1000);
	}

	return NULL;
}

int main(int argc,char* argv[])
{
	int command = 1;
	int speedlvl = 100;

	if (-1 == getCommandFromArgv(argc, argv, &command, &speedlvl))
	{
		printf("getCommandFromArgv error\n"); 
		return -1;
	}

	//init vehicle
	if (-1 == initVehicle())
	{
		printf("initVehicle error\n"); 
		return -1;
	}

	//init SignalCatch
	if (-1 == initSignalCatch())
	{
		printf("initSignalCatch error\n"); 
		return -1;
	}

	// do command
	//while(1)
	//{
	//	DoVehicleCommand(command, speedlvl);
	//}

#if defined (CONTROL_BY_IR)
	// set pin mode
	pinMode(PIN_IR_A, INPUT);
	pinMode(PIN_IR_B, INPUT);
	pinMode(PIN_IR_C, INPUT);
	pinMode(PIN_IR_D, INPUT);
#else if defined (CONTROL_BY_KEYBOARD)
	LockObj clockObj;
	pthread_t threadid;               /* thread id*/
	/*create thread  */
	pthread_create(&threadid, NULL, GetCharFunc, (void *)(&clockObj));
#endif

	char prech = 0;
	unsigned int preTime = GetTickCount();
	char ch = 0;	
	while(1)
	{
#if defined (CONTROL_BY_IR)
		if (digitalRead(PIN_IR_A) == 1)
		{
			ch = 'w';
		}
		else if (digitalRead(PIN_IR_C) == 1)
		{
			ch = '-';
		}
		else if (digitalRead(PIN_IR_D) == 1)
		{
			ch = 'a';
		}
		else if (digitalRead(PIN_IR_B) == 1)
		{
			ch = 'd';
		}
		else {
			//ch = '-';
			//continue;
		}

#else if defined (CONTROL_BY_KEYBOARD)
		clockObj.zWaitObj.Wait();

		clockObj.zSyncObj.SyncStart();
		ch = g_chCommand;
		clockObj.zSyncObj.SyncEnd();
#endif
		//printf("Rcv %c\n",ch);

		if (prech != ch && (GetTickCount() - preTime < 50))
		{
			MoveStop();
			delayMS(50);
		}
		prech = ch;
		
		switch(ch)
		{
		case 'w':
			DoVehicleCommand(1, speedlvl);
			break;
		case 's':
			DoVehicleCommand(2, speedlvl);
			break;
		case 'a':
			DoVehicleCommand(3, speedlvl);
			break;
		case 'd':
			DoVehicleCommand(4, speedlvl);
			break;
		default:
			//MoveStop();
			//delayMS(50);
			break;
		}

		preTime = GetTickCount();
	}

	return 0;
}