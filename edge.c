#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "23643" // the port number for TCP
#define PORT2 "24643" // the port number for UDP
#define PORT_and "22643" //the port number for and_server
#define PORT_or "21643" //the port number for or_server
#define BACKLOG 3 // how many pending connections queue will hold
#define BUFFER_SIZE 500
#define IP "127.0.0.1"
void sigchld_handler(int s)
{
// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//copy string
void my_strcpy(char *target, char *source)
{
	while(*source)
		*target++ = *source++;
	*target = '\0';
}

int main(void)
{
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	int sockfd_and1; //talk to and_server
	//int sockfd_and2; //listen to and_server
	int sockfd_or1; //talk to or_server
	int sockfd_b2; //listen to 2 backend_servers

	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	int line_and;
	int line_or;
	int i,flag;
	int len1,len2;
	int t1,t2;
	int count;
	int check;
	char num[20];
	char seq1[50];
	char seq2[50];
	char result[50];

	char buf[BUFFER_SIZE];
	char buf_1[BUFFER_SIZE];
	char buf_2[BUFFER_SIZE];
	char *token;
	char *token1;
	char *token2;
	char *darray1;
	char str_and[BUFFER_SIZE]="";
	char str_or[BUFFER_SIZE]="";
	char strint[BUFFER_SIZE];
	char buf_12[BUFFER_SIZE][BUFFER_SIZE];
	char buf_21[BUFFER_SIZE][BUFFER_SIZE];
	char buf_result[BUFFER_SIZE][30];
/********************************************************/
	struct addrinfo hints_or1, *servinfo_or1, *p_or1;
	int rv_or1;
	int numbytes_or1;

	memset(&hints_or1, 0, sizeof hints_or1);
	hints_or1.ai_family = AF_UNSPEC;
	hints_or1.ai_socktype = SOCK_DGRAM;
/*******************************************************/
	struct addrinfo hints_b2, *servinfo_b2, *p_b2;
	int rv_b2;
	int numbytes_b2;
	struct sockaddr_storage b_addr;
	socklen_t b_len;

	memset(&hints_b2, 0, sizeof hints_b2);
	hints_b2.ai_family = AF_UNSPEC;
	hints_b2.ai_socktype = SOCK_DGRAM;
	hints_b2.ai_flags = AI_PASSIVE;//Use my IP
/********************************************************/
	struct addrinfo hints_and1, *servinfo_and1, *p_and1;
	int rv_and1;
	int numbytes_and1;

	memset(&hints_and1, 0, sizeof hints_and1);
	hints_and1.ai_family = AF_UNSPEC;
	hints_and1.ai_socktype = SOCK_DGRAM;
/*******************************************************/
/*	struct addrinfo hints_and2, *servinfo_and2, *p_and2;
	int rv_and2;
	int numbytes_and2;
	struct sockaddr_storage and_addr;
	socklen_t and_len;

	memset(&hints_and2, 0, sizeof hints_and2);
	hints_and2.ai_family = AF_UNSPEC;
	hints_and2.ai_socktype = SOCK_DGRAM;
	hints_and2.ai_flags = AI_PASSIVE;//Use my IP*/
/*******************************************************/
	if ((rv_or1 = getaddrinfo(IP, PORT_or, &hints_or1, &servinfo_or1)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_or1));
		return 1;
	}

	if ((rv_b2 = getaddrinfo(NULL, PORT2, &hints_b2, &servinfo_b2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_b2));
		return 1;
	}

	if ((rv_and1 = getaddrinfo(IP, PORT_and, &hints_and1, &servinfo_and1)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_and1));
		return 1;
	}

	/*if ((rv_and2 = getaddrinfo(NULL, PORT2, &hints_and2, &servinfo_and2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_and2));
		return 1;
	}*/

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// UDP socket talk to or_server
	for(p_or1 = servinfo_or1; p_or1 != NULL; p_or1 = p_or1->ai_next) {
		if ((sockfd_or1 = socket(p_or1->ai_family, p_or1->ai_socktype,
						p_or1->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}
	
	// UDP socket listen to or_server
	for(p_b2 = servinfo_b2; p_b2 != NULL; p_b2 = p_b2->ai_next) {
		if ((sockfd_b2 = socket(p_b2->ai_family, p_b2->ai_socktype, p_b2->ai_protocol)) == -1) {
			 perror("talker: socket");
			 continue;
		}
		if (bind(sockfd_b2, p_b2->ai_addr, p_b2->ai_addrlen) == -1){
			close(sockfd_b2);
			perror("listener:bind");
			continue;
		}

		break;
	}
	
	//freeaddrinfo(servinfo_b2);//This one could change the location

	// UDP socket talk to and_server
	for(p_and1 = servinfo_and1; p_and1 != NULL; p_and1 = p_and1->ai_next) {
		if ((sockfd_and1 = socket(p_and1->ai_family, p_and1->ai_socktype,
						p_and1->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}
	
	// UDP socket listen to and_server
/*	for(p_and2 = servinfo_and2; p_and2 != NULL; p_and2 = p_and2->ai_next) {
		if ((sockfd_and2 = socket(p_and2->ai_family, p_and2->ai_socktype, p_and2->ai_protocol)) == -1) {
			 perror("talker: socket");
			 continue;
		}
		if (bind(sockfd_and2, p_and2->ai_addr, p_and2->ai_addrlen) == -1){
			close(sockfd_and2);
			perror("listener:bind");
			continue;
		}

		break;
	}*/
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	//freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	printf("The edge server is up and running\n");

	while(1){	//main accept() loop*************************************************
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s);

		if(recv(new_fd,buf,BUFFER_SIZE,0) == -1)//TCP receive
			perror("receive");
		//printf("%s\n",buf);
		
		//divide the buf for different operations
		token = strtok(buf,"\n");
		
		line_and = 0;
		line_or = 0;
		i = 0;
		while(token != NULL){
			if(token[0]=='a'){
				sprintf(strint,"%d",i);
				strcat(str_and,strint);
				strcat(str_and,",");
				strcat(str_and,token);
				strcat(str_and,"\n");
				i++;
				line_and++;
			}
			else if(token[0]=='o'){
				sprintf(strint,"%d",i);
				strcat(str_or,strint);
				strcat(str_or,",");
				strcat(str_or,token);
				strcat(str_or,"\n");
				i++;
				line_or++;
			}
			token = strtok(NULL,"\n");
		}
		printf("The edge server has received %d lines from the client using TCP over port 23643\n",i);

		len1 = strlen(str_and);
		len2 = strlen(str_or);
		str_and[len1-1] = '\0';
		str_or[len2-1] = '\0';

		//UDP send to or_server
		if ((numbytes_or1 = sendto(sockfd_or1, str_or, strlen(str_or), 0,
						                     p_or1->ai_addr, p_or1->ai_addrlen)) == -1) {
			         perror("talker: sendto");
			         exit(1);
		}
		//freeaddrinfo(servinfo_or1);
		printf("The edge has successfully sent %d lines to Backend-Server OR.\n", line_or);
		//close(sockfd_or1);
		str_or[0] = '\0';

		//UDP send to and_server
		if ((numbytes_and1 = sendto(sockfd_and1, str_and, strlen(str_and), 0,
						                     p_and1->ai_addr, p_and1->ai_addrlen)) == -1) {
			         perror("talker: sendto");
			         exit(1);
		}
		//freeaddrinfo(servinfo_and1);
		printf("The edge has successfully sent %d lines to Backend-Server AND.\n", line_and);
		//close(sockfd_and1);
		str_and[0] = '\0';

		//freeaddrinfo(servinfo_b2);
		b_len = sizeof b_addr;
		flag = 0;
		printf("The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP over port 24643.\n");
		//UDP receive from backend_server
		if ((numbytes_b2 = recvfrom(sockfd_b2, buf_1, BUFFER_SIZE-1 , 0,(struct sockaddr *)&b_addr, &b_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		//printf("test1:%s\n",buf_1);
		buf_1[numbytes_b2] = '\0';
		//printf("test2:%s\n",buf_1);
		if(buf_1[0]=='a'){
		//	token1 = strtok(buf_1,":");
		//	token1 = strtok(NULL,":");
		//	strcpy(buf_1,token1);
			darray1 = buf_1;
			(darray1++);//delete a
		//	printf("test00:%s\n%s\n",darray1,buf_1);
			my_strcpy(buf_1,darray1);
			flag = 1;
		//	printf("AND Results are \n%s",buf_1);
		}
		
		
		if ((numbytes_b2 = recvfrom(sockfd_b2, buf_2, BUFFER_SIZE-1 , 0,(struct sockaddr *)&b_addr, &b_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		buf_2[numbytes_b2] = '\0';
		//printf("test3:%s\n",buf_2);
		if(buf_2[0]=='a'){
		//	token1 = strtok(buf_2,":");
		//	token1 = strtok(NULL,":");
		//	strcpy(buf_2,token1);
			darray1 = buf_2;
			(darray1++);//delete a
		//	printf("test01:%s\n%s\n",darray1,buf_2);
			my_strcpy(buf_2,darray1);
			flag = 2;
		//	printf("AND Results are \n%s",buf_2);
		}

		printf("The computation results are:\n");
		//sort the computation results********************************************
		token1 = strtok(buf_1,"\n");
		t1 = 0;
		while(token1 != NULL){
			strcpy(buf_12[t1],token1);
			t1++;
			token1 = strtok(NULL,"\n");
		}
		
		token2 = strtok(buf_2,"\n");
		t2 = 0;
		while(token2 != NULL){
			strcpy(buf_21[t2],token2);
			t2++;
			token2 = strtok(NULL,"\n");
		}

		check = 0;
		
		if(flag==1){//buf_1 is AND
			for(count = 0;count < t1;count++){
				token1 = strtok(buf_12[count],",");
				strcpy(num,token1);
				token1 = strtok(NULL,",");
				strcpy(seq1,token1);
				token1 = strtok(NULL,",");
				strcpy(seq2,token1);
				token1 = strtok(NULL,",");
				strcpy(result,token1);
				printf("%s and %s = %s\n",seq1,seq2,result);
				
				if(strlen(num)==1){
					check = num[0] - '0';
				}
				else if (strlen(num)==2){
					check = (num[0] - '0')*10 + (num[1] - '0');
				}
				strcpy(buf_result[check],result);
			}
			//printf("AND Results are \n%s",buf_1);
			for(count = 0;count < t2;count++){
				token2 = strtok(buf_21[count],",");
				strcpy(num,token2);
				token2 = strtok(NULL,",");
				strcpy(seq1,token2);
				token2 = strtok(NULL,",");
				strcpy(seq2,token2);
				token2 = strtok(NULL,",");
				strcpy(result,token2);
				printf("%s or %s = %s\n",seq1,seq2,result);

				if(strlen(num)==1){
					check = num[0] - '0';
				}
				else if (strlen(num)==2){
					check = (num[0] - '0')*10 + (num[1] - '0');
				}
				strcpy(buf_result[check],result);

			}
			//printf("OR Results are \n%s",buf_2);
			
		}
		else if(flag==2){//buf_2 is AND
			for(count = 0;count < t1;count++){
				token1 = strtok(buf_12[count],",");
				strcpy(num,token1);
				token1 = strtok(NULL,",");
				strcpy(seq1,token1);
				token1 = strtok(NULL,",");
				strcpy(seq2,token1);
				token1 = strtok(NULL,",");
				strcpy(result,token1);
				printf("%s or %s = %s\n",seq1,seq2,result);

				if(strlen(num)==1){
					check = num[0] - '0';
				}
				else if (strlen(num)==2){
					check = (num[0] - '0')*10 + (num[1] - '0');
				}
				strcpy(buf_result[check],result);
			}

			for(count = 0;count < t2;count++){
				token2 = strtok(buf_21[count],",");
				strcpy(num,token2);
				token2 = strtok(NULL,",");
				strcpy(seq1,token2);
				token2 = strtok(NULL,",");
				strcpy(seq2,token2);
				token2 = strtok(NULL,",");
				strcpy(result,token2);
				printf("%s and %s = %s\n",seq1,seq2,result);

				if(strlen(num)==1){
					check = num[0] - '0';
				}
				else if (strlen(num)==2){
					check = (num[0] - '0')*10 + (num[1] - '0');
				}
				strcpy(buf_result[check],result);
			}
			//printf("OR Results are \n%s",buf_1);
			//printf("AND Results are \n%s",buf_2);
		}
		printf("The edge server has successfully finished receiving all computation results from the Backend-Server OR and Backend-Server AND.\n");	
		
	/*	freeaddrinfo(servinfo_and2);
		and_len = sizeof and_addr;
		//UDP receive from and_server
		if ((numbytes_and2 = recvfrom(sockfd_and2, buf_and, BUFFER_SIZE-1 , 0,(struct sockaddr *)&and_addr, &and_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		
		buf_and[numbytes_and2] = '\0';
		printf("Results are \n%s",buf_and);*/

		//printf("test_final:%s\n",buf_result[0]);
		strcpy(buf,buf_result[0]);
		for(count = 1;count < (t1+t2);count++){
			strcat(buf,"\n");
			strcat(buf,buf_result[count]);
		}
		
		if(!fork()){	//child process
		//	close(sockfd);//child socket doesn't need the listener
			if(send(new_fd, buf, BUFFER_SIZE-1, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		printf("The edge server has successfully finished sending all computation results to the client.\n");
		//close(new_fd);//parent dosen't need this

	}

	return 0;
}

					
