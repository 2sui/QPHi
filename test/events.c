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
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"\
"1234567890123456789012345678901212345678901234567890123456789012"


static qp_pool_manager_t    manager;

qp_int_t
process_handler(qp_event_data_t data, qp_int_t fd, qp_event_stat_t stat, 
    bool read_finish, size_t read_cnt, bool write_finish, size_t write_cnt)
{
    switch (stat) {
        
        case QP_EVENT_NEW: {
            sprintf((char*) data->writebuf.block, "%s", HTTP_RSP);
            data->writebuf_max = strlen(HTTP_RSP);
            return QP_EPOLL_OUT;
        }break;
        
        case QP_EVENT_PROCESS: {
            
            if (write_finish) {
                return QP_ERROR;
            }
            
        }break;
        
        default: {
        }break;
    }
    
    return QP_ERROR;
}

void
init_handler(qp_event_data_t data)
{
    data->readbuf_max = 512;
    data->readbuf.block = qp_pool_manager_alloc(manager, data->readbuf_max, NULL);
    
    data->writebuf_max = 512;
    data->writebuf.block = qp_pool_manager_alloc(manager, data->writebuf_max, 
        NULL);
    
    data->process_handler = process_handler;
}

void
destroy_handler(qp_event_data_t data)
{
    if (data->readbuf.block) {
        qp_pool_manager_free(manager, data->readbuf.block, NULL);
    }
    
    if (data->writebuf.block) {
        qp_pool_manager_free(manager, data->writebuf.block, NULL);
    }
}

int
main()
{
    qp_event_t    emodule;
    qp_socket_t   skt;
    
    if (!(manager = qp_pool_manager_init(NULL, 512, 2050))) {
        fprintf(stderr, "\n Pool create fail.");
        return -1;
    }
    
    
    if (!(skt = qp_socket_init(NULL, AF_INET, SOCK_STREAM, "0.0.0.0", 8080, true, 128)) 
        || !(emodule = qp_event_init(NULL, 1024, 500,  
        init_handler, destroy_handler, true, true, NULL, NULL))) 
    {
        fprintf(stderr, "\n Socket or event init fail.");
        goto end;
    }
    
    qp_socket_set_reuse(skt, QP_SOCKET_SO_REUSE_ADDR, 1);
    
    if (QP_ERROR == qp_socket_listen(skt, 0)) {
        fprintf(stderr, "\n Listen fail.");
        goto end;
    }
    
    if (QP_ERROR == qp_event_addevent(emodule, qp_socket_to_int(skt), 0, true, false)) {
        fprintf(stderr, "\n Add event fail.");
        goto end;
    }
    
    qp_event_tiktok(emodule, 30000);
//    qp_event_disable(emodule);
    
    
    end:
    qp_socket_destroy(skt);
    qp_event_destroy(emodule);
    qp_pool_manager_destroy(manager, true);
    fprintf(stderr, "\n Quit.");
    return 0;
}
