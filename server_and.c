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

#define MYPORT "22643" //port for UDP
#define PORT2 "24643" //destination port

#define IP "127.0.0.1"
#define MAXBUFLEN 100

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
//	char s[INET6_ADDRSTRLE
	char *token;
	char str_and[20][30];
	char and_result[20][30];
	char str_result[MAXBUFLEN]="";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	int sockfd_and1; //talk to edge server
	struct addrinfo hints_and1, *servinfo_and1, *p_and1;
	int rv_and1;
	int numbytes_and1;
	 
	memset(&hints_and1, 0, sizeof hints_and1);
	hints_and1.ai_family = AF_UNSPEC;
	hints_and1.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	if ((rv_and1 = getaddrinfo(IP, PORT2, &hints_and1, &servinfo_and1)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_and1));
		return 1;
	}

	// loop through all the results and make a UDP socket
	for(p_and1 = servinfo_and1; p_and1 != NULL; p_and1 = p_and1->ai_next) {
		if ((sockfd_and1 = socket(p_and1->ai_family, p_and1->ai_socktype,p_and1->ai_protocol)) == -1) {
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
	freeaddrinfo(servinfo);
	printf("The Server AND is up and running using UDP on port 22643\n");
	
	memset(buf,0,sizeof(buf));
	addr_len = sizeof their_addr;
	printf("The Server AND start receiving lines from the edge server for AND computation. The computation results are:\n");
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
					(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	
	//printf("listener: packet contains \n%s", buf);
	
	token = strtok(buf,"\n");
	int i = 0;
	while(token != NULL){
		strcpy(str_and[i],token);
		i++;
		token = strtok(NULL,"\n");
	}
	//printf("send %d lines %s\n",i,str_and[1]);

	int count,count1;
	char Num[20];
	char seq1[20];
	char seq2[20];
	char seq11[20];
	char seq22[20];
	char temp[20];
	char* darray;
	int len1 = 0;
	int len2 = 0;
	for(count = 0;count < i;count++){
		token = strtok(str_and[count],",");
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
		//printf("send %d times %s %s\n",count,token,str_and[1]);

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
		//printf("test001:%s,%s\n",seq1,seq2);
		for(count1 = 0;count1 < (int)strlen(seq1);count1++){
			and_result[count][count1] = '0'+((seq1[count1]-'0')&(seq2[count1]-'0'));
		}
		and_result[count][count1] = '\0';
		
		//printf("test0:%s\n",and_result[count]);
		darray = and_result[count];
		//printf("test1:%s\n",darray);
		while(*(darray++)=='0');//delete 0 before the string
		strcpy(and_result[count],darray-1);
		if(and_result[count][0]=='\0')
			strcpy(and_result[count],"0");
		
		printf("<%s> and <%s> = %s\n",seq1,seq2,and_result[count]);
		//memset(temp,0,sizeof(temp));
		strcpy(temp,and_result[count]);
		printf("test2:%s\n",temp);
		strcpy(and_result[count],Num);
		strcat(and_result[count],",");
		strcat(and_result[count],seq11);
		strcat(and_result[count],",");
		strcat(and_result[count],seq22);
		strcat(and_result[count],",");
		strcat(and_result[count],temp);	
	}
	for(count1 = 0;count1 < count; count1++){
		strcat(str_result,and_result[count1]);
		strcat(str_result,"\n");
	}
	
	printf("test3:%s\n",str_result);
	printf("The Server AND has successfully received %d lines from the edge server and finished all AND computations\n",count);
	
	strcpy(temp,str_result);
	strcpy(str_result,"a");
	strcat(str_result,temp);
	printf("test4:%s\n",str_result);
	if ((numbytes_and1 = sendto(sockfd_and1, str_result, strlen(str_result), 0, p_and1->ai_addr, p_and1->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
	freeaddrinfo(servinfo_and1);

	printf("The Server AND has successfully finished sending all computation results to the edge server\n");

	close(sockfd);
	close(sockfd_and1);

	return 0;
}


