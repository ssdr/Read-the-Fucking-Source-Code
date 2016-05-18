
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_UPSTREAM_H_INCLUDED_
#define _NGX_HTTP_UPSTREAM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_http.h>


#define NGX_HTTP_UPSTREAM_FT_ERROR           0x00000002
#define NGX_HTTP_UPSTREAM_FT_TIMEOUT         0x00000004
#define NGX_HTTP_UPSTREAM_FT_INVALID_HEADER  0x00000008
#define NGX_HTTP_UPSTREAM_FT_HTTP_500        0x00000010
#define NGX_HTTP_UPSTREAM_FT_HTTP_502        0x00000020
#define NGX_HTTP_UPSTREAM_FT_HTTP_503        0x00000040
#define NGX_HTTP_UPSTREAM_FT_HTTP_504        0x00000080
#define NGX_HTTP_UPSTREAM_FT_HTTP_403        0x00000100
#define NGX_HTTP_UPSTREAM_FT_HTTP_404        0x00000200
#define NGX_HTTP_UPSTREAM_FT_UPDATING        0x00000400
#define NGX_HTTP_UPSTREAM_FT_BUSY_LOCK       0x00000800
#define NGX_HTTP_UPSTREAM_FT_MAX_WAITING     0x00001000
#define NGX_HTTP_UPSTREAM_FT_NOLIVE          0x40000000
#define NGX_HTTP_UPSTREAM_FT_OFF             0x80000000

#define NGX_HTTP_UPSTREAM_FT_STATUS          (NGX_HTTP_UPSTREAM_FT_HTTP_500  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_502  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_503  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_504  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_403  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_404)

#define NGX_HTTP_UPSTREAM_INVALID_HEADER     40


#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100
#define NGX_HTTP_UPSTREAM_IGN_VARY           0x00000200


typedef struct {
    ngx_msec_t                       bl_time;
    ngx_uint_t                       bl_state;

    // HTTP/1.1 200 OK ��Ӧ�е�200  u->state->status = u->headers_in.status_n;��ʾ���η���˷��ص�״̬����ֵ��ngx_http_fastcgi_process_header
    ngx_uint_t                       status; //��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line // HTTP/1.1 200 OK ��Ӧ�е�200
    ngx_msec_t                       response_time;//��ʼ����ngx_http_upstream_connect
    ngx_msec_t                       connect_time; //��ʼ����ngx_http_upstream_connect
    ngx_msec_t                       header_time;
    off_t                            response_length; //��ȡ����˷��͹����İ��岿�ֵ����ݲ��ֳ���

    ngx_str_t                       *peer;
} ngx_http_upstream_state_t;


typedef struct {
    ngx_hash_t                       headers_in_hash; //��ngx_http_upstream_init_main_conf�ж�ngx_http_upstream_headers_in��Ա����hash�õ�
    ngx_array_t                      upstreams; /* ngx_http_upstream_srv_conf_t */ //upstream {}����Ϣ��Ӧ�����飬��Ϊ�������ö��upstream{}��
} ngx_http_upstream_main_conf_t;

typedef struct ngx_http_upstream_srv_conf_s  ngx_http_upstream_srv_conf_t;

typedef ngx_int_t (*ngx_http_upstream_init_pt)(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);
typedef ngx_int_t (*ngx_http_upstream_init_peer_pt)(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);


typedef struct {
//���ݲ�ͬ�ĺ�˸�ծ�����㷨����ֵͬ:ngx_http_upstream_init_least_conn ngx_http_upstream_init_hash  ngx_http_upstream_init_chash ngx_http_upstream_init_keepalive ngx_http_upstream_init_round_robin
    //��ngx_http_upstream_init_main_conf��ִ�У�Ȼ����������init��ֵ  ��������ø��ؾ��ⷽʽ��Ĭ��Ϊrr��Ȩ��ѯ��ʽ
    //���������keepalive,��Ϊngx_http_upstream_init_keepalive
    ngx_http_upstream_init_pt        init_upstream;//ngx_http_upstream_init_ip_hash�����ȡ�Ĭ��Ϊngx_http_upstream_init_round_robin ��ngx_http_upstream_init_main_conf��ִ��
//ngx_http_upstream_init_request��ִ��,��ִ�������init_upstream()������ʱ�򣬶�init���и�ֵ��
//ngx_http_upstream_init_hash(init_upstream)��Ӧ��init����Ϊngx_http_upstream_init_hash_peer(init����init_upstream���������ϼӸ�_peer)�����������㷨��init_upstream��init����
//���������keepalive,��Ϊngx_http_upstream_init_keepalive_peer
    ngx_http_upstream_init_peer_pt   init;//(init_upstream)_peer,����ngx_http_upstream_init_round_robin_peer
    void                            *data;//ngx_http_upstream_init_round_robin�и�ֵΪngx_http_upstream_rr_peers_t�����еķ�������Ϣ��ͨ��dataָ��
} ngx_http_upstream_peer_t;

//server backend1.example.com weight=5;
/*
��weight = NUMBER - ���÷�����Ȩ�أ�Ĭ��Ϊ1��
��max_fails = NUMBER - ��һ��ʱ���ڣ����ʱ����fail_timeout���������ã��������������Ƿ����ʱ���������ʧ����������Ĭ��Ϊ1����������Ϊ0���Թرռ�飬��Щ������proxy_next_upstream��fastcgi_next_upstream��404���󲻻�ʹmax_fails���ӣ��ж��塣
��fail_timeout = TIME - �����ʱ���ڲ�����max_fails�����ô�С��ʧ�ܳ������������������������ܲ����ã�ͬ����ָ���˷����������õ�ʱ�䣨����һ�γ�������������֮ǰ����Ĭ��Ϊ10�룬fail_timeout��ǰ����Ӧʱ��û��ֱ�ӹ�ϵ����������ʹ��proxy_connect_timeout��proxy_read_timeout�����ơ�
��down - ��Ƿ�������������״̬��ͨ����ip_hashһ��ʹ�á�
��backup - (0.6.7�����)������еķǱ��ݷ�������崻���æ����ʹ�ñ����������޷���ip_hashָ�����ʹ�ã���
*/
typedef struct { //ngx_http_upstream_srv_conf_s->servers[]�еĳ�Ա   �����ռ�͸�ֵ��ngx_http_upstream_server
    ngx_str_t                        name; ////server 127.0.0.1:8080 max_fails=3  fail_timeout=30s;�е�uriΪ/
    //ָ��洢IP��ַ�������ָ�룬host��Ϣ(��Ӧ���� ngx_url_t->addrs )
    ngx_addr_t                      *addrs; //server   127.0.0.1:8080 max_fails=3  fail_timeout=30s;�е�127.0.0.1
    ngx_uint_t                       naddrs;//���һ���������ʹ�ã�����Ԫ�ظ���(��Ӧ���� ngx_url_t->naddrs )
    ngx_uint_t                       weight;
    ngx_uint_t                       max_fails;
    time_t                           fail_timeout;

    unsigned                         down:1; //�÷�������������״̬��������
    unsigned                         backup:1; //���ݷ����� ���еķǱ��ݷ�������崻���æ����ʹ�ñ�������
} ngx_http_upstream_server_t;


/*
 l  NGX_HTTP_UPSTREAM_CREATE��������־��������д�����־�Ļ���nginx�����ظ��������Լ���Ҫ�����Ƿ���д��
l  NGX_HTTP_UPSTREAM_MAX_FAILS��������server��ʹ��max_fails���ԣ�
l  NGX_HTTP_UPSTREAM_FAIL_TIMEOUT��������server��ʹ��fail_timeout���ԣ�
l  NGX_HTTP_UPSTREAM_DOWN��������server��ʹ��down���ԣ�
���⻹���������ԣ�
l  NGX_HTTP_UPSTREAM_WEIGHT��������server��ʹ��weight���ԣ�
l  NGX_HTTP_UPSTREAM_BACKUP��������server��ʹ��backup���ԡ�
*/
#define NGX_HTTP_UPSTREAM_CREATE        0x0001
#define NGX_HTTP_UPSTREAM_WEIGHT        0x0002
#define NGX_HTTP_UPSTREAM_MAX_FAILS     0x0004
#define NGX_HTTP_UPSTREAM_FAIL_TIMEOUT  0x0008
#define NGX_HTTP_UPSTREAM_DOWN          0x0010
#define NGX_HTTP_UPSTREAM_BACKUP        0x0020

/*
upstream backend {
    server backend1.example.com weight=5;
    server backend2.example.com:8080;
    server unix:/tmp/backend3;
}

server {
    location / {
        proxy_pass http://backend;
    }
}
*/ //�����ռ�Ͳ��ָ�ֵ��ngx_http_upstream_add  server backend1.example.com weight=5;����xxx_pass(proxy_pass ���� fastcgi_pass��)���ᴴ���ýṹ����ʾ���η�������ַ��Ϣ��
struct ngx_http_upstream_srv_conf_s { //upstream {}ģ��������Ϣ,�������൱��server{}һ������xxx_pass���൱��һ��upstream{}
//һ��upstream{}���ýṹ������,�����umcf(ngx_http_upstream_main_conf_t)->upstreams����������umcf��upstreamģ��Ķ��������ˡ�
    ngx_http_upstream_peer_t         peer;
    void                           **srv_conf; //��ֵ��ngx_http_upstream����ʾupstream{}�����Ķ��� srv{}����λ��
    //��¼��upstream{}�������serverָ� server backend1.example.com weight=5;
    ngx_array_t                     *servers;  /* ngx_http_upstream_server_t */ //ngx_http_upstream����ngx_http_upstream_add�д����ռ�

    ngx_uint_t                       flags; //NGX_HTTP_UPSTREAM_CREATE | NGX_HTTP_UPSTREAM_MAX_FAILS��
    ngx_str_t                        host; //upstream xxx {}�е�xxx �����ͨ��xxx_pass ip:port��ʽ�Ļ���hostΪ��(fastcgi_param)
    u_char                          *file_name; //�����ļ�����
    ngx_uint_t                       line; //�����ļ��е��к�
    in_port_t                        port;//ʹ�õĶ˿ںţ�ngx_http_upstream_add()���������, ָ��ngx_url_t-->port��ͨ���ں���ngx_parse_inet_url()�н�����
    in_port_t                        default_port;//Ĭ��ʹ�õĶ˿ںţ�ngx_http_upstream_add()���������, ָ��ngx_url_t-->default_port��
    ngx_uint_t                       no_port;  /* unsigned no_port:1 */

#if (NGX_HTTP_UPSTREAM_ZONE)
    ngx_shm_zone_t                  *shm_zone;
#endif
};


typedef struct {
    ngx_addr_t                      *addr;
    ngx_http_complex_value_t        *value;
} ngx_http_upstream_local_t;

/*
��ʵ�ϣ�HTTP�������ģ����nginx.conf�ļ����ṩ�������������������ngx_http_upstream_conf_t�ṹ���еĳ�Ա�ġ�
*/ //�ڽ�����upstream{}���õ�ʱ�򣬴����ýṹ����location{}����
typedef struct { //upstream���ð���proxy fastcgi wcgi�ȶ��øýṹ
//����ngx_http_upstream_t�ṹ����û��ʵ��resolved��Աʱ��upstream����ṹ��Ż���Ч�����ᶨ�����η�����������
    ngx_http_upstream_srv_conf_t    *upstream; 
    //Ĭ��60s
    ngx_msec_t                       connect_timeout;//����TCP���ӵĳ�ʱʱ�䣬ʵ���Ͼ���д�¼���ӵ���ʱ����ʱ���Եĳ�ʱʱ��
    //xxx_send_timeout(fastcgi memcached proxy) Ĭ��60s
    ngx_msec_t                       send_timeout;//��������ĳ�ʱʱ�䡣ͨ������д�¼���ӵ���ʱ�������õĳ�ʱʱ��
    //fastcgi_read_timeout  XXX_read_timeout����   �����˷��Ϳͻ�������������˷������󣬵ȴ���˷�������Ӧ�ĳ�ʱʱ��
    ngx_msec_t                       read_timeout;//������Ӧ�ĳ�ʱʱ�䡣ͨ�����Ƕ��¼���ӵ���ʱ�������õĳ�ʱʱ��
    ngx_msec_t                       timeout;
    ngx_msec_t                       next_upstream_timeout;

    size_t                           send_lowat; //TCP��SO_SNDLOWATѡ���ʾ���ͻ����������� fastcgi_send_lowat proxy_send_lowat
//�����˽���ͷ���Ļ�����������ڴ��С��ngx_http_upstream_t�е�buffer��������������ת����Ӧ�����λ�����buffering��־λΪ0
//�������ת����Ӧʱ����ͬ����ʾ���հ���Ļ�������С   �����պ�˹�����ͷ����Ϣ��ʱ���ȷ�����ô��ռ�������ͷ���е���Ϣ����ngx_http_upstream_process_header
    //ͷ���в���(Ҳ���ǵ�һ��fastcgi data��ʶ��Ϣ������Ҳ��Я��һ������ҳ����)��fastcgi��ʶ��Ϣ���ٵĿռ���buffer_size����ָ��
    //ָ���Ĵ�С�ռ俪����ngx_http_upstream_process_header
    size_t                           buffer_size; //xxx_buffer_size(fastcgi_buffer_size proxy_buffer_size memcached_buffer_size)
    size_t                           limit_rate;//Ĭ��ֵ0 fastcgi_limit_rate ����proxy memcached�Ƚ�����������  ���Ƶ�����ͻ�����������ٶȣ��������˵��ٶ�
//����buffering��־λΪ1������������ת����Ӧʱ��Ч���������õ�ngx_event_pipe_t�ṹ���busy_size��Ա��
//��buffering��ʽ�£�������໻�ɻ�û�з��͵��ͻ��˵���ҳ�����С����ngx_event_pipe_write_to_downstream��Ч
//Ĭ��ֵΪbuffer_size��������ʵ�����ܹ�Ϊ��˿��ٵĿռ�Ϊbuffer_size+ 5*3k(fastcgi_buffers  5 3K)
//p->busy_size = u->conf->busy_buffers_size; 
    size_t                           busy_buffers_size; //xxx_busy_buffers_size fastcgi_busy_buffers_size Ĭ��ֵΪbuffer_size������
/*
��buffering��־λΪ1ʱ����������ٶȿ��������ٶȣ����п��ܰ��������ε���Ӧ�洢����ʱ�ļ��У���max_temp_file_sizeָ������ʱ�ļ���
��󳤶ȡ�ʵ���ϣ���������ngx_event_pipe_t�ṹ���е�temp_file     fastcgi_max_temp_file_size����
*/
    size_t                           max_temp_file_size; //fastcgi_max_temp_file_size XXX_max_temp_file_size
    size_t                           temp_file_write_size; //fastcgi_temp_file_write_size���ñ�ʾ���������е���Ӧд����ʱ�ļ�ʱһ��д���ַ�������󳤶�

    //����ͨ��xxx_busy_buffers_size(proxy_busy_buffers_size)�����ã�Ĭ��ֵΪ2*buffer_size
    size_t                           busy_buffers_size_conf;  //����ֵ��busy_buffers_size
    /*
��buffering��־λΪ1ʱ����������ٶȿ��������ٶȣ����п��ܰ��������ε���Ӧ�洢����ʱ�ļ��У���max_temp_file_sizeָ������ʱ�ļ���
��󳤶ȡ�ʵ���ϣ���������ngx_event_pipe_t�ṹ���е�temp_file          
*/
    size_t                           max_temp_file_size_conf;
    size_t                           temp_file_write_size_conf;////��ʾ���������е���Ӧд����ʱ�ļ�ʱһ��д���ַ�������󳤶�
    //��������ռ���//��ngx_event_pipe_read_upstream�д����ռ�
    ngx_bufs_t                       bufs;//�Ի�����Ӧ�ķ�ʽת�����η������İ���ʱ��ʹ�õ��ڴ��С //����fastcgi_buffers  5 3K
/*
���ngx_http_upstream_t�ṹ���б��������İ�ͷ��headers in��Ա��ignore_headers���԰��ն�����λʹ��upstream��ת����ͷʱ������ĳЩͷ��
�Ĵ�����Ϊ32λ���ͣ�������ignore_headers�����Ա�ʾ32����Ҫ�������账���ͷ����Ȼ��Ŀǰupstream���ƽ��ṩ8��λ���ں���8��HTTPͷ���Ĵ�
��������
#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100
#define NGX_HTTP_UPSTREAM_IGN_VARY           0x00000200
*/
    ngx_uint_t                       ignore_headers;
/*
�Զ�����λ����ʾһЩ�����룬�������������Ӧʱ������Щ�����룬��ô��û�н���Ӧת�������οͻ���ʱ������ѡ����
һ�����η��������ط����󡣲μ�ngx_http_upstream_next����
*/
    ngx_uint_t                       next_upstream;
/*
��buffering��־Ϊ1�������ת����Ӧʱ�����п��ܰ���Ӧ��ŵ���ʱ�ļ��С���ngx_http_upstream_t�е�store��־λΪ1ʱ��
store_access��ʾ��������Ŀ¼���ļ���Ȩ��
*/
    ngx_uint_t                       store_access;
    //XXX_next_upstream_tries,����fastcgi_next_upstream_tries
    ngx_uint_t                       next_upstream_tries;
/*
����ת����Ӧ��ʽ�ı�־λ��bufferingΪ1ʱ��ʾ�򿪻��棬��ʱ��Ϊ���ε����ٿ������ε����٣��ᾡ�������ڴ���ߴ����л����������ε�
��Ӧ�����bufferingΪ0�����Ὺ��һ��̶���С���ڴ����Ϊ������ת����Ӧ
*/ //Ĭ��Ϊ1  request_buffering�Ƿ񻺴�ͻ��˵���˵İ���  buffering�Ƿ񻺴��˵��ͻ���������İ���
    ngx_flag_t                       buffering; //��xxx_buffering��fastcgi_buffering  �Ƿ񻻳ɺ�˷�����Ӧ������İ���
    //Ĭ��1  request_buffering�Ƿ񻺴�ͻ��˵���˵İ���  buffering�Ƿ񻺴��˵��ͻ���������İ���
    ngx_flag_t                       request_buffering;//�Ƿ񻻳ɿͻ�������İ��� XXX_request_buffering (����proxy_request_buffering fastcgi_request_buffering
    //proxy_pass_request_headers fastcgi_pass_request_headers�����Ƿ�ת��HTTPͷ����
    ngx_flag_t                       pass_request_headers;////�Ƿ�ת���ͻ������������������ͷ�������ȥ 
    ngx_flag_t                       pass_request_body; ////�Ƿ�ת���ͻ�������������İ��嵽���ȥ

/*
��ʾ��־λ������Ϊ1ʱ����ʾ�����η���������ʱ�������Nginx�����οͻ��˼�������Ƿ�Ͽ���
Ҳ����˵����ʹ���οͻ��������ر������ӣ�Ҳ�����ж������η�������Ľ�������ngx_http_upstream_init_request
*/ //Ĭ��off
    ngx_flag_t                       ignore_client_abort; //fastcgi_ignore_client_abort ON | OFF
/*
������������Ӧ�İ�ͷʱ��������������õ�headers_in�ṹ���е�status_n���������400�������ͼ������error_page��ָ���Ĵ�������ƥ�䣬
���ƥ���ϣ�����error_page��ָ������Ӧ����������������η������Ĵ����롣���ngx_http_upstream_intercept_errors����
*/
    ngx_flag_t                       intercept_errors;
/*
buffering��־λΪ1�������ת����Ӧʱ�������塣��ʱ�����cyclic_temp_fileΪl�������ͼ������ʱ�ļ����Ѿ�ʹ�ù��Ŀռ䡣������
��cyclic_temp_file��Ϊ1
*/ //Ĭ��0
    ngx_flag_t                       cyclic_temp_file; //fastcgi_cyclic_temp_file  XXX_cyclic_temp_file
    ngx_flag_t                       force_ranges;

    //xxx_temp_path fastcgi_temp_path����  Ĭ��ֵngx_http_fastcgi_temp_path
    ngx_path_t                      *temp_path; //��buff ering��־λΪ1�������ת����Ӧʱ�������ʱ�ļ���·��
/*
��ת����ͷ����ʵ������ͨ��ngx_http_upstream_hide_headers_hash����������hide_headers��pass_headers��̬���鹹�������Ҫ���ص�HTTPͷ��ɢ�б�
*/ //������洢����ngx_http_xxx_hide_headers��ngx_http_fastcgi_hide_headers ngx_http_proxy_hide_headers��
    ngx_hash_t                       hide_headers_hash; //��default_hide_headers(ngx_http_proxy_hide_headers  ngx_http_fastcgi_hide_headers)�еĳ�Ա��hash���浽conf->hide_headers_hash

/*
hide_headers��������ngx_array_t��̬���飨ʵ���ϣ�upstreamģ�齫��ͨ��hide_headers������hide_headers_hashɢ�б���
����upstreamģ��Ҫ��hide_headers������ΪNULL�����Ա���Ҫ��ʼ��hide_headers��Ա��upstreamģ���ṩ��
ngx_http_upstream_hide_headers hash��������ʼ��hide_headers�����������ںϲ���������ڡ�
*/ //XXX_pass_headers   XXX_hide_headers�����ص���ͻ������hide_headerΪ׼����ngx_http_upstream_hide_headers_hash
//��ת��������Ӧͷ����ngx_http_upstream_t��headers_in�ṹ���е�ͷ���������οͻ���ʱ�����ϣ��ĳЩͷ��ת�������Σ������õ�hide_headers��̬������
    ngx_array_t                     *hide_headers; //proxy_hide_header fastcgi_hide_header
/*
��ת��������Ӧͷ����ngx_http_upstream_t��headers_in�ṹ���е�ͷ���������οͻ���ʱ��upstream����Ĭ�ϲ���ת���硰Date������Server��֮
���ͷ�������ȷʵϣ��ֱ��ת�����ǵ����Σ������õ�pass_headers��̬������
*/ //XXX_pass_headers   XXX_hide_headers�����ص���ͻ������hide_headerΪ׼����ngx_http_upstream_hide_headers_hash
    ngx_array_t                     *pass_headers; // proxy_hide_header  fastcgi_hide_header

    ngx_http_upstream_local_t       *local;//�������η�����ʱ���õı�����ַ //proxy_bind  fastcgi_bind ���õı���IP�˿ڵ�ַ���п����豸�кü���eth��ֻ������һ��

#if (NGX_HTTP_CACHE)
//xxx_cache(proxy_cache fastcgi_cache) abc����xxx_cache_path(proxy_cache_path fastcgi_cache_path) xxx keys_zone=abc:10m;һ�𣬷�����ngx_http_proxy_merge_loc_conf��ʧ�ܣ���Ϊû��Ϊ��abc����ngx_http_file_cache_t
//������õ�proxy_cache xxx�в�������������cycle->shared_memory�л�ȡһ��ngx_shm_zone_t����ֵ��ע���������zone�ṹֻ�����֣�û��ֱ�ﳤ�� ��ngx_http_proxy_cache
//fastcgi_cache ָ��ָ�����ڵ�ǰ��������ʹ���ĸ�����ά��������Ŀ��������Ӧ�Ļ������������ fastcgi_cache_path ָ��塣 
    ngx_shm_zone_t                  *cache_zone; //���ֻ����xxx_cache abc(proxy fascgi_cache)��ngx_shm_zone_t->dataΪNULL������xxx_cache_path�ٴ������¸�abc����������
//���proxy_cache xxx$ss�����д��б����������õ�value�ַ���������cache_value�У���ngx_http_proxy_cache   
    ngx_http_complex_value_t        *cache_value; //����proxy_cache_path ��ngx_http_upstream_cache_get
    //Proxy_cache_min_uses number Ĭ��Ϊ1�����ͻ��˷�����ͬ����ﵽ�涨������nginx�Ŷ���Ӧ���ݽ��л��棻
    ngx_uint_t                       cache_min_uses; //cache_min_uses
    //nginx��ʱ�Ӵ��������ṩһ�����ڵ���Ӧ���������ngx_http_upstream_cache�Ķ�
    /*
�������������fastcgi_cache_use_stale updating����ʾ˵��Ȼ�û����ļ�ʧЧ�ˣ��Ѿ��������ͻ��������ڻ�ȡ������ݣ����Ǹÿͻ����������ڻ�û�л�ȡ������
��ʱ��Ϳ��԰���ǰ���ڵĻ��淢�͸���ǰ����Ŀͻ��� //�������ngx_http_upstream_cache�Ķ�
*/
    ngx_uint_t                       cache_use_stale; //XXX_cache_use_stale(proxy fastcgi_cache_use_stale����)
    //proxy |fastcgi _cache_methods  POST GET HEAD; ��ֵΪλ��������ngx_http_upstream_cache_method_mask��NGX_HTTP_HEAD��
    ngx_uint_t                       cache_methods;//Ĭ��  proxy_cache_methods GET HEAD; 


/*
When enabled, only one request at a time will be allowed to populate a new cache element identified according to the proxy_cache_key 
directive by passing a request to a proxied server. Other requests of the same cache element will either wait for a response to appear 
in the cache or the cache lock for this element to be released, up to the time set by the proxy_cache_lock_timeout directive. 


�����Ҫ���һ������: //proxy_cache_lock Ĭ��off 0  //proxy_cache_lock_timeout ���ã�Ĭ��5S
���������������ͻ��ˣ�һ���ͻ������ڻ�ȡ������ݣ����Һ�˷�����һ���֣���nginx�Ỻ����һ���֣����ҵȴ����к�����ݷ��ؼ������档
�����ڻ���Ĺ���������ͻ���2ҳ������ȥͬ��������uri�ȶ�һ�������ȥ���ͻ��˻���һ������ݣ���ʱ��Ϳ���ͨ�������������������⣬
Ҳ���ǿͻ���1��û������ȫ�����ݵĹ����пͻ���2ֻ�еȿͻ���1��ȡ��ȫ��������ݣ����߻�ȡ��proxy_cache_lock_timeout��ʱ����ͻ���2ֻ�дӺ�˻�ȡ����
*/ //�ο�http://blog.csdn.net/brainkick/article/details/8583335
    ngx_flag_t                       cache_lock;//proxy_cache_lock Ĭ��off 0
    ngx_msec_t                       cache_lock_timeout;//proxy_cache_lock_timeout ���ã�Ĭ��5S
    ngx_msec_t                       cache_lock_age;

    ngx_flag_t                       cache_revalidate;

    /*
�﷨��proxy_cache_valid reply_code [reply_code ...] time;  proxy_cache_valid  200 302 10m; 
proxy_cache_valid  301 1h;  proxy_cache_valid  any 1m; 
*/
    ngx_array_t                     *cache_valid; //���ո�ֵ��ngx_http_cache_t->valid_sec
    
    //xxx_cache_bypass  xx1 xx2���õ�xx2��Ϊ�ջ��߲�Ϊ0���򲻻�ӻ�����ȡ������ֱ�ӳ��˶�ȡ
    //xxx_no_cache  xx1 xx2���õ�xx2��Ϊ�ջ��߲�Ϊ0�����˻��������ݲ��ᱻ����

     //ngx_http_set_predicate_slot���� xxx_cache_bypass  xx1 xx2�е�xx1 xxx2��no_cache������
    //proxy_cache_bypass fastcgi_cache_bypass ����ngx_http_set_predicate_slot��ֵ,��ngx_http_test_predicates����
    ngx_array_t                     *cache_bypass;
    //ngx_http_set_predicate_slot���� xxx_no_cache  xx1 xx2�е�xx1 xxx2��no_cache��������ngx_http_test_predicates����
    ngx_array_t                     *no_cache;
#endif

/*
��ngx_http_upstream_t�е�store��־λΪ1ʱ�������Ҫ�����ε���Ӧ��ŵ��ļ��У�store_lengths����ʾ���·���ĳ��ȣ���store_values��ʾ���·��
*/
    ngx_array_t                     *store_lengths;
    ngx_array_t                     *store_values;

#if (NGX_HTTP_CACHE) //fastcgi_store��fastcgi_cacheֻ����������һ������������
    signed                           cache:2; //fastcgi_cache off��ֵΪ0 ����Ϊ1����ngx_http_fastcgi_cache
#endif
//xxx_store(����scgi_store)  on | off |path   ֻҪ����off,store��Ϊ1����ֵ��ngx_http_fastcgi_store
//�ƶ��˴洢ǰ���ļ���·��������onָ���˽�ʹ��root��aliasָ����ͬ��·����off��ֹ�洢�����⣬�����п���ʹ�ñ���ʹ·��������ȷ��fastcgi_store /data/www$original_uri;
    signed                           store:2;//��ĿǰΪֹ��store��־λ��������ngx_http_upstream_t�е�store��ͬ����ֻ��o��1��ʹ�õ�
/*
�����intercept_errors��־λ������400���ϵĴ����뽫����error_page�ȽϺ����д���ʵ������������ǿ�����һ����������ģ������intercept_404
��־λ��Ϊ1�������η���404ʱ��ֱ��ת���������������Σ�������ȥ��error_page���бȽ�
*/
    unsigned                         intercept_404:1;
/*
���ñ�־λΪ1ʱ���������ngx_http_upstream_t��headers_in�ṹ�����"X-Accel-Buffering"ͷ��������ֵ����yes��no�����ı�buffering
��־λ������ֵΪyesʱ��buffering��־λΪ1����ˣ�change_bufferingΪ1ʱ���п��ܸ������η��������ص���Ӧͷ������̬�ؾ���������
���������Ȼ�����������������
*/
    unsigned                         change_buffering:1;

#if (NGX_HTTP_SSL)
    ngx_ssl_t                       *ssl;
    ngx_flag_t                       ssl_session_reuse;

    ngx_http_complex_value_t        *ssl_name;
    ngx_flag_t                       ssl_server_name;
    ngx_flag_t                       ssl_verify;
#endif

    ngx_str_t                        module; //ʹ��upstream��ģ�����ƣ������ڼ�¼��־
} ngx_http_upstream_conf_t; 

//ngx_http_upstream_headers_in�еĸ�����Ա
typedef struct {
    ngx_str_t                        name;
    //ִ��ngx_http_upstream_headers_in�еĸ�����Ա��handler����
    ngx_http_header_handler_pt       handler; //��ngx_http_fastcgi_process_header mytest_upstream_process_header��ִ��  
    ngx_uint_t                       offset;
    ngx_http_header_handler_pt       copy_handler; //ngx_http_upstream_process_headers��ִ��
    ngx_uint_t                       conf;
    ngx_uint_t                       redirect;  /* unsigned   redirect:1; */ //��ngx_http_upstream_process_headers
} ngx_http_upstream_header_t;


//�ο�mytest_upstream_process_header->ngx_http_parse_header_line
//ngx_http_upstream_headers_in
typedef struct { //���������Ӧ�������ͷ����Ϣ
    ngx_list_t                       headers; //ngx_list_init(&u->headers_in.headers���г�ʼ���������洢ͷ����Ϣ

    //��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line // HTTP/1.1 200 OK ��Ӧ�е�200
     //������η�����Ӧ�������fastcgi��ʽͷ������û�г���"location"�����ʾ����Ҫ�ض���u->headers_in.status_n = 200;
     //��˷���"location"��ֵΪ302�ض��򣬷���ֵδ200
    ngx_uint_t                       status_n;// HTTP/1.1 200 OK ��Ӧ�е�200  Ҳ���Ǻ����status��Ӧ��valueֵ��������ʽ ngx_http_fastcgi_process_header

    //��������ΪHTTP/1.1 200 OK �е�"200 OK "  Ҳ���Ǻ����status��Ӧ��value.data�ַ�������ngx_http_fastcgi_process_header
    //������η�����Ӧ�������fastcgi��ʽͷ������û�г���"location"�����ʾ����Ҫ�ض���ngx_str_set(&u->headers_in.status_line, "200 OK");
    ngx_str_t                        status_line; //��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line // HTTP/1.1 200 OK ��Ӧ�е�200

    ngx_table_elt_t                 *status;//��mytest_process_status_line��ֵ��Դͷ��ngx_http_parse_status_line ��
    ngx_table_elt_t                 *date;/* ngx_http_proxy_process_header˵����ͻ��˷��͵�ͷ�����б������server:  date: ��������û�з����������ֶΣ���nginx����ӿ�value��������ͷ���� */
    ngx_table_elt_t                 *server;/* ngx_http_proxy_process_header˵����ͻ��˷��͵�ͷ�����б������server:  date: ��������û�з����������ֶΣ���nginx����ӿ�value��������ͷ���� */
    ngx_table_elt_t                 *connection;

    ngx_table_elt_t                 *expires;
    /*
     ETag��һ��������Web��Դ�����ļǺţ�token�������͵�Web��Դ����һ��Webҳ����Ҳ������JSON��XML�ĵ������������������жϼǺ���ʲô
     ���京�壬����HTTP��Ӧͷ�н��䴫�͵��ͻ��ˣ������Ƿ������˷��صĸ�ʽ��ETag:"50b1c1d4f775c61:df3"�ͻ��˵Ĳ�ѯ���¸�ʽ������
     �ģ�If-None-Match : W / "50b1c1d4f775c61:df3"���ETagû�ı䣬�򷵻�״̬304Ȼ�󲻷��أ���Ҳ��Last-Modifiedһ��������Etag��Ҫ
     �ڶϵ�����ʱ�Ƚ����á� "etag:XXX" ETagֵ�ı��˵����Դ״̬�Ѿ����޸�
     */ //etag���ü�ngx_http_set_etag
    ngx_table_elt_t                 *etag; //"etag:XXX" ETagֵ�ı��˵����Դ״̬�Ѿ����޸�
    ngx_table_elt_t                 *x_accel_expires;
    //���ͷ����ʹ����X-Accel-Redirect���ԣ�Ҳ���������ļ������ԣ�������������ļ����ء����ض���
/*
x_accel_redirect��ͷ�������⴦�����ͷ��Ҫ��nginx�ṩ��һ�ֻ��ƣ��ú�˵�server�ܹ����Ʒ���Ȩ�ޡ�����������ĳ��ҳ�治�ܱ�
�û����ʣ���ô���û��������ҳ���ʱ�򣬺��serverֻ��Ҫ����X-Accel-Redirect���ͷ��һ��·����Ȼ��nginx����������·�������ݸ��û�.
*/
    ngx_table_elt_t                 *x_accel_redirect;
    ngx_table_elt_t                 *x_accel_limit_rate;

    ngx_table_elt_t                 *content_type;
    ngx_table_elt_t                 *content_length;

    ngx_table_elt_t                 *last_modified;
    //������η��ز���Ҫ�ض�����status_line = "200 OK"
    ngx_table_elt_t                 *location; //�����η��ص���Ӧ����Location����Refreshͷ����ʾ�ض��� ��ngx_http_upstream_headers_in�е�"location"
    ngx_table_elt_t                 *accept_ranges;
    ngx_table_elt_t                 *www_authenticate;
    ngx_table_elt_t                 *transfer_encoding;
    ngx_table_elt_t                 *vary;

#if (NGX_HTTP_GZIP)
    ngx_table_elt_t                 *content_encoding;
#endif

    ngx_array_t                      cache_control;
    ngx_array_t                      cookies;

    off_t                            content_length_n; //�����ͨ��chunk���뷽ʽ���Ͱ��壬����岻��Я��content-length:ͷ���У�����ͨ��chunk���ͷ������
    time_t                           last_modified_time; //���Я����ͷ����"Last-Modified:XXX"��ֵ����ngx_http_upstream_process_last_modified

    //����ͺ�˵�tcp����ʹ��HTTP1.1���°汾�������1����ngx_http_proxy_process_status_line
    unsigned                         connection_close:1;
    //chunked���뷽ʽ�Ͳ���Ҫ����content-length: ͷ���У����峤����chunked���ĸ�ʽָ���������ݳ��ȣ��ο�ngx_http_proxy_process_header
    unsigned                         chunked:1; //�Ƿ���chunk���뷽ʽ  transfer-encoding:chunked
} ngx_http_upstream_headers_in_t;

//resolved�ṹ�壬�����������η������ĵ�ַ
typedef struct { //�����ռ�͸�ֵ��ngx_http_fastcgi_eval
    ngx_str_t                        host; //sockaddr��Ӧ�ĵ�ַ�ַ���,��a.b.c.d
    in_port_t                        port; //�˿�
    ngx_uint_t                       no_port; /* unsigned no_port:1 */

    ngx_uint_t                       naddrs; //��ַ������
    ngx_addr_t                      *addrs; 

    struct sockaddr                 *sockaddr; //���η������ĵ�ַ
    socklen_t                        socklen; //sizeof(struct sockaddr_in);

    ngx_resolver_ctx_t              *ctx;
} ngx_http_upstream_resolved_t;


typedef void (*ngx_http_upstream_handler_pt)(ngx_http_request_t *r,
    ngx_http_upstream_t *u);

/*
upstream��3�ִ���������Ӧ����ķ�ʽ����HTTPģ����θ���
upstreamʹ����һ�ַ�ʽ�������ε���Ӧ�����أ��������ngx_http_request_t�ṹ����
subrequest_in_memory��־λΪ1ʱ�������õ�1�ַ�ʽ����upstream��ת����Ӧ����
�����Σ���HTTPģ��ʵ�ֵ�input_filter����������壻��subrequest_in_memoryΪ0ʱ��
upstream��ת����Ӧ���塣��ngx_http_upstream_conf t���ýṹ���е�buffering��־λΪ1
����������������ڴ�ʹ����ļ����ڻ������ε���Ӧ���壬����ζ�������ٸ��죻��buffering
Ϊ0ʱ����ʹ�ù̶���С�Ļ�����������������ܵ�buffer����������ת����Ӧ���塣
    ע��  ������8���ص������У�ֻ��create_request��process_header��finalize_request
�Ǳ���ʵ�ֵģ�����5���ص�����-input_filter init��input_filter��reinit_request��abort
request��rewrite redirect�ǿ�ѡ�ġ���12�»���ϸ�������ʹ����5����ѡ�Ļص���������
�⣬
*/

/*
ngx_http_upstream_create��������ngx_http_upstream_t�ṹ�壬���еĳ�Ա����Ҫ����HTTPģ���������á�
����upstream����ʹ��ngx_http_upstream_init����
*/ //FastCGI memcached  uwsgi  scgi proxyģ���������ö����ڸýṹ��
//ngx_http_request_t->upstream �д�ȡ  //upstream��Դ������ngx_http_upstream_finalize_request
struct ngx_http_upstream_s { //�ýṹ�еĲ��ֳ�Ա�Ǵ�upstream{}�е������������(ngx_http_upstream_conf_t)��ȡ��
    //������¼��Ļص�������ÿһ���׶ζ��в�ͬ��read event handler
    //ע��ngx_http_upstream_t��ngx_http_request_t���иó�Ա �ֱ���ngx_http_request_handler��ngx_http_upstream_handler��ִ��
    //����ڶ�ȡ��˰����ʱ�����buffering��ʽ�����ڶ�ȡ��ͷ���кͲ��ְ���󣬻����Ϊngx_http_upstream_process_upstream��ʽ��ȡ��˰�������
    ////buffering��ʽ���������󣬺��ͷ����Ϣ�Ѿ���ȡ����ˣ������˻��а�����Ҫ���ͣ��򱾶�ͨ��ngx_http_upstream_process_upstream�÷�ʽ��ȡ
    //��buffering��ʽ���������󣬺��ͷ����Ϣ�Ѿ���ȡ����ˣ������˻��а�����Ҫ���ͣ��򱾶�ͨ��ngx_http_upstream_process_non_buffered_upstream��ȡ
    //����������󣬺��ͷ����Ϣ�Ѿ���ȡ����ˣ������˻��а�����Ҫ���ͣ��򱾶�ͨ��ngx_http_upstream_process_body_in_memory��ȡ
    ngx_http_upstream_handler_pt     read_event_handler; //ngx_http_upstream_process_header
    //����д�¼��Ļص�������ÿһ���׶ζ��в�ͬ��write event handler  
    //ע��ngx_http_upstream_t��ngx_http_request_t���иó�Ա �ֱ���ngx_http_request_handler��ngx_http_upstream_handler��ִ��
    ngx_http_upstream_handler_pt     write_event_handler; //ngx_http_upstream_send_request_handler�û����˷��Ͱ���ʱ��һ�η���û����ɣ��ٴγ���epoll write��ʱ�����

    //��ʾ���������η�������������ӡ� 
    ngx_peer_connection_t            peer;//��ʼ��ֵ��ngx_http_upstream_connect->ngx_event_connect_peer(&u->peer);

    /*
     �������οͻ���ת����Ӧʱ��ngx_http_request_t�ṹ���е�subrequest_in_memory��־סΪ0����������˻�������Ϊ�������ٸ��죨conf
     �����е�buffering��־λΪ1������ʱ��ʹ��pipe��Ա��ת����Ӧ����ʹ�����ַ�ʽת����Ӧʱ��������HTTPģ����ʹ��upstream����ǰ����
     pipe�ṹ�壬�����������ص�coredump����
     */ //ʵ����bufferingΪ1��ͨ��pipe���Ͱ��嵽�ͻ��������
    ngx_event_pipe_t                *pipe; //ngx_http_fastcgi_handler  ngx_http_proxy_handler�д����ռ�

    /* request_bufs��������ʲô������������η���������ʵ��create_request����ʱ��Ҫ������ 
    request_bufs������ķ�ʽ��ngx_buf_t��������������������ʾ������Ҫ���͵����η��������������ݡ�
    ���ԣ�HTTPģ��ʵ�ֵ�create_request�ص����������ڹ���reque st_bufg����
    */ /* �������fastcgi_param�����Ϳͻ�������ͷkey����һ��cl���ͻ��˰�������ռ��һ�����߶��cl������ͨ��next������һ������ǰ�����ӵ�u->request_bufs
        ������Ҫ������˵����ݾ���u->request_bufs���ˣ����͵�ʱ�������ȡ�������ɣ��ο�ngx_http_fastcgi_create_request*/
    /*
    ngx_http_upstream_s->request_bufs�İ�����ԴΪngx_http_upstream_init_request�����u->request_bufs = r->request_body->bufs;Ȼ����
    ngx_http_fastcgi_create_request�л����°ѷ�����˵�ͷ����Ϣ�Լ�fastcgi_param��Ϣ��ӵ�ngx_http_upstream_s->request_bufs��
    */ //�����η��Ͱ���u->request_bufs(ngx_http_fastcgi_create_request),���տͻ��˵İ�����r->request_body
    //�������η�����������ͷ���ݷ����buf    �ռ������ngx_http_proxy_create_request ngx_http_fastcgi_create_request
    ngx_chain_t                     *request_bufs; 

    //�����������η�����Ӧ�ķ�ʽ
    ngx_output_chain_ctx_t           output; //������ݵĽṹ���������Ҫ���͵����ݣ��Լ����͵�output_filterָ��
    ngx_chain_writer_ctx_t           writer; //�ο�ngx_chain_writer������Ὣ���bufһ�������ӵ���� writer��ֵ����u->output.filter_ctx����ngx_http_upstream_init_request
    //����ngx_output_chain��Ҫ���͵����ݶ���������Ȼ���ͣ�Ȼ������������ָ��ʣ�µĻ�û�е���writev���͵ġ�

    //upstream����ʱ�����������Բ�����
    /*
    conf��Ա������������upstreamģ�鴦������ʱ�Ĳ������������ӡ����͡����յĳ�ʱʱ��ȡ�
    ��ʵ�ϣ�HTTP�������ģ����nginx.conf�ļ����ṩ�������������������ngx_http_upstream_conf_t�ṹ���еĳ�Ա�ġ�
    �����г���3����ʱʱ��(connect_timeout  send_imeout read_timeout)�Ǳ���Ҫ���õģ���Ϊ����Ĭ��Ϊ0����������ý���Զ�޷������η�����������TCP���ӣ���Ϊconnect timeoutֵΪ0����
    */ //ʹ��upstream����ʱ�ĸ�������  ����fastcgi��ֵ��ngx_http_fastcgi_handler��ֵ������ngx_http_fastcgi_loc_conf_t->upstream
    ngx_http_upstream_conf_t        *conf; 
    
#if (NGX_HTTP_CACHE) //proxy_pache_cache����fastcgi_path_cache������ʱ��ֵ����ngx_http_file_cache_set_slot
    ngx_array_t                     *caches; //u->caches = &ngx_http_proxy_main_conf_t->caches;
#endif

    /*
     HTTPģ����ʵ��process_header����ʱ�����ϣ��upstreamֱ��ת����Ӧ������Ҫ�ѽ���������Ӧͷ������ΪHTTP����Ӧͷ����ͬʱ��Ҫ
     �Ѱ�ͷ�е���Ϣ���õ�headers_in�ṹ���У����������headers_in�����õ�ͷ����ӵ�Ҫ���͵����οͻ��˵���Ӧͷ��headers_out��
     */
    ngx_http_upstream_headers_in_t   headers_in;  //��Ŵ����η��ص�ͷ����Ϣ��

    //ͨ��resolved����ֱ��ָ�����η�������ַ�����ڽ�����������  �����͸�ֵ��ngx_http_xxx_eval(����ngx_http_fastcgi_eval ngx_http_proxy_eval)
    ngx_http_upstream_resolved_t    *resolved; //����������fastcgi_pass   127.0.0.1:9000;������ַ������ݣ������б����

    ngx_buf_t                        from_client;

    /*
    buffer��Ա�洢���������η�������������Ӧ���ݣ��������ᱻ���ã����Ծ������ж������壺
    a����ʹ��process_header��������������Ӧ�İ�ͷʱ��buffer�н��ᱣ����������Ӧ��ͷ��
    b���������buffering��ԱΪ1�����Ҵ�ʱupstream��������ת�����εİ���ʱ��bufferû�����壻
    c����buffering��־סΪ0ʱ��buffer�������ᱻ���ڷ����ؽ������εİ��壬����������ת����
    d����upstream��������ת�����ΰ���ʱ��buffer�ᱻ���ڷ����������εİ��壬HTTPģ��ʵ�ֵ�input_filter������Ҫ��ע��

    �������η�������Ӧ��ͷ�Ļ��������ڲ���Ҫ����Ӧֱ��ת�����ͻ��ˣ�����buffering��־λΪ0�������ת������ʱ�����հ���Ļ���
����Ȼʹ��buffer��ע�⣬���û���Զ���input_filter����������壬����ʹ��buffer�洢ȫ���İ��壬��ʱbuf fer�����㹻�����Ĵ�С
��ngx_http_upstream_conf_t�ṹ���е�buffer_size��Ա����
    */ //ngx_http_upstream_process_header�д����ռ�͸�ֵ��ͨ����buf����recv������� //buf��С��xxx_buffer_size(fastcgi_buffer_size proxy_buffer_size memcached_buffer_size)
//��ȡ���η��ص����ݵĻ�������Ҳ����proxy��FCGI���ص����ݡ���������httpͷ����Ҳ������body���֡���body���ֻ��event_pipe_t��preread_bufs�ṹ��Ӧ����������Ԥ����buf����ʵ��i��С�Ķ����ġ�
    //��buf�����ǽ���ͷ������Ϣ�ģ�����Ҳ���ܻ�Ѳ��ֻ���ȫ������(�������С��ʱ��)�յ���buf��
    ngx_buf_t                        buffer; //�����η��������յ������ڸ�buffer���������ε�����������request_bufs��
    //��ʾ�������η���������Ӧ����ĳ���    proxy���帳ֵ��ngx_http_proxy_input_filter_init
    off_t                            length; //Ҫ���͸��ͻ��˵����ݴ�С������Ҫ��ȡ��ô������� 

    /*
out_bufs�����ֳ������в�ͬ�����壺
�ٵ�����Ҫת�����壬��ʹ��Ĭ�ϵ�input_filter������Ҳ����ngx_http_upstream_non_buffered_filter�������������ʱ��out bufs����ָ����Ӧ���壬
��ʵ�ϣ�out bufs�����л�������ngx_buf_t��������ÿ����������ָ��buffer�����е�һ���֣��������һ���־���ÿ�ε���recv�������յ���һ��TCP����
�ڵ���Ҫת����Ӧ���嵽����ʱ��buffering��־λΪO�����������������ȣ����������ָ����һ��������ת����Ӧ���������ʱ���ڽ��������εĻ�����Ӧ
     */
    ngx_chain_t                     *out_bufs;
    /*
    ����Ҫת����Ӧ���嵽����ʱ��buffering��־λΪo�����������������ȣ�������ʾ��һ��������ת����Ӧʱû�з����������
     */
    ngx_chain_t                     *busy_bufs;//������ngx_http_output_filter������out_bufs�����������ƶ��������������Ϻ󣬻��ƶ���free_bufs
    /*
    ����������ڻ���out_bufs���Ѿ����͸����ε�ngx_buf_t�ṹ�壬��ͬ��Ӧ����buffering��־λΪ0���������������ȵĳ���
     */
    ngx_chain_t                     *free_bufs;//���еĻ����������Է���

/*
input_filter init��input_filter�ص�����
    input_filter_init��input_filter���������������ڴ������ε���Ӧ���壬��Ϊ�������
ǰHTTPģ�������Ҫ��һЩ��ʼ�����������磬����һЩ�ڴ����ڴ�Ž������м�״̬
�ȣ���ʱupstream���ṩ��input_filter_init��������input_filter��������ʵ�ʴ�������
�������������ص�����������ѡ����ʵ�֣�������Ϊ��������������ʵ��ʱ��upstream
ģ����Զ���������ΪԤ�÷��������Ľ���������upstream��3�ִ������ķ�ʽ������
upstreamģ��׼����3��input_filter_init��input_filter����������ˣ�һ����ͼ�ض���mput_
filter init��input_filter����������ζ�����Ƕ�upstreamģ���Ĭ��ʵ���ǲ�����ģ����Բ�
Ҫ�ض���ù��ܡ�
    �ڶ�������£��������³�����������ʵ��input_filter������
    (1)��ת��������Ӧ�����ε�ͬʱ����Ҫ��һЩ���⴦��
    ���磬ngx_http_memcached_ moduleģ��Ὣʵ����memcachedʵ�ֵ����η���������
����Ӧ���壬ת�������ε�HTTP�ͻ����ϡ������������У���ģ��ͨ���ض����˵�input_
filter���������memcachedЭ���°���Ľ�������������ȫ�������͸��TCP����
    (2)���������ϡ����μ�ת����Ӧʱ��������ȴ�������ȫ����������Ӧ��ſ�ʼ����
����
    �ڲ�ת����Ӧʱ��ͨ���Ὣ��Ӧ���������ڴ��н����������ͼ���յ���������Ӧ��
����������������Ӧ���ܻ�ǳ������ռ�ô����ڴ档���ض�����input_filter�����󣬿�
��ÿ������һ���ְ��壬���ͷ�һЩ�ڴ档
    �ض���input_filter�����������һЩ����������ȡ���ս��յ��İ����Լ�������Ż�
����ʹ�ù̶���С���ڴ滺���������ظ�ʹ�õȡ�ע�⣬���µ����Ӳ����漰input_filter��
�������߿����ڵ�12�����ҵ�input_filter������ʹ�÷�ʽ��
*/
//�������ǰ�ĳ�ʼ������������data�������ڴ����û����ݽṹ����ʵ���Ͼ��������input_filter_ctxָ��  
//ngx_http_XXX_input_filter_init(��ngx_http_fastcgi_input_filter_init ngx_http_proxy_input_filter_init ngx_http_proxy_input_filter_init)  
    ngx_int_t                      (*input_filter_init)(void *data); 
    
/* �������ķ���������data�������ڴ����û����ݽṹ����ʵ���Ͼ��������input_filter_ctxָ�룬��bytes��ʾ���ν��յ��İ��峤�ȡ�
����NGX ERRORʱ��ʾ����������������Ҫ���������򶼽�����upstream����

������ȡ��˵����ݣ���bufferingģʽ��ngx_http_upstream_non_buffered_filter��ngx_http_memcached_filter�ȡ���������ĵ���ʱ��: 
ngx_http_upstream_process_non_buffered_upstream�ȵ���ngx_unix_recv���յ�upstream���ص����ݺ�͵����������Э��ת��������Ŀǰת�����ࡣ
*/ //buffering�����Ӧ����ʹ��ngx_event_pipe_t->input_filter  ��buffering��ʽ��Ӧ��˰���ʹ��ngx_http_upstream_s->input_filter ,��ngx_http_upstream_send_response�ֲ�
    ngx_int_t                      (*input_filter)(void *data, ssize_t bytes); //ngx_http_xxx_non_buffered_filter(��ngx_http_fastcgi_non_buffered_filter ngx_http_proxy_non_buffered_copy_filter)
//���ڴ���HTTPģ���Զ�������ݽṹ����input_filter_init��input_filter�������ص�ʱ����Ϊ�������ݹ�ȥ
    void                            *input_filter_ctx;//ָ�������������������

#if (NGX_HTTP_CACHE)
/*
Ngx_http_fastcgi_module.c (src\http\modules):    u->create_key = ngx_http_fastcgi_create_key;
Ngx_http_proxy_module.c (src\http\modules):    u->create_key = ngx_http_proxy_create_key;
*/ //ngx_http_upstream_cache��ִ��
    ngx_int_t                      (*create_key)(ngx_http_request_t *r);
#endif
    //���췢�����η���������������
    /*
    create_request�ص�����
    create_request�Ļص�������򵥣�����ֻ���ܱ�����1�Σ����������upstream��
ʧ�����Ի��ƵĻ�����
    1)��Nginx��ѭ�����������ѭ����ָngx_worker_process_cycle�������У��ᶨ�ڵص����¼�ģ�飬�Լ���Ƿ��������¼�������
    2)�¼�ģ���ڽ��յ�HTTP���������HTIP���������������ա�������HTTPͷ������Ӧ����mytestģ�鴦����ʱ�����mytestģ
    ���ngx_http_mytest_handler������
    4)����ngx_http_up stream_init��������upstream��
    5) upstreamģ���ȥ����ļ����棬����������Ѿ��к��ʵ���Ӧ�������ֱ�ӷ��ػ��棨��Ȼ��������ʹ�÷�������ļ������ǰ���£���
    Ϊ���ö��߷�������upstream���ƣ����½������ἰ�ļ����档
    6)�ص�mytestģ���Ѿ�ʵ�ֵ�create_request�ص�������
    7) mytestģ��ͨ������r->upstream->request_bufs�Ѿ������÷���ʲô�����������η�������
    8) upstreamģ�齫����resolved��Ա�������resolved��Ա�Ļ����͸��������ú����η������ĵ�ַr->upstream->peer��Ա��
    9)����������TCP�׽��ֽ������ӡ�
    10)���������Ƿ����ɹ������������ӵ�connect�����������̷��ء�
    II) ngx_http_upstreamL init���ء�
    12) mytestģ���ngx_http_mytest_handler��������NGX DONE��
    13)���¼�ģ�鴦�������������¼��󣬽�����Ȩ������Nginx��ѭ����
    */ //���ﶨ���mytest_upstream_create_request�������ڴ������͸����η�������HTTP����upstreamģ�齫��ص��� 
    //��ngx_http_upstream_init_request��ִ��  HTTPģ��ʵ�ֵ�ִ��create_request�������ڹ��췢�����η�����������
    //ngx_http_xxx_create_request(����ngx_http_fastcgi_create_request)
    ngx_int_t                      (*create_request)(ngx_http_request_t *r);//���ɷ��͵����η����������󻺳壨����һ����������

/*
reinit_request���ܻᱻ��λص����������õ�ԭ��ֻ��һ���������ڵ�һ����ͼ�����η�������������ʱ������������ڸ����쳣ԭ��ʧ�ܣ�
��ô�����upstream��conf�����Ĳ���Ҫ���ٴ��������η�����������ʱ�ͻ����reinit_request�����ˡ�ͼ5-4�����˵��͵�reinit_request���ó�����
����򵥵ؽ���һ��ͼ5-4���г��Ĳ��衣
    1) Nginx��ѭ���лᶨ�ڵص����¼�ģ�飬����Ƿ��������¼�������
    2)�¼�ģ����ȷ�������η�������TCP���ӽ����ɹ��󣬻�ص�upstreamģ�����ط�������
    3) upstream�����ʱ���r->upstream->request_sent��־λ��Ϊl����ʾ�����Ѿ������ɹ��ˣ����ڿ�ʼ�����η����������������ݡ�
    4)�����������η�������
    5)���ͷ�����Ȼ���������ģ�ʹ�������������׽��֣��������̷��ء�
    6) upstreamģ�鴦���2���е�TCP���ӽ����ɹ��¼���
    7)�¼�ģ�鴦���걾�������¼��󣬽�����Ȩ������Nginx��ѭ����
    8) Nginx��ѭ���ظ���1���������¼�ģ���������¼���
    9)��ʱ��������������η�����������TCP�����Ѿ��쳣�Ͽ�����ô�¼�ģ���֪ͨupstreamģ�鴦������
    10)�ڷ������Դ�����ǰ���£�upstreamģ��������ԥ���ٴ������������׽�����ͼ�������ӡ�
    11)���������Ƿ����ɹ������̷��ء�
    12)��ʱ���r->upstream->request_sent��־λ���ᷢ�����Ѿ�����Ϊ1�ˡ�
    13)���mytestģ��û��ʵ��reinit_request��������ô�ǲ���������ġ������reinit_request��ΪNULL��ָ�룬�ͻ�ص�����
    14) mytestģ����reinit_request�д������Լ������顣
    15)�����ܵ�9���е�TCP���ӶϿ��¼���������Ȩ�������¼�ģ�顣
    16)�¼�ģ�鴦���걾�������¼��󣬽�������Ȩ��Nginx��ѭ����
*/ //�����η�������ͨ��ʧ�ܺ�����������Թ�����Ҫ�ٴ������η������������ӣ�������reinit_request����
    //�����upstream�ص�ָ���Ǹ���ģ�����õģ�����ngx_http_fastcgi_handler����������fcgi����ػص�������
    //ngx_http_XXX_reinit_request(ngx_http_fastcgi_reinit_request) //��ngx_http_upstream_reinit��ִ��
    ngx_int_t                      (*reinit_request)(ngx_http_request_t *r);//�ں�˷����������õ�����£���create_request���ڶ��ε���֮ǰ��������

/*
�յ����η���������Ӧ��ͻ�ص�process_header���������process_header����NGXAGAIN����ô���ڸ���upstream��û���յ���������Ӧ��ͷ��
��ʱ�����ӱ���upstream������˵���ٴν��յ����η�����������TCP��ʱ���������process_header��������ֱ��process_header��������
��NGXAGAINֵ��һ�׶βŻ�ֹͣ

process_header�ص�����process_header�����ڽ������η��������صĻ���TCP����Ӧͷ���ģ���ˣ�process_header���ܻᱻ��ε��ã�
���ĵ��ô�����process_header�ķ���ֵ�йء���ͼ5-5��ʾ�����process_header����NGX_AGAIN������ζ�Ż�û�н��յ���������Ӧͷ����
����ٴν��յ����η�����������TCP���������������ͷ������Ȼ����process_header��������ͼ5-6�У����process_header����NGX_OK
������������NGX_AGAIN��ֵ������ô��������ӵĺ��������н������ٴε���process_header��
 process header�ص�����������ͼ
����򵥵ؽ���һ��ͼ5-5���г��Ĳ��衣
    1) Nginx��ѭ���лᶨ�ڵص����¼�ģ�飬����Ƿ��������¼�������
    2)�¼�ģ����յ����η�������������Ӧʱ����ص�upstreamģ�鴦��
    3) upstreamģ����ʱ���Դ��׽��ֻ������ж�ȡ���������ε�TCP����
    4)��ȡ����Ӧ���ŵ�r->upstream->bufferָ����ڴ��С�ע�⣺��δ��������Ӧͷ��ǰ������ν��յ��ַ��������н��������ε�
    ��Ӧ���������ش�ŵ�r->upstream->buffer�������С���ˣ��ڽ���������Ӧ��ͷʱ�����buffer������ȫ��ȴ��û�н�������������Ӧ
    ͷ����Ҳ����˵��process_header -ֱ�ڷ���NGX_AGAIN������ô����ͻ����
    5)����mytestģ��ʵ�ֵ�process_header������
    6) process_header����ʵ���Ͼ����ڽ���r->upstream->buffer����������ͼ����ȡ����������Ӧͷ������Ȼ��������η�������Nginxͨ��HTTPͨ�ţ�
    ���ǽ��յ�������HTTPͷ������
    7)���process_header����NGX AGAIN����ô��ʾ��û�н�������������Ӧͷ�����´λ������process_header������յ���������Ӧ��
    8)����Ԫ�����Ķ�ȡ�׽��ֽӿڡ�
    9)��ʱ�п��ܷ����׽��ֻ������Ѿ�Ϊ�ա�
    10)����2���еĶ�ȡ������Ӧ�¼�������Ϻ󣬿���Ȩ�������¼�ģ�顣
    11)�¼�ģ�鴦���걾�������¼��󣬽�������Ȩ��Nginx��ѭ����
*/ //ngx_http_upstream_process_header��ngx_http_upstream_cache_send�����е���
/*
�������η�����������Ӧ�İ�ͷ������NGX_AGAIN��ʾ��ͷ��û�н�������������NGX_HTTP_UPSTREAM_INVALID_HEADER��ʾ��ͷ���Ϸ�������
NGX ERROR��ʾ���ִ��󣬷���NGX_OK��ʾ�����������İ�ͷ
*/ //ngx_http_fastcgi_process_header  ngx_http_proxy_process_status_line->ngx_http_proxy_process_status_line(ngx_http_XXX_process_header) //��ngx_http_upstream_process_header��ִ��
    ngx_int_t                      (*process_header)(ngx_http_request_t *r); //�������η������ظ��ĵ�һ��bit��ʱ���Ǳ���һ��ָ�����λظ����ص�ָ��
    void                           (*abort_request)(ngx_http_request_t *r);//�ڿͻ��˷��������ʱ�򱻵��� ngx_http_XXX_abort_request
   
/*
������ngx_http_upstream_init����upstream���ƺ��ڸ���ԭ�����۳ɹ�����ʧ�ܣ����¸���������ǰ�������finalize_request��
�����μ�ͼ5-1������finalize_request�����п��Բ����κ����飬������ʵ��finalize_request����������Nginx����ֿ�ָ����õ����ش���

���������ʱ������ص�finalize_request�������������ϣ����ʱ�ͷ���Դ�����
�ľ���ȣ�����ô���԰������Ĵ�����ӵ�finalize_request�����С������ж�����mytest_
upstream_finalize_request��������������û���κ���Ҫ�ͷŵ���Դ�����Ը÷���û�������
��ʵ�ʹ�����ֻ����Ϊupstreamģ��Ҫ�����ʵ��finalize_request�ص�����
*/ //����upstream����ʱ����  ngx_http_XXX_finalize_request  //��ngx_http_upstream_finalize_request��ִ��  ngx_http_fastcgi_finalize_request
    void                           (*finalize_request)(ngx_http_request_t *r,
                                         ngx_int_t rc); //�������ʱ����� //��Nginx��ɴ����η���������ظ��Ժ󱻵���
/*
���ض���URL�׶Σ����ʵ����rewrite_redirect�ص���������ô��ʱ�����rewrite_redirect��
���Բ鿴upstreamģ���ngx_http_upstream_rewrite_location���������upstreamģ����յ�������������Ӧͷ����
������HTTPģ���process_header�ص��������������Ķ�Ӧ��Location��ͷ�����õ���ngx_http_upstream_t�е�headers in��Աʱ��
ngx_http_upstream_process_headers�����������յ���rewrite��redirect����
��ˣ�rewrite_ redirect��ʹ�ó����Ƚ��٣�����ҪӦ����HTTP�������ģ��(ngx_http_proxy_module)�� ��ֵΪngx_http_proxy_rewrite_redirect
*/ 
//�����η��ص���Ӧ����Location����Refreshͷ����ʾ�ض���ʱ����ͨ��ngx_http_upstream_process_headers�������õ�����HTTPģ��ʵ�ֵ�rewrite redirect����
    ngx_int_t                      (*rewrite_redirect)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h, size_t prefix);//ngx_http_upstream_rewrite_location��ִ��
    ngx_int_t                      (*rewrite_cookie)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h);

    ngx_msec_t                       timeout;
    //���ڱ�ʾ������Ӧ�Ĵ����롢���峤�ȵ���Ϣ
    ngx_http_upstream_state_t       *state; //��r->upstream_states�����ȡ����ngx_http_upstream_init_request

    //��ʹ��cache��ʱ��ngx_http_upstream_cache������
    ngx_str_t                        method; //��ʹ���ļ�����ʱû������ //GET,HEAD,POST
    //schema��uri��Ա���ڼ�¼��־ʱ���õ�����������û������
    ngx_str_t                        schema; //����ǰ���http,https,mecached://  fastcgt://(ngx_http_fastcgi_handler)�ȡ�
    ngx_str_t                        uri;

#if (NGX_HTTP_SSL)
    ngx_str_t                        ssl_name;
#endif
    //Ŀǰ�������ڱ�ʾ�Ƿ���Ҫ������Դ���൱��һ����־λ��ʵ�ʲ�����õ�����ָ��ķ���
    ngx_http_cleanup_pt             *cleanup;
    //�Ƿ�ָ���ļ�����·���ı�־λ 
    //xxx_store(����scgi_store)  on | off |path   ֻҪ����off,store��Ϊ1����ֵ��ngx_http_fastcgi_store
//�ƶ��˴洢ǰ���ļ���·��������onָ���˽�ʹ��root��aliasָ����ͬ��·����off��ֹ�洢�����⣬�����п���ʹ�ñ���ʹ·��������ȷ��fastcgi_store /data/www$original_uri;
    unsigned                         store:1; //ngx_http_upstream_init_request��ֵ
    //���Ӧ��������ngx_http_upstream_process_request->ngx_http_file_cache_update�н��л���  
    //ngx_http_test_predicates���ڿ��Լ��xxx_no_cache,�Ӷ������Ƿ���Ҫ���������� 

   /*���Cache-Control����ֵΪno-cache��no-store��private������һ��ʱ���򲻻���...������...  ���Я����"x_accel_expires:0"ͷ  �ο�http://blog.csdn.net/clh604/article/details/9064641
    ����Ҳ������0���ο�ngx_http_upstream_process_accel_expires����������ͨ��fastcgi_ignore_headers������Щͷ�����Ӷ����Լ�������*/ 
    //���⣬���û��ʹ��fastcgi_cache_valid proxy_cache_valid ������Чʱ�䣬��Ĭ�ϻ��cacheable��0����ngx_http_upstream_send_response
    unsigned                         cacheable:1; //�Ƿ������ļ����� �ο�http://blog.csdn.net/clh604/article/details/9064641
    unsigned                         accel:1;
    unsigned                         ssl:1; //�Ƿ����SSLЭ��������η�����
#if (NGX_HTTP_CACHE)
    unsigned                         cache_status:3; //NGX_HTTP_CACHE_BYPASS ��
#endif

    /*
    upstream��3�ִ���������Ӧ����ķ�ʽ����HTTPģ����θ���upstreamʹ����һ�ַ�ʽ�������ε���Ӧ�����أ�
    �������ngx_http_request_t�ṹ����subrequest_in_memory��־λΪ1ʱ�������õ�1�ַ�ʽ����upstream��ת����Ӧ���嵽���Σ���HTTPģ
        ��ʵ�ֵ�input_filter����������壻
    ��subrequest_in_memoryΪ0ʱ��upstream��ת����Ӧ���塣
    ��ngx_http_upstream_conf t���ýṹ���е�buffering��־λΪ1ʱ��������������ڴ�ʹ����ļ����ڻ������ε���Ӧ���壬����ζ�������ٸ��죻
        ����buffer���FCGI�����������ݣ��ȴﵽһ����������buffer�����ٴ��͸����տͻ���
    ��bufferingΪ0ʱ����ʹ�ù̶���С�Ļ�����������������ܵ�buffer����������ת����Ӧ���塣
    
    ����ͻ���ת�����η������İ���ʱ�����á�
    ��bufferingΪ1ʱ����ʾʹ�ö���������Լ������ļ���ת�����ε���Ӧ���塣
    ��Nginx�����μ������Զ����Nginx�����οͻ��˼������ʱ����Nginx���ٸ�����ڴ�����ʹ�ô����ļ����������ε���Ӧ���壬
        ����������ģ������Լ������η������Ĳ���ѹ����
    ��bufferingΪ0ʱ����ʾֻʹ���������һ��buffer��������������ת����Ӧ���� �����ν��ն��پ������η��Ͷ��٣������棬�������η��������������������
    */ //fastcgi��ֵ��ngx_http_fastcgi_handler u->buffering = flcf->upstream.buffering; //��xxx_buffering��fastcgi_buffering  �Ƿ񻺴��˷�����Ӧ������İ���
    //�ò���Ҳ����ͨ����˷��ص�ͷ���ֶ�: X-Accel-Buffering:no | yes�������Ƿ�������ngx_http_upstream_process_buffering
    /*
     ����������壬��ôNginx�������ܶ�ض�ȡ��˷���������Ӧ���ݣ��ȴﵽһ����������buffer�����ٴ��͸����տͻ��ˡ�����رգ�
     ��ôNginx�����ݵ���ת����һ��ͬ���Ĺ��̣����Ӻ�˷��������յ���Ӧ���ݾ��������䷢�͸��ͻ��ˡ�
     */ //buffering��ʽ�ͷ�buffering��ʽ�ں���ngx_http_upstream_send_response�ֲ�
      //��xxx_buffering��fastcgi_buffering  proxy_buffering  �Ƿ񻺴��˷�����Ӧ������İ���
    unsigned                         buffering:1; //������ת�����ε���Ӧ����ʱ���Ƿ���������ڴ漰��ʱ�����ļ����ڻ������������͵����ε���Ӧ
    //Ϊ1˵�����κͺ�˵�����ʹ�õ��ǻ���cache(keepalive����)connection��TCP���ӣ�Ҳ����ʹ�õ���֮ǰ�Ѿ��ͺ�˽����õ�TCP����ngx_connection_t
    //�ڻ���ͺ�˵����ӵ�ʱ��ʹ��(Ҳ�����Ƿ�������keepalive con-num������)��Ϊ1��ʾʹ�õ��ǻ����TCP���ӣ�Ϊ0��ʾ�½��ĺͺ�˵�TCP���ӣ���ngx_http_upstream_free_keepalive_peer
    //���⣬�ں�˷�����������������ͷ����ָ��û�а��壬���u->keepalive = !u->headers_in.connection_close;����ngx_http_proxy_process_header
    unsigned                         keepalive:1;//ֻ���ڿ���keepalive con-num����Ч���ͷź��tcp�����ж���ngx_http_upstream_free_keepalive_peer
    unsigned                         upgrade:1; //��˷���//HTTP/1.1 101��ʱ����1  

/*
request_sent��ʾ�Ƿ��Ѿ������η��������������󣬵�request_sentΪ1ʱ����ʾupstream�����Ѿ������η�����������ȫ�����߲��ֵ�����
��ʵ�ϣ������־λ�������Ϊ��ʹ��ngx_output_chain��������������Ϊ�÷�����������ʱ���Զ���δ�������request_bufs�����¼������
Ϊ�˷�ֹ���������ظ����󣬱�����request_sent��־λ��¼�Ƿ���ù�ngx_output_chain����
*/
    unsigned                         request_sent:1; //ngx_http_upstream_send_request_body�з���������嵽��˵�ʱ����1
/*
�����η���������Ӧ����Ϊ��ͷ�Ͱ�β���������Ӧֱ��ת�����ͻ��ˣ�header_sent��־λ��ʾ��ͷ�Ƿ��ͣ�header_sentΪ1ʱ��ʾ�Ѿ���
��ͷת�����ͻ����ˡ������ת����Ӧ���ͻ��ˣ���header_sentû������
*/
    unsigned                         header_sent:1; //��ʾͷ���Ѿ��Ӹ�Э��ջ�ˣ�
};


typedef struct {
    ngx_uint_t                      status;
    ngx_uint_t                      mask;
} ngx_http_upstream_next_t;


typedef struct { //����ռ�͸�ֵ��ngx_http_upstream_param_set_slot�� �洢��ngx_http_fastcgi_loc_conf_t->params_source��
    ngx_str_t   key; //fastcgi_param  SCRIPT_FILENAME  aaa�е�SCRIPT_FILENAME
    ngx_str_t   value; //fastcgi_param  SCRIPT_FILENAME  aaa�е�aaa

    //ngx_http_fastcgi_init_params
    ngx_uint_t  skip_empty; //fastcgi_param  SCRIPT_FILENAME  aaa  if_not_empty������1   ��fastcgiΪ���ñ����������õط���ngx_http_fastcgi_create_request
} ngx_http_upstream_param_t;


ngx_int_t ngx_http_upstream_cookie_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
ngx_int_t ngx_http_upstream_header_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

ngx_int_t ngx_http_upstream_create(ngx_http_request_t *r);
void ngx_http_upstream_init(ngx_http_request_t *r);
ngx_http_upstream_srv_conf_t *ngx_http_upstream_add(ngx_conf_t *cf,
    ngx_url_t *u, ngx_uint_t flags);
char *ngx_http_upstream_bind_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_http_upstream_param_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
ngx_int_t ngx_http_upstream_hide_headers_hash(ngx_conf_t *cf,
    ngx_http_upstream_conf_t *conf, ngx_http_upstream_conf_t *prev,
    ngx_str_t *default_hide_headers, ngx_hash_init_t *hash);


#define ngx_http_conf_upstream_srv_conf(uscf, module)                         \
    uscf->srv_conf[module.ctx_index]


extern ngx_module_t        ngx_http_upstream_module;
extern ngx_conf_bitmask_t  ngx_http_upstream_cache_method_mask[];
extern ngx_conf_bitmask_t  ngx_http_upstream_ignore_headers_masks[];


#endif /* _NGX_HTTP_UPSTREAM_H_INCLUDED_ */
