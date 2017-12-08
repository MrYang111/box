#ifndef BOX_H
#define BOX_H


#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
 #include <sys/epoll.h>
#include "box_util.h"

static inline int __box_start_server(unsigned short port, const char* ip, int backlog)
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int ret = bind(server, (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0)
    {
        box_log("bind error\n");
        exit(1);
    }

    listen(server, backlog);
    return server;
}

static inline void box_set_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

typedef struct box
{
    int epollfd;
} box;

typedef struct box_channel
{
    int sock;
    void (*cb)(struct box_channel* c);

    void* session;
} box_channel;

/* 什么时候算对方说完了 */
typedef struct box_session
{
    char buf[8192];
    int pkt_len; // 报文有多长
    int read_len; // 已经读的数据有多长
    void(*cb)(struct box_session*);
    box_channel* c ;
} box_session;

box_session* box_session_create(box_channel* c, int pkt_len, void(*cb)(box_session*));
void box_session_destroy(box_session*);

void box_init();
void box_fini();
void box_run();
void box_add(box_channel* c);
void box_add_socket(int sock, void(*cb)(box_channel*c));

box_channel* box_channel_create(int fd, void(*cb)(box_channel*c));
void box_channel_destroy(box_channel* c);

void box_start_server(unsigned short port,
                      const char* ip,
                      int backlog,
                      void(*cb)(box_channel*));

#endif // BOX_H
