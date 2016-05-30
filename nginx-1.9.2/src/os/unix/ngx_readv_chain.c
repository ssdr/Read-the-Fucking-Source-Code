
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ssize_t
ngx_readv_chain(ngx_connection_t *c, ngx_chain_t *chain, off_t limit)
{//这个函数用readv将将连接的数据读取放到chain的链表里面，如果有错标记error或者eof。
//返回读取到的字节数
    u_char        *prev;
    ssize_t        n, size;
    ngx_err_t      err;
    ngx_array_t    vec;
    ngx_event_t   *rev;
    struct iovec  *iov, iovs[NGX_IOVS_PREALLOCATE];//16个块

    rev = c->read;

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "readv: eof:%d, avail:%d, err:%d",
                       rev->pending_eof, rev->available, rev->kq_errno);

        if (rev->available == 0) {
            if (rev->pending_eof) {
                rev->ready = 0;
                rev->eof = 1;

                ngx_log_error(NGX_LOG_INFO, c->log, rev->kq_errno,
                              "kevent() reported about an closed connection");

                if (rev->kq_errno) {
                    rev->error = 1;
                    ngx_set_socket_errno(rev->kq_errno);
                    return NGX_ERROR;
                }

                return 0;

            } else {
                return NGX_AGAIN;
            }
        }
    }

#endif

    prev = NULL;
    iov = NULL;
    size = 0;

    vec.elts = iovs; //vec数组中包括NGX_IOVS_PREALLOCATE个struct iovec结构
    vec.nelts = 0;
    vec.size = sizeof(struct iovec);
    vec.nalloc = NGX_IOVS_PREALLOCATE;
    vec.pool = c->pool;

    /* coalesce the neighbouring bufs */

    while (chain) {//遍历chain缓冲链表，不断的申请struct iovec结构为待会的readv做准备，碰到临近2块内存如果正好接在一起，就公用之。
        n = chain->buf->end - chain->buf->last; //该chain->buf中可以使用的内存有这么多

        if (limit) {
            if (size >= limit) {
                break;
            }

            if (size + n > limit) {
                n = (ssize_t) (limit - size);
            }
        }

        if (prev == chain->buf->last) { //说明前面一个chain的end后和面一个chain的last刚好相等，也就是这两个chain内存是连续的 临近2块内存如果正好接在一起，就公用之。
            iov->iov_len += n;

        } else {
            if (vec.nelts >= IOV_MAX) {
                break;
            }

            iov = ngx_array_push(&vec);
            if (iov == NULL) {
                return NGX_ERROR;
            }

            //指向这块内存起始位置，其实之前可能还有数据，注意这不是内存块的开始，而是数据的末尾。有数据是因为上次没有填满一块内存块的数据。
            iov->iov_base = (void *) chain->buf->last;
            iov->iov_len = n;//赋值这块内存的最大大小。
        }

        size += n;
        prev = chain->buf->end;
        chain = chain->next;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "readv: %d, last(iov_len):%d", vec.nelts, iov->iov_len);

    do {
        //read系列函数返回0表示对端发送了FIN包
		//If any portion of a regular file prior to the end-of-file has not been written, read() shall return bytes with value 0.
		//如果是没有数据可读了，会返回-1，然后errno为EAGAIN表示暂时没有数据。
		//从上面可以看出readv可以将对端的数据读入到本端的几个不连续的内存中，而read则只能读入到连续的内存中
        /* On success, the readv() function returns the number of bytes read; the writev() function returns the number of bytes written.  
        On error, -1 is returned, and errno is  set appropriately. readv返回被读的字节总数。如果没有更多数据和碰到文件末尾时返回0的计数。 */
        n = readv(c->fd, (struct iovec *) vec.elts, vec.nelts);

        if (n >= 0) {

#if (NGX_HAVE_KQUEUE)

            if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
                rev->available -= n;

                /*
                 * rev->available may be negative here because some additional
                 * bytes may be received between kevent() and recv()
                 */

                if (rev->available <= 0) {
                    if (!rev->pending_eof) {
                        rev->ready = 0;
                    }

                    if (rev->available < 0) {
                        rev->available = 0;
                    }
                }

                if (n == 0) {//readv返回0表示对端已经关闭连接，没有数据了。

                    /*
                     * on FreeBSD recv() may return 0 on closed socket
                     * even if kqueue reported about available data
                     */

#if 0
                    ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                                  "readv() returned 0 while kevent() reported "
                                  "%d available bytes", rev->available);
#endif

                    rev->ready = 0;
                    rev->eof = 1;
                    rev->available = 0;
                }

                return n;
            }

#endif /* NGX_HAVE_KQUEUE */

            if (n < size && !(ngx_event_flags & NGX_USE_GREEDY_EVENT)) {
                rev->ready = 0; //说明对端发送过来存储在本端内核缓冲区的数据已经读完，  epoll不满足这个if条件
            }

            if (n == 0) {//按照readv返回值，这个应该不是错误，只是表示没数据了 readv返回被读的字节总数。如果没有更多数据和碰到文件末尾时返回0的计数。
                
                rev->eof = 1; //该函数的外层函数发现是0，则认为数据读取完毕
            }

            return n; //对于epoll来说，还是可读的，也就是readv为1
        }

        //说明n<0   On error, -1 is returned, and errno is  set appropriately
    
        err = ngx_socket_errno;

        //readv返回-1，如果不是EAGAIN就有问题。 例如内核缓冲区中没有数据，你也去readv，则会返回NGX_EAGAIN
        if (err == NGX_EAGAIN || err == NGX_EINTR) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "readv() not ready");
            n = NGX_AGAIN;

        } else {
            n = ngx_connection_error(c, err, "readv() failed");
            break;
        }

    } while (err == NGX_EINTR);

    rev->ready = 0;//不可读了。

    if (n == NGX_ERROR) {
        c->read->error = 1;//连接有错误发生。
    }

    return n;
}
