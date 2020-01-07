#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5

void* thread_fun(void *arg);
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

pthread_mutex_t lock;

struct db
{
	char strNameList[1000][2][50] ;
	int pos;
};

struct db data;

char * search(char *buffer1)
{
	
	int i;
	for( i = 0; i<=999 ; i++)
	{

		if (data.strNameList[i][0] != NULL)
		{
			if (strcmp(data.strNameList[i][0], buffer1)==0)
			{
				return data.strNameList[i][1];
				
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


	for( i = 0; i<data.pos ; i++)
	{

		if (strcmp(data.strNameList[i][0], buffer)==0){
			flag = 2;
			break;}
		




	}


	if (flag == 0) {
		posi = data.pos++;
		memcpy(data.strNameList[posi][0], buffer, 50);
		memcpy(data.strNameList[posi][1], buffer2, 50);
		
	}
	if (flag == 2) {

		memcpy(data.strNameList[i][1], buffer2, 50);
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
	data.pos = 0;
	int yes =1, sizes = 10,sizes2 = 10 ;
	char p; int c ;
	int port, NB_PROC;
	int i = 0;
	int err;

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}

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

	if ((listen(socket_fd, 25)== -1)){
		fprintf(stderr, "Listening Failure\n");
		exit(1);
	}


	while(1) {
		
		
		size = sizeof(struct sockaddr_in);  
		
		
		if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1) {
			perror("accept");
			continue;
		}

		pthread_t t;
		int *cl_fd = malloc(sizeof(int));
		*cl_fd = client_fd;
		err = pthread_create(&t, NULL, thread_fun, cl_fd);
		if (err != 0)
		printf("\ncan't create thread :[%s]", strerror(err));
		
	}//END while

	
	

	
	return 0;

	
}



void * thread_fun(void *arg)
{
	int status, num, GET_MODE = 0, PUT_MODE = 0;
	socklen_t size;

	char  n = 110, f = 102;
	char *buff;
	char *ret_buff, ret_buff_ar[50];

	int yes =1, sizes = 10,sizes2 = 10 ;
	char p; 
	int c ;
	int port, NB_PROC;
	int i = 0;
	int err;	
	int client_fd = *(int *)arg;printf("%d",client_fd);
	char * buffer = malloc(10*sizeof(char  ));
	char * buffer2 = malloc(10*sizeof(char  ));
	
	
	
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
				buffer = realloc(buffer,++sizes *sizeof(char));	
				buffer[c++] = p;
			}
			buffer[c] = '\0';
			if(num<=0) break;
			
			
			pthread_mutex_lock(&lock);
			ret_buff = search(buffer);
			if (ret_buff != NULL)
				strcpy(ret_buff_ar, ret_buff);
			pthread_mutex_unlock(&lock);
			

			
			GET_MODE = 0;
			if (ret_buff == NULL){
				if (writen(client_fd, &n, 1)== -1){
					fprintf(stderr, "Failure Sending Message\n");
					break;
				}
			}
			else {
				if (writen(client_fd, &f, 1)== -1){
					fprintf(stderr, "Failure Sending Message\n");
					break;
				}
				if (writen(client_fd, ret_buff_ar, strlen(ret_buff_ar)+1)== -1){
					fprintf(stderr, "Failure Sending Message\n");
					break;
					
				}
			}
		}
		
		if (buffer[0] == 'p'){
			
			GET_MODE = 0;
			PUT_MODE = 1;
			c = 0 ;
			while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
				buffer = realloc(buffer,++sizes *sizeof(char));	
				buffer[c++] = p;
			}
			buffer[c] = '\0';
		if(num<=0) break;

		}

		if (PUT_MODE == 1 ){
			PUT_MODE =2;
			
			
			c = 0 ;
			while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
				buffer2 = realloc(buffer2,++sizes2 *sizeof(char));
				buffer2[c++] = p;
			}
			buffer2[c] = '\0';
		if(num<=0) break;

		}

		if (PUT_MODE == 2 ){
			PUT_MODE =0;
			pthread_mutex_lock(&lock);
			add_var(buffer, buffer2);
			pthread_mutex_unlock(&lock);
		}
		
	}//END while
	close(client_fd); 
	return NULL;
}
