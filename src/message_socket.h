
#ifndef _MESSAGE_SOCKET_H
#define _MESSAGE_SOCKET_H

int mlit_unix_socket_open(int *sock, char *sock_path, int type, int mask);
int mlit_unix_socket_close(int sock);
int mlit_socket_open(int *sock, unsigned int servPort, char *ip);
int mlit_socket_close(int sock);
int mlit_socket_send(int sock, void *data_buffer, int message_size);

#endif /*_MESSAGE_SOCKET_H*/
