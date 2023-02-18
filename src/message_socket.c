
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>
#include<unistd.h>   /*for unlink()  close() */
#include <sys/un.h>
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */

#include "message_log.h"
#include "message_daemon.h"

int mlit_unix_socket_open(int *sock, char *sock_path, int type, int mask)
{
    struct sockaddr_un addr;
    int old_mask;

    if ((sock == NULL) || (sock_path == NULL)) {
        msl_log(MSL_LOG_ERROR, "mlit_unix_socket_open: arguments invalid");
        return -1;
    }

    if ((*sock = socket(AF_UNIX, type, 0)) == -1) {
       msl_log(MSL_LOG_ERROR, "unix socket: socket() error");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, sock_path, sizeof(addr.sun_path));

    unlink(sock_path);

    /* set appropriate access permissions */
    old_mask = umask(mask);

    if (bind(*sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        msl_log(MSL_LOG_ERROR, "unix socket: bind() error");
        return -1;
    }

    if (listen(*sock, 1) == -1) {
        msl_log(MSL_LOG_ERROR,  "unix socket: listen error");
        return -1;
    }

    /* restore permissions */
    umask(old_mask);

    return 0;
}

int mlit_unix_socket_close(int sock)
{
    int ret = close(sock);

    if (ret != 0) {
        msl_log(MSL_LOG_ERROR, "unix socket close failed: %s", strerror(errno));
    }

    return ret;
}

int mlit_socket_open(int *sock, unsigned int servPort, char *ip)
{
    int yes = 1;
    int ret_inet_pton = 1;
    int lastErrno = 0;

#ifdef MLIT_USE_IPv6

    /* create socket */
    if ((*sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
        lastErrno = errno;
        msl_log(MSL_LOG_ERROR, "socket_open: socket() error: %s\n",strerror(lastErrno));
        return MSL_RETURN_ERROR;
    }

#else

    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        lastErrno = errno;
        msl_log(MSL_LOG_ERROR, "socket_open: socket() error: %s\n",strerror(lastErrno));
        return MSL_RETURN_ERROR;
    }

#endif

    //printf("%s: Socket created\n", __FUNCTION__);

    /* setsockpt SO_REUSEADDR */
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        lastErrno = errno;
        msl_log(MSL_LOG_ERROR, "socket_open: Setsockopt error: %s\n",strerror(lastErrno));
        return MSL_RETURN_ERROR;
    }
    /* bind */
#ifdef MLIT_USE_IPv6
    struct sockaddr_in6 forced_addr;
    memset(&forced_addr, 0, sizeof(forced_addr));
    forced_addr.sin6_family = AF_INET6;
    forced_addr.sin6_port = htons(servPort);
    if (0 == strcmp(ip, "0.0.0.0"))
        forced_addr.sin6_addr = in6addr_any;
    else
        ret_inet_pton = inet_pton(AF_INET6, ip, &forced_addr.sin6_addr);
#else
    struct sockaddr_in forced_addr;
    memset(&forced_addr, 0, sizeof(forced_addr));
    forced_addr.sin_family = AF_INET;
    forced_addr.sin_port = htons(servPort);
    ret_inet_pton = inet_pton(AF_INET, ip, &forced_addr.sin_addr);
#endif

    /* inet_pton returns 1 on success */
    if (ret_inet_pton != 1) {
        lastErrno = errno;
        msl_log(MSL_LOG_ERROR, " inet_pton() error: %s. Cannot convert IP address: %s\n",
                 strerror(lastErrno),ip);
        return MSL_RETURN_ERROR;
    }

    if (bind(*sock, (struct sockaddr *)&forced_addr, sizeof(forced_addr)) == -1) {
        lastErrno = errno;     /*close() may set errno too */
        close(*sock);
        msl_log(MSL_LOG_ERROR, " bind() error: %s\n", strerror(lastErrno));
        return MSL_RETURN_ERROR;
    }

    /*listen */
    msl_log(MSL_LOG_INFO,"%s: Listening on ip %s and port: %u\n", __FUNCTION__, ip, servPort);


    if (listen(*sock, 3) < 0) {
        lastErrno = errno;
    	msl_log(MSL_LOG_ERROR,"socket_open: listen() failed with error: %s\n",strerror(lastErrno));
        return MSL_RETURN_ERROR;
    }
    return 0; /* OK */
}
int mlit_socket_close(int sock)
{
    close(sock);
    return MSL_RETURN_OK;
}

int mlit_socket_send(int sock, void *data_buffer, int message_size)
{
    int data_sent = 0;

    while (data_sent < message_size) {
        ssize_t ret = send(sock,
                           (uint8_t*)data_buffer + data_sent,
                           (size_t) (message_size - data_sent),
                           0);

        if (ret < 0) {
            msl_log(MSL_LOG_ERROR,"%s: socket send failed [errno: %d]!\n", __func__, errno);
            return 0;
        } else {
            data_sent += (int) ret;
        }
    }
    return 1;
}

