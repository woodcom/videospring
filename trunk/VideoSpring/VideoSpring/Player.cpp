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
	IBaseFilter *recv;
	IEnumPins *pins;
	IPin *pinOut;

	if(FAILED(CoCreateInstance(CLSID_VideoSpringRecv, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&recv)))
	{
		printf("Failed to create receive filter!\n");
		return 1;
	}

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

	if(FAILED(graph->AddFilter(recv, NULL)))
	{
		printf("Failed to add receive filter to graph!\n");
		return 1;
	}

	if(FAILED(graph->Render(pinOut)))
	{
		printf("Failed to render output pin!\n");
		return 1;
	}

	pinOut->Release();
	pins->Release();
	recv->Release();

	return 0;
}
