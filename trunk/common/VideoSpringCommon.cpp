#include "VideoSpringCommon.h"

int sendMessage(SOCKET s, Message *m)
{
	long bytesSent = 0;
	int bytes = 0;

	do
	{
		int size = TCP_CHUNKSIZE;

		if(sizeof(MessageHeader) - bytesSent < size)
		{
			size = sizeof(MessageHeader) - bytesSent;
		}

		do
		{
			bytes = send(s, (char*)&m->header + bytesSent, size, 0);

			if(bytes == 0)
			{
					printf("Disconnected(send)\n");
					return -1;
			}
			else if(bytes < 0)
			{
#ifdef WIN32
				int error = WSAGetLastError();

				if(error == WSAEWOULDBLOCK)
				{
					continue;
				}
#else
				int error = errno;

				if(error == EAGAIN)
				{
					continue;
				}
#endif
				printf("Error(send) %d\n", error);
				return -1;
			}
			else
			{
				break;
			}
		}
		while(1);

		bytesSent += bytes;
	}
	while(bytesSent != sizeof(MessageHeader));

	if(m->header.length > 0) 
	{
		bytesSent = 0;

		do
		{
			int size = TCP_CHUNKSIZE;

			if(m->header.length - bytesSent < size)
			{
				size = m->header.length - bytesSent;
			}

			do
			{
				bytes = send(s, (char*)m->data + bytesSent, size, 0);

				if(bytes == 0)
				{
						printf("Disconnected(send)\n");
						return -1;
				}
				else if(bytes < 0)
				{
#ifdef WIN32
					int error = WSAGetLastError();

					if(error == WSAEWOULDBLOCK)
					{
						continue;
					}
#else
					int error = errno;

					if(error == EAGAIN)
					{
						continue;
					}
#endif
					printf("Error(send) %d\n", error);
					
					return -1;
				}
				else
				{
					break;
				}
			}
			while(1);

			bytesSent += bytes;
		}
		while(bytesSent != m->header.length);
	}

	return NOERROR;
}

int receiveMessage(SOCKET s, Message &m)
{
	long bytesReceived = 0;
	int bytes = 0;

	do
	{
		int size = TCP_CHUNKSIZE;

		if(sizeof(MessageHeader) - bytesReceived < TCP_CHUNKSIZE) size = sizeof(MessageHeader) - bytesReceived;

		do
		{
			bytes = recv(s, (char*)&m.header + bytesReceived, size, 0);

			if(bytes == 0)
			{
					printf("Disconnected(recv)\n");
					return -1;
			}
			else if(bytes < 0)
			{
#ifdef WIN32
				int error = WSAGetLastError();

				if(error == WSAEWOULDBLOCK)
				{
					printf("Blocking!\n");
					continue;
				}
#else
				int error = errno;

				if(error == EAGAIN)
				{
					continue;
				}
#endif
				printf("Error(recv) %d\n", error);
				return -1;
			}
			else
			{
				break;
			}
		}
		while(1);

		bytesReceived += bytes;
	}
	while(bytesReceived != sizeof(MessageHeader));
	
	if(m.header.length > 0)
	{
		bytesReceived = 0;

		m.data = (BYTE*)malloc(m.header.length);

		do
		{
			int size = TCP_CHUNKSIZE;

			if(m.header.length - bytesReceived < TCP_CHUNKSIZE) size = m.header.length - bytesReceived;

			do
			{
				bytes = recv(s, (char*)m.data + bytesReceived, size, 0);
				
				if(bytes == 0)
				{
						printf("Disconnected(recv)\n");
						return -1;
				}
				else if(bytes < 0)
				{
#ifdef WIN32
					int error = WSAGetLastError();

					if(error == WSAEWOULDBLOCK)
					{
						continue;
					}
#else
					int error = errno;

					if(error == EAGAIN)
					{
						continue;
					}
#endif
					printf("Error(recv) %d\n", error);
					return -1;
				}
				else
				{
					break;
				}
			}
			while(1);

			bytesReceived += bytes;
		}
		while(bytesReceived != m.header.length);
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
