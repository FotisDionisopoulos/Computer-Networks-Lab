#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5

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


struct db
{
	char strNameList[1000][2][50] ;
	int pos;
};

struct db *data;


char * search(char *buffer1)
{
	
	int i;
	for( i = 0; i<=999 ; i++)
	{

		if (data->strNameList[i][0] != NULL)
		{
			if (strcmp(data->strNameList[i][0], buffer1)==0)
			{
				return data->strNameList[i][1];
				
			}
		}
	}
	return NULL;


}

void add_var(char *buffer, char *buffer2)
{

	int i;
	int flag = 0;
	int posi = 0;


	for( i = 0; i<data->pos ; i++)
	{

		if (strcmp(data->strNameList[i][0], buffer)==0){
			flag = 2;
			break;}
		




	}


	if (flag == 0) {
		posi = data->pos++;
		memcpy(data->strNameList[posi][0], buffer, 50);
		memcpy(data->strNameList[posi][1], buffer2, 50);
		
	}
	if (flag == 2) {

		memcpy(data->strNameList[i][1], buffer2, 50);
	}



}




int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct sockaddr_in dest;
	int status,socket_fd, client_fd, num, GET_MODE, PUT_MODE;
	socklen_t size;

	char  n = 110, f = 102;
	char *buff;
	char *ret_buff;

	pid_t pid,wpid;

	int yes =1, sizes = 10,sizes2 = 10 ;
	char p; int c ;
	int port, shmid;
	char * buffer = malloc(10*sizeof(char  ));
	char * buffer2 = malloc(10*sizeof(char  ));
	
	
	port = atoi(argv[1]);

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
		fprintf(stderr, "Socket failure!!\n");
		exit(1);
	}

	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	memset(&server, 0, sizeof(server));
	memset(&dest,0,sizeof(dest));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY; 
	if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    { //sizeof(struct sockaddr) 
		fprintf(stderr, "Binding Failure\n");
		exit(1);
	}

	if ((listen(socket_fd, 5)== -1)){
		fprintf(stderr, "Listening Failure\n");
		exit(1);
	}

	if ((shmid = shmget(IPC_PRIVATE, sizeof(struct db), IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		exit(1);
	}	

	if ((data = shmat(shmid, NULL, 0)) == (struct db*) -1) {
		perror("shmat");
		exit(1);
	}
	data->pos = 0;
	while(1) {
		GET_MODE = 0; PUT_MODE = 0;
		
		size = sizeof(struct sockaddr_in);  
		
		
		if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1) {
			//fprintf(stderr,"Accept Failure\n");
			perror("accept");
			exit(1);
		}
		
		pid = fork();
		
		if (pid < 0) {
			perror("ERROR on fork");
			exit(1);
		}
		if (pid == 0) {
			/* This is the client process */
			close(socket_fd);
			
			
			
			while(1) {
				if ((num = recv(client_fd, buffer, 1,0))== -1) {
					
					perror("recv");
					exit(1);
				}   
				else if (num == 0) {
					//printf("Connection closed\n");
					//return 0;
					break;
				}
				
				if ((buffer[0] != 'p') && (buffer[0] != 'g') ) break;
				
				
				
				buffer[num] = '\0';
				//printf("Message received: %s\n", buffer);
				if (buffer[0]== 'g'){
					GET_MODE = 1;
					
				}
				if ((GET_MODE == 1 )){
					
					c = 0 ;
					
					
					while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
						buffer = realloc(buffer,++sizes *sizeof(char));					if(num == -1){
						fprintf(stderr, "Failure Sending Message\n");
            					close(client_fd);}
						buffer[c++] = p;
					}
					buffer[c] = '\0';
					
					
					ret_buff = search(buffer);
					
					
					
					GET_MODE = 0;
					if (ret_buff == NULL){
						if (writen(client_fd, &n, 1)== -1){
							fprintf(stderr, "Failure Sending Message\n");
							close(client_fd);
						}
					}
					else {
						if (writen(client_fd, &f, 1)== -1){
							fprintf(stderr, "Failure Sending Message\n");
							close(client_fd);
						}
						if (writen(client_fd, ret_buff, strlen(ret_buff)+1)== -1){
							fprintf(stderr, "Failure Sending Message\n");
							close(client_fd);
							
						}
					}	
				}
				
				if (buffer[0] == 'p'){
					
					GET_MODE = 0;
					PUT_MODE = 1;
					c = 0 ;
					while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
						buffer = realloc(buffer,++sizes *sizeof(char));					if(num == -1){
						fprintf(stderr, "Failure Sending Message\n");
            					close(client_fd);}
						buffer[c++] = p;
					}buffer[c] = '\0';
				}
				if (PUT_MODE == 1 ){
					PUT_MODE =2;
					
					
					c = 0 ;
					while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
						buffer2 = realloc(buffer2,++sizes2 *sizeof(char));					if(num == -1){
						fprintf(stderr, "Failure Sending Message\n");
            					close(client_fd);}
						buffer2[c++] = p;
					}buffer2[c] = '\0';
				}
				if (PUT_MODE == 2 ){
					PUT_MODE =0;
					
					add_var(buffer, buffer2);
					
				}

				
				

				
				
				
			}//END while 
			exit(0);
		}//END if
		else {//parent
			
		}

		close(client_fd); 
	}//END while

	
	
	


	//close(client_fd);   
	close(socket_fd);   
	
	return 0;

	
}
