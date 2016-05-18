#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *
ngx_http_sendifle_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_sendifle_mytest_handler(ngx_http_request_t *r);



static ngx_command_t  sendfile_test_commands[] =
{

    {
        ngx_string("mytest_sendfile"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_sendifle_mytest,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },

    ngx_null_command
};

static ngx_http_module_t  sendfile_test_ctx =
{
    NULL,                              /* preconfiguration */
    NULL,                  		/* postconfiguration */

    NULL,                              /* create main configuration */
    NULL,                              /* init main configuration */

    NULL,                              /* create server configuration */
    NULL,                              /* merge server configuration */

    NULL,       			/* create location configuration */
    NULL         			/* merge location configuration */
};

ngx_module_t  sendfile_test =
{
    NGX_MODULE_V1,
    &sendfile_test_ctx,           /* module context */
    sendfile_test_commands,              /* module directives */
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


static char *
ngx_http_sendifle_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    //�����ҵ�mytest���������������ÿ飬clcfò����location���ڵ�����
//�ṹ����ʵ��Ȼ����������main��srv����loc���������Ҳ����˵��ÿ��
//http{}��server{}��Ҳ����һ��ngx_http_core_loc_conf_t�ṹ��
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    //http����ڴ����û�������е�NGX_HTTP_CONTENT_PHASE�׶�ʱ�����
//���������������URI��mytest���������ڵ����ÿ���ƥ�䣬�ͽ���������
//ʵ�ֵ�ngx_http_mytest_handler���������������
    clcf->handler = ngx_http_sendifle_mytest_handler;

    return NGX_CONF_OK;
}


static ngx_int_t ngx_http_sendifle_mytest_handler(ngx_http_request_t *r)
{
    //������GET����HEAD���������򷵻�405 Not Allowed
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
    {
        return NGX_HTTP_NOT_ALLOWED;
    }

    //���������еİ���
    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK)
    {
        return rc;
    }
    ngx_buf_t *b;
    b = ngx_palloc(r->pool, sizeof(ngx_buf_t));

    //Ҫ�򿪵��ļ�
    u_char* filename = (u_char*)"/usr/local/nginx/html/indextest.html";
    b->in_file = 1;
    b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
    b->file->fd = ngx_open_file(filename, NGX_FILE_RDONLY | NGX_FILE_NONBLOCK, NGX_FILE_OPEN, 0);
    b->file->log = r->connection->log;
    b->file->name.data = filename;
    b->file->name.len = sizeof(filename) - 1;
    if (b->file->fd <= 0)
    {
        return NGX_HTTP_NOT_FOUND;
    }

    //֧�ֶϵ�����
    r->allow_ranges = 1;

    //��ȡ�ļ�����
    if (ngx_file_info(filename, &b->file->info) == NGX_FILE_ERROR)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    //���û�����ָ����ļ���
    b->file_pos = 0;
    b->file_last = b->file->info.st_size;

    ngx_pool_cleanup_t* cln = ngx_pool_cleanup_add(r->pool, sizeof(ngx_pool_cleanup_file_t));
    if (cln == NULL)
    {
        return NGX_ERROR;
    }

    cln->handler = ngx_pool_cleanup_file;
    ngx_pool_cleanup_file_t  *clnf = cln->data;

    clnf->fd = b->file->fd;
    clnf->name = b->file->name.data;
    clnf->log = r->pool->log;


    //���÷��ص�Content-Type��ע�⣬ngx_str_t��һ���ܷ���ĳ�ʼ����
//ngx_string�������԰�ngx_str_t��data��len��Ա�����ú�
    ngx_str_t type = ngx_string("text/plain");

    //���÷���״̬��
    r->headers_out.status = NGX_HTTP_OK;
    //��Ӧ�����а������ݵģ�������Ҫ����Content-Length����
    r->headers_out.content_length_n = b->file->info.st_size;
    //����Content-Type
    r->headers_out.content_type = type;

    //����httpͷ��
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
    {
        return rc;
    }

    //���췢��ʱ��ngx_chain_t�ṹ��
    ngx_chain_t		out;
    //��ֵngx_buf_t
    out.buf = b;
    //����nextΪNULL
    out.next = NULL;

    //���һ�����Ͱ��壬http��ܻ����ngx_http_finalize_request����
//��������
    return ngx_http_output_filter(r, &out);
}


