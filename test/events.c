/**
 * events.c
 * 
 * Test for qpCore module.
 */


#include <qphi.h>


#define  HTTP_RSP  \
"HTTP/1.1 OK 200\r\n"\
"Server: qp_test\r\n"\
"connection: close"


static qp_pool_manager_t    manager;

qp_int_t
process_handler(qp_event_data_t* data, qp_event_stat_t stat, size_t read_cnt, 
    size_t write_cnt)
{
    switch (stat) {
        
        case QP_EVENT_NEW: {
            
        }break;
        
        case QP_EVENT_PROCESS: {
            
        }break;
        
        default: {
            
        }break;
    }
    
    return QP_ERROR;
}

int
main(int argc, char** argv)
{
    qp_event_t    emodule;
    qp_socket_t   skt;
    qp_int_t      run = 0;
    
    if (!qp_pool_manager_init(&manager, 1024, 1025)) {
        return -1;
    }
    
    
    if (!qp_socket_init(&skt, AF_INET, SOCK_STREAM, "0.0.0.0", 8080, true) 
        || !qp_event_init(&emodule, 1024, true, true, process_handler, 
        NULL, NULL)) 
    {
        goto end;
    }
    
    if (QP_ERROR == qp_socket_listen(&skt, 0)) {
        goto end;
    }
    
    if (QP_ERROR == qp_event_addevent(&emodule, skt.socket.fd, true, false)) {
        goto end;
    }
    
    run = 1;
    
    qp_event_tiktok(&emodule, &run);
    
    
    end:
    qp_socket_destroy(&skt);
    qp_event_destroy(&emodule);
    qp_pool_manager_destroy(&manager, true);
    return 0;
}

