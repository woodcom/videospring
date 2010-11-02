#ifndef H_VIDSPRINGCOMMON

#define H_VIDSPRINGCOMMON 1

#define TCP_CHUNKSIZE 1024

#if defined(WIN32) || defined(_WINDLL)

#pragma warning( disable : 4995 4005 4067 )

#include <stdint.h>
#include <WinSock2.h>
#include <streams.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <initguid.h>
#include <Wmcodecdsp.h>
#include <dmodshow.h>
#include <dmo.h>
#include <tchar.h>
#include <BaseTyps.h>
#include <atlbase.h>

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
	#include <fcntl.h>

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
	C_RECEIVE,
	C_NEW_PRESENTER,
	C_NEW_CLIENT,
	C_SET_FORCE_KEYFRAME,
	C_CLEAR_FORCE_KEYFRAME,
	C_DROP_PRESENTER
};

struct MessageHeader
{
	uint16_t command;
	uint32_t length;
};

class Message
{
public:
	MessageHeader header;
	BYTE *data;

	Message()
	{
		header.command = 0;
		header.length = 0;
		data = NULL;
	}
};

int sendMessage(SOCKET s, Message *m);
int receiveMessage(SOCKET s, Message &m);
int deleteMessage(Message &m);

#endif
