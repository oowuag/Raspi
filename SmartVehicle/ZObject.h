#ifndef ZOBJECT_H
#define ZOBJECT_H

#if defined(WIN32)
	#include <windows.h>
    #include <process.h>
    #include <tchar.h>
    #include <malloc.h>
#else
    #include <sys/time.h>
    #include <time.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// typedef
typedef char           CHAR;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef short          SHORT;
typedef unsigned short USHORT;
typedef long           LONG;
typedef unsigned long  ULONG;
typedef float          FLOAT;
typedef double         DOUBLE;
typedef unsigned int   UINT;

typedef signed char    int8_t;
typedef unsigned char  uint8_t;
typedef short int      int16_t;
typedef int            int32_t;
# if __WORDSIZE == 64
typedef long int       int64_t;
# else
typedef long long int  int64_t;

#endif

#if defined(WIN32)
#else
typedef void                VOID;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef int                 INT;
typedef const CHAR *LPCSTR, *PCSTR;
typedef void                *LPVOID;
#define TEXT(C) C
// MAX data
#define MAXWORD  0xffff
#define MAXLONG  0x7fffffffL
#define INFINITE 0xffffffffL
#endif

#ifndef MAX_PATH
#   define MAX_PATH 1024
#endif

// MAX MIN
#ifndef MAX
#define MAX(a,b)   ((a)>(b) ? (a) : (b))
#define MIN(a,b)   ((a)<(b) ? (a) : (b))
#endif


// ZObject
class ZObject
{
};

#endif //ZOBJECT_H
