#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/03/01＞         *
 *        CODED                                 :＜ISYS＞                      *
 *                                                                             *
 *        MODIFY DATE                           :＜修正日 yyyy／mm／dd＞       *
 *        CODED                                 :＜修正者＞                    *
 *                                              :＜修正概要＞                  *
 ******************************************************************************/
/*****************************************************************************/
/*****                    <<     head PROGRAM      >>                    *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCVX20                                    */
/*        FUNCTION          ････ コネクション制御(クライアント)              */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/03/01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/03/01 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <arpa/inet.h> nolist
#include <stdint.h>
#include <netdb.h> nolist
#include <netinet/in.h> nolist
#include <sys/socket.h> nolist
/* USER HEADER     */
#ifdef _TANDEM_SOURCE
#ifndef __db_gflin_def__
#define __db_gflin_def__
#include <file.h(db_gflin)> nolist
#endif
#ifndef __db_gfnwi_def__
#define __db_gfnwi_def__
#include <file.h(db_gfnwi)> nolist
#endif
#ifndef __db_gfphi_def__
#define __db_gfphi_def__
#include <file.h(db_gfphi)> nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h> nolist
#endif
#endif
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_ipc_wrapper.h" nolist // IWYU pragma: keep

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_TRANSPORT_TIMER 0x8004
typedef enum __cmp_trans_status_t
{
    e_trans_uninitialized,       // 未初期化状態
    e_trans_wait_initial_odst,
    e_trans_idle,
    e_trans_wait_header,
    e_trans_wait_body,
    e_trans_in_dist_outstanding_overflow
} cmp_trans_status_t;

typedef enum __line_status_t
{
    e_line_disconnected,         // 切断状態(回線状態ファイル込み)
    e_line_connect_in_progress,  // 接続中(接続・再接続タイマー待ち)
    e_line_connected,            // 接続完了
    e_line_uninitialized         // 未初期化状態
} line_status_t;

typedef enum __proc_status_t
{
    e_proc_open,                 // Openコマンド受信済み
    e_proc_close,                // Closeコマンド受信済み
    e_proc_none                  // コマンド受信状態無し
} proc_status_t;

typedef int line_sts_errcd_t;
typedef enum __line_discon_reason_t
{
    e_discon_by_cls_cmd,         // "00"：クローズコマンドによる切断
    e_discon_by_cls_instruct,    // "10"：切断指示/入替指示インタフェースによる切断
    e_discon_by_detect,          // "91"：コネクション切断検出
    e_discon_by_re_con_over,     // "92"：コネクション再接続リトライオーバ
    e_discon_none                // "接続中"
} line_discon_reason_t;

// 電文長属性
typedef enum __data_len_attribute_t
{
    e_binary,
    e_bcd,
    e_ascii,
    e_ebcdic,
    e_data_len_attribute_fault
} d_len_attr_t;
// コネクション後処理情報
typedef enum __aft_con_opt_t
{
    e_none_after_connect,          // 無し
    e_open_require_after_connect,  // "SO"：開局電文を送信
    e_send_data_after_connect      //"DT"：特定データを送信
} aft_con_opt_t;

typedef char       tcpip_prc_name_t[6 + 1];
typedef char       line_sts_update_time_t[20 + 1];
typedef char       prc_sts_update_time_t[14 + 1];
typedef char       c_ip_address_t[15];
typedef char       c_ip_port_t[5];

/* コネクション状態 */
extern const char *line_sts_list[];

/* プロセス状態 */
extern const char *proc_sts_list[];

/* 切断理由 */
extern const char *line_close_reason_list[];

extern long  cncl_current_inbound_outstanding;

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/


#ifndef __type__serverclass_info_t
#define __type__serverclass_info_t
typedef struct __serverclass_info_t serverclass_info_t;
#endif
#pragma fieldalign shared2 __serverclass_info_t
struct __serverclass_info_t{
    char            pathmon_name[16 + 1];     /*PATHMON名*/
    short           pathmon_name_len;         /*PATHMON名*/
    char            s9s_name[16 + 1]; /*サーバクラス名*/
    short           s9s_name_len;     /*サーバクラス名*/
};


#pragma fieldalign shared2 __cmp_trans_t
#pragma fieldalign shared2 __gclst_t
#pragma fieldalign shared2 __transport_config_t
#pragma fieldalign shared2 __in_dist_pathsend_info_t
typedef struct __cmp_trans_t
{
    cmp_trans_status_t     trans_status;
    size_t                 cur_trans_length;
    size_t                 cur_pending_length;
    // 回線情報
    line_status_t          line_status;
    line_sts_update_time_t connect_sts_update_time;
    proc_status_t          proc_status;
    prc_sts_update_time_t  prc_sts_update_time;
    line_sts_errcd_t       line_discon_socket_errcd;
    line_discon_reason_t   line_discon_reason;
    unsigned short         current_local_port;   // 実際に接続したローカルポート

    long                   retry_cnt_short;
    long                   retry_cnt_long;

    short                  socket;
    db_gflin_def           gflin;                // 回線ステータス情報
    db_gfnwi_def           gfnwi;                // NW情報
    db_gfphi_def           gfphi_in_dist;        // 電文振分(inbound)サーバクラス
    db_gfphi_def           gfphi_cmd_if;         // コマンドI/Fサーバクラス
    io_trace_buf_t        *rcv_buff;             // 受信バッファ
    thread_object_t       *snd;
    thread_object_t       *rcv;
    struct __gclst_t
    {
        iom_params_def *gclst_info;              // 回線ステータス情報ファイルのオープン情報(共有)
    } gclst;
    struct __transport_config_t
    {
        tcpip_prc_name_t   tcpip_prc_name;       // transport sokect
        struct sockaddr_in local_if;             // local interface
        struct sockaddr_in remote_host;          // local buffer
        int                so_snd_buff;
        int                so_rcv_buff;
        long               con_complete_timer;   // CONNECT完了待ちタイマー(1/100sec)
        long               send_complete_timer;  // SEND完了待ちタイマー(1/100sec)
        long               recv_complete_timer;  // 後続データ受信待ちタイマー(1/100sec)
        long               idle_mon_timer;       // 無通信監視タイマー(1/100sec)
        long               retry_limit_s;        // 回線障害回復リトライ間隔（ショート）(1/100sec)
        long               retry_limit_l;        // 回線障害回復リトライ間隔（ロング）(1/100sec)
        long               retry_interval_s;     // 回線障害回復リトライ回数（ショート）(1/100sec)
        long               retry_interval_l;     // 回線障害回復リトライ回数（ロング）(1/100sec)
        long               data_len_start_lct;   // データ長開始位置
        long               data_len_size;        // データ長バイト数
        long               data_len_include_id;  // true:lengthを全電文長に含める false:含めない
        long               denbun_start_lct;
        long               header_length;
        d_len_attr_t       data_len_attribute;   // 電文長属性
        aft_con_opt_t      after_connect_opt;    // コネクション後処理情報
    } config;
    serverclass_info_t    in_dist_s9s_info;
    struct __in_dist_pathsend_info_t
    {
        io_mem_q_t        in_dist_p6d_q;
        long              *outstanding;
    } in_dist_p6d_info;
    serverclass_info_t    cmd_if_s9s_info;
} cmp_trans_t;

#pragma fieldalign shared2 __update_gclst_t
typedef short update_gclst_s_t;
typedef union __update_gclst_t
{
    update_gclst_s_t update_gclst_s;
    struct
    {
        bool connection_status : 1;
        bool proccess_status : 1;
        int  filler : 14;
    } update_gclst;
} update_gclst_t;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
// 内部関数
short create_sockadd_in(struct sockaddr_in *sockaddr, const c_ip_address_t c_ip_address, const c_ip_port_t c_ip_port);
line_status_t chk_line_sts(const char *connect_sts);
proc_status_t chk_proc_sts(const char *prc_sts);
line_discon_reason_t chk_line_discon_reason(const char *reason);
short set_socket_option(short socket, int level, int option, int value);
short cncl_trans_send_request(thread_object_t *thread_trans, io_trace_buf_t *send_req);
short cncl_trans_connect_retry(thread_object_t *thread, short close_reason);
short cncl_trans_post_connect_proc_sign_on(thread_object_t *thread_trans);
short cncl_trans_post_connect_proc_dt_send(thread_object_t *thread_trans);
short cncl_trans_msg_rcv_notify_c201_post(thread_object_t *thread_trans);
short cncl_trans_opening_request_c501_send(thread_object_t *thread_trans,long retry_count);
size_t cncl_trans_opening_request_c501(thread_object_t *thread_trans, c501_def *opening_request);
short cncl_trans_send_dt(thread_object_t *thread_trans);
void cncl_delete_trans(thread_object_t *thread_trans);
short cncl_trans_connect(thread_object_t *thread_trans);
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
short cncl_trans_notify_line_status(thread_object_t *trans_thread,bool already_connected);
bool cncl_trans_p6d_error(Event_tag_t *p6d_event);
short cncl_trans_msg_rcv_notify_c201_send(thread_object_t *thread_trans);
void cncl_handle_await_io(int32_t timelimit);
short cncl_process_event_list();

#ifdef _TANDEM_SOURCE
_extensible
#endif
    short
    cncl_trans_read_gclst(thread_object_t *thread_trans, line_status_t *line_status, proc_status_t *trans_proc_status);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    update_gclst_s_t
    cncl_trans_update_gclst(thread_object_t *thread_trans, line_status_t line_status, proc_status_t proc_status,
                            line_sts_errcd_t line_discon_socket_errcd, line_discon_reason_t line_discon_reason);
/* 公開メソッド */
thread_object_t *cncl_create_trans(db_gflin_def *gflin, db_gfnwi_def *gfnwi, db_gfphi_def *in_dist_srvcls,
                                   db_gfphi_def *cmd_if_srvcls, iom_params_def *gclst_info);
thread_object_t *cncl_trans_req_send_select_transport(io_trace_buf_t *io_trace_buf);

// 電文作成
void cncl_trans_request_c107(thread_object_t *trans_thread, bool connect, c107_def *c107_req);
size_t cncl_trans_request_c201(thread_object_t *trans_thread, c201_def *c201_req, const char *rcv_data,
                               size_t data_length, TS_UNIQUE_128 *ts128);

// 内部イベント関数
short cncl_trans_send_error(Event_tag_t *event);
short cncl_trans_connect_complete(Event_tag_t *event);
short cncl_trans_connect_timeout(Event_tag_t *event);
short cncl_trans_retry_timer_expire(Event_tag_t *event);
short cncl_trans_close(Event_tag_t *event);
short cncl_trans_receive(thread_object_t *thread_trans);
short cncl_trans_receive_complete(Event_tag_t *event);
short cncl_trans_receive_timeout(Event_tag_t *event);
short cncl_trans_connect_request(Event_tag_t *event);
short cncl_trans_close_request(Event_tag_t *event);
short cncl_trans_opening_request_c501_send_complete(Event_tag_t *event);
short cncl_trans_msg_rcv_notify_c201_send_complete(Event_tag_t *event);
short cncl_trans_msg_rcv_notify_c201_send_event(Event_tag_t *event_trans);

// データ長デコーダ
size_t cncl_trans_length_decoder(const char *val, size_t length, d_len_attr_t data_len_attribute, int *i_err);
size_t EBCDIC_len_decoder(const unsigned char *num_str, size_t len, size_t *r_len, short *s_err);
size_t BIN_len_decoder(const char *raw_length, size_t raw_length_size, int *i_err);
size_t BCD_len_decoder(const char *raw_length, size_t raw_length_size, int *i_err);

/****************************************************************************/
/*  マクロ定義                                                         */
/****************************************************************************/
#define transport_gflin_key(cmp_trans_ptr)  (gflin_pkey_def*)&((cmp_trans_ptr)->gflin.pri_key)
