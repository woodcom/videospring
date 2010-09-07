#include "VideoSpringCommon.h"

int sendMessage(SOCKET s, Message *m)
{
	long bytesSent = 0;
	int bytes = 0;

	do
	{
		int size = 1024;

		if(sizeof(MessageHeader) - bytesSent < size)
		{
			size = sizeof(MessageHeader) - bytesSent;
		}

		do
		{
			bytes = send(s, (char*)&m->header + bytesSent, size, 0);

			if(bytes == -1)
			{
				printf("Send in sendMessage failed!\n");
				#ifdef WIN32
				int error = WSAGetLastError();
				#else
				int error = errno;
				#endif

				if(error == WSAEWOULDBLOCK)
				{
					continue;
				}

				return error;
			}
			else
			{
				break;
			}
		}
		while(1);

		bytesSent += bytes;
	}
	while(bytesSent < sizeof(MessageHeader));

	if(m->header.length > 0) 
	{
		bytesSent = 0;

		do
		{
			int size = 1024;

			if(m->header.length - bytesSent < size)
			{
				size = m->header.length - bytesSent;
			}

			do
			{
				bytes = send(s, (char*)m->data + bytesSent, size, 0);

				if(bytes == -1)
				{
					printf("Send in sendMessage failed (body)!\n");
					#ifdef WIN32
					int error = WSAGetLastError();
					#else
					int error = errno;
					#endif

					if(error == WSAEWOULDBLOCK)
					{
						continue;
					}

					return error;
				}
				else
				{
					break;
				}
			}
			while(1);

			bytesSent += bytes;
		}
		while(bytesSent < m->header.length);
	}

	return NOERROR;
}

int receiveMessage(SOCKET s, Message &m)
{
	long bytesReceived = 0;
	int bytes = 0;

	do
	{
		int size = 1024;

		if(sizeof(MessageHeader) - bytesReceived < 1024) size = sizeof(MessageHeader) - bytesReceived;

		do
		{
			bytes = recv(s, (char*)&m.header + bytesReceived, size, 0);

			if(bytes == -1)
			{
				printf("Recv in recvMessage failed!\n");
				#ifdef WIN32
				int error = WSAGetLastError();
				#else
				int error = errno;
				#endif

				if(error == WSAEWOULDBLOCK)
				{
					continue;
				}

				return error;
			}
			else
			{
				break;
			}
		}
		while(1);

		bytesReceived += bytes;
	}
	while(bytesReceived < sizeof(MessageHeader));
	
	if(m.header.length > 0)
	{
		bytesReceived = 0;

		m.data = (BYTE*)malloc(m.header.length);

		do
		{
			int size = 1024;

			if(m.header.length - bytesReceived < 1024) size = m.header.length - bytesReceived;

			do
			{
				bytes = recv(s, (char*)m.data + bytesReceived, size, 0);

				if(bytes == -1)
				{
					printf("Recv in recvMessage failed(body)\n");
					#ifdef WIN32
					int error = WSAGetLastError();
					#else
					int error = errno;
					#endif

					if(error == WSAEWOULDBLOCK)
					{
						continue;
					}

					return error;
				}
				else
				{
					break;
				}
			}
			while(1);

			bytesReceived += bytes;
		}
		while(bytesReceived < m.header.length);
	}

	return NOERROR;
}

int deleteMessage(Message &m)
{
	try
	{
		if(m.header.length > 0 && m.data != NULL)
			free(m.data);
	}
	catch(...)
	{
	}
	return NOERROR;
}
