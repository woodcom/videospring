#ifndef H_CAPTURE
#define H_CAPTURE

#include "../../common/VideoSpringCommon.h"

class Capture
{
public:
	IVP8Encoder *encoderControl;
private:
	IGraphBuilder *graph;
	IMediaControl *control;
	IMediaEvent   *event;
	IVideoWindow *video;

	SOCKET server;

	int createGraph();
	int runGraph();

public:
	Capture(const char *ip);
	~Capture();
};
#endif