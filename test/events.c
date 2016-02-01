/**
 * events.c
 * 
 * Test for qpCore module.
 */


#include <qphi.h>


qp_int_t
process_handler(qp_event_data_t* data, qp_event_stat_t stat, size_t read_cnt, 
    size_t write_cnt)
{}

void
init_handler(qp_event_data_t* data)
{}

void
destroy_handler(qp_event_data_t* data)
{}

int
main(int argc, char** argv)
{
    qp_event_t    emodule;
    qp_socket_t   skt;
    qp_int_t      run = 0;
    
    
    if (!qp_socket_init(&skt, AF_INET, SOCK_STREAM, "0.0.0.0", 8080, true) 
        || !qp_event_init(&emodule, 1024, true, true, 
        init_handler, destroy_handler, NULL, NULL)) 
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
    return 0;
}
