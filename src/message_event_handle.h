/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message_event_handle.h
 */


#ifndef _MESSAGE_EVENT_HANDLER_H
#define _MESSAGE_EVENT_HANDLER_H

#include <poll.h>
#include "message_lite.h"
//#include "message_daemon.h"

typedef enum {
    MSL_COM_NONE = 0,
    MSL_COM_CONNECT_TCP,  /** server connect sock*/
    MSL_COM_CONNECT_PIPE,
    MSL_COM_TYPE_MAX
} MSLConnectionType;


typedef struct MslConnection {
    MSLConnectionType type;
    int fd;                   			/**< connect handle */
    struct MslConnection *next;   /**< For server multiple connection type support */
} MslConnection;

typedef struct MslEventHandler{
    struct pollfd *pfd;			/**<pollfd for clients sesseion fd*/
    nfds_t nfds;
    nfds_t max_nfds;
    MslConnection *connections; /**<server listen connect fds*/
    int fd_connect;
} MslEventHandler;

int msl_prepare_event_handling(MslEventHandler *ev);

void msl_add_connection(MslEventHandler *ev,int fd_connect,MSLConnectionType type);
int msl_remove_connection(MslEventHandler *ev,int fd_connect);

void msl_event_handler_add_fd(MslEventHandler *ev, int fd, int mask);
void msl_event_handler_remove_fd(MslEventHandler *ev, int fd);

void msl_show_conneciton(MslEventHandler *ev);
#endif /*_MESSAGE_EVENT_HANDLER_H*/
