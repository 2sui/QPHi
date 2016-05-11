/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: shuai
 *
 * Created on 2015年12月3日, 下午4:06
 */

#include <cstdlib>
#include <qp_http_parse.h>

using namespace std;


#define  MQ_HTTP_RESPON_200_HTML  \
"<html>"\
"<head>"\
"<titile>MQ_Monitor</titile>"\
"</head>"\
"<body>"\
"<h1>Method : %s</h1>"\
"<h2>URL : %s</h2>"\
"</body>"\
"</html>\0"

#define  MQ_HTTP_RESPON_400_HTML \
"<html>"\
"<head>"\
"<titile>MQ_Monitor</titile>"\
"</head>"\
"<body>"\
"<h1>400 Not Found... T_T</h1>"\
"</body>"\
"</html>\0"


/*
 * 
 */
int main(int argc, char** argv) {
    
    /*******************************************************
     * 50-72 行为demo创建监听套接字,用来接受网络连接.
     */
    int skt = socket(AF_INET, SOCK_STREAM, NULL);
    
    if (0 > skt) {
        return -1;
    }
    int set = 1;
    
    setsockopt(skt, SOL_SOCKET, SO_REUSEADDR,
        (const void *)&set, sizeof(set));
    
    struct sockaddr_in  addr;
    socklen_t           len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8090);
    
    if (0 > bind(skt, (struct sockaddr*)&addr, len)) {
        return -1;
    }
    
    if (0 > listen(skt, 128)) {
        return -1;
    }
    /*******************************************************/
    
    int client = -1;
    /* 创建接受缓冲, 长度为 4k */
    char  readbuf[4096];
    /* 创建 http parser 对象 */
    qp_http_parse  parser; 
    
    /* 接受连接 */
    while (0 < (client = accept(skt, NULL, NULL))) {
        /* recv_size 用来保存接受到的数据的字节数 */
        ssize_t recv_size = -1;
        
        /* 连续接收数据 */
        while (0 < (recv_size = read(client, readbuf, 4096))) {
            
            /* 调用parse的request_parse方法解析收到的请求数据, 这里每接受一点数据就
             * 解析一点,直到接受完整的请求后,解析也就完了.
             */
            switch (parser.request_parse(readbuf, recv_size)) {
                
                /* 如果 parser.request_parse 返回 QP_PARSER_SUCCESS 表示解析完成 */
                case QP_PARSER_SUCCESS: {
                    /* 解析成功后就可以使用解析出来的字段了,
                     * 可以调用 parser.respon_set_xxx
                     * 设置响应数据的各个字段的内容了. */
                    
                    /* parser.responce_set_code 设置 HTTP 响应的 响应码,
                     * 如 200 OK ,400 BAD REQUEST ,500 INTERNAL ERROR 等,
                     * 这里这个函数的参数接受:
                     *    MQ_HTTP_RESPON_200_STAT
                     *    MQ_HTTP_RESPON_400_STAT
                     *    MQ_HTTP_RESPON_500_STAT   */
                    parser.responce_set_code(QP_HTTP_RESPON_200_STAT);
                    
                    /* parser.responce_set_connection 设置 HTTP 响应的connection
                     * 字段,即 "connection: close" 或 "connection:keep-alive",
                     * 这里这个函数的参数接受:
                     *    QP_HTTP_RESPON_CLOSE 表示 "connection: close"
                     *    MQ_HTTP_RESPON_KEEPALIVE 表示 "connection:keep-alive"
                     * 
                     * 一般设置 MQ_HTTP_RESPON_CLOSE 就好
                     */
                    parser.responce_set_connection(QP_HTTP_RESPON_CLOSE);
                    
                    /* 这里设置响应的 Body,这个 Body中显示请求中的方法, 如 GET, POST等,
                     * 以及请求的URL.
                     * 
                     *  parser.request_get_parsed_request()->method 获取请求的
                     * 方法, 返回const char* 型
                     * 
                     * parser.request_get_parsed_request()->URL 获取请求的URL,
                     * 返回 const char* 型
                     * 
                     * 其他可用的字段有:
                     * parser.request_get_parsed_request().content,表示 POST 或
                     * PUT 方法携带的数据体, 返回 const char*型
                     * 
                     * parser.request_get_parsed_request().content_len, 表示
                     * 携带的数据体的长度, 返回 size_t 型
                     */
                    sprintf(readbuf, MQ_HTTP_RESPON_200_HTML,
                        parser.request_get_parsed_request()->method,
                        parser.request_get_parsed_request()->URL);
                    
                    /* 设置响应的 Body 为刚才的readbuf */
                    parser.responce_set_body(readbuf);
                    
                    /* struct iovec 为 linux 下的数据类型,保存的是分块的数据 */
                    struct iovec  *writebuf;
                    
                    /* parser.responce_get_frame将组好的响应数据赋值给 writebuf,
                     * 并返回相应中的字段数量 */
                    size_t size = parser.responce_get_frame(&(writebuf));
                    
                    /* writev 也是linux下的函数, 用来将分块数据一起发送出去,这里将
                     * 上一步获取的响应的分快数据发送出去. */
                    writev(client, writebuf, size);
                    goto next;
                         
                } break;
                
                /* 如果 parser.request_parse 返回 QP_PARSER_ERROR 表示解析出错 */
                case QP_PARSER_ERROR: {
                    
                } break;
                
                /* 如果返回 parser.request_parse 其他值,如 QP_PARSER_PARSING 就表示
                 * 当前收到的数据解析完了,但是请求不完整,需要等待身下的数据 */
                default: break;
            }
            /* 进行下次数据读取 */
        }
        
        next:
        /* 接收到的数据小于0 说明连接关闭或出错,则关闭连接 */
        close(client);
    }
    
    return 0;
}

