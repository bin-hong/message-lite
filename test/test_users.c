

/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * @brief test message-users.c
 */

#include<stdlib.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <errno.h>
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */

#include "message_users.h"
#include "message_lite.h"

MSL_User_List *g_user_list=NULL;
void test_msl_userRegsiter()
{
	MSL_Message msg;
	msg.commad=MSL_REG;
	/*set online session fd*/
	new_user->fd =100;
	/*set name*/ 
	strncpy(msg.myName,"user1",strlen(msg.myName)-1);
	/*set passwd*/
	strncpy(msg.destName,"pwd1",strlen(msg.destName)-1);
	
}
int main()
{
	/*prepare list*/
	initUserList(&g_user_list);

	/**/
	
}
