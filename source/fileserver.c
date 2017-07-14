/* tcpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define NUM_FILE 5
#define MAX_FILE_NAME 20


int main()
{
	int sock, i, connected, len, true = 1;
	FILE *fp;
	char buf[1024], recv_command[1024];
	char fname[NUM_FILE][MAX_FILE_NAME] = {"file01.dat", "file02.dat","file03.dat", "txt01.txt", "code01.txt"};
	char *ptr;

	struct sockaddr_in server_addr, client_addr;
	int sin_size;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5001);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
		== -1) {
		perror("Unable to bind");
		exit(1);
	}

	if (listen(sock, 5) == -1) {
		perror("Listen");
		exit(1);
	}

	
	while (1)
	{
		printf("\nTCP-based File Server open... on port 5001");
		fflush(stdout);

		sin_size = sizeof(struct sockaddr_in);
		connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
		printf("\n I got a connection from (%s , %d)\n",
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		fflush(stdout);

		while (1)
		{

			len = recv(connected, recv_command, sizeof(recv_command), 0);
			recv_command[len] = '\0';

			if (strcmp(recv_command, "q") == 0 || strcmp(recv_command, "Q") == 0)
			{
				close(connected);
				break;
			}

			else
			{
				if(strcmp(recv_command, "LIST") == 0)
				{
					strcpy(buf,fname[0]);
					for (i=1; i<NUM_FILE; i++)
					{
						strcat(buf,"\n");
						strcat(buf,fname[i]);
					}
					send(connected, buf, sizeof(buf), 0);
				}
				else if(strncmp(recv_command,"GET",3) == 0)
				{
					ptr = strtok(recv_command," ");
					ptr = strtok(NULL," ");
					for(i = 0; i<NUM_FILE; i++)
					{
						if(strcmp(ptr,fname[i]) == 0)
						{
							fp = fopen(ptr,"r");
							while(!feof(fp))
							{
								memset(&buf,0,sizeof(buf));
								fread(buf,sizeof(buf),1,fp);
								send(connected, buf, sizeof(buf),0);
								len = recv(connected, buf, sizeof(buf),0);
							}
							fclose(fp);
							break;
						}
						if(i == NUM_FILE -1 )
						{
							send(connected, "-1", sizeof("-1"),0);
						}
					}
						
					send(connected, "0", sizeof("0"),0);

				}
				else
				{
					printf("Unknown command from client.\n");
				}
			}
			
			fflush(stdout);
		}
	}

	close(sock);
	return 0;
}