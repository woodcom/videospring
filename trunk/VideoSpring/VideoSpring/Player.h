#ifndef H_PLAYER
#define H_PLAYER

#include <atlbase.h>
#include <DShow.h>
#include <d3d9.h>
#include <vmr9.h>

// {34C5751E-A6E1-4A6A-895E-BC4797001848}
DEFINE_GUID(CLSID_VideoSpringRecv, 
0x34c5751e, 0xa6e1, 0x4a6a, 0x89, 0x5e, 0xbc, 0x47, 0x97, 0x0, 0x18, 0x48);

// {270C94BC-49A7-4C33-ACEA-3211F3C6873B}
DEFINE_GUID(IID_IVideoSpringRecv, 
0x270c94bc, 0x49a7, 0x4c33, 0xac, 0xea, 0x32, 0x11, 0xf3, 0xc6, 0x87, 0x3b);

interface IVideoSpringRecv : public IUnknown
{
	STDMETHOD(SetServerSocket)(SOCKET s) = 0;
	STDMETHOD(SetPresenterId)(long id) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) { return -1; }
	virtual ULONG STDMETHODCALLTYPE AddRef() { return -1; }
	virtual ULONG STDMETHODCALLTYPE Release() { return -1; }
};

class Player
{
private:
	CComPtr<IGraphBuilder> graph;
	CComPtr<IMediaControl> control;
	CComPtr<IMediaEvent>   event;
	CComPtr<IVideoWindow> video;

	SOCKET server;
	long presenterId;

	int createGraph();
	int runGraph();

public:
	Player(unsigned long ip, long id);
	~Player();
};
#endif