/**
 * events.c
 * 
 * Test for QPHi module.
 */


#include <qp_pool.h>
#include <qp_socket.h>
#include <qp_event.h>


#define  HTTP_RSP  \
"HTTP/1.1 200 OK\r\n"\
"Server: qp_test\r\n"\
"connection: close\r\n\r\n"\
"<html>"\
"<title>QPHi Test</title>"\
"<body>"\
"<h1>QPHi</h1>"\
"<p> If you see this, you are getting data from QPHi. </p>"\
"</body>"\
"<html>"


qp_int_t
read_process(qp_int_t index, qp_event_stat_t stat, qp_uchar_t* cache, size_t offset)
{
    if (QP_EVENT_CLOSE == stat || offset < 1) {
        return QP_ERROR;
    }
    
    return 1;
}

qp_int_t
write_process(qp_int_t index, qp_event_stat_t stat, qp_uchar_t* cache, \
    size_t* write_bytes, size_t size)
{
    *write_bytes = strlen(HTTP_RSP) > size ? size : strlen(HTTP_RSP);
    strncpy(cache, HTTP_RSP, *write_bytes);
    return 0; // close after writting
}

int
main()
{
    qp_event_t    event;
    qp_socket_t   skt;
    
    if (!(skt = qp_socket_init(NULL, AF_INET, SOCK_STREAM, "0.0.0.0", 8080, true,\
        128)) || !(event = qp_event_init(NULL, 1024, true, true) )) 
    {
        fprintf(stderr, "\n Socket or event init fail.");
        goto end;
    }
    
    qp_event_regist_read_process_handler(event, read_process);
    qp_event_regist_write_process_handler(event, write_process);
    qp_socket_set_reuse(skt, QP_SOCKET_SO_REUSE_ADDR, 1);
    
    if (QP_ERROR == qp_socket_listen(skt, 0)) {
        fprintf(stderr, "\n Listen fail.");
        goto end;
    }
    
    if (QP_ERROR == qp_event_addevent(event, qp_socket_to_int(skt), 30, true, false)) {
        fprintf(stderr, "\n Add event fail.");
        goto end;
    }
    
    qp_event_dispatch(event, 3000);
    
    end:
    qp_socket_destroy(skt);
    qp_event_destroy(event);
    fprintf(stderr, "\n Quit.");
    return 0;
}
