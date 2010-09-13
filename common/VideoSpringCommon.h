#ifndef H_VIDSPRINGCOMMON

#define H_VIDSPRINGCOMMON 1

#define TCP_CHUNKSIZE 1024

#if defined(WIN32) || defined(_WINDLL)

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
#include <d3d9.h>
#include <vmr9.h>
#include "vp8encoderidl.h"
#include "vp8decoderidl.h"
#include <BaseTyps.h>
#include <stdint.h>

/*** Define GUIDs ***/

// {34C5751E-A6E1-4A6A-895E-BC4797001848}
DEFINE_GUID(CLSID_VideoSpringRecv, 
0x34c5751e, 0xa6e1, 0x4a6a, 0x89, 0x5e, 0xbc, 0x47, 0x97, 0x0, 0x18, 0x48);

// {270C94BC-49A7-4C33-ACEA-3211F3C6873B}
DEFINE_GUID(IID_IVideoSpringRecv, 
0x270c94bc, 0x49a7, 0x4c33, 0xac, 0xea, 0x32, 0x11, 0xf3, 0xc6, 0x87, 0x3b);

// {2EE25BD6-9A2B-4589-A55E-95BD877682F7}
DEFINE_GUID(CLSID_VideoSpringSend,
0x2ee25bd6, 0x9a2b, 0x4589, 0xa5, 0x5e, 0x95, 0xbd, 0x87, 0x76, 0x82, 0xf7);

// {609F316F-B316-4084-8F0F-4F06EFFADE45}
DEFINE_GUID(IID_IVideoSpringSend, 
0x609f316f, 0xb316, 0x4084, 0x8f, 0xf, 0x4f, 0x6, 0xef, 0xfa, 0xde, 0x45);

DEFINE_GUID(MEDIASUBTYPE_VP8,
0x30385056, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_VP8_MUX,
0xED3110EB, 0x5211, 0x11DF, 0x94, 0xAF, 0x00, 0x26, 0xB9, 0x77, 0xEE, 0xAA);


/*** Interfaces ***/

interface IVideoSpringSend : public IUnknown
{
	STDMETHOD(SetServerSocket)(SOCKET s) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) { return -1; }
	virtual ULONG STDMETHODCALLTYPE AddRef() { return -1; }
	virtual ULONG STDMETHODCALLTYPE Release() { return -1; }
};

interface IVideoSpringRecv : public IUnknown
{
	STDMETHOD(SetServerSocket)(SOCKET s) = 0;
	STDMETHOD(SetPresenterId)(long id) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) { return -1; }
	virtual ULONG STDMETHODCALLTYPE AddRef() { return -1; }
	virtual ULONG STDMETHODCALLTYPE Release() { return -1; }
};


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
	C_CLEAR_FORCE_KEYFRAME
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
