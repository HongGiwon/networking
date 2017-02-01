/* tcpclient.c */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


int main()

{

	int sock, len;
	FILE *fp;
	char send_command[1024], buf[1024];
	char *ptr;
	struct hostent *host;
	struct sockaddr_in server_addr;

	printf("Enter IP address of DoD server:");
	gets(send_command);

	host = gethostbyname(send_command);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
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

	while (1)
	{
		len = recv(sock, buf, sizeof(buf),0);
		buf[len] = '\0';
		printf("%s",buf);
		printf("input : ");
		gets(send_command);
		fflush(stdout);

		send(sock, send_command, strlen(send_command),0);

		if (strncmp(send_command,"GET",3) == 0)
		{
			ptr = strtok(send_command," ");
			ptr = strtok(NULL," ");
			fp = fopen(ptr,"wt");
			
			len = recv(sock,buf,sizeof(buf),0);
			buf[len] = '\0';
			printf("\n%s",buf);
			send(sock, "0",strlen("0"),0);

			while(1)
			{
				len = recv(sock,buf,sizeof(buf),0);
				buf[len] = '\0';
				if(strcmp(buf,"0") == 0)
					break;
				if(strcmp(buf,"-1") == 0)
				{
					printf("There is no such file in server.\n");
					break;
				}
				send(sock, "0",strlen("0"),0);
				fprintf(fp,"%s",buf);
			}
			fclose(fp);
			printf("\n-------End File download---------\n");

		}
		else if (strcmp(send_command, "LIST") == 0)
		{
			len = recv(sock,buf,sizeof(buf),0);
			buf[len] = '\0';
			printf("\n%s",buf);
		}
		else if(strcmp(send_command,"q") == 0 || strcmp(send_command,"Q") == 0)
		{
			break;
		}
		else
		{
			printf("Unknown command\n");
		}
		fflush(stdout);

	}
	close(sock);
	return 0;
}