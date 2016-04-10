# QPHi library
----
There are some implements for simplify linux programming, such as file, socket, event, IPC, TCP/IP stack and other useful functions.

### 1.qp_io
`qp_io_t` is basic I/O operation object for read, write and close (for file descriptor, open operation is not included)

### 2.qp_socket
`qp_socket_t` is socket object for network and local translation.You can  build a ipv4/ipv6/unix/raw socket by initiializing it:

```
/* create a socket as server mod listenning on 0.0.0.0:8080 */
qp_socket_t* socket = qp_socket_init(NULL, AF_INET, SOCK_STREAM, "0.0.0.0", 8080, true);

/* if listenning operation fail, destroy it and return */
if (QP_ERROR == qp_socket_listen(socket, 0)) {
    qp_socket_destroy(socket);
    return -1;
}

qp_socket_t* client = NULL;
char	     buffer[256];

while (client = qp_socket_accept(socket, client)) {
    
    if (0 < qp_socket_recv(client, buffer, 256, 0)) {
        qp_socket_send(client, buffer, strlen(buffer), 0);
    }
    
    qp_socket_close(client);
}

qp_socket_destroy(socket);

return;
```
Now you setup a *"echo server"* with few code. And you can get more detail in `qp_socket.h`.
