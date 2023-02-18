
/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/18 . \n
 * License free for any modify
 *
 * \file message-cliet.c
 */
 
#include <stdio.h>      /* for printf() and fprintf() */
#include <errno.h>
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>  /** for system()*/
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <netdb.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <pthread.h> /** for pthread_create()*/
#include "message_client.h"
#include "message_lite.h"
#include "message_log.h"

void usage()
{
} /* usage() */

char G_Input_Buff[MESSAGE_MAX_LEN+NAME_MAX_LEN+2]={0};

typedef struct
{
    int sock;              /**< sock Connection handle/socket */
    char *servIP;       /**< servIP IP adress/Hostname of interface */
    int port;              /**< Port for TCP connections (optional) */
} MlClient;
MlClient ml_client;

int mc_client_connect(MlClient *client)
{

    int connect_errno = 0;
    char portnumbuffer[33];
    struct addrinfo hints, *servinfo, *p;

    int rv;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    if (client == 0){
        return -1;
    }
	//
    snprintf(portnumbuffer, 32, "%d", client->port);

    if ((rv = getaddrinfo(client->servIP, portnumbuffer, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((client->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
             fprintf(stderr,"socket() failed! %s\n", strerror(errno));
            continue;
        }

        if (connect(client->sock, p->ai_addr, p->ai_addrlen) < 0) {
            connect_errno = errno;
            close(client->sock);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
         fprintf(stderr, "ERROR: failed to connect! %s\n", strerror(connect_errno));
        return -1;
    }

    printf("Connected to ml daemon (%s)\n", client->servIP);

	return 1;
}

pthread_t pid;
//-1 unregister or unlogin
//0 registered

void menu()
{
    printf("**Connect to server sucesses. Wecome to Message Lite!****\n\n");
    printf("**command follows\n\n");
    printf(" login   \n\n");
    printf(" regitser \n\n");
    printf(" byte: quit \n\n");
    printf(" @user message (user:send to user name ,message :contents)\n\n");
    printf("   arg1: user [must 5-19 alpha or digit number] \n\n");
    printf("   arg2: message [the messag can be multiple line with '\' at the line end] \n\n");
    printf("  [meesage does not exceed 1023 characters]\n\n");
    printf(" @all message (broadcast)    \n\n");	
    printf(" byte: quit                  \n\n");
    printf("**********************************\n");
}
 

/*
*-1: no login status.
* 0:  regster status
* 1:  login status
*/
int my_status =-1;
char my_name[20];
char * g_str_login="login sucessful!";
void *mc_recv_msg_func(void *arg)
{
	MlClient* client =(MlClient*)arg;
	int len;
	char buf[1050]={0};
	while(1)
	{
		memset(buf,0,sizeof(buf));
		len =read (client->sock,buf,sizeof(buf));
		if(len<=0)
		{
			close(client->sock);
			printf("Disconnected to ml daemon\n");
			return NULL;
		}
		buf[len]='\0';
		printf("%s\n",buf);
		 if(0==strncmp(buf,g_str_login,strlen(g_str_login)))
		 {	printf("--online!\n");
		 	my_status=1;
		 }
		if(1 ==my_status){
			printf("%s|%s>",ml_client.servIP,my_name);
		}
		
	}
	return NULL;

}


typedef enum
{
    MC_INPUT_INVALID =-1, 
    MC_REGISTER,  /**< register*/
    MC_LOGIN, 	 /**< login*/
    MC_LOGOUT,	/**< logout*/
    MC_CONTINUE_INPUT, 	 /**< continue input*/
    MC_SEND,
}ParseResult;

/*
*
*/
ParseResult parseInput(char* input)
{
	/*check */
	ParseResult result =MC_INPUT_INVALID;
	if(NULL==input){
		return result;
	}
	unsigned int len_input = strlen(input);
	if(len_input >MESSAGE_MAX_LEN+NAME_MAX_LEN+2)
	{
		/*input too much*/
		return result;
	}
	else if(0==strcmp(input,"login"))
	{
		result = MC_LOGIN;
	}
	else if(0==strcmp(input,"register"))
	{
		result =MC_REGISTER;
	}
	else if(0==strcmp(input,"bye"))
	{
		result =MC_LOGOUT;
	}
	else if(input[0]=='@')
	{
		//continue check 
		//if(input[len_input]!='\\'){
		//	MC_CONTINUE_INPUT;
		//}
		result =MC_SEND;
	}else {
	}
	return result;
}

ParseResult parseInput2(char* input ,MSL_Message *msg)
{
	if(NULL==input || NULL==msg ){
		return MC_INPUT_INVALID;
	}
	debug_printf("input=%s\n",input);
	unsigned int len=strlen(input);
	unsigned int index=1;
	if(len <=1){
		return MC_INPUT_INVALID;	
	}
	debug_printf("len=%d\n",len);
	while(index<len &&input[index]!='\n')
	{
		if(input[index++] == ' '){
			break;
		}
	}
	debug_printf("index=%d,len=%d\n",index,len);
	if(index>20 ||index<1 ||len<index){
		return MC_INPUT_INVALID;
	}else{
		strncpy(msg->destName,input+1,index-2);
		strncpy(msg->msg_buffer,input+index,len -index);
		debug_printf("name=%s,buf=%s\n",msg->destName,msg->msg_buffer);
		return MC_SEND;
	}
}
void *mc_send_msg_func(void *arg)
{
    char input[1044]={0};

    MSL_Message message;
    ParseResult  input_result=MC_INPUT_INVALID;
    int sockfd = *((int *)arg);
    int ret=0; 
 
    while(1)
    { 
        //ret =system("clear");
        memset(input,0,sizeof(input));     
        ret =scanf("%[^\n]",input);
        getchar();

	 input_result = parseInput(input);	

	 
	 while(MC_CONTINUE_INPUT==input_result)
	 {
	 
	 }
	 
        switch (input_result)
        {
        
        //login
        case MC_LOGIN:
	     if(1!=my_status){
		     memset(&message ,0,sizeof(message));	
	            printf("Please Iput your name(must 2-19 alpha or digit number):\n");
	            ret =scanf("%s",message.myName);
	            getchar();

	            printf("Please Input you passwd(must 2-19 alpha or digit number):\n");
	            ret =scanf("%s",message.destName);
	            getchar();
	            message.commad =MSL_LOGIN;
	            printf("Logining.........\n");
	            sleep(1);
	            send(sockfd,&message,sizeof(message),0);
		     strncpy(my_name,message.myName,sizeof(my_name));
	            sleep(1);
		     my_status=1;		
	     }else{
	            printf("You havn't logoned!");
		     printf("%s>",ml_client.servIP);		
	     }
           // ret =system("clear");
            break;
        //register
        case MC_REGISTER:    
	     memset(&message ,0,sizeof(message));		
            printf("Please Input you passwd(must 5-19 alpha or digit number):\n");
            ret = scanf("%s",message.myName);
            getchar();
            printf("Please Input you passwd(must 5-19 alpha or digit number):\n");
            ret = scanf("%s",message.destName);
            getchar();
	     message.commad =MSL_REG;

            printf("registering,wait.....\n");
            sleep(1);
            send(sockfd,&message,sizeof(message),0);
            sleep(1);
	     printf("%s>",ml_client.servIP);			
            //ret =system("clear");
            break;
 
        //send message
        case MC_SEND:
	     memset(&message ,0,sizeof(message));	
            message.commad =MSL_SendMSG;	
	     if(1==my_status){		
		    input_result =parseInput2(input,&message);
		     if(MC_SEND ==input_result){
		 	   if(strcmp(message.destName,"all")==0)
		 	   {
		 	   	message.commad =MSL_BROADCAST;	
		 	   }
    			   strncpy(message.myName,my_name,sizeof(my_name));
		          send(sockfd,&message,sizeof(message),0);

		     }else{
		     	  printf("input format was not valid!\n");
		         printf("%s|%s>",ml_client.servIP,my_name);
		     }
	     }else{
		    printf("Please login !");	
		    printf("%s>",ml_client.servIP);
	     }
	        break;          
        case MC_LOGOUT:
	     memset(&message ,0,sizeof(message));	
	     message.commad =MC_LOGOUT;
	     strncpy(message.myName,my_name,sizeof(my_name));	 
	     send(sockfd,&message,sizeof(message),0);
		 
 	     my_status =-1;
            //ret =system("clear"); 
            sleep(1);
            exit(-1);
            break;
        default:
            printf("\n");
            break;
        
        }
	if (ret<0){
		   printf("command err\n");
	}
    }
}
 


int userRegister( MlClient *client)
{
	return 0;
}

int usrLogin(MlClient *client)
{
	return 0;
}

int main(int argc, char **argv)
{
       int iRtn = 0;
	pthread_t tid_send;
	pthread_t tid_recv;
	//char input[256];
	//
	//MlClient ml_client;
	ml_client.servIP="127.0.0.1";
	ml_client.port=3355;
	ml_client.sock =-1;
	iRtn =mc_client_connect(&ml_client);
	int sockfd =-1;
	if(iRtn!=1 ||ml_client.sock<0){
		printf("Could not connect to server! please contact admin\n");
		return 0;
	}else{
	       sockfd =ml_client.sock ;
		menu();
		printf("You must register&login before!\n");
	       printf("%s>",ml_client.servIP);
	}
	
	iRtn  = pthread_create(&tid_recv,NULL,(void *)mc_recv_msg_func ,(void *)&sockfd);
	iRtn  = pthread_create(&tid_send,NULL,(void *)mc_send_msg_func,(void *)&sockfd);

	pthread_join(tid_recv,NULL);
	pthread_join(tid_send,NULL);

       close(sockfd);
	return 0;
#if 0
	while(1){
		iRtn = system("clear");
		if(iRtn==-1){
			//do nothing
		}
		if(-1==login_status){
			printf("\t l login\n");
			printf("\t r register\n");
			printf("\t q quit\n");
		}
		
		
		if(scanf("%s", input)!=0)
		{
			if(strncmp("l",input,1)==0){
				usrLogin(&ml_client);
				login_status =1;
			}
			else if(strncmp("r",input,1)==0)
			{
				userRegister(&ml_client);
				login_status =0;
			}else if(strncmp("q",input,1)==0){
				//show commands
				login_status =-2;
			}
			else if(strncmp("@",input,1)==0){
				//show commands
				printf("send message!");
			}
			else if(strncmp("@",input,1)==0){
				printf("invalid message or command!");
			}
		}
		if(-2==login_status){
			exit(0);
		}
	}
#endif
}

