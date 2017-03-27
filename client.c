#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define IP "127.0.0.1"
#define PORT "23643" // the port number for TCP
#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	//char operation[5];
	//char seq1[20];
	//char seq2[20];
	char buf1[MAXDATASIZE]="";

	if (argc != 2) {
		fprintf(stderr,"usage: client file\n");
		exit(1);
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);

	//printf("client: connecting to %s\n", s);
	printf("The client is up and running\n");
	freeaddrinfo(servinfo); // all done with this structure
	
	FILE *r = fopen(argv[1],"r");
	int line = 0;
	while(fscanf(r,"%s\n",buf)!=EOF){
		line++;	
		//printf("%s\n",buf);
		
		strcat(buf1,buf);
		strcat(buf1,"\n");
	}

	if(send(sockfd,buf1,MAXDATASIZE-1, 0) == -1)
		perror("send");


	printf("The client has successfully finished sending %d lines to the edge server\n",line);
			

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	printf("The client has successfully finished receiving all computation results from the edge server\n");

	buf[numbytes] = '\0';
	printf("The final computation result are:\n%s\n",buf);

	close(sockfd);
	return 0;
}


