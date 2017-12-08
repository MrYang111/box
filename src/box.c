#include "box.h"
#include "box_util.h"

static box* b = NULL;

// 初始化所有全局的属性
// 1. 创建了epollfd
void box_init()
{
    if(b == NULL)
        b = box_malloc(sizeof(box));

    b->epollfd = epoll_create(1024);
}

// 处理socket：只是调用回调函数
void box_handle_socket(struct epoll_event* ev)
{
    box_channel* c = (box_channel*)ev->data.ptr;
    c->cb(c);
}

void box_run()
{
    struct epoll_event ev[8];
    while(1)
    {
        // epoll_wait
        int ret = epoll_wait(b->epollfd, ev, 8, 5000);
        if(ret == 0) continue;
        if(ret < 0 && errno == EINTR) continue;
        if(ret < 0) break;
        int i;
        for(i=0; i<ret; ++i)
        {
            // 处理socket
            box_handle_socket(&ev[i]);
        }
    }
}

void box_fini()
{
    close(b->epollfd);
}

void box_add(box_channel* ch)
{
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = ch;
    epoll_ctl(b->epollfd, EPOLL_CTL_ADD, ch->sock, &ev);
}

box_channel* box_channel_create(int fd, void(*cb)(box_channel*c))
{
    box_channel* c = box_malloc(sizeof(box_channel));
    c->sock = fd;
    c->cb = cb;
    return c;
}

void box_channel_destroy(box_channel *c)
{
    close(c->sock);
    box_free(c);
}

// 启动一个tcp服务器
// 当有客户端连接服务器时，回调cb函数
void box_start_server(
                      unsigned short port,
                      const char *ip,
                      int backlog,
                      void(*cb)(box_channel*))
{
    // add server --> box
    int server = __box_start_server(port, ip, backlog);
    box_set_nonblock(server);
    box_channel* c = box_channel_create(server, cb);

    // 为了实现这个功能，抽象了box_channel概念
    // box_channel保存了socket和回调函数
    box_add(c);
}

void box_add_socket(int sock, void (*cb)(box_channel *))
{
    box_set_nonblock(sock);
    box_channel* c = box_channel_create(sock, cb);
    box_add(c);
}

// 接收数据，判断session的数据包是不是已经接收完整
// 如果接收完整，那么回调session的回调函数
void session_read(struct box_channel* c)
{
    box_session*s = (box_session*)c->session;
    int ret = read(c->sock, s->buf + s->read_len, s->pkt_len - s->read_len);
    if(ret > 0)
    {
        s->read_len += ret;
        if(s->read_len == s->pkt_len)
        {
            s->read_len = 0; // 准备下一次接收数据
            s->buf[s->pkt_len] = 0; // 加\0
            s->cb(s);// 告诉上层，报文已经接收好了
        }
    }
    else // <= 0
    {
        // socket有问题了
        box_session_destroy(s);
    }
}

box_session* box_session_create(box_channel* c, int pkt_len,
                                void(*cb)(box_session*))
{
    box_session* s = box_malloc(sizeof(*s));
    s->pkt_len = pkt_len;
    s->read_len = 0;
    s->cb = cb;
    s->c = c;
    c->cb = session_read;
    s->c->session = s;

    return s;
}

void box_session_destroy(box_session* s)
{
    box_channel_destroy(s->c);
    box_free(s);
}
