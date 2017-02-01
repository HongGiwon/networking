/* tcpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define NUM_FILE 10
#define MAX_FILE_NAME 20
#define NUM_FILE_SERVER 2
#define MAX_CLIENT 3

int request_file(int filenum);
int list_refresh();
void *serverthread(void *);

struct info_file
{
	char fname[MAX_FILE_NAME];
	int server_num; //indicate origin server
	int in_dod; //whether file is in dod or not
};
//metadata about files

struct info_file file[NUM_FILE] = {};
char addr_fserver[2][20];
int cur_file,end_thread;
pthread_mutex_t mutex, mutex2;

int main()
{
	int cur_t, i, j, rc, status;
	int sock, len, true = 1;
	int sin_size;
	pthread_t thread[2];

	struct sockaddr_in server_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
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
	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex2, NULL);
	
	printf("Enter IP address of first file server:");
	gets(addr_fserver[0]);
	printf("Enter IP address of second file server:");
	gets(addr_fserver[1]);
	
	end_thread = 0;
	cur_file = 0;
	
	cur_file = list_refresh();
	
	printf("\nTCP-based DoD Server open... on port 5000");
	fflush(stdout);

	for (i=0; i<MAX_CLIENT; i++)
	{
		pthread_create(&thread[i],NULL,&serverthread,(void *)sock);
		printf("\n%d Thread has been created\n",i);
	}

	
	
	while(1)
	{
		sleep(600);

		while(1)
		{
			sleep(600);
			cur_t = (int)time(NULL);
			if(((cur_t/3600)+9)%24 == 0 && (cur_t/60)%60 < 15)
				break;
		}
		
	printf("List of DoD server is now refreshed..\n");
	end_thread = 1;
		
	cur_file = list_refresh();
	end_thread = 0;
	printf("Refreshing done.\n");
	
	}

	for (j=0; j<MAX_CLIENT; j++)
	{
		rc = pthread_cancel(thread[j]);
		rc = pthread_join(thread[j],(void **)&status);
		if (rc == 0)
			printf("%d Thread has been terminated\n",j);
		else
		{
			printf("ERROR: Thread does not end well\n");
			return -1;
		}
				
	}
	
	status = pthread_mutex_destroy(&mutex);
	status = pthread_mutex_destroy(&mutex2);
	close(sock);
	
	return 0;
}

int list_refresh()
{
	int sock, len, i, curf = 0;
	int fs_port[2];
	char send_command[1024], buf[1024];
	char *ptr;
	struct hostent *host;
	struct sockaddr_in server_addr;
	
	fs_port[0] = 5001;
	fs_port[1] = 5002;
	
	for (i=0; i<NUM_FILE_SERVER; i++){
		
		host = gethostbyname(addr_fserver[i]);
	
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("Socket");
			exit(1);
		}
	
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(fs_port[i]);
		server_addr.sin_addr = *((struct in_addr *)host->h_addr);
		bzero(&(server_addr.sin_zero), 8);
	
		if (connect(sock, (struct sockaddr *)&server_addr,
			sizeof(struct sockaddr)) == -1)
		{
			perror("Connect");
			exit(1);
		}
	
		printf("Connection established.\n");
	
		fflush(stdout);
		
		strcpy(send_command,"LIST");
		send(sock, send_command, strlen(send_command),0);
		
		len = recv(sock,buf,sizeof(buf),0);
		buf[len] = '\0';
		
		ptr = strtok(buf,"\n");
		while(ptr != NULL)
		{
			strcpy(file[curf].fname,ptr);
			file[curf].server_num = i;
			file[curf].in_dod = 0;
			curf++;
			ptr = strtok(NULL,"\n");
		}
		
		strcpy(send_command,"q");
		send(sock, send_command, strlen(send_command),0);
		
		close(sock);

	}

	return curf;
}

int request_file(int filenum)
{
	int sock, len, i;
	int fs_port[2];
	int ret = 0;
	char send_command[1024], buf[1024];
	FILE *fp;
	struct hostent *host;
	struct sockaddr_in server_addr;
	
	fs_port[0] = 5001;
	fs_port[1] = 5002;
		
	host = gethostbyname(addr_fserver[file[filenum].server_num]);
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(fs_port[file[filenum].server_num]);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero), 8);
	
	if (connect(sock, (struct sockaddr *)&server_addr,
		sizeof(struct sockaddr)) == -1)
	{
		perror("Connect");
		exit(1);
	}
	
	printf("Connection with file server established.\n");
	
	fflush(stdout);
	
	strcpy(send_command,"GET ");
	strcat(send_command, file[filenum].fname);	
	send(sock, send_command, strlen(send_command),0);
	
	fp = fopen(file[filenum].fname,"wt");

	while(1)
	{
		len = recv(sock,buf,sizeof(buf),0);
		buf[len] = '\0';
		if(strcmp(buf,"0") == 0)
			break;
		if(strcmp(buf,"-1") == 0)
		{
			printf("ERROR : There is no such file in server.\n");
			ret = -1;
			break;
		}
		send(sock, "0",strlen("0"),0);
		fprintf(fp,"%s",buf);
	}
	fclose(fp);
			
		
	strcpy(send_command,"q");
	send(sock, send_command, strlen(send_command),0);
		
	file[filenum].in_dod = 1;
	close(sock);
	return ret;
}

void *serverthread(void* tsock)
{
	int i, connected, len;
	FILE *fp;
	char buf[1024], recv_command[1024];
	char *ptr;

	struct sockaddr_in client_addr;
	int sin_size;
	
	while (1)
	{
		
		pthread_mutex_lock(&mutex);
		//for synchronization. (one thread for one client)

		sin_size = sizeof(struct sockaddr_in);
		connected = accept((int)tsock, (struct sockaddr *)&client_addr, &sin_size);
		printf("\n Another connection from (%s , %d)\n",
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		fflush(stdout);
		
		pthread_mutex_unlock(&mutex);

		while (1)
		{
			while(end_thread == 1)
			{
				sleep(1);
			}

			strcpy(buf, "\n[SERVER] : Command\nLIST\t\t: Get list of available files.\nGET 'filename'  : Download file.\nq or Q\t\t: Quit.\n\0");
			send(connected, buf, strlen(buf), 0);

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
					strcpy(buf,"[SERVER] : These are avaliable files.\n");
					strcat(buf,"---------------------\n");
					while(end_thread == 1)
					{
						sleep(1);
					}
					for (i=0; i<cur_file; i++)
					{
						strcat(buf,file[i].fname);
						strcat(buf,"\n");
					}
					strcat(buf,"---------------------\n");
					send(connected, buf, sizeof(buf), 0);
				}
				else if(strncmp(recv_command,"GET",3) == 0)
				{
					ptr = strtok(recv_command," ");
					ptr = strtok(NULL," ");
					while(end_thread == 1)
					{
						sleep(1);
					}
					for(i = 0; i<cur_file; i++)
					{
						if(strcmp(ptr,file[i].fname) == 0)
						{
							strcpy(buf,"");

							if(file[i].in_dod == 0)
							{
								pthread_mutex_lock(&mutex2); //synchronization for writing file 
								if(file[i].in_dod == 0) 
								// in case of multiple clients demand same file. If one thread download it from file server, others don't have to
								{
									strcat(buf,"[SERVER] : Your file does not exist in DoD server.\n");
									strcat(buf,"[SERVER] : DoD server will download the requested file right now...\n");
									printf("\nRequested file does not exist in DoD server. Connect to file server..\n");
									if(request_file(i) == -1)
									{
										send(connected, "-1", sizeof("-1"),0);
										pthread_mutex_unlock(&mutex2);
										break;
									}
								}
								pthread_mutex_unlock(&mutex2);
							}
							

							
							strcat(buf,"[SERVER] : Your file is being downloaded...\n");
							send(connected, buf, sizeof(buf),0);
							len = recv(connected, buf, sizeof(buf),0);
							
							fp = fopen(ptr,"r");
							while(!feof(fp))
							{
								memset(&buf,0,sizeof(buf));
								fread(buf,sizeof(buf),1,fp);
								send(connected, buf, sizeof(buf),0);
								len = recv(connected, buf, sizeof(buf),0);
							}
							fclose(fp);
							send(connected, "0", sizeof("0"),0);
							break;
						}
						if(i == NUM_FILE -1 )
						{
							send(connected, "-1", sizeof("-1"),0);
						}
					}


				}
				else
				{
					printf("Unknown command from client.\n");
				}
			}
			
			fflush(stdout);
		}
	}

	pthread_exit((void *) 0);
}
