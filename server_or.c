#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "21643" //port for UDP
#define PORT2 "24643" //destination port

#define IP "127.0.0.1"
#define MAXBUFLEN 200

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	    if (sa->sa_family == AF_INET) {
			return &(((struct sockaddr_in*)sa)->sin_addr);
		}   
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;

	char *token;
	char str_or[20][50];
	char or_result[20][50];
	char str_result[MAXBUFLEN]="";

	int i,count,count1;
	char Num[20];
	char seq1[50];
	char seq2[50];
	char seq11[50];
	char seq22[50];
	char temp[50];
	char* darray;
	int len1;
	int len2;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	int sockfd_or1; //talk to edge server
	struct addrinfo hints_or1, *servinfo_or1, *p_or1;
	int rv_or1;
	int numbytes_or1;
	 
	memset(&hints_or1, 0, sizeof hints_or1);
	hints_or1.ai_family = AF_UNSPEC;
	hints_or1.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	if ((rv_or1 = getaddrinfo(IP, PORT2, &hints_or1, &servinfo_or1)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_or1));
		return 1;
	}

	// loop through all the results and make a UDP socket
	for(p_or1 = servinfo_or1; p_or1 != NULL; p_or1 = p_or1->ai_next) {
		if ((sockfd_or1 = socket(p_or1->ai_family, p_or1->ai_socktype,p_or1->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	//freeaddrinfo(servinfo);
	//freeaddrinfo(servinfo_or1);
	printf("The Server OR is up and running using UDP on port 21643\n");
	
	while(1){//maintain socket until the user stops the program******************************
	memset(buf,0,sizeof(buf));
	addr_len = sizeof their_addr;
	
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
					(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	printf("The Server OR start receiving lines from the edge server for OR computation. The computation results are:\n");
	//printf("listener: packet contains \n%s\n", buf);

	token = strtok(buf,"\n");
	i = 0;
	while(token != NULL){
		strcpy(str_or[i],token);
		i++;
		token = strtok(NULL,"\n");
	}
	//printf("send %d lines %s\n",i,str_or[0]);

	len1 = 0;
	len2 = 0;
	for(count = 0;count < i;count++){
		token = strtok(str_or[count],",");
		strcpy(Num,token);
		token = strtok(NULL,",");//skip operator
		token = strtok(NULL,",");
		strcpy(seq1,token);
		token = strtok(NULL,",");
		strcpy(seq2,token);
		len1 = strlen(seq1);
		len2 = strlen(seq2);
		strcpy(seq11,seq1);
		strcpy(seq22,seq2);
		//printf("send %d times %s\n",count,str_or[0]);

		if(len1 < len2){//add 0 before len1
			strcpy(temp,seq1);
			strcpy(seq1,"");
			for(count1 = 0;count1 < (len2-len1);count1++){
				strcat(seq1,"0");
			}
			strcat(seq1,temp);
		}
		else if(len1 > len2){//add 0 before len2
			strcpy(temp,seq2);
			strcpy(seq2,"");
			for(count1 = 0;count1 < (len1-len2);count1++){
				strcat(seq2,"0");
			}
			strcat(seq2,temp);
		}
		for(count1 = 0;count1 < (int)strlen(seq1);count1++){
			or_result[count][count1] = '0'+((seq1[count1]-'0')|(seq2[count1]-'0'));
		}
		or_result[count][count1] = '\0';

		darray = or_result[count];
		while(*(darray++)=='0');//delete 0 before the string
		strcpy(or_result[count],darray-1);
		if(or_result[count][0]=='\0')
			strcpy(or_result[count],"0");
		
		printf("%s or %s = %s\n",seq11,seq22,or_result[count]);
		memset(temp,0,sizeof(temp));
		strcpy(temp,or_result[count]);
		strcpy(or_result[count],Num);
		strcat(or_result[count],",");
		strcat(or_result[count],seq11);
		strcat(or_result[count],",");
		strcat(or_result[count],seq22);
		strcat(or_result[count],",");
		strcat(or_result[count],temp);
		
	}
	for(count1 = 0;count1 < count; count1++){
		strcat(str_result,or_result[count1]);
		strcat(str_result,"\n");
	}

	printf("The Server OR has successfully received %d lines from the edge server and finished all OR computations\n",count);

	if ((numbytes_or1 = sendto(sockfd_or1, str_result, strlen(str_result), 0, p_or1->ai_addr, p_or1->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
	//freeaddrinfo(servinfo_or1);

	printf("The Server OR has successfully finished sending all computation results to the edge server\n");

	str_result[0] = '\0';
	//close(sockfd);
	//close(sockfd_or1);

	}
	return 0;
}
