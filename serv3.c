#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/sem.h>
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
	for( i = 0; i<1000 ; i++)
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
	int posi ;


	for( i = 0; i<data->pos ; i++)
	{

		if (strcmp(data->strNameList[i][0], buffer)==0)
		{
			flag = 2;
			break;
		}
	}


	if (flag == 0) {
		posi = data->pos++;
		memcpy(data->strNameList[posi][0], buffer, 50);
		memcpy(data->strNameList[posi][1], buffer2, 50);
	}

	if (flag == 2)
		memcpy(data->strNameList[i][1], buffer2, 50);



}




int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct sockaddr_in dest;
	struct sembuf up   = {0,  1, 0};
	struct sembuf down = {0, -1, 0};
	int status,socket_fd, client_fd, num, GET_MODE, PUT_MODE, c, my_sem;
	int yes =1, sizes = 10, sizes2 = 10, i, shmid, port, NB_PROC;
	socklen_t size;
	char  n = 110, f = 102, *buff, *ret_buff, p;
	char * buffer = malloc(10*sizeof(char  ));
	char * buffer2 = malloc(10*sizeof(char  ));
	char ret_buff_ar[50];
	pid_t pid,wpid;

	port = atoi(argv[1]);
	NB_PROC = atoi(argv[2]);

	my_sem=  semget(IPC_PRIVATE, 1, 0600); /* create semaphore */
	semop(my_sem, &up, 1);
	
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

	if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    {
		fprintf(stderr, "Binding Failure\n");
		exit(1);
	}

	if ((listen(socket_fd, BACKLOG)== -1)){
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

	size = sizeof(struct sockaddr_in);  
	for ( i=0; i<NB_PROC; i++){ /* Create NB_PROC children */ 
	pid = fork();

	if (pid==0) {


		GET_MODE = 0;
		PUT_MODE = 0;
		while(1){
			if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1){
				perror("accept");
				continue;
			}

			while(1) {
				if ((num = recv(client_fd, buffer, 1,0))== -1) {
					perror("recv");
					exit(1);
				}   
				else if (num == 0) {
					break;
				}

				buffer[num] = '\0';
				if ((buffer[0] != 'p') && (buffer[0] != 'g') ) break;

				if (buffer[0]== 'g')
				GET_MODE = 1;

				if ((GET_MODE == 1 )){

					c = 0 ;
					while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
						buffer = realloc(buffer,++sizes *sizeof(char));	
						buffer[c++] = p;
					}
					buffer[c] = '\0';
					if ((buffer == NULL)|| (num <= 0)) break;
					

					semop(my_sem, &down, 1); 
					ret_buff = search(buffer);
					if (ret_buff != NULL ) strcpy(ret_buff_ar, ret_buff);
					semop(my_sem, &up, 1);


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
						}int  fo =getpid();

						if (writen(client_fd, ret_buff_ar, strlen(ret_buff_ar)+1)== -1){
							fprintf(stderr, "Failure Sending Message\n");
							break;
						}
					}
				}

				if (buffer[0] == 'p'){

					PUT_MODE = 1;
					c = 0 ;
					while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
						buffer = realloc(buffer,++sizes *sizeof(char));	
						buffer[c++] = p;
					}
					buffer[c] = '\0';
					if ((buffer == NULL)|| (num <= 0)) break;
				}

				if (PUT_MODE == 1 ){
					PUT_MODE =2;

					c = 0 ;
					while((num = recv(client_fd, &p, 1,0))>0 && p!='\0'){
						buffer2 = realloc(buffer2,++sizes2 *sizeof(char));
						buffer2[c++] = p;
					}
					buffer2[c] = '\0';
					if ((buffer == NULL)|| (num <= 0)) break;
				}

				if (PUT_MODE == 2 ){
					PUT_MODE =0;	
					semop(my_sem, &down, 1); 				
					add_var(buffer, buffer2);
					semop(my_sem, &up, 1);

				}

			}//END inner while
			close(client_fd);  

		}//END acc while 
		exit(0);
		}//END if
	}//END FOR
	do // Parent
	{    
		wpid = waitpid(pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status)); 
	close(socket_fd);   

	return 0;


}	
