#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT_NUM 1004

void caster(int fromfd, char* msg);
void bottomer(int socker2, char* user, int rnumb, char* color);
void* setupthread(void* thar);

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

typedef struct _NAT {
    char* user;
    int rnumb;
	int socker;
	struct _NAT* next;
	char* color;
} NAT;

typedef struct _sockthr {
    int socker;
} sockthr;

NAT *top = NULL;
NAT *bottom = NULL;
int pp[10];

void caster(int fromfd, char* msg)
{
    struct sockaddr_in CLA;
    socklen_t LENC = sizeof(CLA);
    if (getpeername(fromfd, (struct sockaddr*)&CLA, &LENC) < 0) error("do not know sender, error");
    NAT* current = top;
    NAT* user;
    while (current != NULL) {
        if (current->socker == fromfd) {
            user = current;
        }
        current = current->next;
    }
    
    current = top;
    while (current != NULL) {
        if (current->socker != fromfd && current->rnumb == user->rnumb) {
            char buff[512];

            sprintf(buff, "%s[%s]:%s\033[0m", user->color, user->user, msg);
            int messagoi = strlen(buff);
            int senn = send(current->socker, buff, messagoi, 0);
            if (senn != messagoi) error("sending error");
        }
        current = current->next;
    }
}

void bottomer(int socker2, char* user, int rnumb, char* color)
{
    if (top == NULL) {
        top = (NAT*) malloc(sizeof(NAT));
        top->socker = socker2;
        top->user = user;
        top->rnumb = rnumb;
        top->color = color;
        top->next = NULL;
        bottom = top;
    } else {
        bottom->next = (NAT*) malloc(sizeof(NAT));
        bottom->next->socker = socker2;
        bottom->next->user = user;
        bottom->next->rnumb = rnumb;
        bottom->next->color = color;
        bottom->next->next = NULL;
        bottom = bottom->next;
    }
}

void deletor(int socker2) {
    NAT* temp = top;
    while (temp->socker != socker2) {
        temp = temp->next;
    }
    pp[temp->rnumb]--;
    if (temp == top) {
        top = top->next;
    } else {
        NAT* temp2 = top;
        while (temp2->next != temp) {
            temp2 = temp2->next;
        }
        temp2->next = temp->next;
        if (temp == bottom) {
            bottom = temp2;
        }
    }
}

void* setupthread(void* thar)
{
	pthread_detach(pthread_self());
	int socker = ((sockthr*) thar)->socker;
	free(thar);
	char* buff;
    int senn;
    int reiv;
	do {
		buff = (char*) malloc(256);
		reiv = recv(socker, buff, 255, 0);
		if (reiv < 0) error("reciever error");
		if (reiv == 0) {
			deletor(socker);
			NAT* temp = top;
			printf("All Connected Clients:\n");
			while (temp != NULL) {
				printf("%s - Room %d\n", temp->user, temp->rnumb);
				temp = temp->next;
			}
			break;
		}
		caster(socker, buff);
	} while (reiv > 0);
	close(socker);
	return NULL;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	char* buff = (char*) malloc(256);
	int nr = 0;
	for (int i = 0; i < 10; i++) {
		pp[i] = 0;
	}
	int sockly = socket(AF_INET, SOCK_STREAM, 0);
	if (sockly < 0) error("socket error");

	struct sockaddr_in SRV;
	socklen_t slen = sizeof(SRV);
	memset((char*) &SRV, 0, sizeof(SRV));
	SRV.sin_family = AF_INET;
	SRV.sin_addr.s_addr = INADDR_ANY;
	SRV.sin_port = htons(PORT_NUM);

	int status = bind(sockly, (struct sockaddr*) &SRV, slen);
	if (status < 0) error("general error");

	listen(sockly, 5);
	while(1) {
		int rnumb;
		char* msg = " ";
    
		struct sockaddr_in CLIA;
		socklen_t LENC = sizeof(CLIA);
		int socker3 = accept(sockly,(struct sockaddr *) &CLIA, &LENC);
		if (socker3 < 0) error("general error");
		buff = (char*) malloc(256);
		int reiv = recv(socker3, buff, 255, 0);
		if (reiv < 0) error("reciever error");

        if ((buff[0] == 'n' && buff[1] == 'e' && buff[2] == 'w') || nr == 0) {
			pp[nr]++;
			rnumb = nr;
			nr++;
			msg = "new";
			int senn = send(socker3, msg, strlen(msg), 0);
			buff = (char*) malloc(256);
			msg = (char*) malloc(256);
			sprintf(msg, "%d", rnumb);
			reiv = recv(socker3, buff, 255, 0);
			senn = send(socker3, msg, strlen(msg), 0);
		} 
		else if (buff[0] == ' ') {
			buff = (char*) malloc(256);
			char* temp = (char*) malloc(256);
			for (int i = 0; i < nr; i++) {
				sprintf(temp, "Room %d: %d users/people\n", i, pp[i]);
				strcat(buff, temp);
			}
            printf("\n");
			strcat(buff, "type in a room number or new ('new' will make a new room):\n");
			int senn = send(socker3, buff, strlen(buff), 0);
			reiv = recv(socker3, buff, 255, 0);
			
			if (buff[0] == 'n' && buff[1] == 'e' && buff[2] == 'w') {
				pp[nr]++;
				rnumb = nr;
				nr++;
				msg = "new";
				senn = send(socker3, msg, strlen(msg), 0);
				buff = (char*) malloc(256);
				msg = (char*) malloc(256);
				sprintf(msg, "%d", rnumb);
				reiv = recv(socker3, buff, 255, 0);
				senn = send(socker3, msg, strlen(msg), 0);
			}
			else {
				rnumb = atoi(buff);
				if (rnumb == nr) {
					rnumb = rand() % nr;
					msg = "room number invalid, you will be selected to a randomly choosen room (choose a number that exists or is below 10)";
				} else if (rnumb < 0 || rnumb > nr) {
					msg = "room number invalid, you will be selected to a randomly chosen room (choose a number that exists or is below 10)\n";
					rnumb = rand() % nr;
				}
				pp[rnumb]++;
				senn = send(socker3, msg, strlen(msg), 0);
				msg = (char*) malloc(256);
				sprintf(msg, "%d", rnumb);
				reiv = recv(socker3, buff, 255, 0);
				senn = send(socker3, msg, strlen(msg), 0);
			}
		}
		else {
			rnumb = atoi(buff);
			if (rnumb < 0 || rnumb >= nr) {
				msg = "room number invalid, you will be selected to a randomly chosen room (choose a number that exists or is below 10)\n";
				rnumb = rand() % nr;
			}
			pp[rnumb]++;
			int senn = send(socker3, msg, strlen(msg), 0);
			msg = (char*) malloc(256);
			sprintf(msg, "%d", rnumb);
			reiv = recv(socker3, buff, 255, 0);
			senn = send(socker3, msg, strlen(msg), 0);
		}

		printf("Connected %s\n", inet_ntoa(CLIA.sin_addr));
		buff = (char*) malloc(256);
		reiv = recv(socker3, buff, 255, 0);
		if (reiv < 0) error("error recieving");
		for (int i = 0; i < 256; i++) {
			if (buff[i] == '\n') {
				buff[i] = '\0';
				break;
			}
		}
        
		char* paint[] = {"\033[0;94m", "\033[095m", "\033[0;92m","\033[0;91m", "\033[0;96m", "\033[0;93m"};
        int boolc;
		char* chold;
		do {
            chold = paint[rand() % 6];
			boolc = 0;
			NAT* current = top;
			while (current != NULL) {
				if (current->color == chold) {
					boolc = 1;
					break;
				}
				current = current->next;
			}
		} while (boolc == 1);
        bottomer(socker3, buff, rnumb, chold);

		NAT* temp = top;
		printf("All Connected Clients:\n");
		while (temp != NULL) {
			printf("%s Room %d\n", temp->user, temp->rnumb);
			temp = temp->next;
		}
		sockthr* thar = (sockthr*) malloc(sizeof(sockthr));
		if (thar == NULL) error("general error");
		
		thar->socker = socker3;
		pthread_t ptd;
		if (pthread_create(&ptd, NULL, setupthread, (void*) thar) != 0) error("general error");
	}
    return 0;
}

