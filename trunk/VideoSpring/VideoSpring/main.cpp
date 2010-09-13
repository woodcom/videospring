#include "Capture.h"
#include "Player.h"

int main(int argc, char** argv)
{
	HRESULT hr = CoInitialize(NULL);
	WSAData data;
	hr = WSAStartup(MAKEWORD(2, 2), &data);

	if (FAILED(hr))
	{
	    printf("Error.\n");
		exit(1);
	}

	//char ip[1024];
	char ip[] = "184.73.115.220";

//	printf("Enter server ip: ");
//	scanf("%s", ip);

	printf("Connecting to %s...\n", ip);

	Capture c(ip);
	printf("Capture started.\n");
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
 	serveraddr.sin_addr.s_addr = inet_addr(ip);
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

	printf("Starting...\n");
	while(1)
	{
		fd_set read_set;

		FD_ZERO(&read_set);
		FD_SET(server, &read_set);
		if(select(2, &read_set, NULL, NULL, 0) < 1)
		{
			closesocket(server);
			WSACleanup();
			return S_FALSE;
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
				printf("New presenter: %ld\n", *((long*)msg.data));
				Player *p = new Player(ip, *((long*)msg.data));
				break;
			}

			case C_SET_FORCE_KEYFRAME:
			{
				c.encoderControl->SetForceKeyframe();
				break;
			}

			case C_CLEAR_FORCE_KEYFRAME:
			{
				c.encoderControl->ClearForceKeyframe();
				break;
			}

			default:
				printf("Invalid command received: %d\n", msg.header.command);
			}

			deleteMessage(msg);
		}
	}

	printf("shutting down.\n");
	closesocket(server);
	WSACleanup();

	system("pause");

	return 0;
}


