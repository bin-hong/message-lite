/*!
 * \author
 * hongbin <64404983@qq.com>
 *
 * \copyright Copyright@2023/2/17 . \n
 * License free for any modify
 *
 * \file message-users.c
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
#include "message_log.h"


extern void msl_create_msg_node(MSL_MessageList ** new_msg_node);
extern void msl_init_msg_list(MSL_MessageList ** head);
extern void msl_msg_insert(MSL_MessageList * head, MSL_MessageList *new_node);

/*create new user */ 
void createUserNode(MSL_User_List ** user_node)
{
    *user_node = (MSL_User_List *)malloc(sizeof(MSL_User_List));

    if(NULL == *user_node)
    {
        printf("malloc error!\n");
    }
    debug_printf("init mem!\n");
    memset(*user_node,0,sizeof(MSL_User_List));
    debug_printf("init mem ok!\n");
} 

void initUserList(MSL_User_List ** head)
{
	createUserNode(head);
	(*head)->next=NULL;
}

/*insert into head*/
void msl_user_insert(MSL_User_List * head, MSL_User_List *new_node)
{

	new_node->next =head->next;
	head->next =new_node;
}

#if 0
void msl_show_all_users(MSL_User_List* head)
{
	MSL_User_List * curr = head;
	while(NULL!=curr->next)
	{
		curr =curr->next;
		debug_printf("name:%s  fd:%s online:%d",curr->name,curr->fd,curr->online);
	}
}
#endif
void msl_send_online_users(MSL_User_List* head, int fd)
{
	
       char send_buf[1050] ={'\0'};
	MSL_User_List * curr = head;
	while(NULL!=curr->next)
	{
		curr =curr->next;
		if(curr->isOnline){
	   		(void)snprintf(send_buf, sizeof(send_buf),"%s",curr->name);
			send(fd,send_buf,strlen(send_buf),0);
		}
	}
}

/*
*@return  -1: paramter err
*               0: user exist
*		   1: inser OK
*/
int msl_user_regsiter(MSL_User_List **users, MSL_Message *msg,int fd)
{
	int ret_status =-1;
	if(NULL==users ||NULL== msg ||fd <=0){
		return ret_status;
	}

	ret_status =msl_check_user_status(users,msg,fd);
	if(USER_STATUS_NO_REGISTER==ret_status){
			
		MSL_User_List *new_user ;
		createUserNode(&new_user);

		/*set online session fd*/
		//new_user->fd =fd;
		/*set name*/ 
		strncpy(new_user->name,msg->myName,sizeof(new_user->name)-1);
		/*set passwd*/
		strncpy(new_user->passwd,msg->destName,sizeof(new_user->passwd)-1);

		/*insert into list*/
		msl_user_insert(*users,new_user);
		ret_status = 1;
		
	}else{
		ret_status = 0;
	}
	return ret_status;
}

/*
*
* @return   -2: passwd error 
*		   -1: failed    
*                 0: no exist no regesiter
*		     1: log sucess 
*                 2: had logoned
*/
int msl_user_login(MSL_User_List **users, MSL_Message *msg,int fd)
{
	int ret_status =USER_STATUS_ERROR;
	if(NULL==users ||NULL== msg ||fd <=0){
		return ret_status;
	}
	MSL_User_List *head = *users;
       MSL_User_List *curr = head->next;

	while(curr!=NULL ){
		 if(strcmp(curr->name,msg->myName)==0)/*check name */
		 {
		 	//ret_status =USER_STATUS_NO_LOGONED;
			if(strcmp(curr->passwd,msg->destName)==0){
			 	
				if(curr->fd==fd){
					ret_status =USER_STATUS_HAD_LOGONED; 
				}else{
					ret_status =fd;      /*login sucess*/
					curr->fd =fd;         /*save fd*/
					curr->isOnline =1; /*set online*/
				}

			}else{
				ret_status=USER_STATUS_PWD_ERR;
			}
			
			break;
		 }
		 curr = curr->next;
	}
	if(curr==NULL){
		ret_status =USER_STATUS_NO_REGISTER;
	}
	return ret_status;
}


/*
*
* @return   -2: passwd error 
*		   -1: failed    
*                 0: no exist no regesiter
*		     1: log sucess 
*                 2: had logoned
*/
int msl_user_logout(MSL_User_List **users, MSL_Message *msg,int fd)
{
	int ret_status =USER_STATUS_ERROR;
	if(NULL==users ||NULL== msg ||fd <=0){
		return ret_status;
	}
	
	MSL_User_List *head = *users;
       MSL_User_List *curr = head->next;

	while(curr!=NULL ){
		 if(strcmp(curr->name,msg->myName)==0)/*check name */
		 {
		 	//ret_status =USER_STATUS_NO_LOGONED;
			if(strcmp(curr->passwd,msg->destName)==0){
			 	
				if(curr->fd==fd){
					ret_status =USER_STATUS_SUCESS; 
					curr->fd =-1;         /*set fd disable*/
					curr->isOnline =0;  /*set unline*/
				}else{
					ret_status =USER_STATUS_NO_LOGONED;
				}

			}else{
				ret_status=USER_STATUS_PWD_ERR;
			}
			
			break;
		 }
		 curr = curr->next;
	}
	if(curr==NULL){
		ret_status =USER_STATUS_NO_REGISTER;
	}
	return ret_status;
}

int msl_user_logout_by_session(MSL_User_List **users,int fd)
{
	int ret_status =USER_STATUS_ERROR;
	if(NULL==users ||fd <=0){
		return ret_status;
	}
	
	MSL_User_List *head = *users;
       MSL_User_List *curr = head->next;

	while(curr!=NULL ){ 	
		if(curr->fd==fd){
			ret_status =USER_STATUS_SUCESS; 
			curr->fd =-1;         /*set fd disable*/
			curr->isOnline =0;  /*set unline*/
		}
		curr = curr->next;
	}
	return 1;
}


/*show userlist connections*/
void msl_show_user_list(MSL_User_List **users)
{
    MSL_User_List *head = *users;
    MSL_User_List *curr = head->next;
   
    while (curr) {
	  debug_printf("user name[%s], fd[%d] alive[%d]\n",curr->name, curr->fd,curr->isOnline);
         curr = curr->next;
    }
}


/*
* @ brief  check sender  status.
* @ param1  [IN] users info .
* @ param2  [IN] sender msg info.
* @ param3  [IN] sender user fd.
* @return  MSL_USER_STATUS
*/
int msl_check_user_status(MSL_User_List **users, MSL_Message *msg,int fd)
{
	int ret_status =USER_STATUS_ERROR;
	if(NULL==users ||NULL== msg ||fd <=0){
		return ret_status;
	}

  	MSL_User_List *head = *users;
       MSL_User_List *curr = head->next;
	while(curr!=NULL ){
		 if(strcmp(curr->name,msg->myName)==0)
		 {
		 	ret_status =USER_STATUS_HAD_REGISTER;	
			if(curr->fd==fd ){
				ret_status =USER_STATUS_HAD_LOGONED;   /*has logoned*/
			}else{
				ret_status =USER_STATUS_NO_LOGONED;   /*havn't logoned*/
			}
			break;
		 }
		 curr = curr->next;
	}
	if(curr==NULL){
		ret_status =USER_STATUS_NO_REGISTER;
	}	
	return ret_status;
}

/*
* @ brief  check dest user status.
* @ param1  [IN] global users info .
* @ param2  [OUT] dest user info.
* @return  MSL_USER_STATUS
*/
int msl_check_dest_status(MSL_User_List **users, MSL_Message * msg,MSL_User_List ** dest_user)
{
	int ret_status =USER_STATUS_ERROR;
	if(NULL==users ||NULL== dest_user ){
		return ret_status;
	}
	MSL_User_List *head =*users;
       MSL_User_List *curr = head->next;
	while(curr!=NULL ){
		 if(strcmp(curr->name,msg->destName)==0)
		 {
		 	ret_status =USER_STATUS_HAD_REGISTER;	
			if(curr->fd >1 &&curr->isOnline){
				ret_status =USER_STATUS_HAD_LOGONED;   /*has logoned*/
			}else{
				ret_status =USER_STATUS_NO_LOGONED;   /*havn't logoned*/
			}
			*dest_user =curr; /*found dest user node*/
			break;
		 }
		 curr = curr->next;
	}
	if(curr==NULL){
		ret_status =USER_STATUS_NO_REGISTER;
	}	
	return ret_status;
}


/*
* @ brief  send mesg to dest user
* @ param1  [IN] users info .
* @ param2  [in] msg info.
* @return  0: sucess
*/
int msl_send_dest_message(MSL_User_List *dest ,MSL_Message * msg)
{
	debug_printf("%s IN",__func__);
	int ret_status =-1;
	if(NULL==dest ||NULL== msg ){
		return ret_status;
	}
	MSL_User_List *dest_user =dest;
	
       char send_buf[1050] ={'\0'};

	if(1 ==dest_user->isOnline)
	{
		ret_status=1;
		debug_printf("send msg to [%s]",dest_user->name);
	   	(void)snprintf(send_buf, sizeof(send_buf),"from %s msg:%s", msg->myName,msg->msg_buffer);
		send(dest_user->fd,send_buf,strlen(send_buf),0);
	}
	else if(0 ==dest_user->isOnline)
	 {   
		/*save msg to dest user node*/
		if(NULL==dest_user->unSendMessages){
			 msl_init_msg_list(&(dest_user->unSendMessages));
		}
		/*new*/
		MSL_MessageList *new_msg =NULL;
		msl_create_msg_node(&new_msg);
		
		/*deep copy*/
		memcpy(&(new_msg->message),msg,sizeof(MSL_Message));	
		new_msg->next=NULL;
		
		/*get the head*/
		MSL_MessageList* head= dest_user->unSendMessages;
		
		/*insert the head*/
		msl_msg_insert(head,new_msg);
		
		ret_status =0;
		debug_printf("save to user buffer");
	}
	else{
		debug_printf("unkown status");
		//nothing
	}
	debug_printf("%s out[%d]",__func__,ret_status);  
	return ret_status;
}

/*
* @ brief  send mesg to all user
* @ param1  [IN] users info .
* @ param2  [in] msg info.
* @return  0: sucess
*/
int msl_broadcast_message(MSL_User_List **users ,MSL_Message * msg)
{
	debug_printf("%s IN",__func__);
	int ret_status =-1;
	if(NULL==users ||NULL== msg){
		return ret_status;
	}
	
       MSL_User_List *head = *users;
       MSL_User_List *dest = head->next;
   
       while (dest) {
	     if(0==strcmp(dest->name,msg->myName)){
		/* don't send to myself*/
	      }else{	 	
	         msl_send_dest_message(dest, msg);
	     }
            dest = dest->next;	
       }
	debug_printf("%s out[%d]",__func__,ret_status);     
	return ret_status;
}

/*
* @ brief  send mesg to dest user
* @ param1  [IN] users info .
* @ param2  [in] msg info.
* @return  0: sucess
*/
int msl_send_message(MSL_User_List **users ,MSL_Message * msg)
{
	debug_printf("%s IN",__func__);
	int ret_status =-1;
	if(NULL==users ||NULL== msg ){
		return ret_status;
	}
	//MSL_User_List *myuser =*users;
	MSL_User_List *dest_user =NULL;
	
	ret_status =msl_check_dest_status(users,msg ,&dest_user);
	if( USER_STATUS_HAD_LOGONED ==ret_status ||
	    USER_STATUS_NO_LOGONED ==ret_status)
	{
		msl_send_dest_message(dest_user,msg);
	}
	debug_printf("%s out[%d]",__func__,ret_status);   

	return ret_status;
}

/*show userlist connections*/
int msl_show_user_unsend(MSL_User_List **users,int fd)
{
	debug_printf("%s IN",__func__);
	int ret_status =-1;
	if(NULL==users ||fd < 1 ){
		return ret_status;
	}

       MSL_User_List *head = *users;
       MSL_User_List *dest = head->next;
   	MSL_MessageList *unsend_msgs =NULL;

       while (dest) {
	     if(fd ==dest->fd ){
		 /* found myself*/
		 unsend_msgs=dest->unSendMessages;
		 break;
	     }
            dest = dest->next;	
       }
	 /*check head*/
	if(NULL==unsend_msgs){
		return  -1;
	}
	unsend_msgs =unsend_msgs->next;
	/*check unsend msg*/
	while(NULL!=unsend_msgs){
		debug_printf("unsend msg to [%s]",unsend_msgs->message.destName);
		debug_printf("unsend msg buf[%s]:",unsend_msgs->message.msg_buffer);
		unsend_msgs =unsend_msgs->next;
	}

	debug_printf("%s out[%d]",__func__,ret_status);     
	return 0;
}

/*
* @ brief  send meg fd who login by 
* @ param1  [IN] users info .
* @ param2  [in] fd  for login session.
* @return  0: sucess
*/
int msl_send_save_message(MSL_User_List **users ,int fd)
{
	debug_printf("%s IN",__func__);
	int ret_status =-1;
	if(NULL==users ||fd <1){
		return ret_status;
	}
       char send_buf[1050] ={'\0'};
       MSL_User_List *head = *users;
       MSL_User_List *dest = head->next;
	MSL_MessageList *head_unsend_msgs =NULL;   
   	MSL_MessageList *unsend_msgs =NULL;
	MSL_MessageList *tmp_msgs =NULL;
       while (dest) {
	     if(fd ==dest->fd){
		 /* found myself*/
		 head_unsend_msgs=dest->unSendMessages;
		 break;
	     }
            dest = dest->next;	
       }
	/*check head*/
	if(NULL==head_unsend_msgs){
		return  -1;
	}
	unsend_msgs =head_unsend_msgs->next;
	while(NULL!=unsend_msgs){

		debug_printf("send save msg to [%s]",unsend_msgs->message.destName);
			
	   	(void)snprintf(send_buf, sizeof(send_buf),"from %s msg:%s",
	   		 unsend_msgs->message.myName ,unsend_msgs->message.msg_buffer);
		send(fd,send_buf,strlen(send_buf),0);

		/*save next*/
		tmp_msgs = unsend_msgs->next;
		/*remove this msg*/
		free(unsend_msgs);
		/*next*/
		unsend_msgs =tmp_msgs;
	}
	head_unsend_msgs->next=NULL;
	debug_printf("%s out[%d]",__func__,ret_status);     
	return ret_status;
}
