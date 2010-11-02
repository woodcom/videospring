#ifndef H_CAPTURE
#define H_CAPTURE

//#include <streams.h>
#include "../../common/VideoSpringCommon.h"
#include <fstream>
#define BUFFERS 10
class Capture
{
private:
    HWAVEIN _waveHandle;
    WAVEHDR _waveHeader[BUFFERS];
    WORD *  _waveBuffer[BUFFERS];

	HANDLE pipe;

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	SOCKET server;

	int startAudioCapture();

	static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
public:
	Capture(unsigned long ip);
	~Capture();
};
#endif