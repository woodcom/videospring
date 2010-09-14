#include "../../common/VideoSpringCommon.h"
#include <queue>


class Client;

long client_id = 0;
long total_clients = 0;

fd_set read_set, write_set, except_set;
Client *client_head = NULL;

class Client
{
	public:
		long id;
		
		BYTE *format;
		long formatLength;
		
		BYTE *firstFrame;
		long firstFrameLength;
		
		int ready;
		
		std::queue<Message> buffer;
		
	private:
	
		SOCKET socket;  //accepted socket
		
		char client_info[1024];
		
		DWORD pid;
		
		Client *next; //this will be a singly linked list

		
		long bytesSent;
//		long lastFrame;
//		long renderedFrame;
		int header;
		Message msg;
		int type;
		long receivedBytes;
		
	public:
		int Send(Message &m)
		{
			Message msg;
			msg.header.command = m.header.command;
			msg.header.length = m.header.length;
			
			if(m.header.length > 0)
			{
				msg.data = (BYTE*)malloc(msg.header.length);

				if(msg.data == NULL) 
				{
					printf("Error allocating %ld bytes.  Message will not be sent.\n", msg.header.length);
					return -1;
				}

				memcpy(msg.data, m.data, msg.header.length);
			}
			
			buffer.push(msg);

			return 0;
		}
		
		int Send()
		{
			if(buffer.empty())
			{
				return 0;
			}
			
			if(bytesSent < sizeof(MessageHeader))
			{
				int size = TCP_CHUNKSIZE;
				
				if(sizeof(MessageHeader) - bytesSent < TCP_CHUNKSIZE) size = sizeof(MessageHeader) - bytesSent;
				
				int bytes = send(socket, &buffer.front().header + bytesSent, size, 0);
				
				if (bytes >= 0)
				{
					bytesSent += bytes;
					
					if(bytesSent == sizeof(MessageHeader))
					{
						if(buffer.front().header.length == 0)
						{
							delete(buffer.front().data);
							buffer.front().data = NULL;
							buffer.pop();
							bytesSent = 0;
						}						
					}
				}
				else
				{
					printf("Error sending header!\n");
					return errno;
				}
			}
			else
			{
				long partSent = bytesSent - sizeof(MessageHeader);
				
				int size = TCP_CHUNKSIZE;
				
				if(buffer.front().header.length - partSent < TCP_CHUNKSIZE) size = buffer.front().header.length - partSent;
				
				int bytes = send(socket, buffer.front().data + partSent, size, 0);
				
				if (bytes >= 0)
				{
					bytesSent += bytes;

					if(bytesSent - sizeof(buffer.front().header) == buffer.front().header.length )
					{
						buffer.pop();
						bytesSent = 0;
					}
				}
				else
				{
//					printf("Error sending data of length %ld!\n", buffer.front().header.length);
					return errno;
				}
			}
			
			return 0;
		}
		
		int Receive()
		{
			if(receivedBytes < sizeof(MessageHeader))
			{
				int size = TCP_CHUNKSIZE;
				
				if(sizeof(MessageHeader) - receivedBytes < TCP_CHUNKSIZE) size = sizeof(MessageHeader) - receivedBytes;
				
				int bytes = recv(socket, ((BYTE*)&msg.header) + receivedBytes, size, 0);

				if(bytes >= 0)
				{
					receivedBytes += bytes;

					if(receivedBytes == sizeof(MessageHeader))
					{
						if(msg.header.length == 0)
						{
							receivedBytes = 0;
							if(doCommand(msg) != NOERROR) 
							{
								return errno;
							}
						}
						else
						{
							msg.data = (BYTE*)malloc(msg.header.length);
							if(msg.data == NULL)
							{
								printf("Error allocating %ld bytes.\n", msg.header.length);
								return -1;
							}
						}
					}
				}
				else
				{
					printf("Error receiving header!\n");
					return errno;
				}
			}
			else
			{
				long partLength = receivedBytes - sizeof(MessageHeader);

				long size = TCP_CHUNKSIZE;
				
				if(msg.header.length - partLength < TCP_CHUNKSIZE) size = msg.header.length - partLength;

				int bytes = recv(socket, msg.data + partLength, size, 0);

				if(bytes > 0)
				{
					receivedBytes += bytes;

					if(receivedBytes - sizeof(MessageHeader) == msg.header.length)
					{
						receivedBytes = 0;

						if(doCommand(msg) != NOERROR)
						{
							free(msg.data);
							return errno;
						}

						free(msg.data);
					}
				}
				else
				{
					printf("Error receiving data.  Length = %ld!\n", msg.header.length);
					return errno;
				}
			}

			return 0;
		}

		int doCommand(Message m)
		{
			switch(m.header.command)
			{
				case C_NEW_CLIENT:
				{
					printf("Type: Client\n");
					type = 2;
					memcpy(&pid, m.data, sizeof(DWORD));
					Client *c = client_head;
					while(c != NULL)
					{
						if(c->GetType() == 0 && (strcmp(client_info, c->client_info) != 0 || pid != c->pid) && c->formatLength > 0)
						{
							printf("Sending list of presenters to client...\n");
							Message msg;
							msg.header.command = C_NEW_PRESENTER;
							msg.header.length = sizeof(long);
							msg.data = (BYTE*)&c->id;
							Send(msg);
						}
						c = c->GetNext();
					}
					break;
				}

				case C_SET_PRESENTER_SEND:
				{
					printf("Type: Presenter\n");
					memcpy(&pid, m.data, sizeof(DWORD));
					firstFrameLength = 0;
					free(firstFrame);
					type = 0;

					break;
				}

				case C_SET_CLIENT_RECV:
				{
					if(type != 1)
						printf("Type: Renderer\n");

					memcpy(&pid, m.data, sizeof(DWORD));
					type = 1;

					Message msg;

					msg.header.command = 0;
					msg.header.length = 0;
					msg.data = NULL;

					Client *c = client_head;

					while(c != NULL)
					{
						if(c->GetType() == 0 && c->id == pid)
						{
							msg.header.length = c->formatLength;
							msg.data = c->format;
							break;
						}
						c = c->GetNext();
					}

					return Send(msg);
				
					break;
				}

				case C_BROADCAST:
				{
					if(firstFrameLength == 0)
					{
						firstFrameLength = m.header.length;
						firstFrame = (BYTE*)malloc(firstFrameLength);
						memcpy(firstFrame, m.data, firstFrameLength);
					}
					
					Client *c = client_head;
					while(c != NULL)
					{
						if(c->GetType() == 1 && c->pid == id && c->ready == 1)
						{
							Message msg;

							msg.header.command = 0;
							msg.header.length = m.header.length;
							msg.data = m.data;
							
							if(c->firstFrameLength == 0)
							{
								c->firstFrameLength = 1;
								msg.header.length = firstFrameLength;
								msg.data = firstFrame;

								Message fc;
								fc.header.command = C_SET_FORCE_KEYFRAME;
								fc.header.length = 0;
								Send(msg);
							}
							
							c->Send(msg);
						}
						c = c->GetNext();
					}

					break;
				}

				case C_RECEIVE:
				{
					printf("Ready to receive frames.\n");
					ready = 1;

					break;
				}
 
				case C_SET_FORMAT:
				{
					free(format);
					format = (BYTE*)malloc(m.header.length);
					memcpy(format, m.data, m.header.length);
					formatLength = m.header.length;
					
					Client *c = client_head;
					while(c != NULL) // Notify clients of new presenter
					{
						if(c->GetType() == 2 && (strcmp(client_info, c->client_info) != 0 || pid != c->pid))
						{
							printf("Notifying clients of new presenter...\n");
                                                        Message msg;
                                                        msg.header.command = C_NEW_PRESENTER;
                                                        msg.header.length = sizeof(uint32_t);
                                                        msg.data = (BYTE*)&id;

							c->Send(msg);
						}
						c = c->GetNext();
					}
					break;
				}

				default:
					printf("Unknown command: %d\n", m.header.command);
					break;
			}
			return NOERROR;
		}

		int GetType()
		{
			return type;
		}

		void SetSocket(SOCKET s)
		{
			socket = s;
		}

		Client *add(Client *c)
		{
			Client *cur = this;

			while(cur->next != NULL)
			{
				cur = cur->next;
			}

			cur->next = c;

			return c;
		}

		Client *del(Client *c)
		{
			printf("Deleting Client #%ld\n", c->id);
			Client * cur = client_head;

			while(cur->next != NULL && cur->next != c)
			{
				cur = cur->next;
			}

			if(cur->next == NULL && cur->next != c)
			{
				printf("Couldn't find client to delete\n");
				return cur->next;
			}

			cur->next = c->next;

			delete(c);
			c = NULL;

			return cur->next;
		}

		SOCKET GetSocket()
		{
			return socket;
		}

		Client *GetNext()
		{
			return next;
		}

		void SetNext(Client *n)
		{
			next = n;
		}

		//Constructor
		Client()
		{
			sockaddr_in addr;
			Client(SOCKET_ERROR, addr);
		}

		Client(SOCKET s, sockaddr_in addr)
		{
			strcpy((char*)client_info, (char*)(inet_ntoa(addr.sin_addr)));
			socket = s;
			next = NULL;
			type = -1;
			id = ++client_id;
			total_clients++;
			format = NULL;
			formatLength = 0;
			firstFrameLength = 0;
			firstFrame = NULL;
			ready = 0;
			bytesSent = 0;
			header = 1;
			receivedBytes = 0;
		}

		//destructor
		~Client()
		{
			if(type == 1)
			{
				printf("Dropping presenter...\n");
				Client * c = client_head;

				while(c != NULL)
				{
					if(c->GetType() == 2)
					{
						Message msg;
						msg.header.command = C_DROP_PRESENTER;
						msg.header.length = sizeof(id);
						msg.data = (BYTE*)malloc(sizeof(id));
						memcpy(msg.data, &id, sizeof(id));

						c->Send(msg);
					}

					c = c->GetNext();	
				}
			}
			total_clients--;
			closesocket(socket);
		}
};

int InitSets(SOCKET ListenSocket) 
{
	//Initialize

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&except_set);

	//Assign the ListenSocket to Sets

	FD_SET(ListenSocket, &read_set);
	FD_SET(ListenSocket, &except_set);

	//Iterate the client context list and assign the sockets to Sets

	Client *c = client_head;
	int max = ListenSocket;

	while(c != NULL)
	{
		if(!c->buffer.empty())
		{
			FD_SET(c->GetSocket(), &write_set);
		}
		else
		{
			FD_SET(c->GetSocket(), &read_set);
		}

		if(c->GetSocket() > max) max = c->GetSocket();
			FD_SET(c->GetSocket(), &except_set); 

		//Move to next node on the list
		c = c->GetNext();
	}

	return max;
}

void AcceptConnections(SOCKET ListenSocket)
{
	printf("Accepting Connections...\n");

	int hadLast = 0;
	while(1)
	{
		int max = InitSets(ListenSocket);
#ifdef WIN32
		if (select(0, &read_set, &write_set, &except_set, 0) > 0) 
		{
#else
		/*timeval t;
		memset(&t, 0, sizeof(timeval));*/

		if (select(max + 1, &read_set, &write_set, &except_set, 0) != -1) 
		{
#endif
			if (FD_ISSET(ListenSocket, &read_set)) 
			{
				sockaddr_in clientaddr;
#ifdef WIN32
				int clientlen = sizeof(sockaddr_in);
#else
				socklen_t clientlen = sizeof(sockaddr_in);
#endif

				SOCKET socket = accept(ListenSocket, (sockaddr*)&clientaddr, &clientlen);

				if(socket == INVALID_SOCKET)
				{
					printf("Error accepting socket!");
				}

				printf("Client connected from: %s\n", inet_ntoa(clientaddr.sin_addr)); 

#ifdef WIN32
				u_long no_block = 1;
				ioctlsocket(socket, FIONBIO, &no_block);
#else
				int flags;
				flags = fcntl(socket, F_GETFL, 0);
				if(flags != -1)
				{
					fcntl(socket, F_SETFL, flags | O_NONBLOCK);
				}
#endif
				Client *c = new Client(socket, clientaddr);

				if(client_head == NULL)
				{
					client_head = c;
				}
				else
				{
					client_head->add(c);
				}
			}

			if (FD_ISSET(ListenSocket, &except_set)) 
			{
				printf("Error accepting socket.");
				continue;
			}

			//Iterate the client context list to see if 
			//any of the socket there has changed its state

			int last = 0;
			
			Client *c = client_head;
			while (c != NULL)
			{
				if(c->GetType() == 0 && hadLast > 0)
				{
					c = c->GetNext();
					continue;
				}
				//Check in Read Set
				if (FD_ISSET(c->GetSocket(), &read_set))
				{
					if(c->GetType() == 1) last++;

					int ret = c->Receive();

					if(ret != 0 && ret != EAGAIN && ret != EWOULDBLOCK) // Disconnect client on errors
					{
						if(ret == ENOTCONN || ret == 104)
						{
							printf("Client #%ld Disconnected\n", c->id);

							if(c == client_head)
							{
								Client *tmp = client_head;
								c = client_head = client_head->GetNext();
								delete(tmp);
								tmp = NULL;
							}
							else
							{
								c = client_head->del(c);
							}
						}
						else
						{
							printf("Unhandled Read Error %d\n", ret);
						}

						continue;
					}
				}

				//Check in Write Set
				if (FD_ISSET(c->GetSocket(), &write_set))
				{
					if(c->GetType() == 1) last++;
					
					int ret = c->Send();

					if(ret != 0 && ret != EAGAIN && ret != EWOULDBLOCK) // Disconnect client on errors
					{				
						if(ret == ENOTCONN || ret == 104)
						{
							printf("Client #%ld Disconnected\n", c->id);
	
							if(c == client_head)
							{
								Client *tmp = client_head;
								c = client_head = client_head->GetNext();
								delete(tmp);
								tmp = NULL;
							}
							else
							{
								c = client_head->del(c);
							}
						}
						else
						{
							printf("Unhandled Write Error %d\n", ret);
						}

						continue;
					}
				}

				//Check in except Set
				if (FD_ISSET(c->GetSocket(), &except_set))
				{

					printf("Client #%ld Disconnected %d", c->id, errno);

					if(c == client_head)
					{
						Client *tmp = client_head;
						c = client_head = client_head->GetNext();
						delete(tmp);
						tmp = NULL;
					}
					else
					{
						c = client_head->del(c);
					}
					continue;
				}

				c = c->GetNext();
			}
			
			hadLast = last;
		}
		else //select
		{
			// printf("Error executing select()");
			//return;
		}
	}
}


int main(int argc, char **argv)
{
#ifdef WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(server == INVALID_SOCKET)
	{
		printf("Error creating socket.\n");
	}

	sockaddr_in serveraddr;
	
	memset((void*)&serveraddr,0,sizeof(sockaddr_in));

	serveraddr.sin_family = AF_INET;
 	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(1234);

	if (bind(server, (sockaddr *)&serveraddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("Socket bind failed!\n");
#ifdef WIN32
		WSACleanup();
		system("pause");
#endif
		return 1;
	}

	if(listen(server, SOMAXCONN) != 0)
	{
		printf("Socket listen failed!\n");
#ifdef WIN32
		WSACleanup();
		system("pause");
#endif
		return 1;
	}
	else
	{
		printf("Listening on port 1234.\n");
	}

	AcceptConnections(server);

#ifdef WIN32
	system("pause");
#endif
	return 0;
}

