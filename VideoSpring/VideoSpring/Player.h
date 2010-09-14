#ifndef H_PLAYER
#define H_PLAYER

#include "../../common/VideoSpringCommon.h"

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
	Player(const char *ip, long id);
	~Player();
};
#endif