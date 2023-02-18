/*
* this need gtest
*/

#include <stdio.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <syslog.h>

#include <poll.h>
#include <sys/un.h>

#include "message_users.h"
#include "message_event_handle.h"
#include "message_daemon.h"
#include "message_log.h"

extern "C"
{
void init_poll_fd(struct pollfd *pfd);
int msl_prepare_event_handling(MslEventHandler *ev);
void msl_event_handler_add_fd(MslEventHandler *ev, int fd, int mask);
void msl_event_handler_remove_fd(MslEventHandler *ev, int fd);

void msl_add_connection(MslEventHandler *ev,int fd_connect,MSLConnectionType type);
void int msl_remove_connection(MslEventHandler *ev,int fd_connect);
void MslConnection *msl_event_handler_find_connection(MslEventHandler *ev, int fd);
void msl_show_conneciton(MslEventHandler *ev);

}

MslEventHandler pEvent; 

/*init poll_fd*/
TEST(t_msl_prepare_event_handling, normal)
{
    init_poll_fd(pEvent)	
    EXPECT_EQ(0, msl_prepare_event_handling());
}

TEST(t_msl_event_handler_add_fd, normal)
{
    EXPECT_EQ(0, msl_prepare_event_handling());
}