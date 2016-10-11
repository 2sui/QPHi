
# QPHi

----
[![license](https://img.shields.io/badge/license-MIT%20License-blue.svg)](https://mit-license.org/)
[![tag](https://img.shields.io/badge/tag-v1.1.1-yellow.svg)](https://github.com/2sui/QPHi/tree/1.1.1)
[![Build Status](https://travis-ci.org/2sui/QPHi.svg?branch=master)](https://travis-ci.org/2sui/QPHi)

Linux libraries for developing. *(It`s NOT finished yet!)*

----

## Build and Install
Change current dirctrory to QPHi, run `./build.sh help` that will show usage:

```shell
$ ./build.sh help
Usage:
  ./build.sh -- Release build.
  ./build.sh debug -- Debug build.
  ./build.sh install -- Install after build.
```
First, build the library:

```shell
$ ./build.sh
```
If you want build it with debug info:

```shell
$ ./build.sh debug
```

After that, run `./build.sh install` to install the library (may need `sudo`):

```shell
$ sudo ./build.sh install
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
write_process(qp_int_t index, qp_event_stat_t stat, qp_uchar_t* cache, \
    size_t* write_bytes, size_t size)
{
    *write_bytes = strlen(HTTP_RSP) > size ? size : strlen(HTTP_RSP);
    strncpy(cache, HTTP_RSP, *write_bytes);
    return 0; // close after writting
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


