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
/*****                    <<     SOURCE PROGRAM      >>                  *****/
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
#include <stdio.h>
#ifndef _TANDEM_SOURCE
typedef unsigned int  u_int;
typedef unsigned char u_char;
#endif
#include <errno.h> nolist
#include <limits.h> nolist
#include <netinet/tcp.h> nolist
#include <stdint.h> nolist
#include <string.h> nolist
#ifdef _TANDEM_SOURCE
#include <cextdecs.h(AWAITIOX,FILE_CLOSE_,FILE_GETINFO_,SIGNALTIMEOUT)> nolist
#include <cextdecs.h(CANCELTIMEOUT,CANCEL,CANCELREQ,TS_UNIQUE_CREATE_)> nolist
#include <cextdecs.h(JULIANTIMESTAMP,SERVERCLASS_SEND_INFO_,SERVERCLASS_SEND_)> nolist
#else
#include <cextdecs.h> nolist
#endif
/* USER HEADER     */
#include <GFPCGX50.h> nolist
#include <GFPCGXA0.h> nolist
#include <errcd.h> nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_cmp_ntf_odst_procs.h" nolist
#include "GFPCVX20_cmp_rcv.h" nolist
#include "GFPCVX20_cmp_snd.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_sys.h" nolist
#include "GFPCVX20_thread_factory.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_util.h" nolist
#include <common.h> nolist
#include "GFPCVX20_ipc_wrapper.h" nolist // IWYU pragma: keep

#ifdef _TANDEM_SOURCE
#ifndef __db_gclst__
#define __db_gclst__
#include <file.h(db_gclst)> nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h>  nolist
#endif
#endif

/****************************************************************************/
/*   グローバル変数定義                                                             */
/****************************************************************************/
// inbound振分へのPathsend数 最大 CCC_MAX_inbound 50
long cncl_current_inbound_outstanding = 0;

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define SOCK_PROTOCOL_NOUSE    0
#define SOCK_FLG_NOWAIT        2 /* recv & send */
#define SOCK_SYNC_NOUSE        0

#define FESCINVALIDPATHMONNAME 901
#define FESCSERVERLINKCONNECT  904
#define FESCOPERATIONABORTED   918
#define FESCNOSERVERAVAILABLE  905
#define FESCSERVERCLASSFROZEN  913
#define FESCPATHMONSHUTDOWN    915

/* コネクション状態 */
const char *line_sts_list[] = {
    DEF_CONNECT_STS_DISCONN,   /* 切断                                     */
    DEF_CONNECT_STS_RECONNECT, /* 再接続処理中                             */
    DEF_CONNECT_STS_CONNECT,   /* 接続                                     */
};

/* プロセス状態 */
const char *proc_sts_list[] = {
    DEF_PROC_STS_OPN,                                                /* 回線オープンコマンド受信済み             */
    DEF_PROC_STS_CLS,                                                /* 回線クローズコマンド受信済み             */
};
const char *line_close_reason_list[] = {DEF_DISCON_BY_CLS_CMD,       // クローズコマンドによる切断
                                        DEF_DISCON_BY_CLS_INSTRUCT,  // 切断指示/入替指示インタフェースによる切断
                                        DEF_DISCON_BY_DETECT,        // コネクション切断検出
                                        DEF_DISCON_BY_RE_CON_OVER,   // コネクション再接続リトライオーバ
                                        "  "};

/****************************************************************************/
/*   内部関数定義                                                             */
/****************************************************************************/
void serverclass_info(serverclass_info_t *dst, const db_gfphi_def *s9s_info_rec);

/*****************************************************************************/
/*  FUNCTION        :create_sockadd_in                                       */
/*  CALLING SEQ.    :short create_sockadd_in(struct sockaddr_in *sockaddr,   */
/*                   const c_ip_address_t c_ip_address,                      */
/*                   const c_ip_port_t c_ip_port)                            */
/*  ARGUMENT        :sockaddr    :アドレス構造体へのポインタ                 */
/*                  :c_ip_address:文字列形式のIPアドレス                     */
/*                  :c_ip_port   :文字列形式のポート番号                     */
/*  RETURN CODE     :0(成功),エラーコード                                    */
/*  DESCRIPTION     :IPアドレス文字列とポート番号をsockaddr_inへ設定         */
/*****************************************************************************/
short create_sockadd_in(struct sockaddr_in *sockaddr, const c_ip_address_t c_ip_address, const c_ip_port_t c_ip_port)
{
    size_t r_len = 0;
    short  s_err = 0;
    char   str_ip_address[sizeof(c_ip_address_t) + 1];

    sockaddr->sin_family = AF_INET;
    memset(str_ip_address, 0, sizeof(str_ip_address));
    memcpy(str_ip_address, c_ip_address, sizeof(c_ip_address_t));
    cncl_rm_trspc_wn(str_ip_address, sizeof(str_ip_address));
    sockaddr->sin_addr.s_addr = inet_addr(c_ip_address);
    sockaddr->sin_port        = (short)str2ul_c(c_ip_port, sizeof(c_ip_port_t), &r_len, &s_err);
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_create_trans                                       */
/*  CALLING SEQ.    :thread_object_t *cncl_create_trans(                     */
/*                   db_gflin_def *gflin,                                    */
/*                   db_gfnwi_def *gfnwi,                                    */
/*                   db_gfphi_def *in_dist_srvcls,                           */
/*                   db_gfphi_def *cmd_if_srvcls,                            */
/*                   iom_params_def *gclst_info)                             */
/*  ARGUMENT        :gflin         :回線情報                                 */
/*                  :gfnwi         :NW情報                                   */
/*                  :in_dist_srvcls:インバウンド用のSrvCls情報               */
/*                  :cmd_if_srvcls :コマンドIF用のSrvCls情報                 */
/*                  :gclst_info   :回線ステータス管理用I/O情報               */
/*  RETURN CODE     :生成したtransport用スレッドオブジェクトのポインタ       */
/*  DESCRIPTION     :通信回線の送受信・管理を行うスレッドを生成              */
/*****************************************************************************/
thread_object_t *cncl_create_trans(db_gflin_def *gflin, db_gfnwi_def *gfnwi, db_gfphi_def *in_dist_srvcls,
                                   db_gfphi_def *cmd_if_srvcls, iom_params_def *gclst_info)
{
    Application     *app    = cncl_get_App();
    myinfo_def      *myInfo = &(app->my_info);
    thread_object_t *thread;
    short            s_err;
    size_t           r_len;
    size_t           config_set_index = 0;

    thread                            = cncl_create_thread(transport);
    if (!thread) return NULL;
    cmp_trans_t     *cmp_trans              = cncl_get_component(thread);
    thread_header_t *hd                     = cncl_get_threadInfo(thread);

    // NW情報ファイルからの取得項目の変換定義
    typedef struct __config_set_t
    {
        long  *param_val_l;
        char  *param_val_c;
        size_t length;
        long   default_value;
        char  *param_name;
    } config_set_t;
    config_set_t config_set[] = {
        {&(cmp_trans->config.con_complete_timer),  gfnwi->trans_cntrl_tmr_info.connect_wait_tmr,      8, 1000,
         "gfnwi:tmr.con_wt_tm"                                                                                                      },
        {&(cmp_trans->config.send_complete_timer), gfnwi->trans_cntrl_tmr_info.send_wait_tmr,         8, 1000,
         "gfnwi:tmr.snd_wt_tm"                                                                                                      },
        {&(cmp_trans->config.recv_complete_timer), gfnwi->trans_cntrl_tmr_info.nxt_data_recv_wait,    8, 1000,
         "gfnwi:tmr.rcv_wt_tm"                                                                                                      },
        {&(cmp_trans->config.idle_mon_timer),      gfnwi->trans_cntrl_tmr_info.non_comm_monitor,      8, 1000,
         "gfnwi:tmr.no_com_mon"                                                                                                     },
        {&(cmp_trans->config.retry_limit_s),       gfnwi->trans_cntrl_cnt_info.line_fail_rtr_num_srt, 8, 5,
         "gfnwi:cnt.ln_rty_srt"                                                                                                     },
        {&(cmp_trans->config.retry_limit_l),       gfnwi->trans_cntrl_cnt_info.line_fail_rtr_num_lng, 8, 5,
         "gfnwi:cnt.ln_rty_lng"                                                                                                     },
        {&(cmp_trans->config.retry_interval_s),    gfnwi->trans_cntrl_tmr_info.line_fail_rtr_num_srt, 8, 1000,
         "gfnwi:tmr.ln_rty_srt"                                                                                                     },
        {&(cmp_trans->config.retry_interval_l),    gfnwi->trans_cntrl_tmr_info.line_fail_rtr_num_lng, 8, 1000,
         "gfnwi:tmr.ln_rty_lng"                                                                                                     },
        {&(cmp_trans->config.data_len_start_lct),  gfnwi->denbun_item_lct_info.data_len_start_lct,    5, -1,
         "gfnwi:dbni.d_len_lct"                                                                                                     },
        {&(cmp_trans->config.data_len_size),       gfnwi->denbun_item_lct_info.data_len_size,         2, -1,
         "gfnwi:dbni.d_len_sz"                                                                                                      },
        {&(cmp_trans->config.denbun_start_lct),    gfnwi->denbun_item_lct_info.denbun_start_lct,      5, -1,
         "gfnwi:dbni.d_st_lct"                                                                                                      }
    };
    char *data_len_attribute[] = {
        "BIN",  // ：Binary
        "BCD",  // ：BCD
        "ASC",  // ：ASCII
        "EBC"   // ：EBCDIC
    };
    // 接続情報のコピー
    memset(cmp_trans, 0, sizeof(cmp_trans_t));
    memcpy(&cmp_trans->gflin, gflin, sizeof(db_gflin_def));                      // 回線管理
    memcpy(&cmp_trans->gfnwi, gfnwi, sizeof(db_gfnwi_def));                      // NW情報ファイル
    memcpy(&cmp_trans->gfphi_in_dist, in_dist_srvcls, sizeof(db_gfphi_def));  // 電文振分(inbound)
    memcpy(&cmp_trans->gfphi_cmd_if, cmd_if_srvcls, sizeof(db_gfphi_def));    // コマンドI/Fサーバ

    // グローバルなinbound振分Outstandingへの参照取得
    cmp_trans->in_dist_p6d_info.outstanding = &cncl_current_inbound_outstanding;
    // 接続情報の初期化
    cmp_trans->gclst.gclst_info = gclst_info;
    cmp_trans->socket           = -1;
    hd->stopwait                = false;
    cmp_trans->rcv_buff         = alloc_io_mem(&io_mem);
    hd->io_complete_buffer      = cmp_trans->rcv_buff;
    if (cmp_trans->rcv_buff == NULL) {
        cncl_ems_procedure_error("cncl_create_trans", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    cmp_trans->line_status = e_line_uninitialized;  // 回線状態
    cmp_trans->proc_status = e_proc_none;           // プロセス状態

    memcpy(cmp_trans->config.tcpip_prc_name, gflin->tcpip_prc_name, sizeof(gflin->tcpip_prc_name));
    cncl_rm_trspc_wn(cmp_trans->config.tcpip_prc_name, sizeof(tcpip_prc_name_t));

    cmp_trans->config.so_snd_buff = myInfo->so_sndbuff;
    cmp_trans->config.so_rcv_buff = myInfo->so_rcvbuff;

    // NW情報ファイル取得項目のデータ変換
    while (config_set_index < sizeof(config_set) / sizeof(config_set_t)) {
        *(config_set[config_set_index].param_val_l) =
            str2ul_c(config_set[config_set_index].param_val_c, config_set[config_set_index].length, &r_len, &s_err);
        if (s_err) {
            cncl_ems_param_err(config_set[config_set_index].param_name, s_err, DEF_NERR_PRM_RD_ERR_INV);
            *(config_set[config_set_index].param_val_l) = config_set[config_set_index].default_value;
        }
        config_set_index++;
    }
    if (gfnwi->denbun_item_lct_info.data_len_include_id == 'I'
        || gfnwi->denbun_item_lct_info.data_len_include_id == 'i') {
        cmp_trans->config.data_len_include_id = 1;
    }
    while (cmp_trans->config.data_len_attribute < e_data_len_attribute_fault) {
        if (memcmp(data_len_attribute[cmp_trans->config.data_len_attribute],
                   gfnwi->denbun_item_lct_info.data_len_attribute, 3)
            == 0) {
            break;
        }
        cmp_trans->config.data_len_attribute++;
    }
    if (cmp_trans->config.data_len_attribute == e_data_len_attribute_fault) {
        cncl_ems_param_err("gfnwi:dbni.d_len_att", s_err, DEF_NERR_PRM_RD_ERR_INV);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
    }

    if (cmp_trans->config.data_len_include_id) {
        cmp_trans->config.header_length = cmp_trans->config.data_len_start_lct - 1 + cmp_trans->config.data_len_size;
    }
    else {
        cmp_trans->config.header_length = cmp_trans->config.denbun_start_lct - 1;
    }
    serverclass_info(&cmp_trans->cmd_if_s9s_info, &cmp_trans->gfphi_cmd_if);
    serverclass_info(&cmp_trans->in_dist_s9s_info, &cmp_trans->gfphi_in_dist);

    create_sockadd_in(&(cmp_trans->config.local_if), gflin->ip_adress_src, gflin->port_num_src);
    create_sockadd_in(&(cmp_trans->config.remote_host), gflin->ip_adress_dst, gflin->port_num_dst);

    cmp_trans->rcv = NULL;
    cmp_trans->snd = NULL;
    cncl_trans_read_gclst(thread, &(cmp_trans->line_status), &(cmp_trans->proc_status));
    /// transスレッドが作成されるのは回線が初期化された時だけなので、
    /// 回線ステータスファイルがe_line_disconnected(切断中)でない場合は、回線ステータスを切断済に変更して接続を開始する。
    if (cmp_trans->line_status != e_line_disconnected) {
        cmp_trans->line_status = e_line_disconnected;
        s_err                  = cncl_trans_connect(thread);
        if (s_err) {
            cncl_trans_connect_retry(thread, s_err);
        }
    }
    return thread;
}
/*****************************************************************************/
/*  FUNCTION        :serverclass_info                                        */
/*  CALLING SEQ.    :void serverclass_info(                                  */
/*                   serverclass_info_t *dst,                                */
/*                   const db_gfphi_def *cmd_if_srvcls)                      */
/*  ARGUMENT        :dst         : サーバクラス情報格納先構造体              */
/*                   s9s_info_rec: サーバクラス情報                          */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :物理名情報ファイルから取得したサーバクラス情報を整形し、*/
/*                   serverclass_info_t 構造体に設定する。                   */
/*                   ・ドメイン名が空白ならpathmon名を使用、それ以外は       */
/*                     domain_nameをpathmon_nameとして格納                   */
/*                   ・srv_cls_nameをs9s_nameとして格納                      */
/*                   ・それぞれ末尾の空白位置を探索し有効長を算出            */
/*****************************************************************************/
void serverclass_info(serverclass_info_t *dst, const db_gfphi_def *s9s_info_rec)
{
    char *workptr;
    memset(dst->pathmon_name, ' ', sizeof(dst->pathmon_name));
    if (s9s_info_rec->srv_cls_info.domain_name[0] == ' ') {
        memcpy(dst->pathmon_name, s9s_info_rec->srv_cls_info.pathmon_name,
               sizeof(s9s_info_rec->srv_cls_info.pathmon_name));
    } else {
        memcpy(dst->pathmon_name, s9s_info_rec->srv_cls_info.domain_name,
               sizeof(s9s_info_rec->srv_cls_info.domain_name));
    }
    workptr = memchr(dst->pathmon_name, ' ', sizeof(dst->pathmon_name));
    dst->pathmon_name_len = (short)(workptr - dst->pathmon_name);
    memset(dst->s9s_name, ' ', sizeof(dst->s9s_name));
    memcpy(dst->s9s_name, s9s_info_rec->srv_cls_info.srv_cls_name, sizeof(s9s_info_rec->srv_cls_info.srv_cls_name));
    workptr           = memchr(dst->s9s_name, ' ', sizeof(dst->s9s_name));
    dst->s9s_name_len = (short)(workptr - dst->s9s_name);
}
/*****************************************************************************/
/*  FUNCTION        :chk_line_sts                                            */
/*  CALLING SEQ.    :line_status_t chk_line_sts(const char *connect_sts)     */
/*  ARGUMENT        :connect_sts:回線状態を表す文字列                        */
/*  RETURN CODE     :e_line_disconnected, e_line_connectedなど               */
/*  DESCRIPTION     :文字列から回線状態を判定し列挙値を返す                  */
/*****************************************************************************/
line_status_t chk_line_sts(const char *connect_sts)
{
    line_status_t line_status = e_line_disconnected;
    while (line_status < e_line_uninitialized) {
        if (memcmp(connect_sts, line_sts_list[line_status], 2) == 0) break;
        line_status++;
    }
    return line_status;
}
/*****************************************************************************/
/*  FUNCTION        :chk_proc_sts                                            */
/*  CALLING SEQ.    :proc_status_t chk_proc_sts(const char *prc_sts)         */
/*  ARGUMENT        :prc_sts:プロセス状態を表す文字列                        */
/*  RETURN CODE     :e_proc_open, e_proc_closeなど                           */
/*  DESCRIPTION     :文字列からプロセス状態を判定し列挙値を返す              */
/*****************************************************************************/
proc_status_t chk_proc_sts(const char *prc_sts)
{
    proc_status_t proc_status = e_proc_open;
    while (proc_status < e_proc_none) {
        if (memcmp(prc_sts, proc_sts_list[proc_status], 2) == 0) break;
        proc_status++;
    }
    return proc_status;
}
/*****************************************************************************/
/*  FUNCTION        :chk_line_discon_reason                                  */
/*  CALLING SEQ.    :line_discon_reason_t                                    */
/*                   chk_line_discon_reason(const char *reason)              */
/*  ARGUMENT        :reason:切断理由を表す文字列                             */
/*  RETURN CODE     :e_discon_by_cls_cmd,e_discon_by_detect等                */
/*  DESCRIPTION     :文字列から切断理由を判定し列挙値を返す                  */
/*****************************************************************************/
line_discon_reason_t chk_line_discon_reason(const char *reason)
{
    line_discon_reason_t discon_reason = e_discon_by_cls_cmd;
    while (discon_reason < e_discon_none) {
        if (memcmp(reason, line_close_reason_list[discon_reason], 2) == 0) break;
        discon_reason++;
    }
    return discon_reason;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_read_gclst                                   */
/*  CALLING SEQ.    :short cncl_trans_read_gclst(                            */
/*                   thread_object_t *thread_trans,                          */
/*                   line_status_t *line_status,                             */
/*                   proc_status_t *trans_proc_status)                       */
/*  ARGUMENT        :thread_trans    :transportスレッド                      */
/*                  :line_status     :回線状態を格納するポインタ             */
/*                  :trans_proc_status:プロセス状態を格納するポインタ        */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :回線管理ファイル(GCLST)から現在の状態を読み込む         */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    short
    cncl_trans_read_gclst(thread_object_t *thread_trans, line_status_t *line_status, proc_status_t *trans_proc_status)
{
    Application    *app        = cncl_get_App();
    cmp_trans_t    *cmp_trans  = cncl_get_component(thread_trans);
    iom_params_def *gclst_info = cmp_trans->gclst.gclst_info;
    myinfo_def     *my_info    = &(app->my_info);
    long            tran_id;
    short           s_err;
    size_t          r_len;
    db_gclst_def   *db_gclst = (db_gclst_def *)gclst_info->arg6.rec_area;

    s_err                    = COM_TMF(DEF_COM_TMF_BEGIN, &tran_id, DEF_GFPCVX20);
    if (s_err) {
        cncl_ems_common_module_error("COM_TMF", s_err, no_param, DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_TMF_ERR);
    }
    cncl_prepare_ioparams(gclst_info, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                          (char *)&(cmp_trans->gflin.pri_key), DEF_COM_IOM_KEYTYPE_PRI,
                          sizeof(cmp_trans->gflin.pri_key), sizeof(cmp_trans->gflin.pri_key), DEF_COM_IOM_EXACT,
                          DEF_COM_IOM_LOCK, DEF_COM_IOM_ASCEND, my_info->file_io_timer, no_param, sizeof(db_gclst_def));
    s_err = COM_IOM_MAC(gclst_info);
    if (gclst_info->arg6.guardian_errcode != ZFIL_ERR_OK) {
        cncl_ems_fileio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin.pri_key), DEF_FL_LIN_STS,
                              DEF_COM_IOM_FUNC_STARTREAD, (char *)&(cmp_trans->gflin.pri_key),
                              sizeof(cmp_trans->gflin.pri_key), gclst_info->arg6.guardian_errcode,
                              DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
    }
    if (_arg_present(line_status)) {
        *line_status = chk_line_sts(db_gclst->connect_sts_info.connect_sts);
    }
    if (_arg_present(trans_proc_status)) {
        *trans_proc_status = chk_proc_sts(db_gclst->prc_sts_info.prc_sts);
    }
    cmp_trans->line_discon_reason = chk_line_discon_reason(db_gclst->connect_info.disconnect_rsn);
    cmp_trans->line_discon_socket_errcd =
        str2ul_c(db_gclst->connect_info.err_code, sizeof(db_gclst->connect_info.err_code), &r_len, &s_err);
    memset(cmp_trans->connect_sts_update_time, 0, sizeof(line_sts_update_time_t));
    memset(cmp_trans->prc_sts_update_time, 0, sizeof(prc_sts_update_time_t));
    memcpy(cmp_trans->connect_sts_update_time, db_gclst->connect_sts_info.connect_sts_update_time,
           sizeof(db_gclst->connect_sts_info.connect_sts_update_time));
    memcpy(cmp_trans->prc_sts_update_time, db_gclst->prc_sts_info.prc_sts_update_time,
           sizeof(db_gclst->prc_sts_info.prc_sts_update_time));

    s_err = COM_TMF(DEF_COM_TMF_END, &tran_id, DEF_GFPCVX20);
    if (s_err) {
        cncl_ems_common_module_error("COM_TMF", s_err, no_param, DEF_NERR_TMF_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_TMF_ERR);
    }
    return gclst_info->arg6.guardian_errcode;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_update_gclst                                 */
/*  CALLING SEQ.    :update_gclst_s_t cncl_trans_update_gclst(               */
/*                   thread_object_t *thread_trans,                          */
/*                   line_status_t line_status,                              */
/*                   proc_status_t proc_status,                              */
/*                   line_sts_errcd_t line_discon_socket_errcd,              */
/*                   line_discon_reason_t line_discon_reason)                */
/*  ARGUMENT        :thread_trans    :transportスレッド                      */
/*                  :line_status     :回線状態                               */
/*                  :proc_status     :プロセス状態                           */
/*                  :line_discon_socket_errcd:ソケットエラーコード           */
/*                  :line_discon_reason:切断理由                             */
/*  RETURN CODE     :更新結果(ビットマスク),エラー時異常終了                 */
/*  DESCRIPTION     :回線管理ファイル(GCLST)の状態を更新する                 */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    update_gclst_s_t
    cncl_trans_update_gclst(thread_object_t *thread_trans, line_status_t line_status, proc_status_t proc_status,
                            line_sts_errcd_t line_discon_socket_errcd, line_discon_reason_t line_discon_reason)
{
    Application      *app        = cncl_get_App();
    cmp_trans_t      *cmp_trans  = cncl_get_component(thread_trans);
    iom_params_def   *gclst_info = cmp_trans->gclst.gclst_info;
    myinfo_def       *my_info    = &(app->my_info);
    long              tran_id;
    short             s_err;
    db_gclst_def     *db_gclst = (db_gclst_def *)gclst_info->arg6.rec_area;
    proc_status_t     current_proc_status;
    update_gclst_t    result;
    COM_SDT_arg_2_def datetime_c;
    COM_SDT_arg_3_def datetime_b;
    long long         datetime_l;
    char              localport[6];

    result.update_gclst_s = 0;
    s_err                 = COM_TMF(DEF_COM_TMF_BEGIN, &tran_id, DEF_GFPCVX20);
    if (s_err) {
        cncl_ems_procedure_error("BEGINTRANSACTION", s_err, DEF_NERR_TMF_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_TMF_ERR);
    }
    cncl_prepare_ioparams(gclst_info, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                          (char *)&(cmp_trans->gflin.pri_key), DEF_COM_IOM_KEYTYPE_PRI,
                          sizeof(cmp_trans->gflin.pri_key), sizeof(cmp_trans->gflin.pri_key), DEF_COM_IOM_EXACT,
                          DEF_COM_IOM_LOCK, DEF_COM_IOM_ASCEND, my_info->file_io_timer, no_param, sizeof(db_gclst_def));
    s_err = COM_IOM_MAC(gclst_info);
    if (gclst_info->arg6.guardian_errcode != ZFIL_ERR_OK) {
        cncl_ems_fileio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin.pri_key), DEF_FL_LIN_STS,
                              DEF_COM_IOM_FUNC_STARTREAD, (char *)&(cmp_trans->gflin.pri_key),
                              sizeof(cmp_trans->gflin.pri_key), gclst_info->arg6.guardian_errcode,
                              DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
    }

    COM_SDT(2 /* JPN */, &datetime_c, &datetime_b, &datetime_l);
    if (_arg_present(line_status)) {
        // 回線ステータスを更新する場合のみ変更する。
        result.update_gclst.connection_status = true;
        memcpy(db_gclst->connect_sts_info.connect_sts, line_sts_list[line_status],
               sizeof(db_gclst->connect_sts_info.connect_sts));
        memcpy(db_gclst->connect_sts_info.connect_sts_update_time, (char *)&datetime_c,
               sizeof(db_gclst->connect_sts_info.connect_sts_update_time));
        if (line_status == e_line_connected) {
            // 接続状態であれば、アドレス情報を書込む
            memcpy(&db_gclst->connect_info, cmp_trans->gflin.ip_adress_src, 40);
            snprintf(localport, sizeof(localport), "%05u", cmp_trans->current_local_port);
            memcpy(db_gclst->connect_info.port_num_src, localport, sizeof(localport) - 1);
        } else {
            // 切断状態では、アドレス情報は空白
            memset(&db_gclst->connect_info, ' ', 40);
        }
    }
    if (_arg_present(
            proc_status)) {  // 停止コマンド受信状態で、接続状態が接続中のステータスからプロセスが再起動されることは無いはず。
        current_proc_status = chk_proc_sts(db_gclst->prc_sts_info.prc_sts);
        if (current_proc_status != proc_status) {
            result.update_gclst.proccess_status = true;
            memcpy(db_gclst->prc_sts_info.prc_sts, proc_sts_list[proc_status], sizeof(db_gclst->prc_sts_info.prc_sts));
            memcpy(db_gclst->prc_sts_info.prc_sts_update_time, (char *)&datetime_c,
                   sizeof(db_gclst->prc_sts_info.prc_sts_update_time));
        }
    }
    if (_arg_present(line_discon_socket_errcd)) {
        char buffer[sizeof(db_gclst->connect_info.err_code) + 1];
        snprintf(buffer, sizeof(buffer), "%0*.*d", (int)sizeof(buffer) - 1, (int)sizeof(buffer) - 1,
                 line_discon_socket_errcd);
        memcpy(db_gclst->connect_info.err_code, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(line_discon_reason)) {
        memcpy(db_gclst->connect_info.disconnect_rsn, line_close_reason_list[line_discon_reason], 2);
    }
    if (result.update_gclst_s != 0) {
        cncl_prepare_ioparams(gclst_info, DEF_COM_IOM_FUNC_UPDATE, no_param, no_param, no_param, no_param, no_param,
                              no_param, no_param, no_param, DEF_COM_IOM_LOCKFREE, no_param, no_param, db_gclst,
                              sizeof(db_gclst_def));
        s_err = COM_IOM_MAC(gclst_info);
        if (gclst_info->arg6.guardian_errcode != ZFIL_ERR_OK) {
            cncl_ems_fileio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin.pri_key), DEF_FL_LIN_STS,
                                  DEF_COM_IOM_FUNC_UPDATE, (char *)&(cmp_trans->gflin.pri_key),
                                  sizeof(cmp_trans->gflin.pri_key), gclst_info->arg6.guardian_errcode,
                                  DEF_NERR_FILE_IO_ERR);
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
        }
    }
    s_err = COM_TMF(DEF_COM_TMF_END, &tran_id, DEF_GFPCVX20);
    if (s_err) {
        cncl_ems_procedure_error("ENDTRANSACTION", s_err, DEF_NERR_TMF_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_TMF_ERR);
    }
    db_gclst = (db_gclst_def *)gclst_info->arg5.rec_area;
    memset(cmp_trans->connect_sts_update_time, 0, sizeof(line_sts_update_time_t));
    memset(cmp_trans->prc_sts_update_time, 0, sizeof(prc_sts_update_time_t));
    memcpy(cmp_trans->connect_sts_update_time, db_gclst->connect_sts_info.connect_sts_update_time,
           sizeof(db_gclst->connect_sts_info.connect_sts_update_time));
    memcpy(cmp_trans->prc_sts_update_time, db_gclst->prc_sts_info.prc_sts_update_time,
           sizeof(db_gclst->prc_sts_info.prc_sts_update_time));

    return result.update_gclst_s;
}
/*****************************************************************************/
/*  FUNCTION        :set_socket_option                                       */
/*  CALLING SEQ.    :short set_socket_option(short *socket,                  */
/*                   int level,                                              */
/*                   int option,                                             */
/*                   int value)                                              */
/*  ARGUMENT        :socket:ソケットFDを保持するポインタ                     */
/*                  :level :ソケットオプションレベル                         */
/*                  :option:ソケットオプション指定                           */
/*                  :value :設定する値                                       */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :指定したソケットに対してオプションを設定する            */
/*****************************************************************************/
short set_socket_option(short socket, int level, int option, int value)
{
    myinfo_def *myInfo = &(cncl_get_App()->my_info);
    short       s_err;
    short       fd = socket;
    s_err          = (short)setsockopt_nw(fd, level, option, (char *)&value, (int)sizeof(int), 0);
    if (s_err) {
        cncl_ems_procedure_error("setsockopt_nw", (short)errno, DEF_NERR_SETSOCKOPT_ERR);
        return s_err;
    }
    AWAITIOX(&fd, no_param, no_param, no_param, myInfo->socket_io_timer, no_param);
    FILE_GETINFO_(fd, &s_err, no_param, no_param, no_param, no_param, no_param);
    if (s_err) {
        cncl_ems_procedure_error("setsockopt_nw", (short)s_err, DEF_NERR_SETSOCKOPT_ERR);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_connect                                      */
/*  CALLING SEQ.    :short cncl_trans_connect(thread_object_t *thread_trans) */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :ソケットの生成・bind・connectを行い非同期接続           */
/*****************************************************************************/
short cncl_trans_connect(thread_object_t *thread_trans)
{
    myinfo_def      *myInfo            = &(cncl_get_App()->my_info);
    cmp_trans_t     *cmp_trans         = cncl_get_component(thread_trans);
    thread_header_t *hd                = cncl_get_threadInfo(thread_trans);
    size_t           pre_sockopt_index = 0;
    short            s_err             = 0;
    short            fd;

    typedef struct __pre_sockopt_t
    {
        int level;
        int option;
        int value;
    } pre_sockopt_t;
    pre_sockopt_t pre_sockopt[] = {
        {SOL_SOCKET, SO_REUSEADDR, 1                            },
        {SOL_SOCKET, SO_RCVBUF,    cmp_trans->config.so_rcv_buff},
        {SOL_SOCKET, SO_SNDBUF,    cmp_trans->config.so_snd_buff}
    };

    // 接続開始時切断状態を通知する。
    cncl_trans_notify_line_status(thread_trans, no_param);
    cncl_trans_update_gclst(thread_trans, e_line_connect_in_progress, no_param, no_param, no_param);
    socket_set_inet_name(cmp_trans->config.tcpip_prc_name);
    cmp_trans->socket =
        (short)socket_nw(AF_INET, SOCK_STREAM, SOCK_PROTOCOL_NOUSE, 14 /*SOCK_FLG_NOWAIT*/, SOCK_SYNC_NOUSE);
    if (cmp_trans->socket == -1) {
        cncl_ems_procedure_error("socket_nw", (short)errno, DEF_NERR_SOCKET_GEN_ERR);
        return (short)errno;
    }
    while (s_err == 0) {
        while (s_err == 0 && (pre_sockopt_index < (sizeof(pre_sockopt) / sizeof(pre_sockopt_t)))) {
            switch (pre_sockopt[pre_sockopt_index].option) {
                case SO_RCVBUF:
                case SO_SNDBUF:
                    if (pre_sockopt[pre_sockopt_index].value == 0) {
                        break;
                    }
                default:
                    s_err = (short)set_socket_option(cmp_trans->socket, pre_sockopt[pre_sockopt_index].level,
                                                     pre_sockopt[pre_sockopt_index].option,
                                                     pre_sockopt[pre_sockopt_index].value);
            }
            pre_sockopt_index++;
        }
        if (s_err) {
            break;
        }
        s_err = (short)bind_nw(cmp_trans->socket, (struct sockaddr *)&(cmp_trans->config.local_if),
                               sizeof(struct sockaddr_in), 0);
        if (s_err) {
            cncl_ems_procedure_error("bind_nw", (short)errno, DEF_NERR_SOCKET_GEN_ERR);
            break;
        }
        fd = cmp_trans->socket;
        AWAITIOX(&fd, no_param, no_param, no_param, myInfo->socket_io_timer, no_param);
        FILE_GETINFO_(cmp_trans->socket, &s_err, no_param, no_param, no_param, no_param, no_param);
        if (s_err) {
            cncl_ems_procedure_error("bind_nw", (short)s_err, DEF_NERR_SETSOCKOPT_ERR);
            break;
        }
        Event_tag_t *socket_event = cncl_get_socket_tag(thread_trans, cmp_trans->socket, cncl_trans_connect_complete);
        if (!socket_event) {
            // タイマーイベントが取得出来ないのでメモリ不足 Abendする。
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
        }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
        s_err = (short)connect_nw(cmp_trans->socket, (struct sockaddr *)&(cmp_trans->config.remote_host),
                                  sizeof(struct sockaddr_in), (long)socket_event);
#pragma clang diagnostic pop
        if (s_err) {
            cncl_ems_procedure_error("connect_nw", (short)errno, DEF_NERR_CONNECT_ERR);
            break;
        }
        Event_tag_t *timer_event = cncl_get_timer_tag(thread_trans, cmp_trans->config.con_complete_timer,
                                                      cncl_trans_connect_timeout, DEF_TRANSPORT_TIMER);
        if (!timer_event) {
            // タイマーイベントが取得出来ないのでメモリ不足 Abendする。
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
        }
        cmp_trans->line_status = e_line_connect_in_progress;
        return 0;
    }
    FILE_CLOSE_(cmp_trans->socket, no_param);
    cmp_trans->socket = -1;
    cncl_remove_event(hd->io_ev_tag);
    hd->io_ev_tag = NULL;
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_send_error                                   */
/*  CALLING SEQ.    :short cncl_trans_send_error(Event_tag_t *event)         */
/*  ARGUMENT        :event:送信エラー時のイベント情報                        */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :送信エラー検出時に回線をクローズしリトライ処理          */
/*****************************************************************************/
short cncl_trans_send_error(Event_tag_t *event)
{
    // trans handlerだがsnd threadが来るので、sndコンポーネントからtransスレッドを取得する。
    cmp_snd_t   *cmp_snd   = cncl_get_component(event->thread);
    cmp_trans_t *cmp_trans = cncl_get_component(cmp_snd->p_trans);

    cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                      cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                      cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_SEND_ERR);
    cmp_snd->snd_status = e_cncl_snd_fault;
    Event_tag_t *close_request =
        cncl_get_scheduled_tag(cmp_snd->p_trans, -1, cncl_trans_close, event->io_info.fs_err, 0);
    cncl_post_event(close_request);
    return 0;
}
/****************************************************************************/
/*  FUNCTION        : get_socket_name                                       */
/*  CALLING SEQ.    : short get_socket_name                                 */
/*                           (const short socket,                           */
/*                            struct sockaddr_in *local_if,                 */
/*                            int *local_if_len)                            */
/*                                                                          */
/*  ARGUMENT        : socket [in]                                           */
/*                      対象となるソケットディスクリプタ                    */
/*                  : local_if [out]                                        */
/*                      ローカルインタフェース情報（sockaddr_in構造体）     */
/*                  : local_if_len [in/out]                                 */
/*                      local_ifの構造体サイズ／取得後のサイズ              */
/*                                                                          */
/*  RETURN CODE     : 0     - 正常終了                                      */
/*                  : < 0   - getsockname_nwまたはFILE_GETINFO_による失敗   */
/*                                                                          */
/*  DESCRIPTION     : 指定されたソケットのローカルアドレス情報を取得し、    */
/*                    アプリケーション内部のタイムアウト管理下でIO監視後、  */
/*                    ソケット情報を取得する。                              */
/*                    getsockname_nwおよびFILE_GETINFO_の処理中に発生した   */
/*                    エラーはcncl_ems_procedure_errorにて記録される。      */
/****************************************************************************/
short get_socket_name(const short socket, struct sockaddr_in *local_if, int *local_if_len)
{
    myinfo_def *myInfo = &(cncl_get_App()->my_info);
    short       s_err;
    short       fd = socket;
    s_err          = (short)getsockname_nw(socket, (struct sockaddr *)local_if, local_if_len, 0);
    if (s_err) {
        cncl_ems_procedure_error("getsockname_nw", (short)errno, DEF_NERR_CONNECT_ERR);
        return s_err;
    }
    AWAITIOX(&fd, no_param, no_param, no_param, myInfo->socket_io_timer, no_param);
    FILE_GETINFO_(fd, &s_err, no_param, no_param, no_param, no_param, no_param);
    if (s_err) {
        cncl_ems_procedure_error("getsockname_nw", (short)s_err, DEF_NERR_CONNECT_ERR);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_connect_complete                             */
/*  CALLING SEQ.    :short cncl_trans_connect_complete(Event_tag_t *event)   */
/*  ARGUMENT        :event:非同期connect完了通知イベント                     */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :connect完了後のソケットオプション設定と受信開始         */
/*****************************************************************************/
short cncl_trans_connect_complete(Event_tag_t *event)
{
    thread_object_t   *thread_trans       = event->thread;
    cmp_trans_t       *cmp_trans          = cncl_get_component(thread_trans);
    short              s_err              = 0;
    size_t             post_sockopt_index = 0;
    struct sockaddr_in current_local_if;
    int                local_if_len = sizeof(current_local_if);

    typedef struct __post_sockopt_t
    {
        int level;
        int option;
        int value;
    } post_sockopt_t;
    post_sockopt_t post_sockopt[] = {
        {IPPROTO_TCP, TCP_NODELAY,  1},
        {SOL_SOCKET,  SO_KEEPALIVE, 1},
    };

    cncl_cancel_timer_tag(thread_trans);

    if (event->io_info.fs_err) {
        FILE_CLOSE_(cmp_trans->socket, no_param);
        cmp_trans->socket = -1;
        cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                          cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                          cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_CONNECT_ERR);
        s_err = cncl_trans_connect_retry(thread_trans, event->io_info.fs_err);
        return s_err;
    }
    // 特にエラーでも何もしない。
    get_socket_name(cmp_trans->socket, &current_local_if, &local_if_len);
    cmp_trans->current_local_port = ntohs(current_local_if.sin_port);

    while (post_sockopt_index < (sizeof(post_sockopt) / sizeof(post_sockopt_t))) {
        s_err = set_socket_option(cmp_trans->socket, post_sockopt[post_sockopt_index].level,
                                  post_sockopt[post_sockopt_index].option, post_sockopt[post_sockopt_index].value);
        // 接続はしているので切断はしない
        if (s_err) {
            // 切断しないならば特にすることはない。
        }
        post_sockopt_index++;
    }
    cmp_trans->trans_status    = e_trans_wait_initial_odst;
    cmp_trans->line_status     = e_line_connected;
    cmp_trans->retry_cnt_short = 0;
    cmp_trans->retry_cnt_long  = 0;
    // 接続が完了したので通知を出す。
    cncl_trans_notify_line_status(thread_trans,no_param);
    cncl_trans_update_gclst(thread_trans, e_line_connected, cmp_trans->proc_status, event->io_info.fs_err,
                            e_discon_none);
    cmp_trans->rcv = cncl_create_rcv(thread_trans);
    cmp_trans->snd = cncl_create_snd(thread_trans, cncl_trans_send_error);

    cncl_trans_post_connect_proc_dt_send(thread_trans);
    cncl_ems_nw_connect_normal(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name,
                               &(cmp_trans->config.remote_host.sin_addr), cmp_trans->config.remote_host.sin_port,
                               &(cmp_trans->config.local_if.sin_addr), cmp_trans->config.local_if.sin_port,
                               DEF_NERR_NOMAL);
    cncl_trans_receive(thread_trans);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_post_connect_proc_sign_on                    */
/*  CALLING SEQ.    :short cncl_trans_post_connect_proc_sign_on              */
/*                                           (thread_object_t *thread_trans) */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :接続後にシングオンの追加処理を行う                      */
/*****************************************************************************/
short cncl_trans_post_connect_proc_sign_on(thread_object_t *thread_trans)
{
    cmp_trans_t *cmp_trans = cncl_get_component(thread_trans);
    short        s_err     = 0;
    if (!memcmp(cmp_trans->gfnwi.connect_nxt_prc_info.connect_nxt_prc_kind, DEF_CONNECT_NEXT_OPN_SEND,
                sizeof(DEF_CONNECT_NEXT_OPN_SEND) - 1)) {
        s_err = cncl_trans_opening_request_c501_send(thread_trans, 0);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_post_connect_proc_dt_send                    */
/*  CALLING SEQ.    :short cncl_trans_post_connect_proc_dt_send              */
/*                                           (thread_object_t *thread_trans) */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :接続後に特定データ送信の追加処理を行う                  */
/*****************************************************************************/
short cncl_trans_post_connect_proc_dt_send(thread_object_t *thread_trans)
{
    cmp_trans_t *cmp_trans = cncl_get_component(thread_trans);
    short        s_err     = 0;
    if (!memcmp(cmp_trans->gfnwi.connect_nxt_prc_info.connect_nxt_prc_kind, DEF_CONNECT_NEXT_DATA_SEND,
                sizeof(DEF_CONNECT_NEXT_DATA_SEND) - 1)) {
        s_err = cncl_trans_send_dt(thread_trans);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_connect_retry                                */
/*  CALLING SEQ.    :short cncl_trans_connect_retry(                         */
/*                   thread_object_t *thread_trans,                          */
/*                   int close_reason)                                       */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*                  :close_reason:切断理由                                   */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :接続リトライ(ショート・ロング)を行い切断も判断          */
/*****************************************************************************/
short cncl_trans_connect_retry(thread_object_t *thread_trans, short close_reason)
{
    cmp_trans_t *cmp_trans = cncl_get_component(thread_trans);
    long         interval  = 0;

    if (cmp_trans->retry_cnt_short < cmp_trans->config.retry_limit_s) {
        // ショートリトライ開始
        interval = cmp_trans->config.retry_interval_s;
        cmp_trans->retry_cnt_short++;
    } else if (cmp_trans->retry_cnt_long < cmp_trans->config.retry_limit_l) {
        // ロングリトライ開始
        if (cmp_trans->retry_cnt_long == 0) {
            // ショートリトライオーバー
            cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                              cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                              cmp_trans->config.local_if.sin_port, close_reason, DEF_NERR_SHT_RETRY_OVER);
        }
        interval = cmp_trans->config.retry_interval_l;
        cmp_trans->retry_cnt_long++;
    }
    if (interval > 0) {
        Event_tag_t *timer_event = cncl_get_timer_tag(thread_trans, interval, cncl_trans_retry_timer_expire, DEF_TRANSPORT_TIMER);
        if (!timer_event) {
            cncl_trans_update_gclst(thread_trans, e_line_disconnected, no_param, close_reason, e_discon_by_detect);
        }
    } else {
        // 切断
        // ロングリトライオーバー
        cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                          cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                          cmp_trans->config.local_if.sin_port, close_reason, DEF_NERR_LNG_RETRY_OVER);

        cmp_trans->line_status = e_line_disconnected;
        cncl_trans_update_gclst(thread_trans, e_line_disconnected, no_param, close_reason, e_discon_by_re_con_over);
    }
    cncl_delete_rcv(cmp_trans->rcv);
    cncl_delete_snd(cmp_trans->snd);
    cmp_trans->rcv = NULL;
    cmp_trans->snd = NULL;
    // cncl_trans_notify_line_status(thread_trans);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_notify_line_status                           */
/*  CALLING SEQ.    :short cncl_trans_notify_line_status(                    */
/*                   thread_object_t *trans_thread                           */
/*                   bool already_connected)                                 */
/*  ARGUMENT        :trans_thread:transportスレッド                          */
/*                  :already_connected: 接続済                               */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :回線状態変更を通知するC107電文を作成し送信              */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    short
    cncl_trans_notify_line_status(thread_object_t *trans_thread, bool already_connected)
{
    thread_object_t *thread_ntf_odst;
    cmp_trans_t     *cmp_trans             = cncl_get_component(trans_thread);
    io_trace_buf_t  *io_trace_buf          = alloc_io_mem(&io_mem);
    bool             open_request_required = cmp_trans->line_status != e_line_connected ? false : true;
    // 既に接続済→接続済通知の場合、開局要求をキャンセルする。
    if(_arg_present(already_connected)){
        open_request_required = already_connected ? false : open_request_required;
    }

    if (!io_trace_buf) {
        cncl_ems_procedure_error("cncl_trans_notify_line_status", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    if (cncl_thread_factory.ntf_obound.count == 0) {
        cncl_ems_procedure_error("cncl_trans_notify_line_status", 0, DEF_NERR_SYSIF_LGC_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_SYSIF_LGC_ERR);
    }
    thread_ntf_odst = cncl_thread_factory.ntf_obound.top;
    memset(io_trace_buf, 0, sizeof(io_trace_buf_t));
    cncl_trans_request_c107(trans_thread, cmp_trans->line_status != e_line_connected ? false : true,
                            (c107_def *)io_trace_buf->data_info.rec_area);
    cncl_ntf_odst_procs_add_req(thread_ntf_odst, trans_thread, io_trace_buf, open_request_required);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_request_c107                                 */
/*  CALLING SEQ.    :void cncl_trans_request_c107(                           */
/*                   thread_object_t *trans_thread,                          */
/*                   bool connect,                                           */
/*                   c107_def *c107_req)                                     */
/*  ARGUMENT        :trans_thread:transportスレッド                          */
/*                  :connect     :接続か切断か                               */
/*                  :c107_req    :C107電文構造体へのポインタ                 */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :回線状態通知に使うC107電文を生成する                    */
/*****************************************************************************/
void cncl_trans_request_c107(thread_object_t *trans_thread, bool connect, c107_def *c107_req)
{
    cmp_trans_t *cmp_trans = cncl_get_component(trans_thread);
    char         localport[6];

    memset(c107_req, ' ', sizeof(c107_def));
    memcpy(c107_req->common_header.interface_code, "C107", sizeof(c107_req->common_header.interface_code));
    c107_req->common_header.error_code = 0;
    memset(c107_req->common_header.internal_error_code, '0', sizeof(c107_req->common_header.internal_error_code));
    c107_req->common_header.control_data_length = sizeof(c107_def) - sizeof(common_header_def);

    memcpy(&c107_req->line_info, &cmp_trans->gflin.pri_key, sizeof(cmp_trans->gflin.pri_key));
    // TODO: c107_req.line_info.dest_connection_name << コネクション制御クライアントは空白のままとする。
    memcpy(&c107_req->connection_status_info, &cmp_trans->gflin.pri_key,
           sizeof(cmp_trans->gflin.pri_key) - sizeof(cmp_trans->gflin.pri_key.connect_id));
    memcpy(c107_req->connection_status_info.connection_status, line_sts_list[cmp_trans->line_status], 2);
    memcpy(c107_req->connection_status_info.connection_status_time, cmp_trans->connect_sts_update_time,
           sizeof(line_sts_update_time_t) - 1);
    memcpy(c107_req->process_state_info.process_status_time, cmp_trans->prc_sts_update_time,
           sizeof(prc_sts_update_time_t) - 1);
    memcpy(&c107_req->connection_info, cmp_trans->gflin.ip_adress_src, 40);
    if (!connect) {
        char buffer[sizeof(c107_req->connection_info.connection_error_code) + 1];
        snprintf(buffer, sizeof(buffer), "%0*.*d", (int)sizeof(buffer) - 1, (int)sizeof(buffer) - 1,
                 cmp_trans->line_discon_socket_errcd);
        memcpy(&c107_req->connection_info.connection_error_code, buffer, sizeof(buffer) - 1);
        memcpy(&c107_req->connection_info.disconnect_reason, line_close_reason_list[cmp_trans->line_discon_reason], 2);
    } else {
        snprintf(localport, sizeof(localport), "%05u", cmp_trans->current_local_port);
        memcpy(c107_req->connection_info.src_port, localport, sizeof(localport) - 1);
    }
    c107_req->sockaddr_in = cmp_trans->config.remote_host;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_request_c201                                 */
/*  CALLING SEQ.    :size_t cncl_trans_request_c201(                         */
/*                   thread_object_t *trans_thread,                          */
/*                   c201_def *c201_req,                                     */
/*                   const char *rcv_data,                                   */
/*                   size_t data_length,                                     */
/*                   TS_UNIQUE_128 *ts128)                                   */
/*  ARGUMENT        :trans_thread:transportスレッド                          */
/*                  :c201_req   :C201電文構造体へのポインタ                  */
/*                  :rcv_data   :受信データ                                  */
/*                  :data_length:受信データサイズ                            */
/*                  :ts128      :タイムスタンプ情報                          */
/*  RETURN CODE     :組み立てたC201電文のバイト数                            */
/*  DESCRIPTION     :受信メッセージ通知に使うC201電文を生成する              */
/*****************************************************************************/
size_t cncl_trans_request_c201(thread_object_t *trans_thread, c201_def *c201_req, const char *rcv_data,
                               size_t data_length, TS_UNIQUE_128 *ts128)
{
    cmp_trans_t   *cmp_trans = cncl_get_component(trans_thread);
    datetime20_t   datetime20;
    datetime_hex_t datetime_hex;
    size_t         length;
    char           localport[6];

    memset(c201_req, ' ', sizeof(c201_def));
    memcpy(c201_req->common_header.interface_code, "C201", sizeof(c201_req->common_header.interface_code));
    c201_req->common_header.error_code = 0;
    memset(c201_req->common_header.internal_error_code, '0', sizeof(c201_req->common_header.internal_error_code));
    c201_req->common_header.control_data_length =
        (short)(sizeof(c201_req->text_recv_notify) + sizeof(c201_req->msg_info.msg_len) + data_length);

    memcpy(&(c201_req->text_recv_notify.recv_con_id), &(cmp_trans->gflin.pri_key), sizeof(gflin_pkey_def));
    memcpy(&(c201_req->text_recv_notify.recv_con_info), cmp_trans->gflin.ip_adress_src, 40);
    snprintf(localport, sizeof(localport), "%05u", cmp_trans->current_local_port);
    memcpy(c201_req->text_recv_notify.recv_con_info.src_port, localport, sizeof(localport) - 1);

    inter_pret_ts_unique((short *)ts128, datetime20, datetime_hex);
    memcpy(c201_req->text_recv_notify.recv_timestamp.time_stamp, datetime20, sizeof(datetime20_t) - 1);
    memcpy(c201_req->text_recv_notify.recv_timestamp.ts_unique_data, datetime_hex, sizeof(datetime_hex_t) - 1);
    c201_req->msg_info.msg_len = (short)data_length;
    memcpy(c201_req->msg_info.msg_data, rcv_data, data_length);
    length = (char *)&(c201_req->msg_info.msg_data) - (char *)c201_req + data_length;
    return length;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_send_request                                 */
/*  CALLING SEQ.    :short cncl_trans_send_request(                          */
/*                   thread_object_t *thread_trans,                          */
/*                   io_trace_buf_t *send_req)                               */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*                  :send_req   :送信バッファ                                */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :指定スレッドの送信用キューにデータを登録し送信要求を    */
/*                   登録する                                                */
/*****************************************************************************/
short cncl_trans_send_request(thread_object_t *thread_trans, io_trace_buf_t *send_req)
{
    // ここで渡されているのはc201電文全体
    cmp_trans_t     *cmp_trans  = cncl_get_component(thread_trans);
    thread_object_t *thread_snd = cmp_trans->snd;
    size_t           r_len;
    short            s_err;

    if (!thread_snd) {
        return -1;
    }
    size_t length = str2ul_c(send_req->data_info.rec_len, sizeof(send_req->data_info.rec_len), &r_len, &s_err);
    cncl_snd_request_post(thread_snd, send_req, length);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_req_send_select_transport                    */
/*  CALLING SEQ.    :thread_object_t *cncl_trans_req_send_select_transport(  */
/*                   io_trace_buf_t *io_trace_buf)                           */
/*  ARGUMENT        :io_trace_buf:送信要求                                   */
/*  RETURN CODE     :送信先のtransportスレッド,送信不可ならNULL              */
/*  DESCRIPTION     :受信要求(C202電文)から送信先を特定し送信要求            */
/*****************************************************************************/
thread_object_t *cncl_trans_req_send_select_transport(io_trace_buf_t *io_trace_buf)
{
    // ここで渡されているのはc202電文全体
    thread_object_t *thread_trans;
    thread_object_t *thread_snd;
    cmp_trans_t     *cmp_trans;
    cmp_snd_t       *cmp_snd;

    c202_def        *send_req = (c202_def *)io_trace_buf->data_info.rec_area;
    thread_trans              = cncl_search_trans_thread(&(send_req->text_send_info.recv_con_id));
    if (thread_trans) {
        cmp_trans  = cncl_get_component(thread_trans);
        thread_snd = cmp_trans->snd;
        if(!thread_snd) {
            //接続されていない。
            return NULL;
        }
        cmp_snd    = cncl_get_component(thread_snd);
        if (cmp_snd->snd_req_q.count >= CCC_MAX_sendqueue) {
            return NULL;
        }
        cncl_trans_send_request(thread_trans, io_trace_buf);
    }
    return thread_trans;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_connect_timeout                              */
/*  CALLING SEQ.    :short cncl_trans_connect_timeout(Event_tag_t *event)    */
/*  ARGUMENT        :event:connect処理のタイムアウトイベント                 */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :接続中タイムアウト発生時にconnectキャンセル・リトライ   */
/*                  :を行う                                                  */
/*****************************************************************************/
short cncl_trans_connect_timeout(Event_tag_t *event)
{
    // TODO:EMSメッセージ
    cmp_trans_t     *cmp_trans = cncl_get_component(event->thread);
    thread_header_t *hd        = cncl_get_threadInfo(event->thread);
    short            s_err;
    // socketのI/O(openをキャンセル)
    CANCEL(cmp_trans->socket);
    // openのI/Oイベントをキャンセル
    cncl_remove_event(hd->io_ev_tag);
    hd->io_ev_tag = NULL;
    cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                      cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                      cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_CONNECT_ERR);
    FILE_CLOSE_(cmp_trans->socket, no_param);
    cmp_trans->socket = -1;
    //  リトライ開始
    s_err             = cncl_trans_connect_retry(event->thread, event->io_info.fs_err);
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_retry_timer_expire                           */
/*  CALLING SEQ.    :short cncl_trans_retry_timer_expire(Event_tag_t *event) */
/*  ARGUMENT        :event:リトライタイマー満了イベント                      */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :接続リトライタイマー満了時に再度connectを行う           */
/*****************************************************************************/
short cncl_trans_retry_timer_expire(Event_tag_t *event)
{
    return cncl_trans_connect(event->thread);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_post_receive                                 */
/*  CALLING SEQ.    :short cncl_trans_post_receive(Event_tag_t *event)       */
/*  ARGUMENT        :event:受信後に再度受信を行うイベント                    */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :受信完了後に次の受信を続ける処理を行う                  */
/*****************************************************************************/
short cncl_trans_post_receive(Event_tag_t *event)
{
    return cncl_trans_receive(event->thread);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_receive                                      */
/*  CALLING SEQ.    :short cncl_trans_receive(thread_object_t *thread_trans) */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :データの受信(ヘッダ・ボディ)を実施し、電文解析          */
/*****************************************************************************/
short cncl_trans_receive(thread_object_t *thread_trans)
{
    cmp_trans_t     *cmp_trans = cncl_get_component(thread_trans);
    cmp_rcv_t       *cmp_rcv   = cncl_get_component(cmp_trans->rcv);
    thread_header_t *hd        = cncl_get_threadInfo(thread_trans);
    rcv_status_t     rcv_status;
    short            rcv_err = 0;
    int              i_err;
    size_t           check_ipc_total_length;
    switch (cmp_trans->trans_status) {
        case e_trans_uninitialized:
            // ここには来ない
            break;
        case e_trans_idle:
        case e_trans_wait_header:
            if (hd->stopwait) break;
            rcv_status = cncl_rcv_receive(cmp_trans->rcv, cmp_trans->socket, cmp_trans->rcv_buff->data_info.rec_area,
                                          cmp_trans->config.header_length, cncl_trans_receive_complete,
                                          cncl_trans_receive_timeout, &rcv_err);
            // ヘッダ部受信完了
            if (rcv_status == e_cncl_rcv_cmpl) {
                cmp_trans->cur_trans_length = cncl_trans_length_decoder(
                    cmp_trans->rcv_buff->data_info.rec_area + cmp_trans->config.data_len_start_lct - 1,
                    cmp_trans->config.data_len_size, cmp_trans->config.data_len_attribute, &i_err);

                // 電文ボディー部長計算
                // 要求した所定のヘッダ長(cmp_trans->config.header_length)の受信が完了したので、残データ長を求める。
                if (cmp_trans->config.data_len_include_id) {
                    // データ長がヘッダー調を含む
                    cmp_trans->cur_pending_length = cmp_trans->cur_trans_length - cmp_trans->config.header_length;
                } else {
                    // データ長がヘッダー長を含まない
                    cmp_trans->cur_pending_length = cmp_trans->cur_trans_length;
                }

                // 電文長チェック
                // 電文全長がc201の電文エリア9999(最大電文長)を越えた場合または、ヘッダ長より電文長が短い場合
                check_ipc_total_length =
                    cmp_trans->cur_trans_length
                    + (cmp_trans->config.data_len_include_id ? 0 : cmp_trans->config.header_length);
                if (check_ipc_total_length > MAX_TEXT_BUF_LEN
                    || check_ipc_total_length < (size_t)cmp_trans->config.header_length) {
                    cncl_ems_ipc_length_error(
                        transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                        cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                        cmp_trans->config.local_if.sin_port, check_ipc_total_length, DEF_NERR_RCV_ERR);
                    ((cmp_rcv_t *)cncl_get_component(cmp_trans->rcv))->rcv_status = e_cncl_rcv_fault;
                    Event_tag_t *ev =
                        cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, 0 /* LENGTH ERROR */, NULL);
                    cncl_post_event(ev);
                    break;
                }

                // 残データ無し(ヘッダのみの電文)
                if (cmp_trans->cur_pending_length == 0) {
                    // 電文長0(bodyなし)の場合はボディ部の受信を実行しないで電文振分(inbound)にPathsendする。
                    // (1)trans_statusをヘッダ受信待ちに遷移
                    cmp_trans->trans_status = e_trans_wait_header;
                    // rcvスレッドのステータスをリセット
                    cmp_rcv->initial_read = true;
                    // (2)cncl_trans_msg_rcv_notify_c201_post内でtrans_statusが更新されることがあるので、(1)->(2)の順序でなければならない。
                    cncl_trans_msg_rcv_notify_c201_post(thread_trans);
                    cncl_trans_receive(thread_trans);
                    //Event_tag_t *ev = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_post_receive, 0, NULL);
                    //cncl_post_event(ev);
                    break;
                } else {
                    // ボディー部有
                    cmp_trans->trans_status = e_trans_wait_body;
                }
                // このままe_trans_wait_bodyを処理する。
            }
            // TCP/IP socket 受信エラー
            else if (rcv_status == e_cncl_rcv_fault) {
                // cncl_get_threadInfo(cmp_trans->rcv)->stopwait                 = true;
                ((cmp_rcv_t *)cncl_get_component(cmp_trans->rcv))->rcv_status = e_cncl_rcv_fault;
                Event_tag_t *ev = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, rcv_err, NULL);
                cncl_post_event(ev);
                break;
            }
            // 受信待ち(rcv_status == e_cncl_rcv_inpg: cncl_rcv_receive内でrecv発行済)
            else {
                break;
            }
        case e_trans_wait_body:
            rcv_status = cncl_rcv_receive(
                cmp_trans->rcv, cmp_trans->socket,
                cmp_trans->rcv_buff->data_info.rec_area + cmp_trans->config.header_length,  // body部の続きから読込む
                cmp_trans->cur_pending_length, cncl_trans_receive_complete, cncl_trans_receive_timeout, &rcv_err);
            // 要求した所定の1電文を受信完了
            if (rcv_status == e_cncl_rcv_cmpl) {
                // 電文振分(inbound)にPathsendする。
                // (1)trans_statusをヘッダ受信待ちに遷移
                cmp_trans->trans_status = e_trans_wait_header;
                // rcvスレッドのステータスをリセット
                cmp_rcv->initial_read = true;
                // (2)cncl_trans_msg_rcv_notify_c201_post内でtrans_statusが更新されることがあるので、(1)->(2)の順序でなければならない。
                cncl_trans_msg_rcv_notify_c201_post(thread_trans);
                cncl_trans_receive(thread_trans);
                //Event_tag_t *ev       = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_post_receive, 0, NULL);
                //cncl_post_event(ev);
            }
            // TCP/IP socket 受信エラー
            else if (rcv_status == e_cncl_rcv_fault) {
                // cncl_get_threadInfo(cmp_trans->rcv)->stopwait                 = true;
                ((cmp_rcv_t *)cncl_get_component(cmp_trans->rcv))->rcv_status = e_cncl_rcv_fault;
                Event_tag_t *ev = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, rcv_err, NULL);
                cncl_post_event(ev);
            }
            // 受信待ち(rcv_status == e_cncl_rcv_inpg: cncl_rcv_receive内でrecv発行済)
            else {
                break;
            }
            break;
        case e_trans_in_dist_outstanding_overflow:
        case e_trans_wait_initial_odst: {
            // Wait for the distributors(outbound) to be initialized
            Event_tag_t *ev = cncl_get_scheduled_tag(
                thread_trans, JULIANTIMESTAMP(no_param, no_param, no_param, no_param) + 1000 /* 0.01sec */,
                cncl_trans_post_receive, 0, NULL);
            cncl_post_event(ev);
        } break;
    }
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_receive_complete                             */
/*  CALLING SEQ.    :short cncl_trans_receive_complete(Event_tag_t *event)   */
/*  ARGUMENT        :event:recv完了通知イベント                              */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :受信完了の後処理を行い、必要なら次の受信を行う          */
/*****************************************************************************/
short cncl_trans_receive_complete(Event_tag_t *event)
{
    // trans handlerだがrcv threadが来るので、rcvコンポーネントからtransスレッドを取得する。
    thread_object_t *thread_rcv   = event->thread;
    cmp_rcv_t       *cmp_rcv      = cncl_get_component(thread_rcv);
    cmp_trans_t     *cmp_trans    = cncl_get_component(cmp_rcv->p_trans);
    thread_object_t *thread_trans = cmp_rcv->p_trans;
    cmp_rcv->remain_len = cmp_rcv->total_len = event->io_info.len;

    cncl_cancel_timer_tag(thread_rcv);
    if(EXTRACEMODE)
    {
        trace_end(cmp_rcv->rcv_buff, DEF_TRACE_IO_TYPE_RD, event->io_info.len, event->io_info.fs_err);
    }

    //* receive エラーが発生したので、closeリクエストを投げて終了(send側の終了を待つ) */
    if (event->io_info.fs_err != 0) {
        cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                          cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                          cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_RCV_ERR);
        cmp_rcv->rcv_status = e_cncl_rcv_fault;
        Event_tag_t *close_request =
            cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, event->io_info.fs_err, 0);
        cncl_post_event(close_request);
        return 0;
    }
    //* FINを受信した、closeリクエストを投げて終了(send側の終了を待つ) */
    if (event->io_info.len == 0) {
        // ↓のエラーはいらないかも。FIN受信なので)
        cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                          cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                          cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_RCV_ERR);
        cmp_rcv->rcv_status        = e_cncl_rcv_close;
        Event_tag_t *close_request = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, 0 /* FIN */, 0);
        cncl_post_event(close_request);
        return 0;
    }
    // receiveが正常終了したのでアプリ側で受信処理を行う。
    return cncl_trans_receive(thread_trans);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_receive_timeout                              */
/*  CALLING SEQ.    :short cncl_trans_receive_timeout(Event_tag_t *event)    */
/*  ARGUMENT        :event:受信タイムアウトイベント                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :受信タイムアウト発生時にソケットをキャンセルし回線終了  */
/*                  :処理を行う                                              */
/*****************************************************************************/
short cncl_trans_receive_timeout(Event_tag_t *event)
{
    // trans handlerだがrcv threadが来るので、rcvコンポーネントからtransスレッドを取得する。
    cmp_rcv_t       *cmp_rcv       = cncl_get_component(event->thread);
    cmp_trans_t     *cmp_trans     = cncl_get_component(cmp_rcv->p_trans);
    thread_header_t *rcv_thread_hd = cncl_get_threadInfo(event->thread);
    thread_object_t *thread_trans  = cmp_rcv->p_trans;
    Event_tag_t     *close_event   = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, 40 /*TIMEOUT*/, NULL);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    CANCELREQ(cmp_trans->socket, (long)rcv_thread_hd->io_ev_tag);
#pragma clang diagnostic pop
    if (EXTRACEMODE) {
            trace_end(cmp_rcv->rcv_buff, DEF_TRACE_IO_TYPE_RD, 0,  event->io_info.fs_err);
    }
    cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                      cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                      cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_RCV_ERR);
    cncl_remove_event(rcv_thread_hd->io_ev_tag);
    cmp_rcv->rcv_status = e_cncl_rcv_fault;
    cncl_post_event(close_event);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_close                                        */
/*  CALLING SEQ.    :short cncl_trans_close(Event_tag_t *event)              */
/*  ARGUMENT        :event:回線クローズ要求イベント                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :受信中・送信中の状態を見てクローズ可能なら切断          */
/*****************************************************************************/
short cncl_trans_close(Event_tag_t *event)
{
    cmp_trans_t     *cmp_trans = cncl_get_component(event->thread);
    cmp_rcv_t       *cmp_rcv   = cncl_get_component(cmp_trans->rcv);
    cmp_snd_t       *cmp_snd   = cncl_get_component(cmp_trans->snd);
    thread_header_t *hd_trans  = cncl_get_threadInfo(event->thread);
    thread_header_t *hd_rcv    = cncl_get_threadInfo(cmp_trans->rcv);
    thread_header_t *hd_snd    = cncl_get_threadInfo(cmp_trans->snd);
    bool             s_close   = false;
    bool             r_close   = false;
    short            s_err     = 0;

    // 仕様上どちらか一方のみがNULLということはないが、NULLの場合
    // 切断済、接続中、接続リトライ中のいずれかなのでこれ以上処理は必要ない
    if (!cmp_rcv || !cmp_snd) return 0;

    // rcvが以下でなければcloseしてよい
    // e_cncl_rcv_inpg、e_cncl_rcv_inpg_more
    switch (cmp_rcv->rcv_status) {
        case e_cncl_rcv_inpg_more:
            break;
        case e_cncl_rcv_fault:
            s_close = true;
        case e_cncl_rcv_inpg:
        case e_cncl_rcv_idle:
        case e_cncl_rcv_cmpl:
        case e_cncl_rcv_close:
            r_close = true;
            break;
    }
    // sndが以下でなければcloseしてよい。
    // e_cncl_snd_inpg
    // e_cncl_snd_faultの場合は直ちにすべて破棄する。
    // それ以外の場合はリトライする。
    //
    switch (cmp_snd->snd_status) {
        case e_cncl_snd_inpg:
            break;
        case e_cncl_snd_fault:
            r_close = true;
        case e_cncl_snd_idle:
        case e_cncl_snd_cmpl:
            s_close = true;
            break;
    }

    if (s_close && r_close) {
        // ソケットをクローズし、タイマーをキャンセルする。
        if (cmp_trans->socket != -1) {
            CANCEL(cmp_trans->socket);
            FILE_CLOSE_(cmp_trans->socket, no_param);
            cmp_trans->socket = -1;
        }
        // ソケットのイベントをキャンセルする。
        cncl_remove_event(hd_trans->io_ev_tag);
        cncl_remove_event(hd_rcv->io_ev_tag);
        cncl_remove_event(hd_snd->io_ev_tag);
        // connectあるいはリトライタイマーをキャンセルする。
        cncl_cancel_timer_tag(event->thread);
        cncl_cancel_timer_tag(cmp_trans->rcv);
        cncl_cancel_timer_tag(cmp_trans->snd);
        // 回線状態を切断済みに更新
        cmp_trans->line_status = e_line_disconnected;
        cncl_trans_notify_line_status(event->thread, no_param);
        cncl_delete_rcv(cmp_trans->rcv);
        cmp_trans->rcv = NULL;
        cncl_delete_snd(cmp_trans->snd);
        cmp_trans->snd = NULL;
        if (cmp_trans->proc_status != e_proc_close) {
            // 接続リトライする場合は回線状態を接続処理中に更新
            cncl_trans_update_gclst(event->thread, e_line_connect_in_progress, no_param,
                                    event->scheduled_event_info.sparam, e_discon_by_detect);
            s_err = cncl_trans_connect_retry(event->thread, event->scheduled_event_info.sparam);
        } else {
            // 切断要求を受けている場合は回線状態を切断に更新しリトライしない。
            cncl_trans_update_gclst(event->thread, e_line_disconnected, cmp_trans->proc_status,
                                    event->scheduled_event_info.sparam, e_discon_by_cls_cmd);
        }
    } else {
        Event_tag_t *close_request =
            cncl_get_scheduled_tag(event->thread, -1, cncl_trans_close, event->scheduled_event_info.sparam, 0);
        cncl_post_event(close_request);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :BCD_len_decoder                                         */
/*  CALLING SEQ.    :size_t BCD_len_decoder(const char *raw_length,          */
/*                   size_t raw_length_size,                                 */
/*                   int *i_err)                                             */
/*  ARGUMENT        :raw_length    :BCDデータ                                */
/*                  :raw_length_size:BCD長                                   */
/*                  :i_err         :エラー状態を格納するポインタ             */
/*  RETURN CODE     :デコードした数値,エラー時0                              */
/*  DESCRIPTION     :BCD形式のデータから数値をデコードする                   */
/*****************************************************************************/
size_t BCD_len_decoder(const char *raw_length, size_t raw_length_size, int *i_err)
{
    size_t        result = 0;
    unsigned char high, low;
    size_t        i;

    *i_err = 0;
    for (i = 0; i < raw_length_size; i++) {
        high = (char)(((raw_length[i]) >> 4) & 0x0f);
        low  = (char)((raw_length[i]) & 0x0f);
        if (high > 9 || low > 9) {
            *i_err = EINVAL;
            return result;
        };
        result = result * 100 + high * 10 + low;
    }
    return result;
}
/*****************************************************************************/
/*  FUNCTION        :BIN_len_decoder                                         */
/*  CALLING SEQ.    :size_t BIN_len_decoder(const char *raw_length,          */
/*                   size_t raw_length_size,                                 */
/*                   int *i_err)                                             */
/*  ARGUMENT        :raw_length    :バイナリデータ                           */
/*                  :raw_length_size:データ長                                */
/*                  :i_err         :エラー状態を格納                         */
/*  RETURN CODE     :デコードした数値,エラー時0                              */
/*  DESCRIPTION     :バイナリ形式のデータから数値を取得する                  */
/*****************************************************************************/
size_t BIN_len_decoder(const char *raw_length, size_t raw_length_size, int *i_err)
{
    char   buffer[8];
    size_t val = SIZE_MAX;
    *i_err     = 0;
    memset(buffer, 0, sizeof(buffer));
    switch (raw_length_size) {
        case 1:
            val = (size_t)*raw_length;
            break;
        case 2:
            val = (size_t)*(unsigned short *)raw_length;
            break;
        case 3:
        case 4:
            memmove(buffer + (sizeof(unsigned long) - raw_length_size), raw_length, raw_length_size);
            val = (size_t)*(unsigned long *)buffer;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            memmove(buffer + (sizeof(unsigned long long) - raw_length_size), raw_length, raw_length_size);
            val = (size_t)*(unsigned long *)buffer;
            break;
        default:
            *i_err = EINVAL;
    }
    return val;
}
/*****************************************************************************/
/*  FUNCTION        :EBCDIC_len_decoder                                      */
/*  CALLING SEQ.    :size_t EBCDIC_len_decoder(const unsigned char *num_str, */
/*                   size_t len, size_t *r_len, short *s_err)                */
/*  ARGUMENT        :num_str:文字列(EBCDIC)                                  */
/*                  :len    :データ長                                        */
/*                  :r_len  :実際に読み取った長さ                            */
/*                  :s_err  :エラー状態を返すポインタ                        */
/*  RETURN CODE     :変換した数値,エラー時LONG_MAX等                         */
/*  DESCRIPTION     :EBCDIC表記の数字文字列をデコードし数値を返す            */
/*****************************************************************************/
size_t EBCDIC_len_decoder(const unsigned char *num_str, size_t len, size_t *r_len, short *s_err)
{
    size_t val = 0;
    bool   ws_flg;
    *s_err = 0;
    *r_len = 0;
    if (len > 10) {
        *s_err = ERANGE;
        return LONG_MAX;
    }
    while (len--) {
        if (*num_str == 0x40 && !ws_flg) {
            ;
        } else {
            if (*num_str == 0x40 && ws_flg) break;
            if (*num_str < 0xF0 || *num_str > 0xF9) {
                if (!(*num_str == 0x40)) {
                    *s_err = EINVAL;
                }
                return val;
            }
            val    = val * 10 + *num_str - 0xF0;
            ws_flg = true;
        }
        num_str++;
        *r_len += 1;
    }
    return val;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_length_decoder                               */
/*  CALLING SEQ.    :size_t cncl_trans_length_decoder(const char *val,       */
/*                   size_t length,                                          */
/*                   d_len_attr_t data_len_attribute,                        */
/*                   int *i_err)                                             */
/*  ARGUMENT        :val     :長さ情報が格納されたバッファ                   */
/*                  :length :バッファサイズ                                  */
/*                  :data_len_attribute:長さのフォーマット属性               */
/*                  :i_err   :デコード結果(エラー含む)を返すポインタ         */
/*  RETURN CODE     :解釈したサイズ,エラー時SIZE_MAX                         */
/*  DESCRIPTION     :さまざまな形式(BCD,BIN,ASC,EBCDIC)で記録された          */
/*                  :データ長を取得                                          */
/*****************************************************************************/
size_t cncl_trans_length_decoder(const char *val, size_t length, d_len_attr_t data_len_attribute, int *i_err)
{
    size_t retval = SIZE_MAX;
    short  s_err;
    size_t r_len;
    switch (data_len_attribute) {
        case e_binary:
            retval = BIN_len_decoder(val, length, i_err);
            break;
        case e_bcd:
            retval = BCD_len_decoder(val, length, i_err);
            break;
        case e_ascii:
            retval = str2ul_c(val, length, &r_len, &s_err);
            *i_err = s_err;
            break;
        case e_ebcdic:
            retval = EBCDIC_len_decoder((const unsigned char *)val, length, &r_len, &s_err);
            *i_err = s_err;
            break;
        default:
            *i_err = EINVAL;
            break;
    }
    return retval;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_msg_rcv_notify_c201_post                     */
/*  CALLING SEQ.    :short cncl_trans_msg_rcv_notify_c201_post(              */
/*                   thread_object_t *thread_trans)                          */
/*  ARGUMENT        :thread_trans:トランスポートスレッド                     */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :受信済み電文からC201通知電文を生成し、                  */
/*                   電文振分(inbound)向けPATHSENDキューへ登録する。         */
/*                   ・データ長をヘッダ有無設定に応じて算出                  */
/*                   ・C201要求用バッファを確保し、要求電文を編集            */
/*                   ・生成電文をin_dist_p6d_qへ追加(enqueue)                */
/*                   ・キュー件数が閾値(CCC_MAX_recvqueue)以上なら受信停止   */
/*                   ・正常時は送信処理(cncl_trans_msg_rcv_notify_c201_send) */
/*                     を実行                                                */
/*****************************************************************************/
short cncl_trans_msg_rcv_notify_c201_post(thread_object_t *thread_trans)
{
    cmp_trans_t    *cmp_trans = cncl_get_component(thread_trans);

    size_t          data_length;

    io_trace_buf_t *c201_req;
    size_t          req_send_len;

    TS_UNIQUE_128   ts128;
    TS_UNIQUE_CREATE_((short *)&ts128);

    if (cmp_trans->config.data_len_include_id) {
        data_length = cmp_trans->cur_trans_length;
    } else {
        data_length = cmp_trans->cur_trans_length + cmp_trans->config.header_length;
    }
    c201_req     = alloc_io_mem(&io_mem);
    if (!c201_req) {
        cncl_ems_procedure_error("cncl_trans_msg_rcv_notify_c201_post", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    req_send_len = (short)cncl_trans_request_c201(thread_trans, (c201_def *)c201_req->data_info.rec_area,
                                                  cmp_trans->rcv_buff->data_info.rec_area, data_length,
                                                  &ts128 /*&hd->io_ev_tag->TS128*/);
    enqueue_io_mem(&cmp_trans->in_dist_p6d_info.in_dist_p6d_q, c201_req, req_send_len);
    // 電文振分に未送信の電文受信通知がキューにCCC_MAX_recvqueue件以上たまった場合は、次のrecvの実行を中止する。
    // recive/trans <= CCC_MAX_recvqueue
    if (cmp_trans->in_dist_p6d_info.in_dist_p6d_q.count >= CCC_MAX_recvqueue) {
        cmp_trans->trans_status = e_trans_in_dist_outstanding_overflow;
    }
    return cncl_trans_msg_rcv_notify_c201_send(thread_trans);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_msg_rcv_notify_c201_send                     */
/*  CALLING SEQ.    :short cncl_trans_msg_rcv_notify_c201_send(              */
/*                   thread_object_t *thread_trans)                          */
/*  ARGUMENT        :thread_trans:トランスポートスレッド                     */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :C201受信通知をPATHSENDで送信する。                      */
/*                   ・Outstanding数が閾値を超えている場合は送信を延期する   */
/*                   ・キューから通知要求を取得し、送信準備を行う            */
/*                   ・送信前に要求を待避し、Pathsendタグを取得する          */
/*                   ・非同期送信を実施し、トレースモード時は送信を記録      */
/*                   ・送信完了後にOutstandingカウンタをインクリメント       */
/*                   ・受信停止中かつキューが閾値未満の場合は受信を再開する  */
/*****************************************************************************/
short cncl_trans_msg_rcv_notify_c201_send(thread_object_t *thread_trans)
{
    myinfo_def     *myInfo    = &cncl_get_App()->my_info;
    cmp_trans_t    *cmp_trans = cncl_get_component(thread_trans);

    io_trace_buf_t *c201_req;
    io_trace_buf_t *c201_saved;
    size_t          req_send_len;

    short           s_err, s_pathedn_err, s_fs_err;

    // 電文振分(inbound)に対するPathsenのdoutstandingが制限を超えているならば延期する。
    // pathsend/CCC(process) <= CCC_MAX_inbound
    if (*cmp_trans->in_dist_p6d_info.outstanding >= CCC_MAX_inbound) {
        cncl_ems_pathsend_outstanding_exceeded(
            transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
            cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
            cmp_trans->config.local_if.sin_port, (short)*cmp_trans->in_dist_p6d_info.outstanding,
            DEF_NERR_NOMAL);
        Event_tag_t *re_post =
            cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_msg_rcv_notify_c201_send_event, 0, 0);
        cncl_post_event(re_post);
        return 0;
    }

    // 要求の取出し
    c201_req = dequeue_io_mem(&cmp_trans->in_dist_p6d_info.in_dist_p6d_q, &req_send_len);
    // 電文振分に未送信の電文受信通知がキューにCCC_MAX_recvqueue件以下かつ、新規のreceveを停止中の場合は
    // 受信を再開する。
    if (cmp_trans->in_dist_p6d_info.in_dist_p6d_q.count < CCC_MAX_recvqueue
        && cmp_trans->trans_status == e_trans_in_dist_outstanding_overflow) {
        cmp_trans->trans_status = e_trans_idle;
    }
    if (!c201_req) {
        // 空では有り得ないが、送信する物がないのならば正常終了する。
        return 0;
    }
    // 再送待避用メモリの確保
    c201_saved = alloc_io_mem(&io_mem);
    if (!c201_saved) {
        cncl_ems_procedure_error("cncl_trans_msg_rcv_notify_c201_send", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    // トレース開始
    if (EXTRACEMODE) {
        char trace_work[64];
        snprintf(trace_work, sizeof(trace_work), "%s/%s", cmp_trans->in_dist_s9s_info.pathmon_name,
                 cmp_trans->in_dist_s9s_info.s9s_name);
        trace_start(c201_req, DEF_TRACE_FILE_ID_PATHSEND, trace_work, DEF_TRACE_IO_TYPE_WR, req_send_len);
    }
    // 送信前電文の待避
    memcpy(c201_saved, c201_req, req_send_len);

    // Pathsend用のタグを取得
    Event_tag_t *c201_p6d =
        cncl_get_pathsend_tag(thread_trans, &cmp_trans->cmd_if_s9s_info, cncl_trans_msg_rcv_notify_c201_send_complete,
                              c201_req, c201_saved, (short)req_send_len, 0);

    if (!c201_p6d) {
        cncl_ems_procedure_error("cncl_get_pathsend_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    // 電文振分(inbound)に通知
    s_err = (short)SERVERCLASS_SEND_(cmp_trans->in_dist_s9s_info.pathmon_name, cmp_trans->in_dist_s9s_info.pathmon_name_len,
                              cmp_trans->in_dist_s9s_info.s9s_name, cmp_trans->in_dist_s9s_info.s9s_name_len,
                              c201_req->data_info.rec_area, (short)req_send_len, MAX_TEXT_BUF_LEN, no_param,
                              myInfo->pathsend_io_timer, 1, &c201_p6d->p6d_event_info.pathsend_fd, (int32_t)c201_p6d);
#pragma clang diagnostic pop
    if (s_err) {
        s_err = (short)SERVERCLASS_SEND_INFO_(&s_pathedn_err, &s_fs_err);
        cncl_ems_procedure_error("SERVERCLASS_SEND_INFO_", s_pathedn_err, DEF_NERR_SYSIF_LGC_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_SYSIF_LGC_ERR);
    }
    if (EXTRACEMODE) {
        trace_end(c201_req, DEF_TRACE_IO_TYPE_WR, req_send_len, s_err);
    }
    // 電文振分(Inbound)向けPathsendのOutstanding数をインクリメントする。
    (*(cmp_trans->in_dist_p6d_info.outstanding))++;
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_trans_msg_rcv_notify_c201_send_event               */
/*  CALLING SEQ.    :short cncl_trans_msg_rcv_notify_c201_send_event(        */
/*                   Event_tag_t *event_trans)                               */
/*  ARGUMENT        :event_trans:トランスポートスレッド用イベントタグ        */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :C201通知送信処理をイベントから起動する。                */
/*                  ・inbound向けPATHSENDのOutstanding数が閾値を超えている場合*/
/*                    はスケジュールイベントを再ポストして送信を延期する。   */
/*                  ・閾値内であれば即座にcncl_trans_msg_rcv_notify_c201_send*/
/*                    を呼び出し送信を実行する。                             */
/*****************************************************************************/
short cncl_trans_msg_rcv_notify_c201_send_event(Event_tag_t *event_trans)
{
    thread_object_t *thread_trans = event_trans->thread;
    cmp_trans_t     *cmp_trans    = cncl_get_component(thread_trans);

    // 電文振分(inbound)に対するPathsenのdoutstandingが制限を超えているならば延期する。
    if (*cmp_trans->in_dist_p6d_info.outstanding >= CCC_MAX_inbound) {
        Event_tag_t *re_post =
            cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_msg_rcv_notify_c201_send_event, 0, 0);
        cncl_post_event(re_post);
        return 0;
    }
    return cncl_trans_msg_rcv_notify_c201_send(thread_trans);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_msg_rcv_notify_c201_send_complete            */
/*  CALLING SEQ.    :short cncl_trans_msg_rcv_notify_c201_send_complete(     */
/*                   Event_tag_t *event)                                     */
/*  ARGUMENT        :event:電文振分(Inbound)向けC201送信完了イベント         */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :C201通知のPATHSEND完了処理を行う。                      */
/*                   ・正常応答でerror_code!=0の場合やp6dエラー時はリトライ  */
/*                   ・リトライ不可の場合はメモリ解放し終了                  */
/*                   ・リトライ回数超過時はEMSへ出力しメモリ解放             */
/*                   ・リトライ可能かつ回数内の場合は再送信を行う            */
/*                   ・Outstandingカウンタのインクリメント/デクリメント管理  */
/*****************************************************************************/
short cncl_trans_msg_rcv_notify_c201_send_complete(Event_tag_t *event)
{
    myinfo_def      *myInfo       = &cncl_get_App()->my_info;
    thread_object_t *thread_trans = event->thread;
    cmp_trans_t     *cmp_trans    = cncl_get_component(thread_trans);
    bool             retry_able;
    long             retry_count = event->p6d_event_info.retry_count;
    r201_def        *r201_rep    = (r201_def *)(event->p6d_event_info.send_buffer->data_info.rec_area);
    short            s_err, s_pathedn_err, s_fs_err;

    // 電文振分(Inbound)向けPathsendのOutstanding数をデクリメントする。
    (*(cmp_trans->in_dist_p6d_info.outstanding))--;
    if (EXTRACEMODE) {
        trace_end(event->p6d_event_info.send_buffer, DEF_TRACE_IO_TYPE_RD, event->io_info.len, event->io_info.fs_err);
    }
    do {
        if (!event->io_info.fs_err) {
            // pathsend正常、エラー応答ならリトライする。
            if (r201_rep->common_header.error_code) {
                // リトライする
                cncl_ems_p6d_error(no_param, event->p6d_event_info.s9s_info->pathmon_name,
                       event->p6d_event_info.s9s_info->s9s_name, r201_rep->common_header.error_code, DEF_NERR_PSEND_ERR_RE_OK);
                break;
            }
        } else {
            // pathendのエラー
            retry_able = cncl_trans_p6d_error(event);
            if (retry_able) {
                // リトライ可能。
                break;
            }
        }
        // 正常orリトライ不可
        free_io_mem(&io_mem, event->p6d_event_info.send_buffer);
        free_io_mem(&io_mem, event->p6d_event_info.org_msg);
        return 0;
    } while (0);
    // リトライ
    if (++retry_count > myInfo->psend_retry_cnt) {  // リトライオーバー
        cncl_ems_p6d_error(no_param, event->p6d_event_info.s9s_info->pathmon_name,
                           event->p6d_event_info.s9s_info->s9s_name, event->io_info.fs_err, DEF_NERR_PSEND_ERR_RE_OUT);
        free_io_mem(&io_mem, event->p6d_event_info.send_buffer);
        free_io_mem(&io_mem, event->p6d_event_info.org_msg);
    } else {
        // リトライ
        io_trace_buf_t *c201_req     = event->p6d_event_info.send_buffer;
        io_trace_buf_t *c201_saved   = event->p6d_event_info.org_msg;
        short           req_send_len = event->p6d_event_info.send_len;
        memcpy(c201_req, c201_saved, event->p6d_event_info.send_len);
        Event_tag_t *c201_p6d = cncl_get_pathsend_tag(thread_trans, &cmp_trans->cmd_if_s9s_info,
                                                      cncl_trans_msg_rcv_notify_c201_send_complete, c201_req,
                                                      c201_saved, req_send_len, retry_count);
        if (!c201_p6d) {
            cncl_ems_procedure_error("cncl_get_pathsend_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
        }
        // trace_startは不要(traceごとメッセージを復旧しているので)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
        // 電文振分(inbound)に通知
        s_err = (short)SERVERCLASS_SEND_(
            cmp_trans->in_dist_s9s_info.pathmon_name, cmp_trans->in_dist_s9s_info.pathmon_name_len,
            cmp_trans->in_dist_s9s_info.s9s_name, cmp_trans->in_dist_s9s_info.s9s_name_len,
            c201_req->data_info.rec_area, req_send_len, MAX_TEXT_BUF_LEN, no_param, myInfo->pathsend_io_timer, 1,
            &c201_p6d->p6d_event_info.pathsend_fd, (int32_t)c201_p6d);
#pragma clang diagnostic pop
        if (s_err) {
            s_err = (short)SERVERCLASS_SEND_INFO_(&s_pathedn_err, &s_fs_err);
            cncl_ems_procedure_error("SERVERCLASS_SEND_INFO_", s_pathedn_err, DEF_NERR_SYSIF_LGC_ERR);
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_SYSIF_LGC_ERR);
        }
        if (EXTRACEMODE) {
            trace_end(c201_req, DEF_TRACE_IO_TYPE_WR, req_send_len, s_err);
        }
        // 電文振分(Inbound)向けPathsendのOutstanding数をインクリメントする。
        (*(cmp_trans->in_dist_p6d_info.outstanding))++;
    }
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_trans_connect_request                              */
/*  CALLING SEQ.    :short cncl_trans_connect_request(Event_tag_t *event)    */
/*  ARGUMENT        :event:外部からのOPEN指示イベント                        */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :切断中ならconnect処理を開始する                         */
/*****************************************************************************/
short cncl_trans_connect_request(Event_tag_t *event)
{
    short            s_err        = 0;
    thread_object_t *thread_trans = event->thread;
    cmp_trans_t     *cmp_trans    = cncl_get_component(thread_trans);
    if (cmp_trans->line_status == e_line_disconnected) {
        cmp_trans->proc_status = e_proc_open;
        s_err                  = cncl_trans_connect(thread_trans);
    } else if (cmp_trans->line_status == e_line_connected) {
        cncl_trans_notify_line_status(thread_trans, true);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_close_request                                */
/*  CALLING SEQ.    :short cncl_trans_close_request(Event_tag_t *event)      */
/*  ARGUMENT        :event:外部からのCLOSE指示イベント                       */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :回線状態に応じてクローズ処理やタイマーキャンセル        */
/*****************************************************************************/
short cncl_trans_close_request(Event_tag_t *event)
{
    cmp_trans_t     *cmp_trans = cncl_get_component(event->thread);
    thread_header_t *hd        = cncl_get_threadInfo(event->thread);
    switch (cmp_trans->line_status) {
        case e_line_connected: {  //<- close_eventはここでしか使わないので明示的にスコープを宣言する。
            // 接続中-切断要求受信
            cmp_trans->proc_status = e_proc_close;
            // sndスレッドとrcvスレッドの処分が必要なので、ディスパッチしてcncl_trans_closeを呼出す。
            Event_tag_t *close_event =
                cncl_get_scheduled_tag(event->thread, -1, cncl_trans_close, 0 /* cmd request */, NULL);
            cncl_post_event(close_event);
        } break;
        case e_line_connect_in_progress:
            // スレッドがconnect実行中の場合、sndスレッドとrcvスレッドはNULLなのでここで切断処理は終了する。
            // closeコマンド受信済み
            cmp_trans->proc_status = e_proc_close;
            // ソケットをクローズし、タイマーをキャンセルする。
            if (cmp_trans->socket != -1) {
                CANCEL(cmp_trans->socket);
                FILE_CLOSE_(cmp_trans->socket, no_param);
                cmp_trans->socket = -1;
            }
            // ソケットのイベントをキャンセルする。
            cncl_remove_event(hd->io_ev_tag);
            // connectあるいはリトライタイマーをキャンセルする。
            cncl_cancel_timer_tag(event->thread);
            // 回線状態を切断済みに更新
            cmp_trans->line_status = e_line_disconnected;
            cncl_trans_notify_line_status(event->thread, no_param);
            // 回線ステータスファイルを切断に更新する。
            cncl_trans_update_gclst(event->thread, cmp_trans->line_status, cmp_trans->proc_status, 0 /* cmd request */,
                                    e_discon_by_cls_cmd);
        default:
            // 切断状態、切断要求受信(動作なし)
            break;
    }
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_send_dt                                      */
/*  CALLING SEQ.    :short cncl_trans_send_dt(thread_object_t *thread_trans) */
/*  ARGUMENT        :thread_trans:transportスレッド                          */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :接続直後に特定データを送信(オプション処理)              */
/*****************************************************************************/
short cncl_trans_send_dt(thread_object_t *thread_trans)
{
    cmp_trans_t    *cmp_trans = cncl_get_component(thread_trans);
    io_trace_buf_t *sp_data   = alloc_io_mem(&io_mem);
    if (!sp_data) {
        cncl_ems_procedure_error("cncl_trans_send_dt", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    size_t    sp_data_len;
    size_t    c202_req_len;
    c202_def *c202_req = (c202_def *)sp_data->data_info.rec_area;

    memset(sp_data, 0, sizeof(io_trace_buf_t));
    hexbin_encoder(c202_req->msg_info.msg_data, sizeof(c202_req->msg_info.msg_data),
                   cmp_trans->gfnwi.connect_nxt_prc_info.spc_data,
                   sizeof(cmp_trans->gfnwi.connect_nxt_prc_info.spc_data), &sp_data_len);
    c202_req->msg_info.msg_len = (short)sp_data_len;
    c202_req_len =
        sizeof(c202_def) - sizeof(c202_req->msg_info) + sizeof(c202_req->msg_info.msg_len) + c202_req->msg_info.msg_len;
    cncl_snd_request(cmp_trans->snd, sp_data, c202_req_len);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_opening_request_c501                         */
/*  CALLING SEQ.    :size_t cncl_trans_opening_request_c501(                 */
/*                   thread_object_t *thread_trans,                          */
/*                   c501_def *opening_request)                              */
/*  ARGUMENT        :thread_trans    :transportスレッド                      */
/*                  :opening_request :C501電文バッファ                       */
/*  RETURN CODE     :C501電文サイズ                                          */
/*  DESCRIPTION     :開局要求用のC501電文データを編集する                    */
/*****************************************************************************/
size_t cncl_trans_opening_request_c501(thread_object_t *thread_trans, c501_def *opening_request)
{
    cmp_trans_t *cmp_trans = cncl_get_component(thread_trans);
    enum
    {
        e_connection_layer,
        e_interface_layer,
        e_station_layer,
        e_end_of_layer
    } if_mgr_layer;
    char open_close_mng_lyr_val[] = {DEF_OPN_CLS_MNG_LYR_CO, DEF_OPN_CLS_MNG_LYR_IF, DEF_OPN_CLS_MNG_LYR_ST};

    memset(opening_request, ' ', sizeof(c502_def));
    memmove(opening_request->common_header.interface_code, DEF_IPC_IFCD_CMD_REQ, sizeof(DEF_IPC_IFCD_CMD_REQ));
    // エラーコード(初期値)
    opening_request->common_header.error_code = 0;
    memmove(opening_request->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL) - 1);
    // データ部長
    opening_request->common_header.control_data_length = sizeof(c501_def) - sizeof(common_header_def);

    // 自動開局(コネクション確立):2014
    memmove(opening_request->command_info.command_name, DEF_IPC_CMD_CNT_OPN_AUT_CON, sizeof(DEF_IPC_CMD_CNT_OPN));

    // 管理レベル判定
    for (if_mgr_layer = e_connection_layer; if_mgr_layer < e_end_of_layer; if_mgr_layer++) {
        if (cmp_trans->gfnwi.mng_lyr_info.open_close_mng_lyr == open_close_mng_lyr_val[if_mgr_layer]) break;
    }
    // 回線レベル
    switch (if_mgr_layer) {
        case e_connection_layer:
            // コネクション論理ID
            memcpy(&opening_request->command_info.connection_logical_name, &cmp_trans->gflin.pri_key,
                   sizeof(gflin_pkey_def));
            break;
        case e_station_layer:    // station
            memmove(opening_request->command_info.station_ext_name, cmp_trans->gfnwi.nw_id_info.nw_station, 11);
            break;
        case e_interface_layer:  // interface
            memmove(opening_request->command_info.interface_ext_name, cmp_trans->gfnwi.nw_id_info.nw_if, 20);
            break;
        default:
            break;
    }
    // レコード数 (開局コマンドはデータ部無し)
    opening_request->record_count = 0;
    return sizeof(c501_def);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_opening_request_c501_send_complete           */
/*  CALLING SEQ.    :short cncl_trans_opening_request_c501_send_complete(    */
/*                   Event_tag_t *event)                                     */
/*  ARGUMENT        :event: C501送信完了を通知するイベントタグ               */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :C501開局要求送信後の完了処理を行う。                    */
/*                   ・fs_err==0 かつ応答error_code!=0 の場合リトライ実施    */
/*                   ・fs_err!=0 の場合、p6dエラー処理(cncl_trans_p6d_error) */
/*                     によりリトライ可否を判定                              */
/*                   ・リトライ可能であればリトライ処理へ                    */
/*                   ・リトライ不可または正常完了の場合はメモリ解放          */
/*                   ・リトライ回数が閾値(psend_retry_cnt)を超過した場合は   */
/*                     EMS出力(DEF_NERR_PSEND_ERR_RE_OUT)                    */
/*                   ・リトライ可能かつ閾値内なら再送信                      */
/*****************************************************************************/
short cncl_trans_opening_request_c501_send_complete(Event_tag_t *event)
{
    myinfo_def *myInfo = &cncl_get_App()->my_info;
    bool        retry_able;
    long        retry_count = event->p6d_event_info.retry_count;
    r501_def   *r501_rep    = (r501_def *)(event->p6d_event_info.send_buffer->data_info.rec_area);
    if (EXTRACEMODE) {
        trace_end(event->p6d_event_info.send_buffer, DEF_TRACE_IO_TYPE_RD, event->io_info.len, event->io_info.fs_err);
    }
    do {
        if (!event->io_info.fs_err) {
            // pathsend正常、エラー応答ならリトライする。
            if (r501_rep->common_header.error_code) {
                // リトライする
                break;
            }
        } else {
            // pathendのエラー
            retry_able = cncl_trans_p6d_error(event);
            if (retry_able) {
                // リトライ可能。
                break;
            }
        }
        // リトライ不可
        free_io_mem(&io_mem, event->p6d_event_info.send_buffer);
        return 0;
    } while (0);
    free_io_mem(&io_mem, event->p6d_event_info.send_buffer);
    // リトライ
    if (++retry_count > myInfo->psend_retry_cnt) {  // リトライオーバー
        cncl_ems_p6d_error(no_param, event->p6d_event_info.s9s_info->pathmon_name,
                           event->p6d_event_info.s9s_info->s9s_name, event->io_info.fs_err, DEF_NERR_PSEND_ERR_RE_OUT);
    } else {
        cncl_trans_opening_request_c501_send(event->thread, retry_count);
    }
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_trans_opening_request_c501_send                    */
/*  CALLING SEQ.    :short cncl_trans_opening_request_c501_send(             */
/*                   thread_object_t *thread_trans, long retry_count)        */
/*  ARGUMENT        :thread_trans:トランスポートスレッド                     */
/*                   retry_count :リトライ回数（送信試行番号）               */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :C501開局要求を生成しPATHSENDで送信する。                */
/*                   ・送信用バッファを確保し、要求電文を編集                */
/*                   ・パスセンドタグを取得し非同期送信を実施                */
/*                   ・トレースモード時は送信内容を記録                      */
/*                   ・送信失敗時はSERVERCLASS_SEND_INFO_で詳細を取得し      */
/*                     EMS出力後に異常終了                                   */
/*****************************************************************************/
short cncl_trans_opening_request_c501_send(thread_object_t *thread_trans, long retry_count)
{
    myinfo_def     *myinfo    = &cncl_get_App()->my_info;
    cmp_trans_t    *cmp_trans = cncl_get_component(thread_trans);
    io_trace_buf_t *c501_req;
    Event_tag_t    *c501_p6d;
    short           s_err, s_pathedn_err, s_fs_err;
    char            trace_work[64];

    c501_req = alloc_io_mem(&io_mem);
    if (!c501_req) {
        cncl_ems_procedure_error("cncl_trans_opening_request_c501_send", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    cncl_trans_opening_request_c501(thread_trans, (c501_def *)c501_req->data_info.rec_area);

    c501_p6d =
        cncl_get_pathsend_tag(thread_trans, &cmp_trans->cmd_if_s9s_info, cncl_trans_opening_request_c501_send_complete,
                              c501_req, NULL, (short)sizeof(c501_def), retry_count);
    if (!c501_p6d) {
        cncl_ems_procedure_error("cncl_get_pathsend_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    if (EXTRACEMODE) {
        snprintf(trace_work, sizeof(trace_work), "%s/%s", cmp_trans->cmd_if_s9s_info.pathmon_name,
                 cmp_trans->cmd_if_s9s_info.s9s_name);
        trace_start(c501_req, DEF_TRACE_FILE_ID_PATHSEND, trace_work, DEF_TRACE_IO_TYPE_WR, sizeof(c501_def));
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    /*PATHSEND実施*/
    s_err = SERVERCLASS_SEND_(cmp_trans->cmd_if_s9s_info.pathmon_name, cmp_trans->cmd_if_s9s_info.pathmon_name_len,
                              cmp_trans->cmd_if_s9s_info.s9s_name, cmp_trans->cmd_if_s9s_info.s9s_name_len,
                              c501_req->data_info.rec_area, (short)sizeof(c501_def), (short)sizeof(r501_def), no_param,
                              myinfo->pathsend_io_timer, 1, &c501_p6d->p6d_event_info.pathsend_fd, (int32_t)c501_p6d);
#pragma clang diagnostic pop
    if (s_err) {
        s_err = SERVERCLASS_SEND_INFO_(&s_pathedn_err, &s_fs_err);
        cncl_ems_procedure_error("SERVERCLASS_SEND_INFO_", s_pathedn_err, DEF_NERR_SYSIF_LGC_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_SYSIF_LGC_ERR);
    }
    if (EXTRACEMODE) {
        trace_end(c501_req, DEF_TRACE_IO_TYPE_WR, sizeof(c501_def), s_err);
    }

    return s_err;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_delete_trans                                       */
/*  CALLING SEQ.    :void cncl_delete_trans(thread_object_t *thread_trans)   */
/*  ARGUMENT        :thread_trans:削除対象のtransportスレッド                */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :transport用スレッドを破棄し関連バッファを解放           */
/*****************************************************************************/
void cncl_delete_trans(thread_object_t *thread_trans)
{
    cmp_trans_t *cmp_trans;
    if (!thread_trans) return;
    cmp_trans = cncl_get_component(thread_trans);
    free_io_mem(&io_mem, cmp_trans->rcv_buff);
    cncl_delete_rcv(cmp_trans->rcv);
    cncl_delete_snd(cmp_trans->snd);
    cncl_delete_thread(thread_trans);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_trans_p6d_error                                    */
/*  CALLING SEQ.    :bool cncl_trans_p6d_error(                              */
/*                   Event_tag_t *p6d_event)                                 */
/*  ARGUMENT        :p6d_event:PATHSEND完了イベントタグ                      */
/*  RETURN CODE     : true  - リトライ可能と判定された場合                   */
/*                  : false - リトライ不可または致命的エラー                 */
/*  DESCRIPTION     :PATHSEND失敗時のエラーコードを解析し、リトライ可否を判定。*/
/*                   ・SERVERCLASS_SEND_INFO_でp6d_err,fs_errを取得          */
/*                   ・タイムアウトや特定のエラーはリトライ可とする          */
/*                   ・それ以外は不可としEMSへエラー出力                     */
/*                   ・最終的にリトライ可能フラグを返却                      */
/*  DESCRIPTION     :                                                        */
/*    PATHSEND失敗時のエラーコードを解析し、内部エラーコードにマッピングする。*/
/*    また、リトライ可否を判定して呼び出し元に返却する。                     */
/*      1. SERVERCLASS_SEND_INFO_ により P6D エラー／FS エラーを取得         */
/*      2. エラー種別ごとに内部エラーコード(inter_errcd)を設定               */
/*      3. リトライ可否 (retry_able) を判定                                  */
/*      4. EMS 出力関数 cncl_ems_p6d_error にログを送出                      */
/*      5. 判定結果 (retry_able) を返却                                      */
/*****************************************************************************/
bool cncl_trans_p6d_error(Event_tag_t *p6d_event)
{
    short post_errcd;          // ログ出力用に使用するエラーコード（p6d_err or fs_err）
    short p6d_err;             // P6D (Pathsend) 側エラーコード
    short fs_err;              // FS (ファイルシステム) 側エラーコード
    char *inter_errcd;         // 内部エラーコード（ログ／上位層での解釈用）
    bool  retry_able = false;  // リトライ可否フラグ（初期値: 不可）

    // サーバクラス通信のエラー情報を取得
    SERVERCLASS_SEND_INFO_(&p6d_err, &fs_err);
    // 初期値として post_errcd に Pathsend側エラーを設定
    post_errcd = p6d_err;
    // Pathsend エラーコードごとの処理分岐
    switch (p6d_err) {
        case ZFIL_ERR_TIMEDOUT:
            // Pathsend レベルでタイムアウト → 再送可能。
            inter_errcd = DEF_NERR_PSEND_TIMEOUT;
            break;
        case ZFIL_ERR_PATHDOWN:
            // PATHDOWN リトライ可
            inter_errcd = DEF_NERR_PSEND_ERR_RE_OK;
            retry_able  = true;
            break;
        case FESCSERVERLINKCONNECT:
        case FESCOPERATIONABORTED:
            // サーバリンク接続エラー／操作中断 → FS 側エラーを評価
            post_errcd = fs_err;
            switch (fs_err) {
                case ZFIL_ERR_TIMEDOUT:
                    // FS レベルでタイムアウト → タイムアウト扱い
                    inter_errcd = DEF_NERR_PSEND_TIMEOUT;
                    break;
                case ZFIL_ERR_PATHDOWN:
                    // FS レベルでPATHDOWN → 再送可能とみなす
                    inter_errcd = DEF_NERR_PSEND_ERR_RE_OK;
                    retry_able  = true;
                    break;
                default:
                    // 上記以外の FS エラー → リトライ不可
                    inter_errcd = DEF_NERR_PSEND_ERR_RE_NG;
                    break;
            }
            break;
        case FESCNOSERVERAVAILABLE:
        case FESCSERVERCLASSFROZEN:
        case FESCPATHMONSHUTDOWN:
            // サーバ未起動／クラス凍結／PathMon 終了 → 致命的エラー
            inter_errcd = DEF_NERR_PSEND_ERR_RE_NG;
            break;
        default:
            // 上記以外の P6D エラー → 致命的エラー
            inter_errcd = DEF_NERR_PSEND_ERR_RE_NG;
            post_errcd  = p6d_event->io_info.fs_err;
    }
    // エラー情報を EMS に出力
    cncl_ems_p6d_error(no_param, p6d_event->p6d_event_info.s9s_info->pathmon_name,
                       p6d_event->p6d_event_info.s9s_info->s9s_name, post_errcd, inter_errcd);
    // リトライ可否を返却
    return retry_able;
}
