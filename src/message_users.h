

#ifndef _MESSAGE_USER_H_
#define _MESSAGE_USER_H_

#include "message_lite.h"

typedef enum{

	USER_STATUS_NO_REGISTER =-5,
	USER_STATUS_HAD_REGISTER =-4,
	USER_STATUS_NO_LOGONED =-3,
	USER_STATUS_HAD_LOGONED =-2,	
	USER_STATUS_PWD_ERR =-1,
	USER_STATUS_ERROR=0,
	USER_STATUS_SUCESS =1,
	
}MSL_USER_STATUS;

typedef struct User_elem
{
	char name [25];
	char passwd[25];
	char isOnline;
	int fd;
	struct Message *unSendMessage;
}MSL_USER;

/**
 \struct User_List
  register User list;
 */
typedef struct User_List
{
	char name [25];
	char passwd[25];
	char isOnline;
	int fd;
	struct User_List *next;
	MSL_MessageList *unSendMessages;
	unsigned int messag_count;
} MSL_User_List;


void initUserList(MSL_User_List ** head);
int msl_user_login(MSL_User_List ** users, MSL_Message *message,int fd);
int msl_user_regsiter(MSL_User_List ** users, MSL_Message *message,int fd);
int msl_user_logout(MSL_User_List **users, MSL_Message *msg,int fd);
int msl_check_user_status(MSL_User_List **users, MSL_Message *msg,int fd);
int msl_check_dest_status(MSL_User_List **users, MSL_Message *msg,MSL_User_List**dest_user);
int msl_send_message(MSL_User_List **users ,MSL_Message * msg);
int msl_broadcast_message(MSL_User_List **users ,MSL_Message * msg);
void msl_show_user_list(MSL_User_List **users);
int msl_send_save_message(MSL_User_List **users ,int fd);
int msl_show_user_unsend(MSL_User_List **users,int fd);
#endif //_MESSAGE_USER_H_