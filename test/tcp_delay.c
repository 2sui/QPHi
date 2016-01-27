
/*
 * tcp_delay_server.c 
 *
 * Test for qpCore module.
 */


#include <qpCore.h>

#define SERV_ADDR             "172.16.72.16"
#define SERV_PORT             8000
#define BUFSIZE               1024
#define INFO(info...)         fprintf(stderr, "\n"info)

int 
main(int argc, char** argv)
{
    bool is_server = false;
    bool some_error = false;
    qp_socket_t        skt, client;
    qp_ulong_t         beg, end;
    struct timeval     time;
    qp_uchar_t         buf[BUFSIZE];
    
    if (!(argc < 2 || strcmp(argv[1], "server"))) {
        is_server = true;
    }
    
    if (NULL == qp_socket_init(&skt, AF_INET, SOCK_STREAM, SERV_ADDR, SERV_PORT, 
        is_server))
    {
        INFO("Socket init fail.");
        return QP_ERROR;
    }
    
    if (is_server) {
        
        if (QP_ERROR == qp_socket_set_reuse(&skt, QP_SOCKET_SO_REUSE_ADDR 
            /* | QP_SOCKET_SO_REUSE_PORT*/)) 
        {
            some_error = true;
            INFO("Socket set reuse fail.");
            goto end;
        }
        
        if (QP_ERROR == qp_socket_listen(&skt, 0)) {
            some_error = true;
            INFO("Socket set listen fail.");
            goto end;
        }
        
        while (qp_socket_accept(&skt, &client)) {
            
            while (1) {
                
                if (0 > qp_socket_recv(&client, buf, BUFSIZE, 0)) {
                    break;
                }
                
                gettimeofday(&beg, NULL);
                INFO("Serv recv %l byte at %u.%u", client.socket.retsno, 
                    beg.tv_sec, beg.tv_usec/1000);
            }
            
            qp_socket_destroy(&client);
        }
        
    } else {
        
    }
    
    end:
    qp_socket_destroy(&skt);
    
    if (some_error) {
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}
