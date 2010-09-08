#include "../../common/VideoSpringCommon.h"

class Client;

long client_id = 0;
long total_clients = 0;

fd_set read_set, write_set, except_set;
Client *client_head = NULL;

#define BUFF_SIZE 10

struct FrameBuff
{
	long number;
	BYTE *frame;
	long frameLength;
};

class Client
{
	public:
		long id;
		BYTE *format;
		long formatLength;
		int wait;
		BYTE *firstFrame;
		long firstFrameLength;
		FrameBuff frames[BUFF_SIZE];

	private:
		SOCKET socket;  //accepted socket
		char client_info[1024];
		DWORD pid;
		Client *next; //this will be a singly linked list

		// Statistics
		long totalBytes;
		long sentBytes;
		long receivedBytes;

		long lastFrame;
		long renderedFrame;

		int type;

	public:
		void doCommand(Message m)
		{
			switch(m.header.command)
			{
			case C_SET_PRESENTER_SEND:
				printf("\nType: Presenter\n");
				memcpy(&pid, m.data, sizeof(DWORD));
				firstFrameLength = 0;
				free(firstFrame);
				type = 0;
				break;
			case C_SET_CLIENT_RECV:
			{
				if(type != 1)
					printf("\nType: Client\n");

				memcpy(&pid, m.data, sizeof(DWORD));
				type = 1;

				Message msg;
				msg.header.length = 0;
				msg.data = NULL;

				Client *c = client_head;

				while(c != NULL)
				{
					if(c->GetType() == 0 && (strcmp(client_info, c->client_info) != 0 || pid != c->pid) != 0 && c->formatLength > 0)
					{
						msg.header.length = c->formatLength;
						msg.data = c->format;
						c->wait = 0;
						break;
					}
					c = c->GetNext();
				}

				sendMessage(socket, &msg);
				
				break;
			}
			case C_BROADCAST:
				if(firstFrameLength == 0)
				{
					firstFrameLength = m.header.length;
					firstFrame = (BYTE*)malloc(firstFrameLength);
					memcpy(firstFrame, m.data, firstFrameLength);
					lastFrame = 0;
				}
				else
				{
					lastFrame++;
				}
				frames[lastFrame % BUFF_SIZE].frameLength = m.header.length;
				free(frames[lastFrame % BUFF_SIZE].frame);
				frames[lastFrame % BUFF_SIZE].frame = (BYTE*)malloc(m.header.length);
				memcpy(frames[lastFrame % BUFF_SIZE].frame, m.data, m.header.length);
				frames[lastFrame % BUFF_SIZE].number = lastFrame;
				if(lastFrame > BUFF_SIZE - 2)
				{
					wait = 1;
				}
				break;
			case C_RECEIVE:
			{
				Client *c = client_head;
				while(c != NULL)
				{
					if(c->GetType() == 0 && (strcmp(client_info, c->client_info) != 0 || pid != c->pid))
					{
						c->wait = 0;
						Message m;
						if(firstFrameLength == 0)
						{
							firstFrameLength = c->firstFrameLength;
							m.header.length = c->firstFrameLength;
							m.data = c->firstFrame;
							renderedFrame = 0;
						}
						else
						{
							BYTE *frame = NULL;
							long frameLength = 0;
							long newFrame = 0;
							for(int n = 0; n < BUFF_SIZE; n++)
							{
								if(c->frames[n].number == renderedFrame + 1)
								{
									m.data = c->frames[n].frame;
									m.header.length = c->frames[n].frameLength;
									newFrame = renderedFrame + 1;
									break;
								}
								if(c->frames[n].number == c->lastFrame)
								{
									m.data = c->frames[n].frame;
									m.header.length = c->frames[n].frameLength;
									newFrame = c->lastFrame;
								}
							}
							if(newFrame == 0 || newFrame == renderedFrame)
							{
								Message m;
								m.header.command = 0;
								m.header.length = 0;
								m.data = NULL;
								sendMessage(socket, &m);
								return;
							}

							if(newFrame == c->lastFrame && newFrame != renderedFrame + 1)
							{
								printf("%d frames dropped\n", newFrame - renderedFrame);
							}

							renderedFrame = newFrame;
						}
						sendMessage(socket, &m);
						return;
					}
					c = c->GetNext();
				}
				Message m;
				m.header.command = 0;
				m.header.length = 0;
				m.data = NULL;
				sendMessage(socket, &m);
				break;
			}
			case C_SET_FORMAT:
				printf("\nFormat Received\n");
				free(format);
				format = (BYTE*)malloc(m.header.length);
				memcpy(format, m.data, m.header.length);
				formatLength = m.header.length;
				wait = 1;
				break;
			default:
				printf("\nUnknown command: %d\n", m.header.command);
				break;
			}
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
		 printf("\nAdding Client #%ld\n", c->id);
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
		 printf("\nDeleting Client #%ld\n", c->id);
		 Client * cur = client_head;

		 while(cur->next != NULL && cur->next != c)
		 {
			 cur = cur->next;
		 }

		 if(cur->next == NULL && cur->next != c)
		 {
			 printf("\nCouldn't find client to delete\n");
			 return cur->next;
		 }

		 cur->next = c->next;

		 delete(c);

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
		totalBytes = 0;
		sentBytes = 0;
		receivedBytes = 0;
		format = NULL;
		formatLength = 0;
		wait = 0;
		firstFrameLength = 0;
		firstFrame = NULL;
		lastFrame = 0;
		renderedFrame = 0;
		for(int n = 0 ; n < BUFF_SIZE; n++)
		{
			frames[n].number = -1;
			frames[n].frame = NULL;
			frames[n].frameLength = 0;
		}
		printf("\nCreated Client #%ld\n", id);
     }

     //destructor

     ~Client()
     {
		 total_clients--;
         closesocket(socket);
     }
};

int InitSets(SOCKET ListenSocket) 
{
     //Initialize

     FD_ZERO(&read_set);
//     FD_ZERO(&write_set);
 //    FD_ZERO(&except_set);

     //Assign the ListenSocket to Sets

     FD_SET(ListenSocket, &read_set);
   //  FD_SET(ListenSocket, &except_set);

     //Iterate the client context list and assign the sockets to Sets

     Client *c = client_head;
	int max = ListenSocket;
     while(c != NULL)
     {
          //FD_SET(c->GetSocket(), &write_set);
          FD_SET(c->GetSocket(), &read_set);
	if(c->GetSocket() > max) max = c->GetSocket();
       //   FD_SET(c->GetSocket(), &except_set); 

          //Move to next node on the list

          c = c->GetNext();
     }
	return max;
}

void AcceptConnections(SOCKET ListenSocket)
{
	printf("Accepting Connections...\n");
     while(1)
     {
          int max = InitSets(ListenSocket);
#ifdef WIN32
          if (select(0, &read_set, &write_set, &except_set, 0) > 0) 
          {
#else
  //        timeval t;
//          memset(&t, 0, sizeof(timeval));
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

                    printf("\nClient connected from: %s\n", inet_ntoa(clientaddr.sin_addr)); 

                    //u_long no_block = 1;
                    //ioctlsocket(socket, FIONBIO, &no_block);
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
                    printf("\nError accepting socket.");
                    continue;
               }

               //Iterate the client context list to see if 

               //any of the socket there has changed its state

               Client *c = client_head;
               while (c != NULL)
               {
                    //Check in Read Set

                    if (FD_ISSET(c->GetSocket(), &read_set))
                    {
						if(c->GetType() == 0 && c->wait) 
						{
							c = c->GetNext();
							continue;
						}
						Message m;
						int error = 0;
						if((error = receiveMessage(c->GetSocket(), m)) != 0)
						{
							printf("Client #%ld Disconnected %d", c->id, error);
							if(c == client_head)
							{
								Client *tmp = client_head;
								c = client_head = client_head->GetNext();
								delete(tmp);
							}
							else
							{
								c = client_head->del(c);
							}

                            continue;
                        }
						else
						{
							c->doCommand(m);
							deleteMessage(m);
						}
                    }

				/*	if(FD_ISSET(c->GetSocket(), &except_set))
					{
						printf("Client #%d Disconnected", c->id);
						if(c == client_head)
						{
							Client *tmp = client_head;
							c = client_head = client_head->GetNext();
							delete(tmp);
						}
						else
						{
							c = client_head->del(c);
						}
						continue;
					}*/

                    c = c->GetNext();
               }

          }
          else //select
          {
              // printf("\nError executing select()");
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

