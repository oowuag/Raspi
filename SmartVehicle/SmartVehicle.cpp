// Smart Vehicle

#include <stdio.h>
#include <signal.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <assert.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

//#define CONTROL_BY_KEYBOARD 1
//#define CONTROL_BY_IR       1
#define CONTROL_BY_SOCKET   1

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

#if defined (CONTROL_BY_KEYBOARD)
	//restore old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
#endif

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

#if defined (CONTROL_BY_KEYBOARD)
	int res = -1;	
	//stroe old setting
	res = tcgetattr(STDIN_FILENO, &org_opts);
	assert(res == 0);

	//set new terminal params  
	new_opts = org_opts;
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE );
	//new_opts.c_lflag &= ~(ICANON | ECHO); //is also ok
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
#endif

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


void *SocketRcvFunc(void *arg)
{
	LockObj *pLockObj = (LockObj *)arg;
	if (NULL == pLockObj)
	{
		return NULL;
	}

	const int HELLO_WORLD_SERVER_PORT = 4000;
	const int LENGTH_OF_LISTEN_QUEUE = 20;
	const int BUFFER_SIZE = 256;

    struct sockaddr_in server_addr;
	int server_socket;
	int opt = 1;
   
    bzero(&server_addr,sizeof(server_addr)); 
	
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
	
	char addrstr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(server_addr.sin_addr.s_addr), addrstr, INET_ADDRSTRLEN);
	printf("Server: family=%d, address=%s, port=%d\n", AF_INET, addrstr, server_addr.sin_port); // prints server info

	/* create a socket */
    server_socket = socket(PF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        return 0;
    }
 
    /* bind socket to a specified address*/
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        return 0;
    }

    /* listen a socket */
    if(listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        printf("Server Listen Failed!"); 
        return 0;
    }

	bool _bStop = false;

	/* run server */
    while (!_bStop) 
    {
        struct sockaddr_in client_addr;	
        socklen_t length;
		char buffer[BUFFER_SIZE];
		int client_socket = -1;

		printf("wait client socket connecting...\n");
		/* accept socket from client */
		length = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
        if( client_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
		printf("client socket connected!\n");   

		char ch = '\0';
		/* receive data from client */
		while(1)
		{
			bzero(buffer, BUFFER_SIZE);
			length = recv(client_socket, buffer, BUFFER_SIZE, 0);
			if (length < 0)
			{
				printf("Server Recieve Data Failed!\n");
				break;
			}
			// client shutdown ok
			else if (length == 0)
			{
				usleep(1000 * 1000); //sleep 1s
				printf("Client shutdown OK!\n");
				break;
			}

			if('q' == buffer[0])
			{
				printf("Quit from client!\n");
				break;
			}

			//printf("Rcv:%*.*s\n", length, length, (char *)buffer);

			if (length >= 1)
			{
				ch = buffer[0];
				//printf("SocketRcv 0x:%02X, char:%c\n", ch, ch);

				if (ch != 'w' && ch != 's' && ch != 'a' && ch != 'd')
					continue;

				pLockObj->zSyncObj.SyncStart();
				g_chCommand = ch;
				pLockObj->zSyncObj.SyncEnd();
				pLockObj->zWaitObj.Notify();
			}
		}

		close(client_socket);
	}


    close(server_socket);
	printf("close server_socket\n");

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
#if defined (CONTROL_BY_KEYBOARD)
	LockObj clockObj;
	pthread_t threadid;               /* thread id*/
	/*create thread  */
	pthread_create(&threadid, NULL, GetCharFunc, (void *)(&clockObj));
#elif defined (CONTROL_BY_SOCKET)
	LockObj clockObj;
	pthread_t threadid;               /* thread id*/
	/*create thread  */
	pthread_create(&threadid, NULL, SocketRcvFunc, (void *)(&clockObj));
#elif defined (CONTROL_BY_IR)
	// set pin mode
	pinMode(PIN_IR_A, INPUT);
	pinMode(PIN_IR_B, INPUT);
	pinMode(PIN_IR_C, INPUT);
	pinMode(PIN_IR_D, INPUT);
#else
#endif

	char prech = 0;
	unsigned int preTime = GetTickCount();
	char ch = 0;	
	while(1)
	{
#if defined (CONTROL_BY_KEYBOARD) || (CONTROL_BY_SOCKET)
		clockObj.zWaitObj.Wait();

		clockObj.zSyncObj.SyncStart();
		ch = g_chCommand;
		clockObj.zSyncObj.SyncEnd();
#elif defined (CONTROL_BY_IR)
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
#else
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