
/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message-daemon.c
 */
 #include<stdlib.h>
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

#include "message_lite.h"

#include "message_socket.h"
#include "message_log.h"
#include "message_daemon.h"
#include "message_event_handle.h"

#define _MSL_PACKAGE_VERSION ("0.0.1")

extern int msl_handle_event(MSLDaemonLocal *DaemonLocal);
void ms_get_version(char *buf, size_t size)
{
    if ((buf == NULL) && (size <= 0)) {
        msl_log(MSL_LOG_ERROR, "Wrong parameter: Null pointer\n");
        return;
    }

/* Could change it read from config */

    snprintf(buf,
             size,
             "MessageLite Package Version: %s @%s %s\n",
             _MSL_PACKAGE_VERSION,
             __DATE__,
             __TIME__);
}

#define DAEMON_TEXTBUFSIZE 500
/**
 * Print usage information of tool.
 */
void usage()
{
    char version[DAEMON_TEXTBUFSIZE]={0};
    ms_get_version(version, DAEMON_TEXTBUFSIZE);

    printf("%s", version);
    printf("Usage: message-daemon [options]\n");
    printf("Options:\n");
    printf("  -i  address         ip address\n");
    printf("  -h          Usage\n");
    printf("  -p port    port to monitor for incoming requests (Default: 3491)\n");
} /* usage() */

#if 0

/**
 * Option handling
 */
int option_handling(MSLDaemonLocal *daemon_local, int argc, char *argv[])
{
     return 0;

}  /* option_handling() */
#endif

void *dealCommandfromConsole(void *pVar)
{
	char input[256];
	while(1){
		
		if(scanf("%s", input)!=0)
		{
			if(strncmp("users",input,sizeof(input))==0){
				//show register users
 				msl_log(MSL_LOG_INFO, "user list:\n");
			}
			else if(strncmp("alive",input,sizeof(input))==0)
			{
				//show alive users
				msl_log(MSL_LOG_INFO, "alive user list:\n");
			}else{
				//show commands
			}
		}
	}
}
void msl_local_cleanup()
{
	
}

int msl_daemon_local_connection_init(MSLDaemonLocal *daemon_local)
{
    int fd = -1;

    /* create and open pipe to receive incoming connections t */
    //TODO:
    
    daemon_local->port =3355;  /*colud read from config file(default:3355) */
    /* create and open socket to receive incoming connections from client */	
    if (mlit_socket_open(&fd, daemon_local->port, "0.0.0.0") == MSL_RETURN_OK) {
	  
	  msl_add_connection(&(daemon_local->pEvent),fd,MSL_COM_CONNECT_TCP);
    }
    else {
        msl_log(MSL_LOG_INFO,"Could not initialize main socket.\n");
        return MSL_RETURN_ERROR;
    }
    return MSL_RETURN_OK;
}

int main(int argc, char **argv)
{
	int back =0;
	int iRtn = 0;

	MSLDaemonLocal daemon_local;
#if 0
    /* Command line option handling */
    if ((back = option_handling(&daemon_local, argc, argv)) < 0) {
        if (back != -2)
            fprintf (stderr, "option_handling() failed!\n");

        return -1;
    }
	
    /* Configuration file option handling */
    if ((back = option_file_parser(&daemon_local)) < 0) {
        if (back != -2)
            fprintf (stderr, "option_file_parser() failed!\n");

        return -1;
    }
#endif

	/*init user list*/  
	/*it can also read and save user by sqlit3 or other db */
	/*now it managed in memory, no save it */
	initUserList(&(daemon_local.user_list));

	/*[server] init message command 's deal*/
	msl_prepare_event_handling(&(daemon_local.pEvent));
	
	/*create thread for handler console*/
	pthread_t pid;
	iRtn = pthread_create(&pid, NULL, dealCommandfromConsole, NULL);
	if(0!=iRtn){
		msl_log(MSL_LOG_INFO,"Thread crate error ");
	}
	//server prepare connection
	msl_daemon_local_connection_init(&daemon_local);

	/* in_sock  is fd  for commuicate for client and server*/
	/* can use muti-thread each in_sock or thread pools watch the in_sock send and write*/
	/* this use  pollfd watch the in_sock for send and write*/

	//pthread_t pid_communicate;
	//iRtn = pthread_create(&pid_communicate, NULL, msl_handle_event, &daemon_local);
	
       msl_log(MSL_LOG_INFO,"Messagelite Daemon launched... ");
	/* Even connect loop. */
	while(back >=0)
	{
		back =msl_handle_event(&daemon_local);
	}
	
	/* Even handling loop. */
	 msl_log(MSL_LOG_INFO,"Exiting Messagelite daemon... ");

	msl_local_cleanup();
	return 0;
}

