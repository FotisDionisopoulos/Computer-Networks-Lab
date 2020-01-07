#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAXSIZE 1024

ssize_t writen(int fd, const void *vptr, size_t n)
 {
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;
    ptr = vptr;
	nleft = n;
	while (nleft > 0) {

		if  ((nwritten = write(fd, ptr, nleft)) <= 0 ){ 
			if (errno == EINTR) 
				nwritten = 0;
          	else 
          		return -1; /* error */ 
		} 
		nleft -= nwritten;
        ptr += nwritten;
	} 
	return n;
}


int main(int argc, char *argv[])
{
    struct sockaddr_in server_info;
    struct hostent *he;
    struct pollfd ufds;
    int socket_fd,num;
    char buffer[1024];

    char buff[1024], code;
    int port, count, rv;

    char buf1[256], buf2[256];
    port = atoi(argv[2]);

    if (argc < 2) {
        fprintf(stderr, "Usage: client hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1]))==NULL) {
        fprintf(stderr, "Cannot get host name\n");
        exit(1);
    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket Failure!!\n");
        exit(1);
    }

    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(port);
    server_info.sin_addr = *((struct in_addr *)he->h_addr);

    if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))<0) {
        perror("");
        exit(1);
    }

    ufds.fd = socket_fd;
    ufds.events = POLLIN;


	for( count = 3; count < argc; count++ ){
        	
		if  ((strcmp(argv[count],"get")==0)){
			code = 103;
			
			
			if ((writen(socket_fd,&code, 1))== -1) {
            			fprintf(stderr, "Failure Sending Message\n");
            			close(socket_fd);
            			exit(1);
       			 }
			
			if (argv[count+1]==NULL) break;
			if ((writen(socket_fd,argv[count+1], strlen(argv[count+1])+1))== -1) {
            			fprintf(stderr, "Failure Sending Message\n");
            			close(socket_fd);
            			exit(1);
       			 }
			
				
			    	
				

				
					
				    // check for events on s1:
				    
					num = recv(socket_fd, buffer, 1,0);
					if(num == -1){
						fprintf(stderr, "Failure Sending Message\n");
            			close(socket_fd);
						}
					if (buffer[0] == 'n'){
							printf("\n");
							count +=1;
							continue;
							}
					else if (buffer[0] == 'f')
					while((num = read(socket_fd, &buffer, 1))>0 && buffer[0]!='\0'){
							if(num == -1)
							{
								fprintf(stderr, "Failure Sending Message\n");
								close(socket_fd);
							}
								printf("%c", buffer[0]);
							}
					printf("\n");

				
			count +=1;
			continue;
		}
		if  ((strcmp(argv[count],"put")==0)){
			code = 112;
			
			
			
			if ((send(socket_fd,&code, 1,0))== -1) {
            			fprintf(stderr, "Failure Sending Message\n");
            			close(socket_fd);
            			exit(1);
       			 }
			
			if (argv[count+1]==NULL) break;
			if ((send(socket_fd,argv[count+1], strlen(argv[count+1])+1,0))== -1) {
            			fprintf(stderr, "Failure Sending Message\n");
            			close(socket_fd);
            			exit(1);
       			 }

			
			if (argv[count+2]==NULL) break;
			if ((send(socket_fd,argv[count+2], strlen(argv[count+2])+1,0))== -1) {
            			fprintf(stderr, "Failure Sending Message\n");
            			close(socket_fd);
            			exit(1);
       			 }
			count +=2;
			continue;
		}
	}
	shutdown(socket_fd, SHUT_WR);




    close(socket_fd);   

}

