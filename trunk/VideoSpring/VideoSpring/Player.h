#ifndef H_PLAYER
#define H_PLAYER

#include "../../common/VideoSpringCommon.h"

class Player
{
private:
	IGraphBuilder *graph;
	IMediaControl *control;
	IMediaEvent   *event;
	IVideoWindow *video;

	int createGraph();
	int runGraph();

public:
	Player();
	~Player();
};
#endif