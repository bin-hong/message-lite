/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message_event_handle.c
 */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <errno.h>
#include<stdlib.h>
#include <poll.h>
#include <sys/un.h>

#include "message_users.h"
#include "message_event_handle.h"
#include "message_daemon.h"
#include "message_log.h"

#define MSL_EV_TIMEOUT_MSEC 1000
#define MSL_EV_BASE_FD      16

#define MSL_EV_MASK_REJECTED (POLLERR | POLLNVAL)

/*declear*/
void msl_recv_message_handler(MSLDaemonLocal *daemon_local,int fd);
int msl_handle_event(MSLDaemonLocal *DaemonLocal);
int msl_event_process_client_connect(MSLDaemonLocal *daemon_local ,int fd);

/** @brief Initialize a pollfd structure
 *
 * That ensures that no event will be mis-watched.
 *
 * @param pfd The element to initialize
 */
void init_poll_fd(struct pollfd *pfd)
{
    pfd->fd = -1;
    pfd->events = 0;
    pfd->revents = 0;
}

/** @brief Prepare the event handler
 *
 * This will create the base poll file descriptor list.
 *
 * @param ev The event handler to prepare.
 *
 * @return 0 on success, -1 otherwise.
 */
int msl_prepare_event_handling(MslEventHandler *ev)
{
    int i = 0;
	
    if (ev == NULL)
        return MSL_RETURN_ERROR;

    ev->pfd = calloc(MSL_EV_BASE_FD, sizeof(struct pollfd));

    if (ev->pfd == NULL) {
        fprintf (stderr,"new poll instance failed!\n");
        return -1;
    }

    for (i = 0; i < MSL_EV_BASE_FD; i++)
        init_poll_fd(&ev->pfd[i]);

    ev->nfds = 0;
    ev->max_nfds = MSL_EV_BASE_FD;
    ev->connections=NULL;
    ev->fd_connect=-1;
    return 0;
}

  /*step2 support muti connect type(tcp ,pipe)*/

void msl_add_connection(MslEventHandler *ev,int fd_connect,MSLConnectionType type)
{

    if(NULL ==ev){
	  return;
    }
   msl_show_conneciton(ev);
    debug_printf("add connection..fd=[%d] was conect is tcp[%d]\n",fd_connect,type);
#if 	1
    /*create connect*/
    MslConnection * new_connect = (MslConnection *)malloc(sizeof(MslConnection));
    memset(new_connect, 0, sizeof(MslConnection));
    new_connect->fd =fd_connect;
    new_connect->type = type;
    new_connect->next=NULL;
	
    MslConnection **temp = &(ev->connections);
    while (*temp != NULL)
    {
        temp = &(*temp)->next;
    }
    *temp = new_connect;

   /*show*/
   msl_show_conneciton(ev);
    ev->fd_connect =fd_connect;
#else
    ev->fd_connect =fd_connect;
#endif

    msl_event_handler_add_fd(ev, fd_connect,POLLIN);
	
}

int msl_remove_connection(MslEventHandler *ev,int fd_connect)
{
    if ((ev == NULL)  )
        return MSL_RETURN_ERROR;

    MslConnection *curr = ev->connections;
    MslConnection *prev = curr;
    /* Find the address where to_remove value is registered */
    while (curr && (curr->fd != fd_connect)) {
         prev = curr;
         curr = curr->next;
    }

    if (!curr) {
        /* Must not be possible as we check for existence before */
        msl_log(MSL_LOG_ERROR,"Connection not found for removal.\n");
        return -1;
    }
    else if (curr == ev->connections)
    {
        ev->connections = curr->next;
    }
    else {
        prev->next = curr->next;
    }

    /* Now we can release */
    if(NULL!=curr){
    	free(curr);
    	close(fd_connect);
    }
    ev->fd_connect=-1;
    msl_event_handler_remove_fd(ev, fd_connect);
    return 0;
}

MslConnection *msl_event_handler_find_connection(MslEventHandler *ev, int fd)
{
    MslConnection *connect = ev->connections;

    while (connect != NULL) {
        if (connect->fd == fd)
            return connect;
        connect = connect->next;
    }
    return connect;
}

/*this funcion for test list connections*/
void msl_show_conneciton(MslEventHandler *ev)
{
    MslConnection *curr = ev->connections;
    MslConnection *prev = curr;

    while (curr) {
         prev = curr;
         curr = curr->next;
	  debug_printf("connection[%d], type[%d]\n",prev->fd, prev->type);
    }
}


/** @brief Enable a file descriptor to be watched
 *
 * Adds a file descriptor to the descriptor list. If the list is to small,
 * increase its size.
 *
 * @param ev The event handler structure, containing the list
 * @param fd The file descriptor to add
 * @param mask The mask of event to be watched
 */
 void msl_event_handler_add_fd(MslEventHandler *ev, int fd, int mask)
{
    if (ev->max_nfds <= ev->nfds) {
        int i = (int) ev->nfds;
        int max = (int) (2 * ev->max_nfds);
        struct pollfd *tmp = realloc(ev->pfd, (size_t) (max) * sizeof(*ev->pfd));

	if (!tmp) {
		fprintf (stderr, "Unable to register new fd for the event handler.\n");
		return;
	}
	
	ev->pfd = tmp;
	ev->max_nfds = max;
	
        for (; i < max; i++)
            init_poll_fd(&ev->pfd[i]);
    }

    ev->pfd[ev->nfds].fd = fd;
    ev->pfd[ev->nfds].events = mask;
    ev->nfds++;
}

/** @brief Disable a file descriptor for watching
 *
 * The file descriptor is removed from the descriptor list, the list is
 * compressed during the process.
 *
 * @param ev The event handler structure containing the list
 * @param fd The file descriptor to be removed
 */
void msl_event_handler_remove_fd(MslEventHandler *ev, int fd)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int nfds = (unsigned int) ev->nfds;

    close(fd);
    for (; i < nfds; i++, j++) {
        if (ev->pfd[i].fd == fd) {
            init_poll_fd(&ev->pfd[i]);
            j++;
            ev->nfds--;
        }

        if (i == j)
            continue;

        /* Compressing the table */
        if (i < ev->nfds) {
            ev->pfd[i].fd = ev->pfd[j].fd;
            ev->pfd[i].events = ev->pfd[j].events;
            ev->pfd[i].revents = ev->pfd[j].revents;
        }
        else {
            init_poll_fd(&ev->pfd[i]);
        }
    }
}


/** @brief Catch and process incoming events.
 *
 * This function waits for events on all connections. Once an event raise,
 *
 * @param daemon_local : containing needed information.
 *
 * @return 0 on success,
 */
int msl_handle_event(MSLDaemonLocal *daemon_local)
{
    int ret = 0;
    unsigned int i = 0;
    
    if  (daemon_local == NULL)
        return MSL_RETURN_ERROR;

    MSLConnectionType type= MSL_COM_NONE;
    MslEventHandler *pEvent = &(daemon_local->pEvent);

    //MSL_EV_TIMEOUT_MSEC	
    ret = poll(pEvent->pfd, pEvent->nfds, -1);
    debug_printf("a new event");
    if (ret <= 0) {
        /* We are not interested in EINTR has it comes
         * either from timeout or signal.
         */
        if (errno == EINTR)
            ret = 0;

        if (ret < 0)
            msl_log(MSL_LOG_FATAL, "poll() failed: %s\n", strerror(errno));

        return ret;
    }
   debug_printf(" pEvent nfds=[%d] \n",(int)pEvent->nfds);
    for (i = 0; i < pEvent->nfds; i++) {
        int fd = 0;

        if (pEvent->pfd[i].revents == 0)
            continue;

	 type =MSL_COM_NONE;
	 /* check this fd is connect fd*/
	#if 1
	 MslConnection *con = msl_event_handler_find_connection(pEvent, pEvent->pfd[i].fd);
        
        if (con) { 
            type = con->type;
        }
	#else
		if(pEvent->pfd[i].fd==pEvent->fd_connect){
			type =MSL_COM_CONNECT_TCP;
		}
	#endif 
	 fd = pEvent->pfd[i].fd;

	 /* First of all handle error events */
        if (pEvent->pfd[i].revents & MSL_EV_MASK_REJECTED) {
	    debug_printf("fd=[%d] was disconect is tcp[%d]",fd,type);
            /* An error occurred, we need to clean-up the concerned event
             */
            if (type == MSL_COM_CONNECT_TCP){
		   msl_remove_connection(pEvent,fd);
		   //mlit_socket_close(fd);
            }	   
            else
	     {
                msl_event_handler_remove_fd(pEvent, fd);
		  daemon_local->client_connections--;
		  
		  /*disable fd that in userlist*/
		
            	}
	     	 
            continue;
        }

	/*hand the event*/
	if(MSL_COM_NONE ==type){
		/* handler receive meage that from client*/
		debug_printf("A message was coming. from fd=[%d]",fd);
		msl_recv_message_handler(daemon_local,fd);
	}else if (MSL_COM_CONNECT_TCP ==type ||MSL_COM_CONNECT_PIPE ==type){
	       /* connect event.  then new clent fd and watch */
		debug_printf("fd=[%d] is connect ",fd);  
		msl_event_process_client_connect(daemon_local,fd);
	 	
	}
	
    }

    return 0;
}

#if 0
//for thead pool deal func
void* msl_handle_event_loop(void *arg)
{
	MSLDaemonLocal *daemon_local =(MSLDaemonLocal *)arg;
	MslEventHandler *pEvent = daemon_local->pEvent;
}
#endif


int msl_event_process_client_connect(MSLDaemonLocal *daemon_local ,int fd)
{
    socklen_t cli_size;
    struct sockaddr_un cli;

   debug_printf(" IN [%s]\n",__func__);
    int in_sock = -1;
    char local_str[500] = { '\0' };

    if (daemon_local == NULL)  {
        msl_log(MSL_LOG_ERROR,"Invalid function parameters for %s\n",__func__);
        return -1;
    }

    /* event from TCP server socket, new connection */
    cli_size = sizeof(cli);

    if ((in_sock = accept(fd, (struct sockaddr *)&cli, &cli_size)) < 0) {
        msl_log(MSL_LOG_ERROR, "accept() for socket %d failed: %s\n", fd, strerror(errno));
        return -1;
    }

    /* Set socket timeout in reception */
    struct timeval timeout_send;
    timeout_send.tv_sec = daemon_local->timeoutOnSend;
    timeout_send.tv_usec = 0;

    if (setsockopt (in_sock,SOL_SOCKET,SO_SNDTIMEO,
                    (char *)&timeout_send, sizeof(timeout_send)) < 0)
         msl_log(MSL_LOG_ERROR,  "setsockopt failed\n");

     msl_event_handler_add_fd(&(daemon_local->pEvent),in_sock,POLLIN);
     daemon_local->client_connections++;

	/* print connection info about connected */
    (void)snprintf(local_str, 500,
             "New client connection #%d was created, Total Clients : %d",
             in_sock, daemon_local->client_connections);
	
    msl_log(MSL_LOG_INFO, "%s%s", local_str, "\n");

    /*find the user list and send unsendmessage*/
    //TODO

    return 0;
}

void show_message(MSL_Message* message)
{
	if(NULL ==message){
		debug_printf("message was null!");
	}
	if(message->commad==MSL_REG){
		debug_printf("msg_cmd: Register!");
	}else if(message->commad==MSL_LOGOUT){
		debug_printf("msg_cmd: logout!");
	}else if(message->commad==MSL_LOGIN){
		debug_printf("msg_cmd: login!");
	}else if(message->commad==MSL_SendMSG){
		debug_printf("msg_cmd: message!");
	}else if(message->commad==MSL_BROADCAST){
		debug_printf("msg_cmd: broadcast!");
	}
	
	
	debug_printf("msg_cmd:%d",message->commad);
	debug_printf("msg_name:%s",message->myName);
	debug_printf("destName:%s",message->destName);
	debug_printf("msg_buffer:%s",message->msg_buffer);
}

/*
*@brief  receive msg and reply
*@param fd :from client session
*/
void msl_recv_message_handler(MSLDaemonLocal *daemon_local,int fd)
{
    int ret;

    MSL_User_List **users = &(daemon_local->user_list);
    MSL_Message RecvInfo;
    //MSL_Message ReplyInfo;
    ret = recv(fd,&RecvInfo,sizeof(RecvInfo),0);

    /* direct reply the message!for test*/
#if 1
    show_message(&RecvInfo);
    char arr[128] ={'\0'};
    (void)snprintf(arr, 128,
             "reply[%s]:Server received you message\n",
             RecvInfo.myName);
    send(fd,arr,strlen(arr),0); 
#endif

    int cmd_status =-5;
    if(ret == 0)
    {
	msl_log(MSL_LOG_ERROR, "fd[%d] was disconnected",fd);
	msl_event_handler_remove_fd(&(daemon_local->pEvent), fd);
       daemon_local->client_connections--;

	return; 
    }  
    else
    {  
        /*register*/
        if(MSL_REG ==RecvInfo.commad)
        {	
        	cmd_status = msl_user_regsiter(users,&RecvInfo,fd);
	       if(msl_user_regsiter(users,&RecvInfo,fd)>=1)
		{
         	  char arr[128] ={"From server: Register ok\n"};
                send(fd,arr,strlen(arr),0);
		}
        }

        /*login*/
        else if(MSL_LOGIN ==RecvInfo.commad)
        {
            cmd_status =msl_check_user_status(users,&RecvInfo,fd);
            if(USER_STATUS_NO_REGISTER==cmd_status )
            {
         	  char arr[128] ={"From server: Please Register\n"};
                send(fd,arr,strlen(arr),0);
            }else if(USER_STATUS_NO_LOGONED ==cmd_status){
		/*
		* 1. login
		* 2. send unsend message to this login user
		*/
	        cmd_status = msl_user_login(users,&RecvInfo,fd);
		 if(cmd_status >1 ){
			/*reply successful*/
	         	  char arr[128] ={"login sucessful!"};
	                send(fd,arr,strlen(arr),0);
		 }
		 /*debuf for print msg*/
		 msl_show_user_unsend(users,fd);
		 /*send unsend msg*/
	        msl_send_save_message(users,fd);
            }
        }
	 else if(MSL_LOGOUT ==RecvInfo.commad){
	 	
        } 
        /* check online user*/
        else if(MSL_LOOKUSERS ==RecvInfo.commad)
        {
        
            if(USER_STATUS_NO_LOGONED == msl_check_user_status(users,&RecvInfo,fd))
            {   
                char arr[128] ={"From server: Please Login\n"};
                send(fd,arr,strlen(arr),0);
            }
            else
            {
                //send usr list
            }
        } 

        /*send message*/
	 else if(MSL_SendMSG==RecvInfo.commad ||MSL_BROADCAST ==RecvInfo.commad)
        {    
        	cmd_status =msl_check_user_status(users,&RecvInfo,fd);
		debug_printf("check user status=%d",cmd_status);	
        	if(USER_STATUS_HAD_LOGONED ==cmd_status ){
	        	if(MSL_BROADCAST ==RecvInfo.commad){
				msl_broadcast_message(users,&RecvInfo);
			}else{
	              	msl_send_message(users,&RecvInfo);  
			}
        	}else if(USER_STATUS_NO_LOGONED ==cmd_status )
        	{
                     char arr[128] ={"From server: Please Login\n"};
                     send(fd,arr,strlen(arr),0);
        	}
        	else if(USER_STATUS_NO_REGISTER==cmd_status )
             {
			char arr[128] ={"From server: Please Register\n"};
			send(fd,arr,strlen(arr),0);
             }else{
			//todo
	      }
        }
	 /*debug for show list */
	 msl_show_user_list(users);
    }
    return;
}
