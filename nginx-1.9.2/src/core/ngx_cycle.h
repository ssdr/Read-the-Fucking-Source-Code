
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

//��ngx_http_upstream_cache_get�л�ȡzone��ʱ���ȡ����fastcgi_cache proxy_cache���õ�zone����˱�������fastcgi_cache (proxy_cache) abc;�е�xxx��xxx_cache_path(proxy_cache_path fastcgi_cache_path) xxx keys_zone=abc:10m;һ��
//���еĹ����ڴ涼ͨ��ngx_http_file_cache_s->shpool���й���   ÿ�������ڴ��Ӧһ��ngx_slab_pool_t��������ngx_init_zone_pool
struct ngx_shm_zone_s { //��ʼ����ngx_shared_memory_add�������Ĺ����ڴ洴����ngx_init_cycle->ngx_init_cycle
    void                     *data;//ָ��ngx_http_file_cache_t����ֵ��ngx_http_file_cache_set_slot
    ngx_shm_t                 shm; //ngx_init_cycle->ngx_shm_alloc->ngx_shm_alloc�д�����Ӧ�Ĺ����ڴ�ռ�
    //ngx_init_cycle��ִ��
    ngx_shm_zone_init_pt      init; // "zone" proxy_cache_path fastcgi_cache_path������������Ϊngx_http_file_cache_init   ngx_http_upstream_init_zone   
    void                     *tag; //��������������ڴ������ĸ�ģ��
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};

/*
��8-2 ngx_cycle_t�ṹ��֧�ֵ���Ҫ����
�������������������������������������ש��������������������������������������ש���������������������������������������
��    ������                        ��    ��������                          ��    ִ������                          ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��                                  ��                                      ��  ���س�ʼ���ɹ���������ngx_cycle_    ��
��  ngx_cycle_t *ngx_init_cycle     ��  old_cycle��ʾ��ʱ��ngx_cycle_t      ��t�ṹ�壬�ú������Ḻ���ʼ��ngx_     ��
��                                  ��ָ�룬һ�����������ngx_cycle_t��     ��cycle_t�е����ݽṹ�����������ļ���   ��
��(ngx_cycle_t *old_cycle)          ��                                      ����������ģ�顢�򿪼����˿ڡ���ʼ��    ��
��                                  �������е������ļ�·���Ȳ���            ��                                      ��
��                                  ��                                      �����̼�ͨ�ŷ�ʽ�ȹ��������ʧ�ܣ���    ��
��                                  ��                                      ������NULL��ָ��                        ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��  ngx_int_t ngx_process_          ��  cycleͨ���Ǹոշ����ngx_cycle_t    ��  ������Nginxʱ����Я����Ŀ¼����     ��
��                                  ���ṹ��ָ�룬�����ڴ��������ļ�·      ������ʼ��cycle��������ʼ������Ŀ¼��   ��
��options (ngx_cycle_t *cycle)      ��                                      ������Ŀ¼��������������nginx��conf��   ��
��                                  ������Ϣ                                ��                                      ��
��                                  ��                                      �����ļ�·��                            ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��                                  ��                                      ��  �����в�������������Nginx�Ĳ�       ��
��                                  ��                                      ����ʱ���ϵ�Nginx���̻�ͨ����������     ��
��  ngx_int_t ngx_add_inherited     ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ����NGINX����������Ҫ�򿪵ļ�����       ��
��sockets(ngx_cycle_t��cycle)       ������ָ��                              ���ڣ��µ�Nginx���̻�ͨ��ngx_add_       ��
��                                  ��                                      ��inherited- sockets������ʹ���Ѿ���  ��
��                                  ��                                      ����TCP�����˿�                         ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��  ngxjnt_t ngx_open_listening_    ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ��  ��������cycle��listening��̬��    ��
��sockets (ngx_cycle_t *cycle)      ������ָ��                              ����ָ������Ӧ�˿�                      ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��   void ngx_configure_listening_  ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ��  ����nginx.conf�е������������Ѿ�    ��
��sockets(ngx_cycle_t��cycle)       ������ָ��                              �������ľ��                            ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��void ngx_close_listening_         ��    cycle�ǵ�ǰ���̵�ngx_cycle_t��    ��  �ر�cycle��listening��̬�����Ѿ�    ��
��sockets(ngx_cycle_t *cycle)       ������ָ��                              ���򿪵ľ��                            ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��void ngx_master_process_          ��    cycle�ǵ�ǰ���̵�ngx_cycle_t��    ��  ����master���̵Ĺ���ѭ��            ��
��cycle(ngx_cycle_t *cycle)         ������ָ��                              ��                                      ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��   void ngx_single_process_       ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ��  ���뵥����ģʽ����master��worker    ��
��cycle (ngx_cycle_t *cycle)        ������ָ��                              �����̹���ģʽ���Ĺ���ѭ��              ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��                                  ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ��                                      ��
��                                  ������ָ�룬n�������ӽ��̵ĸ�����       ��                                      ��
��                                  ��type��������ʽ������ȡֵ��Χ����      ��                                      ��
��                                  ����5����                               ��                                      ��
��   void ngx_start_worker_         ��    1)  NGX_PROCESS_RESPAWN;          ��  ����n��worker�ӽ��̣������ú�       ��
��processes (ngx_cycle_t *cycle,    ��    2) NGX__ PROCESS NORESPAWN;       ��ÿ���ӽ�����master������֮��ʹ��      ��
��                                  ��    3) NGX��PROCESS_JUST_SPAWN;       ��socketpairϵͳ���ý���������socket    ��
��ngx_int_t n, ngx_int_t type)      ��                                      ��                                      ��
��                                  ��    4) NGX.PROCESS JUST_RESPAWN;      �����ͨ�Ż���                          ��
��                                  ��    5) NGX_PROCESS��DETACHED.         ��                                      ��
��                                  ��type��ֵ��Ӱ��       ngn_process_t    ��                                      ��
��                                  ���ṹ���respawn. detached. just_spawn ��                                      ��
��                                  ����־λ��ֵ                            ��                                      ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��                                  ��                                      ��  �����Ƿ�ʹ���ļ�����ģ�飬Ҳ����    ��
��   void ngx_start_cache_          ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ��cycle�д洢·���Ķ�̬�������Ƿ���     ��
��manager_processes(ngx_cycle_t     ������ָ�룬respawn�������ӽ��̵ķ�     ��·����manage��־�򿪣��������Ƿ�      ��
��                                  ��ʽ��  ����ngx_start_worker_processes  ������cache manage�ӽ��̣�ͬ������      ��
��*cycle, ngx_uint_t respawn)       ��                                      ��                                      ��
��                                  �������е�type����������ȫ��ͬ          ��loader��־�����Ƿ�����cache loader    ��
��                                  ��                                      ���ӽ���                                ��
�ǩ����������������������������������贈�������������������������������������贈��������������������������������������
��   void ngx_pass_open_channel     ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��      ��  �������Ѿ��򿪵�channel��ͨ��       ��
��(ngx_cycle_t *cycle, ngx_         ������ָ�룬ch�ǽ�Ҫ���ӽ��̷��͵�      ��socketpair���ɵľ������ͨ�ţ�����    ��
��channel_t *ch)                    ����Ϣ                                  ��ch��Ϣ                                ��
�������������������������������������ߩ��������������������������������������ߩ���������������������������������������
���������������������ש����������������ש����������������������������������ש�������������������������������������
��    ����          ����              ��    ��������                      ��    ִ������                        ��
�ǩ������������������ߩ����������������贈���������������������������������贈������������������������������������
��   void ngx_signal_worker_          ��                                  ��                                    ��
��processes (ngx_cycle_t *cycle,      ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ��  ����worker���̽��յ����ź�        ��
��                                    ������ָ�룬signo���ź�             ��                                    ��
��int signo)                          ��                                  ��                                    ��
�ǩ������������������������������������贈���������������������������������贈������������������������������������
��                                    ��                                  ��  ���master���̵������ӽ��̣���    ��
��  ngx_uint_t ngx_reap_children      ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ����ÿ���ӽ��̵�״̬��ngx_process_t�� ��
��(ngx_cycle_t *cycle)                ������ָ��                          �������еı�־λ���ж��Ƿ�Ҫ�����ӽ�  ��
��                                    ��                                  ���̡�����pid�ļ���                   ��
�ǩ������������������������������������贈���������������������������������贈������������������������������������
��  voidngx_maste,r_process exit      ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ��                                    ��
��(ngx_cycle_t *cyc, le)              ������ָ��                          ��  �˳�master���̹�����ѭ��          ��
�ǩ������������������ש����������������贈���������������������������������贈������������������������������������
��  void ngx_wo?    ��Lker_process_   ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ��                                    ��
��cycle (ngx_cycle  ��-t *cycle, void ������ָ�룬���ﻹδ��ʼʹ��data��  ��  ����worker���̹�����ѭ��          ��
��*data)            ��                ����������data -��ΪNULL            ��                                    ��
�ǩ������������������ߩ����������������贈���������������������������������贈������������������������������������
��   void ngx_worker_process_         ��  cycle����ǰ���̵�ngx_cycle_t��  ��  ����worker���̹���ѭ��֮ǰ�ĳ�    ��
��init (ngx_cycle_t *cycle, ngx_      ������ָ�룬priority��worker���̵�  ��ʼ������                            ��
��uint_t priority)                    ��ϵͳ���ȼ�                        ��                                    ��
�ǩ������������������������������������贈���������������������������������贈������������������������������������
��  void ngx_woriker_process_         ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ��                                    ��
��exit (ngx_cycle_t, *cycle)          ������ָ��                          ��  �˳�worker���̹�����ѭ��          ��
�ǩ������������������������������������贈���������������������������������贈������������������������������������
��  void ngx_cac:he_manager_          ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ��  ִ�л����������ѭ������������  ��
��process_cycle(ngx_cycle_t           ������ָ�룬data�Ǵ��˵�ngx_cache_  ���ļ�����ģ��������أ��ڱ����в���  ��
��*cycle, void *data)                 ��manager_ctx_t�ṹ��ָ��           ����ϸ̽��                            ��
�ǩ������������������������������������贈���������������������������������贈������������������������������������
��   void ngx_process_events_         ��  cycle�ǵ�ǰ���̵�ngx_cycle_t��  ��  ʹ���¼�ģ�鴦���ֹ�������Ѿ���  ��
��                                    ��                                  ���������¼����ú������°�ģ��ʵ�֣�  ��
��and_timers (ngx_cycle_t *cycle)     ������ָ��                          ��                                    ��
��                                    ��                                  ��                                    ��
���������������������������������������ߩ����������������������������������ߩ�������������������������������������
*/
//http://tech.uc.cn/?p=300 ��������������ݽṹ�ο�
//��ʼ���ο�ngx_init_cycle��������һ��ȫ�����͵�ngx_cycle_s����ngx_cycle
struct ngx_cycle_s {
    /*     ����������ģ��洢������Ľṹ��ָ�룬     ��������һ�����飬�����СΪngx_max_module��������Nginx��module����һ����     
    ÿ�������Ա����һ��ָ�룬ָ����һ���洢��ָ������飬��˻ῴ��void ****    ����ջ��������������Nginx-ģ�鿪����ܹ�������
    һ��302ҳ��ͼ��    ���⣬���ͼҲ����http://img.my.csdn.net/uploads/201202/9/0_1328799724GTUk.gif    

    ÿ�������ж���һ��Ψһ��ngx_cycle_t���Ľṹ�壬����һ����Աconf_ctxά��������ģ������ýṹ�壬
  ��������void ****conf_ctx��conf_ctx����Ϊ����ָ��һ����Ա��Ϊָ������飬����ÿ����Աָ����ָ������һ��
  ��Ա��Ϊָ������飬��2���������еĳ�Աָ��Ż�ָ���ģ�����ɵ����ýṹ�塣������Ϊ���¼�ģ
  �顢httpģ�顢mailģ�����Ƶģ��������ڲ�ͬ��NGX_CORE_MODULE���͵�
  �ض�ģ����������Ȼ����NGX_CORE_MODULE���͵ĺ���ģ�����������ʱ��������һ����ȫ�ֵģ�
  ����������κ�{}���ÿ�ģ�������Ҫ��������˫������ơ�������ʶΪNGX_DIRECT_CONF���͵���
  ����ʱ�����void****���͵�conf_ctxǿ��ת��Ϊvoid**��Ҳ����˵����ʱ����conf_ctxָ���ָ������
  �У�ÿ����Աָ�벻��ָ���������飬ֱ��ָ�����ģ�����ɵ������ڹ��塣��ˣ�NGX_DIRECT_CONF
  ����NGX_CORE_MODULE���͵ĺ���ģ��ʹ�ã�����������ֻӦ�ó�����ȫ�������С�
    */ //��ʼ����ngx_init_cycle������Ϊhttp{} server{} location{}����Ŀռ䶼�ɸ�ָ��ָ���¿��ٵĿռ�
    //NGX_CORE_MODULE����ģ�鸳ֵ��ngx_init_cycle 
    //http{}ngx_http_module���ģ�鸳ֵ�ط���ngx_http_block
    //�ο�:http://tech.uc.cn/?p=300   ngx_http_conf_ctx_t������ָ��ctx�洢��ngx_cycle_t��conf_ctx��ָ���ָ�����飬��ngx_http_module��indexΪ�±������Ԫ��
    //��ngx_http_block�У�conf_ctx�е�����ָ��ᱻ��ֵ��ngx_conf_s��ctx������ngx_conf_s->ctx�͵���conf_ctx����ؿռ�
    //�����conf_ctx[]�����Աָ��ֱ��ָ��NGX_CORE_MODULE���͵����������п��ٵ�ngx_http_conf_ctx_t�ռ䣬����ngx_http_block�п��ٵĿռ�

    /*
    Ϊʲô�����������4���ף���Ϊ������ָ��һ�����ָ������飬��������е�ָ���Աͬʱ��ָ��������һ�����ָ������飬������4��4��
    ����conf- ctx�İ����˰ɡ�ֻ��ӵ�������conf_ctx���ſ��Կ�������һ��ģ����create_conf�в����Ľṹ��ָ�롣ͬ��HTTPģ���mail
    ģ��Ҳ���������ģ���Щģ���ͨ�ýӿ���Ҳ��create_conf�������������ָ��������Ƶķ�ʽ��š�
    ͼ�λ��ο�:�������NGINX�е�ͼ9-2
    */

/*
    �ں��Ľṹ��ngx_cycle_t��conf_ctx��Աָ���ָ�������У���7��ָ����ngx_http_moduleģ��ʹ�ã�ngx_http_moduleģ���index���Ϊ6��
������0��ʼ����������ngx_modules���������е�7���ڴ��ȫ�����ýṹ���conf_ctx�����У���7����Աָ��ngx_http_moduleģ�飩�����ָ��
����Ϊָ�����http{}��ʱ���ɵ�ngx_http_conf_ctx_t�ṹ�壬��ngx_http_conf_ctx_t��3����Ա��ֱ�ָ���·����3��ָ�����顣�µ�ָ��������
��Ա��������ÿ��HTTPģ���ctx_index���ָ����ctx_index��HTTPģ���б���������HTTPģ������ţ������磬��6��HTTPģ���ctx_index��5
��ctx_indexͬ����0��ʼ����������ô��ngx_http_conf_ctx_t��3�������У���6����Ա��ָ���6��HTTPģ���create_main_conf��create_srv_conf��
create_loc_conf���������Ľṹ�壬��Ȼ�������Ӧ�Ļص�����û��ʵ�֣���ָ���ΪNULL��ָ�롣
*/ 
/*
ͼ��ο�ͼ10-1  ��ngx_init_cycle  conf.ctx = cycle->conf_ctx; //���������ngx_conf_param�������õ�ʱ�������conf.ctx��ֵ������ʵ���Ͼ��Ƕ�cycle->conf_ctx[i]
�������ngx_cycle_t���Ľṹ�����ҵ�main��������ýṹ���أ�Nginx�ṩ��ngx_http_cycle_get_module_main_conf�����ʵ���������
*/
    void                  ****conf_ctx; //�ж��ٸ�ģ��ͻ��ж��ٸ�ָ����Щģ���ָ�룬��ngx_init_cycle   ngx_max_module
    ngx_pool_t               *pool; // �ڴ��

    /*    ��־ģ�����ṩ�����ɻ���ngx_log_t��־����Ĺ��ܣ������logʵ�������ڻ�û��ִ��ngx_init_cycle����ǰ��    
    Ҳ���ǻ�û�н�������ǰ���������Ϣ��Ҫ�������־���ͻ���ʱʹ��log���������������Ļ��    
    ��ngx_init_cycle����ִ�к󣬽������nginx.conf�����ļ��е�������������ȷ����־�ļ�����ʱ���log���¸�ֵ��    */
    //ngx_init_cycle�и�ֵcycle->log = &cycle->new_log;
    ngx_log_t                *log; //ָ��ngx_log_init�е�ngx_log���������error_log��ָ��������ú�����ļ���������ngx_error_log��������ngx_log_open_default������
    
    /* ��nginx.conf�����ļ���ȡ����־�ļ�·���󣬽���ʼ��ʼ��error_log��־�ļ�������log���������������־����Ļ��    
    ��ʱ����new_log������ʱ�Ե����log��־������ʼ���ɹ��󣬻���new_log�ĵ�ַ���������logָ��    */
    // ���û������error_log����ngx_log_open_default����ΪNGX_ERROR_LOG_PATH�����ͨ��error_log�����ù���ͨ��ngx_log_set_log��ӵ���new_log->next������������
    /* ȫ�������õ�error_log xxx�洢��ngx_cycle_s->new_log��http{}��server{}��local{}���õ�error_log������ngx_http_core_loc_conf_t->error_log,
    ��ngx_log_set_log,���ֻ����ȫ��error_log��������http{}��server{}��local{}����ngx_http_core_merge_loc_conf conf->error_log = &cf->cycle->new_log;  */
    //ngx_log_insert���룬��ngx_log_error_core�ҵ���Ӧ�������־���ý����������Ϊ��������error_log��ͬ�������־�洢�ڲ�ͬ����־�ļ���
    ngx_log_t                 new_log;//�������error_log��ָ��������ú�����ļ���������ngx_error_log��������ngx_log_open_default������

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    /*  ����poll��rtsig�������¼�ģ�飬������Ч�ļ��������Ԥ�Ƚ�����Щngx_connection t�ṹ
�壬�Լ����¼����ռ����ַ�����ʱfiles�ͻᱣ������ngx_connection_t��ָ����ɵ����飬files_n����ָ
������������ļ������ֵ��������files�����Ա */
    ngx_connection_t        **files; //sizeof(ngx_connection_t *) * cycle->files_n  ��ngx_event_process_init  ngx_get_connection

    /*
        ��ͼ9-1�п��Կ�������ngx_cycle_t�е�connections��free_connections��������Ա������һ�����ӳأ�����connectionsָ��������
    �ӳ�������ײ�����free_connections��ָ���һ��ngx_connection_t�������ӡ����еĿ�������ngx_connection_t����data��Ա����9.3.1�ڣ���
    Ϊnextָ�봮����һ����������ˣ�һ�����û���������ʱ�ʹ�free_connectionsָ�������ͷ��ȡһ�����е����ӣ�ͬʱfree_connections��ָ
    ����һ���������ӡ����黹����ʱֻ��Ѹ����Ӳ��뵽free_connections�����ͷ���ɡ�
     */ //��ngx_event_process_init, ngx_connection_t�ռ�������еĶ�дngx_event_t�洢�ռ䶼�ڸú���һ���Է����
    ngx_connection_t         *free_connections;// �������ӳأ���free_connection_n���ʹ��
    ngx_uint_t                free_connection_n;// �������ӳ������ӵ�����

    //ngx_connection_s�е�queue��ӵ���������
    /*
    ͨ�������������ж������Ƿ�����������������Ļ����ͻ�Ѹ�ngx_close_connection->ngx_free_connection�ͷų���������
    ���֮ǰfree_connections��û�п���ngx_connection_t��c = ngx_cycle->free_connections;�Ϳ��Ի�ȡ���ղ��ͷų�����ngx_connection_t
    ��ngx_drain_connections
    */ 
    ngx_queue_t               reusable_connections_queue;/* ˫������������Ԫ��������ngx_connection_t�ṹ�壬��ʾ���ظ�ʹ�����Ӷ��� ��ʾ�������õ����� */

//ngx_http_optimize_servers->ngx_http_init_listening->ngx_http_add_listening->ngx_create_listening�ѽ�������listen��������Ϣ��ӵ�cycle->listening��
    //ͨ��"listen"���ô���ngx_listening_t���뵽��������
    ngx_array_t               listening;// ��̬���飬ÿ������Ԫ�ش�����ngx_listening_t��Ա����ʾ�����˿ڼ���صĲ���

    /*    ��̬������������������nginx����Ҫ������Ŀ¼�������Ŀ¼�����ڣ��ͻ���ͼ������������Ŀ¼ʧ�ܾͻᵼ��nginx����ʧ�ܡ�    */
    //ͨ�����������ļ���ȡ����·����ӵ������飬����nginx.conf�е�client_body_temp_path proxy_temp_path���ο�ngx_conf_set_path_slot
    //��Щ���ÿ��������ظ���·������˲���Ҫ�ظ�������ͨ��ngx_add_path�����ӵ�·���Ƿ��ظ������ظ�����ӵ�paths��
    ngx_array_t               paths;//�����Ա nginx_path_t ��
    ngx_array_t               config_dump;

    /*    ������������Ԫ��������ngx_open_file_t �ṹ�壬����ʾnginx�Ѿ��򿪵������ļ�����ʵ�ϣ�nginx��ܲ�����open_files����������ļ���    
    �����ɶԴ˸���Ȥ��ģ������������ļ�·������nginx��ܻ���ngx_init_cycle �����д���Щ�ļ�    */
    //�����������������ļ��Ĵ���ngx_init_cycle�д�
    ngx_list_t                open_files; //��nginx.conf�����ļ��е�access_log�������ļ��ͱ����ڸ������У��ο�ngx_conf_open_file
    //����ngx_shm_zone_t��ngx_init_cycle����ngx_shared_memory_addҲ���ܴ����µ�ngx_shm_zone_t��Ϊÿ��ngx_shm_zone_t�������乲���ڴ�ռ���ngx_init_cycle
    ngx_list_t                shared_memory;// ������������Ԫ��������ngx_shm_zone_t�ṹ�壬ÿ��Ԫ�ر�ʾһ�鹲���ڴ�

    ngx_uint_t                connection_n;// ��ǰ�������������Ӷ������������connections��Ա���ʹ��
    ngx_uint_t                files_n; //ÿ�������ܹ��򿪵�����ļ���  ��ֵ��ngx_event_process_init

    /*
    ��ͼ9-1�п��Կ�������ngx_cycle_t�е�connections��free_connections��������Ա������һ�����ӳأ�����connectionsָ���������ӳ�������ײ���
    ��free_connections��ָ���һ��ngx_connection_t�������ӡ����еĿ�������ngx_connection_t����data��Ա����9.3.1�ڣ���Ϊnextָ�봮����һ��
    ��������ˣ�һ�����û���������ʱ�ʹ�free_connectionsָ�������ͷ��ȡһ�����е����ӣ�ͬʱfree_connections��ָ����һ��������
    �ӡ����黹����ʱֻ��Ѹ����Ӳ��뵽free_connections�����ͷ���ɡ�

    ��connectionsָ������ӳ��У�ÿ����������Ҫ�Ķ�/д�¼�������ͬ��������Ŷ�Ӧ��read_events��write_events��/д�¼����飬
    ��ͬ�������3�������е�Ԫ�������ʹ�õ�
     */
    ngx_connection_t         *connections;// ָ��ǰ�����е��������Ӷ�����connection_n���ʹ��

    /*
    �¼��ǲ���Ҫ�����ģ���ΪNginx������ʱ�Ѿ���ngx_cycle_t��read_events��Ա��Ԥ���������еĶ��¼�������write_events��Ա��Ԥ���������е�д�¼�

    ��connectionsָ������ӳ��У�ÿ����������Ҫ�Ķ�/д�¼�������ͬ��������Ŷ�Ӧ��read_events��write_events��/д�¼����飬��ͬ�������
    3�������е�Ԫ�������ʹ�õġ�ͼ9-1�л���ʾ���¼��أ�Nginx��Ϊÿһ������һ��������Ҫһ�����¼���һ��д�¼����ж������Ӿͷ�����ٸ�����
    д�¼������������ӳ��е���һ����������¼���д�¼���Ӧ�����أ��ܼ򵥡����ڶ��¼���д�¼������ӳ�����3����С��ͬ��������ɣ����Ը�������
    ��žͿɽ�ÿһ�����ӡ����¼���д�¼���Ӧ�����������Ӧ��ϵ��ngx_event_core_moduleģ��ĳ�ʼ�������о��Ѿ������ˣ��μ�9.5�ڣ�����3������
    �Ĵ�С������cycle->connection_n������
     */
    ngx_event_t              *read_events;// ָ��ǰ�����е����ж��¼�����connection_nͬʱ��ʾ���ж��¼�������
    ngx_event_t              *write_events;// ָ��ǰ�����е�����д�¼�����connection_nͬʱ��ʾ����д�¼�������

    /*    �ɵ�ngx_cycle_t ��������������һ��ngx_cycle_t �����еĳ�Ա������ngx_init_cycle ���������������ڣ�    
    ��Ҫ����һ����ʱ��ngx_cycle_t���󱣴�һЩ������ 
    �ٵ���ngx_init_cycle ����ʱ�Ϳ��԰Ѿɵ�ngx_cycle_t ���󴫽�ȥ��    ����ʱold_cycle����ͻᱣ�����ǰ�ڵ�ngx_cycle_t����    */
    ngx_cycle_t              *old_cycle;

    ngx_str_t                 conf_file;// �����ļ�����ڰ�װĿ¼��·������ Ĭ��Ϊ��װ·���µ�NGX_CONF_PATH,��ngx_process_options
    ngx_str_t                 conf_param;// nginx ���������ļ�ʱ��Ҫ���⴦�����������Я���Ĳ�����һ����-g ѡ��Я���Ĳ���
    ngx_str_t                 conf_prefix;    // nginx�����ļ�����Ŀ¼��·��  ngx_prefix ��ngx_process_options
    ngx_str_t                 prefix; //nginx��װĿ¼��·�� ngx_prefix ��ngx_process_options
    ngx_str_t                 lock_file;// ���ڽ��̼�ͬ�����ļ�������
    ngx_str_t                 hostname; // ʹ��gethostnameϵͳ���õõ���������  ��ngx_init_cycle�д�д��ĸ��ת��ΪСд��ĸ
};

typedef struct { //��ngx_cycle_s�е�conf_ctxָ������
     ngx_flag_t               daemon;
     ngx_flag_t               master; //��ngx_core_module_init_conf�г�ʼ��Ϊ1  ͨ������"master_process"����

     ngx_msec_t               timer_resolution; //��timer_resolutionȫ�������н������Ĳ���,��ʾ����msִ�ж�ʱ���жϣ�Ȼ��epoll_wail�᷵�ظ����ڴ�ʱ��

     ngx_int_t                worker_processes;  //������worker��������ͨ��nginx���ã�Ĭ��Ϊ1  "worker_processes"����
     ngx_int_t                debug_points;

     ngx_int_t                rlimit_nofile;
     off_t                    rlimit_core;//worker_rlimit_core 1024k;  coredump�ļ���С

     int                      priority;

     /*
     worker_processes 4;
     worker_cpu_affinity 0001 0010 0100 1000; �ĸ��������̷ֱ����ĸ�ָ����he��������
     
     �����5he������������
     worker_cpu_affinity 00001 00010 00100 01000 10000; �����������
     */  //�ο�ngx_set_cpu_affinity
     ngx_uint_t               cpu_affinity_n; //worker_cpu_affinity��������
     uint64_t                *cpu_affinity;//worker_cpu_affinity 00001 00010 00100 01000 10000;ת����λͼ�������0X11111

     char                    *username;
     ngx_uid_t                user;
     ngx_gid_t                group;

     ngx_str_t                working_directory;//working_directory /var/yyz/corefile/;  coredump���·��
     ngx_str_t                lock_file;

     ngx_str_t                pid;
     ngx_str_t                oldpid;

     ngx_array_t              env;
     char                   **environment;
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
uint64_t ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_dump_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
