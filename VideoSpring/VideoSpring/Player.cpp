#include "Player.h"

Player::Player()
{
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

	createGraph();
	runGraph();
}

Player::~Player()
{
	video->Release();
	control->Release();
	event->Release();
	graph->Release();
}

int Player::runGraph()
{
	control->Run();

	return 0;
}

int Player::createGraph()
{
	IBaseFilter *recv, *decoder, *vmr9;
	IVMRMixerControl9 *vmr9control;
	IVMRFilterConfig9 *vmr9config;
	IVP8PostProcessing *decoderControl;
	IEnumPins *pins;
	IPin *pinOut, *decIn, *decOut;


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

	if(FAILED(vmr9->QueryInterface(IID_IVMRMixerControl9, (void**)&vmr9control)))
	{
		printf("Failed to get VMR9 control!\n");
		return 1;
	}

	if(FAILED(vmr9->QueryInterface(IID_IVMRFilterConfig9, (void**)&vmr9config)))
	{
		printf("Failed to get VMR9 config!\n");
		return 1;
	}

	/*** Configure Filters ***/

	decoderControl->SetFlags(0);
	vmr9config->SetNumberOfStreams(16); // In future may use up to 16
	vmr9config->SetRenderingMode(VMRMode_Windowed);

	VMR9NormalizedRect r;
	r.left = 0;
	r.right = .5;
	r.top = 0;
	r.bottom = .5;

	vmr9control->SetOutputRect(0, &r);


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

	pins->Release();

	if(FAILED(recv->EnumPins(&pins)))
	{
		printf("Failed to enumerate pins!\n");
		return 1;
	}

	if(FAILED(pins->Next(1, &pinOut, NULL)))
	{
		printf("Failed to get next pin!\n");
		return 1;
	}


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

	if(FAILED(graph->Connect(pinOut, decIn)))
	{
		printf("Failed to connect receive filter to decoder!\n");
		return 1;
	}

	if(FAILED(graph->Render(decOut)))
	{
		printf("Failed to render output pin!\n");
		return 1;
	}

	decOut->Release();
	decIn->Release();
	pinOut->Release();
	pins->Release();
	recv->Release();

	return 0;
}
