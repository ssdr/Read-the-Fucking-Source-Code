#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/*
HTTP����ģ��ĵ�λ��������������HTTP����ģ���ǲ�ͬ�ģ��������Ĺ����ǶԷ��͸��û���HTTP��Ӧ����һЩ�ӹ�
HTTP����ģ�鲻��ȥ���ʵ���������
*/

typedef struct
{
    ngx_flag_t		enable;
} ngx_http_myfilter_conf_t;

typedef struct
{
    ngx_int_t   	add_prefix;
} ngx_http_myfilter_ctx_t;

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;

//���ڰ�����������ǰ׺
static ngx_str_t filter_prefix = ngx_string("[my filter prefix]");

static void* ngx_http_myfilter_create_conf(ngx_conf_t *cf);
static char *
ngx_http_myfilter_merge_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t ngx_http_myfilter_init(ngx_conf_t *cf);
static ngx_int_t
ngx_http_myfilter_header_filter(ngx_http_request_t *r);
static ngx_int_t
ngx_http_myfilter_body_filter(ngx_http_request_t *r, ngx_chain_t *in);

static ngx_command_t  ngx_http_myfilter_commands[] =
{
    {
        ngx_string("add_prefix"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_myfilter_conf_t, enable),
        NULL
    },

    ngx_null_command
};


static ngx_http_module_t  ngx_http_myfilter_module_ctx =
{
    NULL,                                  /* preconfiguration����  */
    ngx_http_myfilter_init,                /* postconfiguration���� */

    NULL,                                  /*create_main_conf ���� */
    NULL,                                  /* init_main_conf���� */

    NULL,                                  /* create_srv_conf���� */
    NULL,                                  /* merge_srv_conf���� */

    ngx_http_myfilter_create_conf,         /* create_loc_conf���� */
    ngx_http_myfilter_merge_conf           /* merge_loc_conf���� */
};


ngx_module_t  ngx_http_myfilter_module =
{
    NGX_MODULE_V1,
    &ngx_http_myfilter_module_ctx,     /* module context */
    ngx_http_myfilter_commands,        /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

/*
�����������е�HTTP����ģ��ʱ������������е�Ԫ������ô��nextָ�������������أ��ܼ򵥣�ÿ��HTTP����ģ���ڳ�ʼ��ʱ�������ҵ�����
����Ԫ��ngx_http_top_header_filterָ���ngx_http_top_body_filterָ�룬��ʹ��static��̬���͵�ngx_http_next_header_filter��
ngx_http_next_body_filterָ�뽫�Լ����뵽������ײ������������ˡ���������һ����ÿ������ģ����ngx_http_next_ header_ filter��ngx_http_next- body_filter�����
���壺
static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;
    ע�⣬ngx_http_next_header_filter��ngx_http_next_body_filter��������static��̬������Ϊʲô�أ���Ϊstatic���Ϳ�����������
���������ڵ�ǰ�ļ�����Ч������������е�HTTP����ģ�鶼�и��Ե�ngx_http_next_header_filter��ngx_http_next_body_filterָ�롣
��������ÿ��HTTP����ģ���ʼ��ʱ���Ϳ���������������ָ��ָ����һ��HTTP����ģ���ˡ����磬���������д���һ������ǰHTTP����ģ��
�Ĵ�������ӵ������ײ���
ngx_http_next_header_filter = ngx_http_top_header filter;
ngx_http_top_header filter = ngx_http_myfilter_header_filter;
ngx_ht tp_next_body_f ilter = ngx_http_top_body_f ilter ;
ngx_http_top_body_f ilt er = ngx_ht tp_myf ilter_body_f ilter ;
    �������ڳ�ʼ������ģ��ʱ���Զ����ngx_http_myfilter_header_filter��ngx_http_myfilter_body_filter��������ʱ���뵽��������ײ���
���ұ�ģ�������ļ���static���͵�ngx_http_next_header_filterָ���ngx_http_next_body_filterָ��Ҳָ����������ԭ�����ײ�����ʵ��ʹ���У�
�����Ҫ������һ��HTTP����ģ�飬ֻ��Ҫ����ngx_http_next_header_filter(r)����ngx_http_next_ body_filter(r��chain)�Ϳ����ˡ�
*/

//��������ngx_http_send_header��ִ��ngx_http_myfilter_header_filter��Ȼ����ngx_http_myfilter_header_filter��ִ����һ��filter��Ҳ����
//ngx_http_next_header_filter�д洢֮ǰ��filter����������ѭ����ȥ����ô���е�filter��������õ�ִ��

//����ͨ��ngx_http_output_filterѭ������
static ngx_int_t ngx_http_myfilter_init(ngx_conf_t *cf)
{
    //���뵽ͷ��������������ײ�
    ngx_http_next_header_filter = ngx_http_top_header_filter; //ngx_http_next_header_filterָ����ʱ����ngx_http_top_header_filter��
    ngx_http_top_header_filter = ngx_http_myfilter_header_filter; //��ngx_http_myfilter_header_filter�м���������һ��filter

    //���뵽���崦����������ײ�
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_myfilter_body_filter;

    return NGX_OK;
}

static ngx_int_t ngx_http_myfilter_header_filter(ngx_http_request_t *r)
{
    ngx_http_myfilter_ctx_t   *ctx;
    ngx_http_myfilter_conf_t  *conf;

    //������Ƿ��سɹ�����ʱ�ǲ���Ҫ����Ƿ��ǰ׺�ģ�ֱ�ӽ�����һ������ģ��
//������Ӧ���200������
    if (r->headers_out.status != NGX_HTTP_OK)
    {
        return ngx_http_next_header_filter(r);
    }

//��ȡhttp������
    ctx = ngx_http_get_module_ctx(r, ngx_http_myfilter_module);
    if (ctx)
    {
        //��������������Ѿ����ڣ���˵��
// ngx_http_myfilter_header_filter�Ѿ������ù�1�Σ�
//ֱ�ӽ�����һ������ģ�鴦��
        return ngx_http_next_header_filter(r);
    }

//��ȡ�洢�������ngx_http_myfilter_conf_t�ṹ��
    conf = ngx_http_get_module_loc_conf(r, ngx_http_myfilter_module);

//���enable��ԱΪ0��Ҳ���������ļ���û������add_prefix�����
//����add_prefix������Ĳ���ֵ��off����ʱֱ�ӽ�����һ������ģ�鴦��
    if (conf->enable == 0)
    {
        return ngx_http_next_header_filter(r);
    }

//����http�����Ľṹ��ngx_http_myfilter_ctx_t
    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_myfilter_ctx_t));
    if (ctx == NULL)
    {
        return NGX_ERROR;
    }

//add_prefixΪ0��ʾ����ǰ׺
    ctx->add_prefix = 0;

//����������������õ���ǰ������
    ngx_http_set_ctx(r, ctx, ngx_http_myfilter_module);

//myfilter����ģ��ֻ����Content-Type��"text/plain"���͵�http��Ӧ
    if (r->headers_out.content_type.len >= sizeof("text/plain") - 1
        && ngx_strncasecmp(r->headers_out.content_type.data, (u_char *) "text/plain", sizeof("text/plain") - 1) == 0)
    {
        //1��ʾ��Ҫ��http����ǰ����ǰ׺
        ctx->add_prefix = 1;

//�������ģ���Ѿ���Content-Lengthд����http����ĳ��ȣ�����
//���Ǽ�����ǰ׺�ַ�����������Ҫ������ַ����ĳ���Ҳ���뵽
//Content-Length��
        if (r->headers_out.content_length_n > 0)
            r->headers_out.content_length_n += filter_prefix.len;
    }

//������һ������ģ���������
    return ngx_http_next_header_filter(r);
}

/*
����ͨ��һ���򵥵�������˵����ο���HTTP����ģ�顣�����������ģ��û���������static��̬�ļ�ģ������˴����������URI���ش���
�е��ļ����û��������ǿ����Ĺ���ģ��ͻ��ڷ��ظ��û�����Ӧ����ǰ��һ���ַ�������[my filter prefix]������Ҫʵ�ֵĹ��ܾ�����ô
�򵥣���Ȼ�������������ļ��о����Ƿ����˹��ܡ�
*/
static ngx_int_t
ngx_http_myfilter_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_http_myfilter_ctx_t   *ctx;
    ctx = ngx_http_get_module_ctx(r, ngx_http_myfilter_module);
    
    //�����ȡ���������ģ����������Ľṹ���е�add_prefixΪ0����2ʱ�����������ǰ׺����ʱֱ�ӽ�����һ��http����ģ�鴦��
    if (ctx == NULL || ctx->add_prefix != 1)
    {
        return ngx_http_next_body_filter(r, in);
    }

//��add_prefix����Ϊ2��������ʹngx_http_myfilter_body_filter
//�ٴλص�ʱ��Ҳ�����ظ����ǰ׺
    ctx->add_prefix = 2;

//��������ڴ���з����ڴ棬���ڴ洢�ַ���ǰ׺
    ngx_buf_t* b = ngx_create_temp_buf(r->pool, filter_prefix.len);
//��ngx_buf_t�е�ָ����ȷ��ָ��filter_prefix�ַ���
    b->start = b->pos = filter_prefix.data;
    b->last = b->pos + filter_prefix.len;

//��������ڴ��������ngx_chain_t�������շ����ngx_buf_t���õ���buf��Ա�У���������ӵ�ԭ�ȴ����͵�http����ǰ��
    ngx_chain_t *cl = ngx_alloc_chain_link(r->pool);
    cl->buf = b;
    cl->next = in;

//������һ��ģ���http���崦������ע����ʱ������������ɵ�cl����
    return ngx_http_next_body_filter(r, cl);
}

static void* ngx_http_myfilter_create_conf(ngx_conf_t *cf)
{
    ngx_http_myfilter_conf_t  *mycf;

    //�����洢������Ľṹ��
    mycf = (ngx_http_myfilter_conf_t  *)ngx_pcalloc(cf->pool, sizeof(ngx_http_myfilter_conf_t));
    if (mycf == NULL)
    {
        return NULL;
    }

    //ngx_flat_t���͵ı��������ʹ��Ԥ�躯��ngx_conf_set_flag_slot
//��������������������ʼ��ΪNGX_CONF_UNSET
    mycf->enable = NGX_CONF_UNSET;

    return mycf;
}

static char *
ngx_http_myfilter_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_myfilter_conf_t *prev = (ngx_http_myfilter_conf_t *)parent;
    ngx_http_myfilter_conf_t *conf = (ngx_http_myfilter_conf_t *)child;

//�ϲ�ngx_flat_t���͵�������enable
    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

