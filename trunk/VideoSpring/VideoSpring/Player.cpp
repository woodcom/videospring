#include "Player.h"

Player::Player(const char *ip, long id)
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
 	serveraddr.sin_addr.s_addr = inet_addr(ip);
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
	CComPtr<IBaseFilter> recv, decoder, vmr9;
	CComPtr<IVMRMixerControl9> vmr9control;
	CComPtr<IVMRFilterConfig9> vmr9config;
	CComPtr<IVP8PostProcessing> decoderControl;
	CComPtr<IVideoSpringRecv> recvControl;

	CComPtr<IEnumPins> pins;
	CComPtr<IPin> pinOut, decIn, decOut, renderPins[16];

	/*** Load Filters ***/

	if(FAILED(CoCreateInstance(CLSID_VideoSpringRecv, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&recv)))
	{
		printf("Failed to create receive filter!\n");
		return 1;
	}

	if(FAILED(CoCreateInstance(CLSID_VP8Decoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&decoder)))
	{
		printf("Failed to load encoding filter!\n");
		return 1;
	}

	if(FAILED(CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&vmr9)))
	{
		printf("Failed to load VMR9 filter!\n");
		return 1;
	}


	/*** Load Filter Controls ***/

	if(FAILED(decoder->QueryInterface(IID_IVP8PostProcessing, (void**)&decoderControl)))
	{
		printf("Failed to get encoder control!\n");
		return 1;
	}
	
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


	/*** Configure Filters ***/

	decoderControl->SetFlags(0);


	/*** Get Pins ***/

	if(FAILED(decoder->EnumPins(&pins)))
	{
		printf("Failed to enumerate encoder pins!\n");
		return 1;
	}

	if(FAILED(pins->Next(1, &decIn, NULL)))
	{
		printf("Failed to get next decoder pin!");
		return 1;
	}

	if(FAILED(pins->Next(1, &decOut, NULL)))
	{
		printf("Failed to get next decoder pin!");
		return 1;
	}

	CComPtr<IEnumPins> pins2, pins3;

	if(FAILED(vmr9->EnumPins(&pins2)))
	{
		printf("Failed to enumerate pins!\n");
		return 1;
	}

	for(int n = 0; n < 1; n++)
	{
		if(FAILED(pins2->Next(1, &renderPins[n], NULL)))
		{
			printf("Failed to get next render pin!\n");
			continue;
		}
	}

	if(FAILED(recv->EnumPins(&pins3)))
	{
		printf("Failed to enumerate pins!\n");
		return 1;
	}

	if(FAILED(pins3->Next(1, &pinOut, NULL)))
	{
		printf("Failed to get next pin!\n");
		return 1;
	}

	if(FAILED(pinOut->QueryInterface(IID_IVideoSpringRecv, (void**)&recvControl)))
	{
		printf("Failed to get recv control!\n");
		return 1;
	}

	recvControl->SetServerSocket(server);
	recvControl->SetPresenterId(presenterId);

	/*** Add Filters to Graph ***/

	if(FAILED(graph->AddFilter(decoder, NULL)))
	{
		printf("Failed to add decoder filter to graph!\n");
		return 1;
	}

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
	

	if(FAILED(graph->Connect(pinOut, decIn)))
	{
		printf("Failed to connect receive filter to decoder!\n");
		return 1;
	}


//	graph->Render(decOut);
	if(FAILED(graph->Connect(decOut, renderPins[0])))
	{
		printf("Failed to connect output pin to renderer!\n");
		return 1;
	}


	VMR9NormalizedRect r;

	r.left = 0;
	r.right = 1;
	r.top = 0;
	r.bottom = 1;

	vmr9control->SetOutputRect(0, &r);

	return 0;
}
