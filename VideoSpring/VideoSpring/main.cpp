#include "../../common/VideoSpringCommon.h"
#include "Capture.h"
#include "Player.h"
#include <map>
#include <d3d9.h>

#pragma comment (lib, "d3d9.lib")

CComPtr<IDirect3D9> d3d;
CComPtr<IDirect3DDevice9> d3ddev;

void initD3D(HWND hwnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;

	d3d->CreateDevice(D3DADAPTER_DEFAULT,
						D3DDEVTYPE_HAL,
						hwnd,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						&d3dpp,
						&d3ddev);
}

const char g_szClassName[] = "myWindowClass";

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


int main(int argc, char** argv)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	WSAData data;
	hr = WSAStartup(MAKEWORD(2, 2), &data);

	if (FAILED(hr))
	{
	    printf("Error.\n");
		exit(1);
	}
	
	char hostname[] = "condor.lifespringschool.org";

	HOSTENT *host;
	unsigned long ip;

	if(!(host = gethostbyname(hostname)))
	{
		printf("Error resolving hostname.\n");
		exit(1);
	}

	if(host->h_addr_list && host->h_addr_list[0])
	{
		memcpy(&ip, host->h_addr_list[0], sizeof(ip));
	}
	else
	{
		printf("No ip found for host.\n");
		exit(1);
	}

//	printf("Enter server ip: ");
//	scanf("%s", ip);

	printf("Connecting to %s...\n", hostname);

/*	Player p(ip);
	printf("Player started.\n");*/

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		printf("Error creating socket.\n");
		WSACleanup();
		return -1;
	}
	
	sockaddr_in serveraddr;

	serveraddr.sin_family = AF_INET;
 	serveraddr.sin_addr.s_addr = ip;
	serveraddr.sin_port = htons(1234);

	if(connect(server, (SOCKADDR*)&serveraddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("Error connecting to server.\n");
		closesocket(server);
		WSACleanup();
		return -1;
	}

	Message m;

	m.header.command = C_NEW_CLIENT;
	DWORD pid = GetCurrentProcessId();
	m.data = (BYTE*)&pid;
	m.header.length = sizeof(uint32_t);

	if(sendMessage(server, &m) == -1)
	{
		printf("Error registering client with server.\n");
		closesocket(server);
		WSACleanup();
		return -1;
	}
	
	Capture *c;

/*	printf("Capture (y/n): ");
	char buffer[1024];
	scanf("%s", buffer);
	if(buffer[0] == 'y')*/
	{
		c = new Capture(ip);
		printf("Capture started.\n");
	}

	std::map<long, Player *> players;

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = NULL;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "The title of my window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
        NULL, NULL, NULL, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, 1);
    UpdateWindow(hwnd);

	initD3D(hwnd);








	printf("Starting...\n");
	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);

		fd_set read_set;

		FD_ZERO(&read_set);
		FD_SET(server, &read_set);

		timeval t;
		t.tv_sec = 0;
		t.tv_usec = 0;

		if(select(2, &read_set, NULL, NULL, &t) == -1)
		{
			break;
		}
	
		if(FD_ISSET(server, &read_set))
		{
			Message msg;
			ZeroMemory(&msg, sizeof(Message));

			printf("Receiving message...\n");
			if(receiveMessage(server, msg) == -1)
			{
				printf("Error receiving message from server %ld!\n", WSAGetLastError());
				break;
			}
			printf("Com: %ld, Length: %ld\n", msg.header.command, msg.header.length);
			switch(msg.header.command)
			{
			case C_NEW_PRESENTER:
			{
				long presenterId = *((long*)msg.data);
				printf("New presenter: %ld\n", presenterId);
				players[presenterId] = new Player(ip, *((long*)msg.data));
				break;
			}

			case C_DROP_PRESENTER:
			{
				long presenterId = *((long*)msg.data);
				printf("Dropping presenter: %ld\n", presenterId);
				delete(players[presenterId]);
				players.erase(players.find(presenterId));
				break;
			}

			case C_SET_FORCE_KEYFRAME:
			{
//				c.encoderControl->SetForceKeyframe();
				break;
			}

			case C_CLEAR_FORCE_KEYFRAME:
			{
	//			c.encoderControl->ClearForceKeyframe();
				break;
			}

			default:
				printf("Invalid command received: %d\n", msg.header.command);
			}

			deleteMessage(msg);
		}
	}

	printf("shutting down...");

	std::map<long, Player *>::iterator it;

	if(!players.empty())
	{
		for(it = players.begin(); it != players.end(); it++)
		{
			delete((*it).second);
		}
	}

	closesocket(server);
	WSACleanup();
	CoUninitialize();

	printf("done.\n");

	return Msg.wParam;
}
