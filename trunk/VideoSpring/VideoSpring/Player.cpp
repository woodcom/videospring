#include "Player.h"

Player::Player()
{
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&graph);
	graph->QueryInterface(IID_IMediaControl, (void **)&control);
	graph->QueryInterface(IID_IMediaEvent, (void **)&event);
	graph->QueryInterface(IID_IVideoWindow, (void **)&video);

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

	CoCreateInstance(CLSID_VideoSpringRecv, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&recv);

	recv->EnumPins(&pins);
	pins->Next(1, &pinOut, NULL);

	graph->AddFilter(recv, NULL);
	graph->Render(pinOut);

	pinOut->Release();
	pins->Release();
	recv->Release();

	return 0;
}
