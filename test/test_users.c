

/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/19 . \n
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


#include "message_lite.h"
#include "message_users.h"
#include "message_log.h"

void test_msl_user_regsiter(MSL_User_List ** user_list)
{
	int ret;
	/*normal*/
	MSL_Message msg[10] ;
	memset(msg,0 ,sizeof(msg));
	int fd[10];
	msg[0].commad = MSL_REG;

	strcpy(msg[0].myName ,"user1");
	strcpy(msg[0].destName ,"pwd1");
	fd[0]=1;
	
	ret = msl_user_regsiter(user_list, &msg[0], fd[0]);
	if(1 ==ret)
	{
	    debug_printf("check register  usr1  ---[OK]!");
	}else{
	      debug_printf("check register usr1 ---[NG]!");
	}

	msg[1].commad = MSL_REG;
	strcpy(msg[1].myName ,"user2");
	strcpy(msg[1].destName ,"pwd2");
	fd[1]=2;
	
	ret = msl_user_regsiter(user_list, &msg[1], fd[1]);
	if(1 ==ret)
	{
	      debug_printf("check register  usr2 ---[OK]!");
	}else{
	      debug_printf("check register usr2 ---[NG]!");
	}
	
	msg[2].commad = MSL_REG;
	strcpy(msg[2].myName ,"user3");
	strcpy(msg[2].destName ,"pwd3");
	fd[2]=3;
	ret = msl_user_regsiter(user_list, &msg[2], fd[2]);
	if(1 ==ret)
	{
	    debug_printf("check register usr3 ---[OK]!");
	}else{
	      debug_printf("check register usr3 ---[NG]!");
	}
	
	/*param err*/
	ret = msl_user_regsiter(user_list, NULL, fd[1]);
	if(-1 == ret)
	{
	    debug_printf("check param err ---[ OK]!");
	}else{
		debug_printf("check param err ---[ NG]!");
	}
	
	/* check exist */
	ret = msl_user_regsiter(user_list, &msg[2], fd[2]);
	if(0 ==ret)
	{
	    debug_printf("check register exsit ---[OK]!");
	}else{
	    debug_printf("check register exsit ---[NG]!");
	}

	
}
void test_msl_user_login(MSL_User_List ** user_list)
{
	//TODO:
}
int main()
{
	/*prepare list*/
	MSL_User_List *g_user_list=NULL;	
	initUserList(&g_user_list);

	/*test user Regsiter*/
	test_msl_user_regsiter(&g_user_list);

	/*test user login*/
	test_msl_user_login(&g_user_list);
	
	return 0;
}
