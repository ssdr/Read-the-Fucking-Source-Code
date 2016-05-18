
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static ngx_int_t ngx_http_postpone_filter_add(ngx_http_request_t *r,
    ngx_chain_t *in);
static ngx_int_t ngx_http_postpone_filter_init(ngx_conf_t *cf);


static ngx_http_module_t  ngx_http_postpone_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_postpone_filter_init,         /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


/*
��6-1  Ĭ�ϼ������Nginx��HTTP����ģ��
���������������������������������������ש�������������������������������������������������������������������
��Ĭ�ϼ������Nginx��HTTP����ģ��     ��    ����                                                          ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ���������ڷ���200�ɹ�ʱ������������If-              ��
��                                    ��Modified-Since����If-Unmodified-Sinceͷ��ȡ������������ļ���ʱ   ��
��ngx_http_not_modified_filter_module ��                                                                  ��
��                                    ���䣬�ٷ��������û��ļ�������޸�ʱ�䣬�Դ˾����Ƿ�ֱ�ӷ���304     ��
��                                    �� Not Modified��Ӧ���û�                                           ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ���������е�Range��Ϣ������Range�е�Ҫ�󷵻��ļ���һ���ָ�      ��
��ngx_http_range_body_filter_module   ��                                                                  ��
��                                    ���û�                                                              ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTP�������������û����͵�ngx_chain_t�ṹ��HTTP��         ��
��                                    ���帴�Ƶ��µ�ngx_chain_t�ṹ�У����Ǹ���ָ��ĸ��ƣ�������ʵ��     ��
��ngx_http_copy_filter_module         ��                                                                  ��
��                                    ��HTTP��Ӧ���ݣ���������HTTP����ģ�鴦���ngx_chain_t���͵ĳ�       ��
��                                    ��Ա����ngx_http_copy_filter_moduleģ�鴦���ı���                 ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ������������ͨ���޸�nginx.conf�����ļ����ڷ���      ��
��ngx_http_headers_filter_module      ��                                                                  ��
��                                    �����û�����Ӧ����������HTTPͷ��                                  ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ�������������ִ��configure����ʱ�ᵽ��http_        ��
��ngx_http_userid_filter_module       ��                                                                  ��
��                                    ��userid moduleģ�飬������cookie�ṩ�˼򵥵���֤������           ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ���Խ��ı����ͷ��ظ��û�����Ӧ��������nginx��conf�е���������   ��
��ngx_http_charset_filter_module      ��                                                                  ��
��                                    �����б��룬�ٷ��ظ��û�                                            ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ֧��SSI��Server Side Include����������Ƕ�룩���ܣ����ļ����ݰ�  ��
��ngx_http_ssi_filter_module          ��                                                                  ��
��                                    ��������ҳ�в����ظ��û�                                            ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTP����������5.5.2����ϸ���ܹ��ù���ģ�顣����Ӧ����     ��
��ngx_http_postpone_filter_module     ��subrequest��������������ʹ�ö��������ͬʱ��ͻ��˷�����Ӧʱ    ��
��                                    ���ܹ�������ν�ġ������ǿ����չ����������˳������Ӧ            ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ���ض���HTTP��Ӧ���壨����ҳ�����ı��ļ�������gzipѹ������      ��
��ngx_http_gzip_filter_module         ��                                                                  ��
��                                    ����ѹ��������ݷ��ظ��û�                                          ��
�ǩ������������������������������������贈������������������������������������������������������������������
��ngx_http_range_header_filter_module ��  ֧��rangeЭ��                                                   ��
�ǩ������������������������������������贈������������������������������������������������������������������
��ngx_http_chunked_filter_module      ��  ֧��chunk����                                                   ��
�ǩ������������������������������������贈������������������������������������������������������������������
��                                    ��  ����HTTPͷ���������ù���ģ�齫���r->headers out�ṹ��        ��
��                                    ���еĳ�Ա���л�Ϊ���ظ��û���HTTP��Ӧ�ַ�����������Ӧ��(��         ��
��ngx_http_header_filter_module       ��                                                                  ��
��                                    ��HTTP/I.1 200 0K)����Ӧͷ������ͨ������ngx_http_write filter       ��
��                                    �� module����ģ���еĹ��˷���ֱ�ӽ�HTTP��ͷ���͵��ͻ���             ��
�ǩ������������������������������������贈������������������������������������������������������������������
��ngx_http_write_filter_module        ��  ����HTTP������������ģ�鸺����ͻ��˷���HTTP��Ӧ              ��
���������������������������������������ߩ�������������������������������������������������������������������

*/ngx_module_t  ngx_http_postpone_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_postpone_filter_module_ctx,  /* module context */
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


/*
    ���ˣ������󴴽���ϣ�һ����˵������Ĵ�����������ĳ�������content handler����ĳ��filter�ڣ�������ĺ������Կ���������û�����ϱ�ִ�У�
ֻ�Ǳ����������������posted_requests�����У�����ʲôʱ�����ִ���أ�֮ǰ˵��posted_requests��������ngx_http_run_posted_requests������
��������ôngx_http_run_posted_requests����������ʲôʱ����ã���ʵ��������ĳ������Ķ���д���¼���handler�У�ִ�����������صĴ����
�����ã�����������������һ��PHASE��ʱ������ngx_http_run_posted_requests����ʱ������������С�

    ��ʱʵ�ʻ���1��������Ҫ���������nginx�Ƕ���̣��ǲ��ܹ����������ģ����һ�����������˵�ǰ���̣����൱���������������accept��������
��������ͬʱ�ý���Ҳ����accept�����󣩣�һ�������������ĳЩԭ����Ҫ�������������io����nginx�����������ø������һЩ״̬����epoll
�������Ӧ���¼���Ȼ��תȥ�����������󣬵ȵ����¼�����ʱ�ټ��������������������Ϊ����ζ��һ�����������Ҫ���ִ�л��������ɣ���
��һ������Ķ����������˵����ζ��������ɵ��Ⱥ�˳����ܺ����Ǵ�����˳���ǲ�һ���ģ����Ա�����һ�ֻ�������ǰ��ɵ������󱣴���������
���ݣ�������ֱ�������out chain��ͬʱҲ�ܹ��õ�ǰ�ܹ���out chain������ݵ�����ʱ��������������ݡ�����Igor����ngx_connection_t�е�
data�ֶΣ��Լ�һ��body filter����ngx_http_postpone_filter������ngx_http_finalize_request�����е�һЩ�߼������������⡣

�ο�:http://blog.csdn.net/fengmo_q/article/details/6685840




˵��:
root_rΪԭʼ���ϲ������r��postponedΪ��r->postponedָ�룬
sbuxy_r�е�x������Ǹ�������ĸ�����ʱx��y�����������ʱ������ĵ�y��������
datax����subx_r���������һ������,����ͨ��ngx_http_postpone_filter_add��ӵ�r->postponed����

                                          -----root_r     
                                          |postponed
                                          |
                            -------------sub1_r-------sub2_r-------data_root(����root_r����)
                            |                           |postponed                    
                            |postponed                  |
                            |                           sub21_r-----data2(����sub2_r����)
                            |                           |
                            |                           |
                            |                           -----data2(����sub21_r����)
                            |
                          sub11_r--------sub12_r-----data1(����sub1_r����)
                            |               |
                            |postponed      |postponed
                            |               |
                            -----data11     -----data12(����sub12_r����)

    ͼ�е�root�ڵ㼴Ϊ����������postponed����������ҹ�����3���ڵ㣬SUB1�����ĵ�һ��������DATA1����������һ�����ݣ�SUB2�����ĵ�2��������
������2��������ֱ��������Լ������������ݡ�ngx_connection_t�е�data�ֶα�����ǵ�ǰ������out chain�������ݵ��������¿�ͷ˵�������ͻ���
�����ݱ��밴�������󴴽���˳���ͣ����Ｔ�ǰ����������ķ�����SUB11->DATA11->SUB12->DATA12->(SUB1)->DATA1->SUB21->SUB22->(SUB2)->(ROOT)����
��ͼ�е�ǰ�ܹ����ͻ��ˣ�out chain���������ݵ�������Ȼ����SUB11�����SUB12��ǰִ����ɣ�����������DATA121��ֻҪǰ�������нڵ�δ������ϣ�
DATA121ֻ���ȹ�����SUB12��postponed�����¡����ﻹҪע��һ�µ���c->data�����ã���SUB11ִ���겢�ҷ���������֮����һ����Ҫ���͵Ľڵ�Ӧ����
DATA11�����Ǹýڵ�ʵ���ϱ���������ݣ�����������������c->data��ʱӦ��ָ�����ӵ�и����ݽڵ��SUB1����

�������ݵ��ͻ������ȼ�:
1.���������ȼ��ȸ������
2.ͬ��(һ��r�������������)���󣬴��������ȼ��ɸߵ���(��Ϊ�ȴ������������ȷ������ݵ��ͻ���)
�������ݵ��ͻ���˳����Ƽ�ngx_http_postpone_filter
*/


//���http://blog.csdn.net/fengmo_q/article/details/6685840ͼ�λ��Ķ�
//����Ĳ���in���ǽ�Ҫ���͸��ͻ��˵�һ�ΰ��壬
static ngx_int_t //subrequestע��ngx_http_run_posted_requests��ngx_http_subrequest ngx_http_postpone_filter ngx_http_finalize_request����Ķ�
ngx_http_postpone_filter(ngx_http_request_t *r, ngx_chain_t *in)
{//һ����������յ�������ݺ󲻻��������͵��ͻ��ˣ�����ͨ��ngx_http_finalize_request->ngx_http_set_write_handler�������ͺ�˵�����
    ngx_connection_t              *c;
    ngx_http_postponed_request_t  *pr;

    //c��Nginx�����οͻ��˼�����ӣ�c->data�������ԭʼ����
    c = r->connection;

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http postpone filter \"%V?%V\" %p", &r->uri, &r->args, in);

    /*
                          -----root_r     
                          |postponed
                          |
            -------------sub1_r-------sub2_r-------data_root(����root_r����)
            |                           |postponed                    
            |postponed                  |
            |                           sub21_r-----data2(����sub2_r����)
            |                           |
            |                           |
            |                           -----data2(����sub21_r����)
            |
          sub11_r--------sub12_r-----data1(����sub1_r����)
            |               |
            |postponed      |postponed
            |               |
            -----data11     -----data12(����sub12_r����)

          �����if�ж�ֻ��rΪsub11_r������r == c->data�������ǰr����sub11_r�����
     */
    //�����ǰ����r��һ����������Ϊc->dataָ��ԭʼ����
    //˵��r���ϼ�r�ĵ�һ��������Ҳ����http://blog.csdn.net/fengmo_q/article/details/6685840ͼ�����SUB1  SUB11 SUB21
    if (r != c->data) {//�ο�ngx_http_subrequest�е�c->data = sr  ngx_connection_t�е�data�ֶα�����ǵ�ǰ������out chain�������ݵ�����Ҳ�������ȼ���ߵ�����
        /* ��ǰ��������out chain�������ݣ�������������ݣ��½�һ���ڵ㣬 
       ���������ڵ�ǰ�����postponed��β�������ͱ�֤�����ݰ��򷢵��ͻ��� */  
        if (in) {
            /*
              ��������͵�in���岻Ϊ�գ����in�ӵ�postponed���������ڵ�ǰ�����ngx_http_postponed_request_t�ṹ���out�����У�
              ͬʱ����NGX_OK������ζ�ű��β����in���巢���ͻ���
               */
            ngx_http_postpone_filter_add(r, in); //�Ѹ�������r�����ȡ�������ݹҽӵ���r->postponed��
            return NGX_OK;
        }

#if 0
        /* TODO: SSI may pass NULL */
        ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                      "http postpone filter NULL inactive request");
#endif
        //�����ǰ�����������󣬶�in������Ϊ�գ���ôֱ�ӷ��ؼ���
        return NGX_OK;
    }

    /* �������ʾ��ǰ���������out chain�������ݣ��������postponed������û��������Ҳû�����ݣ� 
       ��ֱ�ӷ��͵�ǰ����������in���߼�������out chain��֮ǰû�з�����ɵ����� */  
    //���postponedΪ�գ���ʾ����rû���������������Ӧ��Ҫת��
    if (r->postponed == NULL) {
        //ֱ�ӵ�����һ��HTTP����ģ���������in���弴�ɡ����û�д���Ļ����ͻῪʼ�����οͻ��˷�����Ӧ
        if (in || c->buffered) {
            return ngx_http_next_body_filter(r->main, in);
        }

        return NGX_OK;
    }

    

    /* r->postponed != NULL��ǰ�����postponed������֮ǰ�ʹ�����Ҫ����Ľڵ㣬���½�һ���ڵ㣬���浱ǰ����������in�� 
       ���������뵽postponed��β */  
    //���ˣ�˵��postponed�����������������������Ӧ��Ҫת���ģ������Ȱ�in����ӵ���ת����Ӧ
    if (in) {
        ngx_http_postpone_filter_add(r, in);
    }

    //ѭ������postponed�����������������ת���İ�  /* ����postponed�����еĽڵ� */  
    do {
        pr = r->postponed;

    /* ����ýڵ㱣�����һ�������������ӵ��������posted_requests�����У��Ա��´ε���ngx_http_run_posted_requests������������ӽڵ� */  
    //���pr->request������������뵽ԭʼ�����posted_requests�����У��ȴ�HTTP����´ε����������ʱ���������μ�11.7�ڣ�
        if (pr->request) { //˵��pr�ڵ����������������������

            ngx_log_debug2(NGX_LOG_DEBUG_HTTP, c->log, 0,
                           "http postpone filter wake \"%V?%V\"",
                           &pr->request->uri, &pr->request->args);

            r->postponed = pr->next;

             /* ���պ����������������У���Ϊ��ǰ���󣨽ڵ㣩��δ�����������(�ڵ�)�� 
               �����ȴ�����������󣬲��ܼ������������ӽڵ㡣 
               ���ｫ������������Ϊ������out chain�������ݵ�����  */  
               //���http://blog.csdn.net/fengmo_q/article/details/6685840ͼ�λ��Ķ�
            c->data = pr->request;

            //Ϊ�������󴴽�ngx_http_posted_request_t��ӵ����ϲ�root����r��posted_requests��
            return ngx_http_post_request(pr->request, NULL); /* ��������������������posted_requests���� */  
            //������ȼ��͵�������������ߵ������ͨ��ngx_http_postpone_filter_add���浽r->postpone��Ȼ��r��ӵ�pr->request->posted_requests,����ڸ����ȼ�������
            //���ݵ����󣬻��֮ǰ���������ĵ����ȼ����������Ҳһ����ngx_http_run_posted_requests�д������ͣ��Ӷ���֤�������͵��ͻ�������ʱ�������������ȼ�˳���͵�
        }

        /* ����ýڵ㱣��������ݣ�����ֱ�Ӵ���ýڵ㣬�������͵�out chain */  
        
        //������һ��HTTP����ģ��ת��out�����б���Ĵ�ת���İ���
        if (pr->out == NULL) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "http postpone filter NULL output");

        } else {
            ngx_log_debug2(NGX_LOG_DEBUG_HTTP, c->log, 0,
                           "http postpone filter output \"%V?%V\"",
                           &r->uri, &r->args);

            if (ngx_http_next_body_filter(r->main, pr->out) == NGX_ERROR) {
                return NGX_ERROR;
            }
        }

        r->postponed = pr->next;//������postponed����

    } while (r->postponed);

    return NGX_OK;
}

/*
    ���ˣ������󴴽���ϣ�һ����˵������Ĵ�����������ĳ�������content handler����ĳ��filter�ڣ�������ĺ������Կ���������û�����ϱ�ִ�У�
ֻ�Ǳ����������������posted_requests�����У�����ʲôʱ�����ִ���أ�֮ǰ˵��posted_requests��������ngx_http_run_posted_requests������
��������ôngx_http_run_posted_requests����������ʲôʱ����ã���ʵ��������ĳ������Ķ���д���¼���handler�У�ִ�����������صĴ����
�����ã�����������������һ��PHASE��ʱ������ngx_http_run_posted_requests����ʱ������������С�

    ��ʱʵ�ʻ���1��������Ҫ���������nginx�Ƕ���̣��ǲ��ܹ����������ģ����һ�����������˵�ǰ���̣����൱���������������accept��������
��������ͬʱ�ý���Ҳ����accept�����󣩣�һ�������������ĳЩԭ����Ҫ�������������io����nginx�����������ø������һЩ״̬����epoll
�������Ӧ���¼���Ȼ��תȥ�����������󣬵ȵ����¼�����ʱ�ټ��������������������Ϊ����ζ��һ�����������Ҫ���ִ�л��������ɣ���
��һ������Ķ����������˵����ζ��������ɵ��Ⱥ�˳����ܺ����Ǵ�����˳���ǲ�һ���ģ����Ա�����һ�ֻ�������ǰ��ɵ������󱣴���������
���ݣ�������ֱ�������out chain��ͬʱҲ�ܹ��õ�ǰ�ܹ���out chain������ݵ�����ʱ��������������ݡ�����Igor����ngx_connection_t�е�
data�ֶΣ��Լ�һ��body filter����ngx_http_postpone_filter������ngx_http_finalize_request�����е�һЩ�߼������������⡣

�ο�:http://blog.csdn.net/fengmo_q/article/details/6685840

˵��:
root_rΪԭʼ���ϲ������r��postponedΪ��r->postponedָ�룬
sbuxy_r�е�x������Ǹ�������ĸ�����ʱx��y�����������ʱ������ĵ�y��������
datax����subx_r���������һ������,����ͨ��ngx_http_postpone_filter_add��ӵ�r->postponed����

                                          -----root_r     
                                          |postponed
                                          |
                            -------------sub1_r-------sub2_r-------data_root(����root_r����)
                            |                           |postponed                    
                            |postponed                  |
                            |                           sub21_r-----data2(����sub2_r����)
                            |                           |
                            |                           |
                            |                           -----data2(����sub21_r����)
                            |
                          sub11_r--------sub12_r-----data1(����sub1_r����)
                            |               |
                            |postponed      |postponed
                            |               |
                            -----data11     -----data12(����sub12_r����)

    ͼ�е�root�ڵ㼴Ϊ����������postponed����������ҹ�����3���ڵ㣬SUB1�����ĵ�һ��������DATA1����������һ�����ݣ�SUB2�����ĵ�2��������
������2��������ֱ��������Լ������������ݡ�ngx_connection_t�е�data�ֶα�����ǵ�ǰ������out chain�������ݵ��������¿�ͷ˵�������ͻ���
�����ݱ��밴�������󴴽���˳���ͣ����Ｔ�ǰ����������ķ�����SUB11->DATA11->SUB12->DATA12->(SUB1)->DATA1->SUB21->SUB22->(SUB2)->(ROOT)����
��ͼ�е�ǰ�ܹ����ͻ��ˣ�out chain���������ݵ�������Ȼ����SUB11�����SUB12��ǰִ����ɣ�����������DATA121��ֻҪǰ�������нڵ�δ������ϣ�
DATA121ֻ���ȹ�����SUB12��postponed�����¡����ﻹҪע��һ�µ���c->data�����ã���SUB11ִ���겢�ҷ���������֮����һ����Ҫ���͵Ľڵ�Ӧ����
DATA11�����Ǹýڵ�ʵ���ϱ���������ݣ�����������������c->data��ʱӦ��ָ�����ӵ�и����ݽڵ��SUB1����

�������ݵ��ͻ������ȼ�:
1.���������ȼ��ȸ������
2.ͬ��(һ��r�������������)���󣬴��������ȼ��ɸߵ���(��Ϊ�ȴ������������ȷ������ݵ��ͻ���)
*/

//�ο�http://blog.csdn.net/fengmo_q/article/details/6685840�е�ͼ�λ�����˵��
static ngx_int_t
ngx_http_postpone_filter_add(ngx_http_request_t *r, ngx_chain_t *in)
{ //postponed�����ngx_http_postpone_filter_add ��ɾ����ngx_http_finalize_request
    ngx_http_postponed_request_t  *pr, **ppr;

    if (r->postponed) {
        for (pr = r->postponed; pr->next; pr = pr->next) { /* void */ }

        if (pr->request == NULL) { //˵�����һ��ngx_http_postponed_request_t�ڵ�ֻ�Ǽ򵥵Ĵ洢in�����ݣ���ֱ��ʹ�øýṹ����
            goto found;
        }

        ppr = &pr->next;

    } else {
        ppr = &r->postponed;
    }

    pr = ngx_palloc(r->pool, sizeof(ngx_http_postponed_request_t));
    if (pr == NULL) {
        return NGX_ERROR;
    }

    *ppr = pr; //���´�����pr��ӵ�r->postponedβ��

    pr->request = NULL; //in����ͨ�������µ�ngx_http_postponed_request_t��ӵ�r->postponed�У�������ʱ���´�����pr�ڵ��requestΪNULL
    pr->out = NULL;
    pr->next = NULL;

found:

    if (ngx_chain_add_copy(r->pool, &pr->out, in) == NGX_OK) {
        return NGX_OK;
    }

    return NGX_ERROR;
}


static ngx_int_t
ngx_http_postpone_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_postpone_filter;

    return NGX_OK;
}
