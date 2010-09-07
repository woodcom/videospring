#ifndef H_VIDSPRINGCOMMON

#define H_VIDSPRINGCOMMON 1

#ifdef WIN32

#include <WinSock2.h>
#include <streams.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <initguid.h>
#include <Wmcodecdsp.h>
#include <dmodshow.h>
#include <dmo.h>
#include <strsafe.h>
#include <DShow.h>
#include "vp8encoderidl.h"

// {34C5751E-A6E1-4A6A-895E-BC4797001848}
DEFINE_GUID(CLSID_VideoSpringRecv, 
0x34c5751e, 0xa6e1, 0x4a6a, 0x89, 0x5e, 0xbc, 0x47, 0x97, 0x0, 0x18, 0x48);

// {2EE25BD6-9A2B-4589-A55E-95BD877682F7}
DEFINE_GUID(CLSID_VideoSpringSend,
0x2ee25bd6, 0x9a2b, 0x4589, 0xa5, 0x5e, 0x95, 0xbd, 0x87, 0x76, 0x82, 0xf7);

/*#ifndef CLSID_VP8Encoder
DEFINE_GUID(CLSID_VP8Encoder,
0xED3110F5, 0x5211, 0x11DF, 0x94, 0xAF, 0x00, 0x26, 0xB9, 0x77, 0xEE, 0xAA);
#endif*/
/*
DEFINE_GUID(CLSID_CColorConvertDMO,
0x98230571, 0x0087, 0x4204, 0xB0, 0x20, 0x32, 0x82, 0x53, 0x8E, 0x57, 0xD3);
*/
#ifndef OUR_GUID_ENTRY
    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
#endif

OUR_GUID_ENTRY(MEDIASUBTYPE_VP8,
0x30385056, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71)

OUR_GUID_ENTRY(MEDIASUBTYPE_VP8_MUX,
0xED3110EB, 0x5211, 0x11DF, 0x94, 0xAF, 0x00, 0x26, 0xB9, 0x77, 0xEE, 0xAA)

#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <errno.h>

	typedef uint32_t DWORD;
	typedef unsigned char BYTE;
	typedef int SOCKET;

	#ifndef NULL
		#define NULL 0
	#endif

	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	#define NOERROR 0
	#define WSAEWOULDBLOCK ENOTBLK
	#define closesocket(socket) close(socket)
#endif

enum
{
	C_SET_PRESENTER_SEND,
	C_SET_PRESENTER_RECV,
	C_SET_CLIENT_SEND,
	C_SET_CLIENT_RECV,
	C_SET_READY,
	C_SET_FORMAT,
	C_BROADCAST,
	C_RECEIVE
};

struct MessageHeader
{
	uint16_t command;
	uint32_t length;
};

struct Message
{
	MessageHeader header;
	BYTE *data;
};

int sendMessage(SOCKET s, Message *m);
int receiveMessage(SOCKET s, Message &m);
int deleteMessage(Message &m);

#endif
