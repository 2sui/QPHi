
/*
 * tcp_delay_server.c 
 *
 * Test for qpCore module.
 */


#include <qpCore.h>

#define SERV_ADDR             "127.0.0.1"
#define SERV_PORT             80
#define BUFSIZE               1024
#define USER                  "nobody"
#define INFO(info...)         fprintf(stderr, "\n"info)

int 
main(int argc, char** argv)
{
    qp_socket_t        skt, client;
    qp_ulong_t         beg, end;
    struct timeval     ntime;
    int                count;
    qp_uchar_t         buf[BUFSIZE];
    bool is_server = false;
    bool some_error = false;
    
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
        
        if (QP_ERROR == qp_socket_set_reuse(&skt, QP_SOCKET_SO_REUSE_ADDR, 1)) 
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
        
        if (QP_ERROR == qp_change_user_by_name(USER)) {
            some_error = true;
            INFO("Change user fail");
            goto end;
        }
        
        while (qp_socket_accept(&skt, &client)) {
            end = 0;
            gettimeofday(&ntime, NULL);
            beg = ntime.tv_sec * 1000 + ntime.tv_usec / 1000;
            INFO("Serv accept at %lu. sec:%lu, usec:%lu", beg, ntime.tv_sec, ntime.tv_usec);
            /* for disable Nagle */
            qp_socket_set_nodelay(&client, 1);
            qp_socket_set_quickack(&client, 1);
            
            while (1) {
                
                /* test for Nagle */
                if (1 > qp_socket_recv(&client, buf, BUFSIZE, 0)) {
                    break;
                }
                
                gettimeofday(&ntime, NULL);
                end = ntime.tv_sec * 1000 + ntime.tv_usec / 1000;
                
                /* The client send 160 bytes in total , if Nagle is not disabled, 
                 * the server will recv 16 byte for first time and recv 114 bytes 
                 * for the second, beacuse the first 16 bytes will be sent at once,
                 * and then the client wait for ACK, but the server will never 
                 * send any data, so the client will never recv an ACK in 40~200ms,
                 * and wait for data number in buffer is MSS or close() is called.
                 * And fortunately, the close is called by client and the rest 
                 * 114 bytes is sent to server.
                 */
                INFO("Serv recv %ld byte at %lu, Nagle diff time: %lu.", \
                    client.socket.retsno, end, end - beg);
                
                beg = end;
            }
            
            qp_socket_destroy(&client);
            gettimeofday(&ntime, NULL);
            beg = ntime.tv_sec * 1000 + ntime.tv_usec / 1000;
            INFO("Serv peer close at %lu.", beg);
        }
        
    } else {
        
        if (QP_SUCCESS == qp_socket_connect(&skt)) {
            count = 21;
            end = 0;
            gettimeofday(&ntime, NULL);
            beg = ntime.tv_sec * 1000 + ntime.tv_usec / 1000;
            INFO("Client connect at %lu.", beg);
            
            /* for disable Nagle */
            qp_socket_set_nodelay(&skt, 1);
            qp_socket_set_quickack(&skt, 1);
            
            while (count--) {
                
                if (1 > qp_socket_send(&skt, buf, 16, 0)) {
                    break;
                }
                
                gettimeofday(&ntime, NULL);
                end = ntime.tv_sec * 1000 + ntime.tv_usec / 1000;
                
                INFO("Client send %ld byte at %lu, send diff time: %lu.", \
                    skt.socket.retsno, end, (end - beg));
                
                beg = end;
                
                if (!(count % 3)) {
                    sleep(1);
                }
                
            }
        }
    }
    
    end:
    qp_socket_destroy(&skt);
    
    if (some_error) {
        return QP_ERROR;
    }
    
    return QP_SUCCESS;
}
