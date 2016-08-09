
# QPHi

----
[![license](https://img.shields.io/badge/license-MIT%20License-blue.svg)](https://mit-license.org/)
[![tag](https://img.shields.io/badge/tag-v1.0.0-yellow.svg)](https://github.com/2sui/QPHi/tree/1.0.0)
[![Build Status](https://travis-ci.org/2sui/QPHi.svg?branch=master)](https://travis-ci.org/2sui/QPHi)

Linux libraries for developing.

----

## Build and Install
Change current dirctrory to [_src/QPCore_](./src/QPCore) in **QPHi**:

```
$ QPHi$ cd src/QPCore
```
Then run `cmake` in [_src/QPCore_](./src/QPCore) to pre-build library:

```
QPHi/src/QPCore$ cmake
```
And run `make && sudo make install` to build and install library (You may need to type in your sudo password):

```
QPHi/src/QPCore$ make && sudo make install
```

----

## Module List
##### qp_socket
Socket module for TCP and Unix socket. (example: [events.c](./test/events.c), [tcp_delay.c](./test/tcp_delay.c))

##### qp_file
File module with general I/O and direct I/O.(example: [pcapreader.c](./test/pcapreader.c))

##### qp_event
Simple event loop.(example: [events.c](./test/events.c))

##### qp_ipc
Linux ipc wrapper.(example: [lock.c](./test/lock.c))

##### qp_pool
Memory pool manager(example: [events.c](./test/events.c))

##### qp_process
Process and thread manager.(example: [lock.c](./test/lock.c))

##### qp_system
System method wrapper.

##### qp_tiny_stack
Tiny tcp/ip stack implementations.(example: [pcapreader.c](./test/pcapreader.c))

----

## Usage
There are also some examples showing how to use those libraries in [`QPHi/test`](./test) and [`QPHi/example`](./example):

In [`QPHi/test/events.c`](./test/events.c):

__read_process:__

```
qp_int_t
read_process(qp_int_t index, qp_event_stat_t stat, qp_uchar_t* cache, size_t offset)
{
    if (QP_EVENT_CLOSE == stat || offset < 1) {
        return QP_ERROR;
    }
    
    return 1;
}

```

__write_process:__

```
qp_int_t
write_process(qp_int_t index, qp_event_stat_t stat, qp_int_t read_ret, \
    qp_uchar_t* cache, size_t size)
{
    switch (read_ret) {
        case 1: {
            size_t ret = strlen(HTTP_RSP);
            strncpy(cache, HTTP_RSP, ret);
            return ret;
        }
                    
        default:
            return 0;
    }
}

```

__main:__

```
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
    
    qp_event_dispatch(event, 0);
    
    end:
    qp_socket_destroy(skt);
    qp_event_destroy(event);
    fprintf(stderr, "\n Quit.");
    return 0;
}

```

----

## License
QPHi is released under a MIT License.See [LICENSE](./LICENSE) file for details.

----

##### created by 2sui


