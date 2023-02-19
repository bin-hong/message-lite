/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message-lite.h
 */
#ifndef _MESSAGE_LITE_H_
#define _MESSAGE_LITE_H_

//#define NULL (void *)0)
/**
 \struct COMMAND
   command type define;
 */
typedef enum
{
    MSL_REG,   	/**< register user */
    MSL_LOGIN,  	/**<  user login */
    MSL_LOGOUT, /**< user logout */
    MSL_SendMSG, /**< send message */
    MSL_BROADCAST, /**< send message */    
    MSL_LOOKUSERS, /**<  show online users */
}MSL_COMMAND;

/**
 \struct Message
  send messg struct define;
 */
typedef struct Message
{
	MSL_COMMAND commad;
	char myName[20];
	char destName[20];  /*also save passwd when register or login*/
	char msg_buffer[1024];
}MSL_Message;

typedef struct MSL_MessageList
{
	MSL_Message message;
	struct MSL_MessageList *next;
}MSL_MessageList;

#endif //_MESSAGE_LITE_H_

