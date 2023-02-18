

/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message-msgs.c
 */

#include<stdlib.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <errno.h>
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "message_lite.h"
#include "message_log.h"

/*create new user */ 
void msl_create_msg_node(MSL_MessageList ** new_msg_node)
{
    *new_msg_node = (MSL_MessageList *)malloc(sizeof(MSL_MessageList));

    if(NULL == *new_msg_node)
    {
        printf("malloc error!\n");
    }
    memset(*new_msg_node,0,sizeof(MSL_MessageList));
} 

void msl_init_msg_list(MSL_MessageList ** head)
{
	msl_create_msg_node(head);
	(*head)->next=NULL;
}

/*insert into head*/
void msl_msg_insert(MSL_MessageList * head, MSL_MessageList *new_node)
{
	new_node->next =head->next;
	head->next =new_node;
}
