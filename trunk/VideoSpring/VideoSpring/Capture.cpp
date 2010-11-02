#include "Capture.h"

#pragma comment (lib, "Winmm.lib")

void CALLBACK Capture::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	static long increment = 1;
	Capture *_this = (Capture *)dwInstance;
	static int buffer = 0;
	if(uMsg == WIM_DATA)
	{
		long length = _this->_waveHeader[buffer].dwBufferLength;

		DWORD bytesWritten;
		WriteFile(_this->pipe, _this->_waveHeader[buffer].lpData, _this->_waveHeader[buffer].dwBytesRecorded, &bytesWritten, NULL);
		if(bytesWritten != _this->_waveHeader[buffer].dwBytesRecorded)
		{
			printf("Error!\n");
		}
		/*** Remove old buffer ***/

		waveInUnprepareHeader(hwi, &_this->_waveHeader[buffer], sizeof(WAVEHDR));
		delete[] _this->_waveBuffer[buffer];

		/*** Create new buffer ***/

		ZeroMemory(&_this->_waveHeader[buffer], sizeof(_this->_waveHeader[buffer]));
		_this->_waveBuffer[buffer] = new (std::nothrow) WORD[length];
		_this->_waveHeader[buffer].dwBufferLength = length;
		_this->_waveHeader[buffer].lpData = reinterpret_cast<LPSTR>(_this->_waveBuffer[buffer]);
		waveInPrepareHeader(hwi, &_this->_waveHeader[buffer], sizeof(_this->_waveHeader[buffer]));
	    waveInAddBuffer(hwi, &_this->_waveHeader[buffer], sizeof(_this->_waveHeader[buffer]));

		buffer++;
		if(buffer > BUFFERS - 1) buffer = 0;
	}
	increment++;
}

int Capture::startAudioCapture()
{
   WAVEFORMATEX waveFormat = {0};
    waveFormat.cbSize = 0;
    waveFormat.nSamplesPerSec = 44100;
    waveFormat.nChannels = 1;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8)*waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;

    MMRESULT mmr = waveInOpen(&_waveHandle, WAVE_MAPPER, &waveFormat,
                      (DWORD)waveInProc, (DWORD_PTR)this,
                      CALLBACK_FUNCTION | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE);
    if (mmr != MMSYSERR_NOERROR)
    {
        printf("Failed to open wave in.\n");
        return false;
    }

	for(int n = 0; n < BUFFERS; n++)
	{
		ZeroMemory(&_waveHeader[n], sizeof(_waveHeader[n]));
		_waveBuffer[n] = new (std::nothrow) WORD[64];
		if (_waveBuffer[n] == NULL)
		{
	        printf("Failed to allocate buffer for header 1.\n");
			return false;
		}
		_waveHeader[n].dwBufferLength = int(64);
		_waveHeader[n].lpData = reinterpret_cast<LPSTR>(_waveBuffer[n]);


		mmr = waveInPrepareHeader(_waveHandle, &_waveHeader[n], sizeof(_waveHeader[n]));
		if (mmr != MMSYSERR_NOERROR)
		{
	        printf("Failed to prepare header 1.\n");
			return false;
		}

	    mmr = waveInAddBuffer(_waveHandle, &_waveHeader[n], sizeof(_waveHeader[n]));
	    if (mmr != MMSYSERR_NOERROR)
	    {
			printf("Failed to add buffer 1.\n");
			return false;
		}
	}

    mmr = waveInStart(_waveHandle);
    if (mmr != MMSYSERR_NOERROR)
    {
        printf("Failed to start.\n");
        return false;
    }

	
}

DWORD WINAPI InstanceThread(LPVOID parms)
{
	system("\"c:\\projects2010\\videospring\\bin\\ffmpeg.exe\" -f vfwcap -r 15 -s 640x480 -rtbufsize 1024000000 -i 0 -f s16be -ac 1 -ar 44100 -rtbufsize 1024000000 -i \\\\.\\pipe\\videospring -rtbufsize 1024000000 -vglobal 1 http://new.lifespringschool.org:8090/instructor.ffm");
	return 1;
}

Capture::Capture(unsigned long ip)
{
	memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		printf("Error creating socket.\n");
		return;
	}
	
	sockaddr_in serveraddr;

	serveraddr.sin_family = AF_INET;
 	serveraddr.sin_addr.s_addr = ip;
	serveraddr.sin_port = htons(1234);

	if(connect(server, (SOCKADDR*)&serveraddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("Error connecting to server.\n");
		closesocket(server);
		return;
	}

	DWORD pid = GetCurrentProcessId();

	Message m;
	m.header.command = C_SET_PRESENTER_SEND;
	m.header.length = sizeof(DWORD);
	m.data = (BYTE*)&pid;

	sendMessage(server, &m);

		pipe = INVALID_HANDLE_VALUE;
		printf("creating pipe");

		if((pipe = CreateNamedPipe("\\\\.\\pipe\\videospring", PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 100, 10000000, 10000000, 0, NULL)) == INVALID_HANDLE_VALUE)
		{
			printf("Failed to create named pipe.\n");
			return;
		}

		startAudioCapture();

	if(!CreateProcess(NULL, "c:\\projects2010\\videospring\\bin\\ffmpeg.exe -f vfwcap -rtbufsize 1024000000  -r 15 -s 640x480 -i 0 -f s16be -acodec pcm_s16be -ab 705600 -sample_fmt s16 -ac 1 -ar 44100 -i \\\\.\\pipe\\videospring -vglobal 1 http://new.lifespringschool.org:8090/instructor.ffm", NULL, NULL, false, 0, NULL, NULL, &si, &pi))
	{
		printf("Failed to create process. %Ld\n", GetLastError());
		return;
	}

}

Capture::~Capture()
{
	TerminateProcess(pi.hProcess, 0);
}

