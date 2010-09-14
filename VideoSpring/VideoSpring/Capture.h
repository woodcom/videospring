#ifndef H_CAPTURE
#define H_CAPTURE

#include "../../common/VideoSpringCommon.h"

class Capture
{
public:
	CComPtr<IVP8Encoder> encoderControl;
private:
	CComPtr<IGraphBuilder> graph;
	CComPtr<IMediaControl> control;
	CComPtr<IMediaEvent>   event;
	CComPtr<IVideoWindow> video;

	SOCKET server;

	int createGraph();
	int runGraph();

public:
	Capture(const char *ip);
	~Capture();
};
#endif