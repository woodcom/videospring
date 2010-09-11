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

	SOCKET server;
	long presenterId;

	int createGraph();
	int runGraph();

public:
	Player(const char *ip, long id);
	~Player();
};
#endif