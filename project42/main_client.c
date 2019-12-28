#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>

#define PORT_NUM 1004

void* SENDIT(void* holder);
void* TAKEIT(void* holder);


typedef struct _ART {
    int socker;
} ART;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void* SENDIT(void* holder)
{
    pthread_detach(pthread_self());
    int socker2 = ((ART*) holder)->socker;
    free(holder);
    char* buff = (char*) malloc(256);
    int i;
    while (1) {
        buff = (char*) malloc(255);
        fgets(buff, 255, stdin);
        if (strlen(buff) == 1) {
            buff[0] = '\0';}
        i = send(socker2, buff, strlen(buff), 0);
        if (i < 0) error("general error");
        if (i == 0){
            break;}
    }
    return NULL;
}

void* TAKEIT(void* holder)
{
	pthread_detach(pthread_self());
    int socker2 = ((ART*) holder)->socker;
    char* buff = (char*) malloc(512);
	free(holder);
	int i = recv(socker2, buff, 512, 0);
	while (i > 0) {
		printf("\n%s\n", buff);
		buff = (char*) malloc(512);
		i = recv(socker2, buff, 512, 0);
	}
	return NULL;
}


int main(int argc, char *argv[])
{
    char* msg = "";
	int blank = 0;
    char* buff = (char*) malloc(256);
	if (argc < 2) error("You did not give a hostname or IP address...");
	int socker2 = socket(AF_INET, SOCK_STREAM, 0);
	if (socker2 < 0) error("socket opening error");
    
	struct sockaddr_in SRV;
	socklen_t lengther = sizeof(SRV);
	memset((char*) &SRV, 0, sizeof(SRV));
    SRV.sin_port = htons(PORT_NUM);
	SRV.sin_addr.s_addr = inet_addr(argv[1]);
    SRV.sin_family = AF_INET;
	printf("attempt to link to %s...\n", inet_ntoa(SRV.sin_addr));
    
    
	int status = connect(socker2, (struct sockaddr *) &SRV, lengther);
    char* roomer = " ";
	if (argc == 3) {
		roomer = argv[2];
	} else {
		blank = 1;
	}
	int q = send(socker2, roomer, strlen(roomer), 0);
	if (status < 0) error("connection error");
	q = recv(socker2, buff, 256, 0);
	if (blank == 1) {
		printf("%s", buff);
		buff = (char*) malloc(256);
		fgets(buff, 255, stdin);
		q = send(socker2, buff, strlen(buff), 0);
		msg = (char*) malloc(256);
		q = recv(socker2, msg, 256, 0);
		if (strcmp(msg, "new") != 0){
			printf("%s\n", msg);
			msg = "";
		}
	}
	else if (strcmp(buff, "new") == 0) {
		msg = "";
	} 
	else {
		printf("%s\n", buff);
	}

	q = send(socker2, "Created", 7, 0); //OG OK,3,0
	buff = (char*) malloc(256);
	q = recv(socker2, buff, 256, 0);
	printf("Connected to %s with %s room number %s\n", argv[1], msg, buff);
	buff = (char*) malloc(256);
	printf("Give yourself a username: ");
	fgets(buff, 255, stdin);
	q = send(socker2, buff, strlen(buff), 0);
	if (q < 0) error("socket writing error");

    ART* holder;
	pthread_t ptd1;
	pthread_t ptd2;
	
	holder = (ART*) malloc(sizeof(ART));
	holder->socker = socker2;
	pthread_create(&ptd1, NULL, SENDIT, (void*) holder);
    holder = (ART*) malloc(sizeof(ART));
	holder->socker = socker2;
	pthread_create(&ptd2, NULL, TAKEIT, (void*) holder);

	pthread_join(ptd1, NULL);
    close(socker2);
    return 0;
}

