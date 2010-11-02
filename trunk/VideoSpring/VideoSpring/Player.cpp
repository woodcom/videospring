#include "Player.h"
#include "../../common/VideoSpringCommon.h"

DWORD WINAPI readThread(LPVOID parms)
{
	struct PARMS
	{
		LPVOID pipe;
		SOCKET socket;
		long presenterId;
	};

	PARMS *p = (PARMS*)parms;

	HANDLE pipe = p->pipe;

	if(pipe == NULL)
	{
		printf("Invalid pipe.\n");
		return -1;
	}
	
	BOOL connected = false;
	int err = 0;

	connected = ConnectNamedPipe(pipe, NULL) ? true : ((err = GetLastError()) == ERROR_PIPE_CONNECTED);

	if(!connected)
	{
		printf("Error connecting pipe. %d\n", err);
		return -1;
	}

	DWORD pid = p->presenterId;

	Message m;

	m.header.command = C_SET_PRESENTER_RECV;
	m.header.length = sizeof(DWORD);
	m.data = (BYTE*)&pid;

	sendMessage(p->socket, &m);

	while(1)
	{
		receiveMessage(p->socket, m);

		bool success;
		DWORD bytes = 0;

		success = (bool)WriteFile(pipe, m.data, m.header.length, &bytes, NULL);

		if(!success || bytes == 0)
		{
			if(GetLastError() == ERROR_BROKEN_PIPE)
			{
				printf("Write Pipe stopping...\n");
			}
			else
			{
				printf("WriteFile failed, pipe stopping...\n");
			}
			break;
		}
	}

	FlushFileBuffers(pipe);
	DisconnectNamedPipe(pipe);
	CloseHandle(pipe);

	return 1;
}


Player::Player(unsigned long ip, long id)
{
	presenterId = id;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		printf("Error creating socket.\n");
		return;
	}
	
	sockaddr_in serveraddr;

	serveraddr.sin_family = AF_INET;
 	serveraddr.sin_addr.s_addr = ip;
	serveraddr.sin_port = htons(1234);

	if(connect(server, (SOCKADDR*)&serveraddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("Error connecting to server.\n");
		closesocket(server);
		return;
	}

	if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph)))
	{
		printf("Failed to create graph!\n");
		return;
	}

	if(FAILED(graph->QueryInterface(IID_IMediaControl, (void **)&control)))
	{
		printf("Failed to create media control!\n");
		return;
	}

	if(FAILED(graph->QueryInterface(IID_IMediaEvent, (void **)&event)))
	{
		printf("Failed to create media event!\n");
		return;
	}

	if(FAILED(graph->QueryInterface(IID_IVideoWindow, (void **)&video)))
	{
		printf("Failed to create video window!\n");
		return;
	}

	if(!createGraph())
	{
		runGraph();
	}

}

Player::~Player()
{
	printf("Cleaning up player graph...");

	video->put_Visible(OAFALSE);
	video->put_Owner(NULL);

	if(control != NULL)
		control->Stop();

	printf("done.\n");
}

int Player::runGraph()
{
	control->Run();

	return 0;
}

int Player::createGraph()
{
	CComPtr<IBaseFilter> recv, vmr9;
	CComPtr<IVMRMixerControl9> vmr9control;
	CComPtr<IVMRFilterConfig9> vmr9config;
	CComPtr<IFileSourceFilter> sourceControl;

	CComPtr<IPin> fileOut;

	/*** Load Filters ***/

	if(FAILED(CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&recv)))
	{
		printf("Failed to create receive filter!\n");
		return 1;
	}

	if(FAILED(CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&vmr9)))
	{
		printf("Failed to load VMR9 filter!\n");
		return 1;
	}

	/*** Load Filter Controls ***/

	if(FAILED(recv->QueryInterface(&sourceControl)))
	{
		printf("Failed to get encoder control!\n");
		return 1;
	}

	HANDLE pipe = INVALID_HANDLE_VALUE;

	if((pipe = CreateNamedPipe("\\\\.\\pipe\\videospringstream.ogg", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE, 10, 1000000, 1000000, 0, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("Failed to create named pipe.\n");
		return 1;
	}
	else
	{
		struct PARMS
		{
			LPVOID pipe;
			SOCKET socket;
			long presenterId;
		};
			
		PARMS *parms = new PARMS;
		parms->pipe = pipe;
		parms->socket = server;
		parms->presenterId = presenterId;

		if(CreateThread(NULL, 0, readThread, (LPVOID) parms, 0, NULL) == NULL)
		{
			printf("Failed to create thread.\n");
			return 1;
		}
	}

	sourceControl->Load(L"\\\\.\\pipe\\videospringstream.ogg", NULL);
	if(FAILED(vmr9->QueryInterface(IID_IVMRFilterConfig9, (void**)&vmr9config)))
	{
		printf("Failed to get VMR9 config!\n");
		return 1;
	}

	// This configuration is required before loading the mixer control
	vmr9config->SetNumberOfStreams(1); // In future may use up to 16
	vmr9config->SetRenderingMode(VMRMode_Windowed);

	if(FAILED(vmr9->QueryInterface(IID_IVMRMixerControl9, (void**)&vmr9control)))
	{
		printf("Failed to get VMR9 control!\n");
		return 1;
	}

	/*** Get Pins ***/

	if(FAILED(recv->FindPin(L"Output", &fileOut)))
	{
		printf("Failed to get output pin!\n");
		return 1;
	}

	/*** Add Filters to Graph ***/

	if(FAILED(graph->AddFilter(recv, NULL)))
	{
		printf("Failed to add receive filter to graph!\n");
		return 1;
	}
	
	if(FAILED(graph->AddFilter(vmr9, NULL)))
	{
		printf("Failed to add VMR9 filter to graph!\n");
		return 1;
	}
	
	graph->Render(fileOut);

	VMR9NormalizedRect r;

	r.left = 0;
	r.right = 1;
	r.top = 0;
	r.bottom = 1;

	vmr9control->SetOutputRect(0, &r);

	return 0;
}
