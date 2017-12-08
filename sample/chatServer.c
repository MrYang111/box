
#include <box/box.h>

// channel <--> session
void handle_packet(char* pkt, box_session* s)
{

}

void session_header(box_session* s);
void session_packet(box_session* s)
{
    box_log("recv data is %s\n", s->buf);

    s->pkt_len = 4; // 下一次接收的数据应该len
    s->cb = session_header;
}

void session_header(box_session* s)
{
    int len = atoi(s->buf);
    // 重新配置session的参数
    s->pkt_len = len; // 下一次接收的数据应该len
    s->cb = session_packet;
}

// 有客户端连接时，调用的回调函数
void server_callback(box_channel* c)
{
    while(1)
    {
        int fd = accept(c->sock, NULL, NULL);
        if(fd < 0)
            break;
   //     box_add_socket(fd, read_callback);

        box_channel* c = box_channel_create(fd, NULL);
        box_session_create(c, 4, session_header);
        box_add(c);
    }
}

int main()
{
    box_init();

    box_start_server(9999, "0.0.0.0", 250, server_callback);

    box_run();
    box_fini();
    return 0;
}
