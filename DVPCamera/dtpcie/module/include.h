#pragma once

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef void * HANDLE;
#define ASSERT  assert
#define TRACE   printf
#define INVALID_HANDLE_VALUE 0
#define INFINITE 0xffffffff
#define _MAX_PATH 260 /* max. length of full pathname */
//#define HANDLE int
#define MAX_PATH 260
#define TRUE true
#define FALSE false
#define __stdcall
#define __declspec(x)
#define __cdecl
//#define max(a,b) (((a) > (b)) ? (a) : (b))
//#define min(a,b) (((a) < (b)) ? (a) : (b))

typedef int BOOL;
typedef unsigned char BYTE;
typedef float FLOAT;
typedef FLOAT *PFLOAT;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned char *PUCHAR;
typedef short SHORT;
typedef unsigned short USHORT;
typedef unsigned short *PUSHORT;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef ULONGLONG *PULONGLONG;
/* ULONG 是32bit的 */
typedef unsigned int ULONG;
//typedef unsigned long ULONG;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned int *PUINT;
typedef unsigned int UINT32;
typedef unsigned int *PUINT32;
typedef unsigned long long UINT64;
typedef unsigned long long *PUINT64;
typedef void VOID;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef DWORD *LPDWORD;
typedef unsigned long UINT_PTR;
typedef UINT_PTR SIZE_T;
typedef LONGLONG USN;
typedef BYTE BOOLEAN;
typedef void *PVOID;
typedef void *PVOID64;
typedef void *LPVOID;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;
