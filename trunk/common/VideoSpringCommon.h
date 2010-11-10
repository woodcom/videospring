#ifndef H_VIDSPRINGCOMMON

#define H_VIDSPRINGCOMMON 1

#define TCP_CHUNKSIZE 1024

#if defined(WIN32) || defined(_WINDLL)

#include "../VideoSpring/VideoSpring/includes.h"

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
	C_SET_AUDIO_SENDER,
	C_SET_VIDEO_SENDER,
	C_SET_AUDIO_RECEIVER,
	C_SET_VIDEO_RECEIVER,
	C_SET_MEDIA_TYPE,
	C_SET_FORMAT,
	C_BROADCAST_AUDIO,
	C_BROADCAST_AUDIO_KEYFRAME,
	C_BROADCAST_VIDEO,
	C_BROADCAST_VIDEO_KEYFRAME,
	C_RECEIVE,
	C_NEW_PRESENTER,
	C_NEW_CLIENT,
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
