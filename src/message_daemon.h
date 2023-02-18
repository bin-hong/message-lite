#ifndef _MESSAGE_DAEMON_H_
#define _MESSAGE_DAEMON_H_

#include <semaphore.h>
#include "message_users.h"
#include "message_event_handle.h"

#if 0
/*
 * The parameter of registered users list
 */
typedef struct
{
    MslClientUser *user_list; /**< Pointer to MslClientUsers */
    int num_client; /**< Number of client int user_list*/
    MslClientUser *oline_list; /**< Pointer to oline list */
    int num_oline; /**< Num of oline in oline list */
} MslDaemonRegisteredUsers;
#endif

/**
 * The parameters of a daemon.
 */
typedef struct
{
	char *ip;
	char *port;
	
} MSLDaemon;



typedef enum
{
    MSL_RETURN_WRONG_PARAMETER = -2,
    MSL_RETURN_ERROR = -1,
    MSL_RETURN_OK = 0,
    MSL_RETURN_TRUE = 1
}MSLReturnValue;


/**
 * The global parameters of a message server .
 */
typedef struct MSLDaemonLocal
{
    MSL_User_List *user_list; /**< registered users */
    MslEventHandler pEvent; /**< event */

    int port;                    /*tcp port 3355(defaut)*/
    int timeoutOnSend;
    //int fd_connect;
    int client_connections;    /**< counter for nr. of client connections */

}MSLDaemonLocal;

/*  for thread ,this unuse*/
/*extern*/ sem_t msl_daemon_mutex;
// need sem_init(&msl_daemon_mutex, 0, 1)
#define MSL_DAEMON_SEM_LOCK() do{\
    while ((sem_wait(&msl_daemon_mutex) == -1) && (errno == EINTR)) \
        continue;       /* Restart if interrupted */ \
    } while(0)

#define MSL_DAEMON_SEM_FREE() { sem_post(&msl_daemon_mutex); }

#endif//_MESSAGE_DAEMON_H_