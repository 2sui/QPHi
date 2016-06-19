
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

```
	static qp_pool_manager_t    manager;
	
	/*
	   ignore some code ......
	*/
	
	qp_event_t    emodule; 
    qp_socket_t   skt;
    
    /* Initialize the memory pool manager with 2050 buckets and 512Bytes for each bucket */
    if (!(manager = qp_pool_manager_init(NULL, 512, 2050)) {
        fprintf(stderr, "\n Pool create fail.");
        return -1;
    }
    
    /* Initialize a tcp socket server, bind address "0.0.0.0", port 8080 
     * and initialize the event module (allow 1024 event fd).
     */
    if (!(skt = qp_socket_init(NULL, AF_INET, SOCK_STREAM, "0.0.0.0", 8080, true, 128)) 
        || !(emodule = qp_event_init(NULL, 1024, true, true, 
        init_handler, destroy_handler, NULL, NULL))) 
    {
        fprintf(stderr, "\n Socket or event init fail.");
        goto end;
    }
    
    /* set socket reusing address */
    qp_socket_set_reuse(skt, QP_SOCKET_SO_REUSE_ADDR, 1);
    
    /* listen */
    if (QP_ERROR == qp_socket_listen(skt, 0)) {
        fprintf(stderr, "\n Listen fail.");
        goto end;
    }
    
    /* add the listening socket to event moduel */
    if (QP_ERROR == qp_event_addevent(emodule, qp_socket_to_int(skt), true, false)) {
        fprintf(stderr, "\n Add event fail.");
        goto end;
    }
    
    /* start event loop */
    qp_event_tiktok(emodule, 30000);
    
    /* stop event loop, may be in other thread. */
    // qp_event_disable(emodule);
    
    
    end:
    /* close socket */
    qp_socket_destroy(skt);
    /* close event module */
    qp_event_destroy(emodule);
    /* destroy memory pool */
    qp_pool_manager_destroy(manager, true);

```

----

## License
QPHi is released under a MIT License.See [LICENSE](./LICENSE) file for details.

----

##### created by 2sui


