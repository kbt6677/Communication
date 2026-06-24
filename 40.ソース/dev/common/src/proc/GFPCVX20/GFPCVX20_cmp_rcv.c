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
#include <assert.h>                     nolist
#include <stdlib.h>                     nolist
#include <string.h>                     nolist
#ifdef _TANDEM_SOURCE
#include <tal.h> nolist
#include <cextdecs.h(SIGNALTIMEOUT,FILE_GETINFO_)> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif
/* USER HEADER     */
#include <errcd.h>                      nolist
#include "GFPCVX20_cmp_rcv.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_util.h" nolist

/*****************************************************************************/
/*  FUNCTION        :cncl_rcv_receive                                        */
/*  CALLING SEQ.    :rcv_status_t cncl_rcv_receive(                          */
/*                   thread_object_t *thread_rcv,                            */
/*                   short socket,                                           */
/*                   char *sq_buff,//io_trace_buf_t::data_info.rec_area      */
/*                   size_t req_len,                                         */
/*                   event_func completion_trans_func,                       */
/*                   event_func timeout_trans_func,                          */
/*                   short * last_err)                                       */
/*  ARGUMENT        :thread_rcv          :受信制御用のスレッド               */
/*                  :socket              :受信用ソケット                     */
/*                  :sq_buff             :受信データ格納先                   */
/*                  :req_len             :要求する受信バイト数               */
/*                  :completion_trans_func:受信完了時に呼ばれる関数          */
/*                  :timeout_trans_func  :受信タイムアウト時の処理           */
/*                  :last_err            :recv_nwの処理結果                  */
/*  RETURN CODE     :e_cncl_rcv_cmpl, e_cncl_rcv_inpg, e_cncl_rcv_fault等    */
/*  DESCRIPTION     :指定バイト数のデータを受信し、完了もしくはタイムアウト  */
/*                  :を管理する                                              */
/*****************************************************************************/
rcv_status_t cncl_rcv_receive(thread_object_t *thread_rcv, short socket, char *sq_buff, size_t req_len,
                              event_func completion_trans_func, event_func timeout_trans_func, short *last_err)
{
    cmp_rcv_t *cmp_rcv = (cmp_rcv_t *)cncl_get_component(thread_rcv);
    int        ret;
    short      ret_last_error;
    size_t     sq_dlt_length;
    size_t     cp_length;  // 今回コピーする量。

    // 念のため処理結果として正常を設定しておく。
    if (last_err) *last_err = 0;

    // 新規電文読込開始時のみステータス初期化
    if (cmp_rcv->rcv_status == e_cncl_rcv_idle) {
        cmp_rcv->req_len = req_len;  // 初回呼出し時に要求長をセーブ
        cmp_rcv->seq_len = 0;        // 初回呼出し時に呼出し側に引き渡し済データ長を初期化する。
        cmp_rcv->socket  = socket;
    }

    // cmp_rcv->req_lenが0になるまでcmp_rcv->remain_lenが許す限りコピーする。
    // cmp_rcv->remain_lenが0になってもcmp_rcv->req_lenが0にならない場合は受信要求を行う。
    sq_dlt_length = cmp_rcv->total_len - cmp_rcv->remain_len;

    if (cmp_rcv->remain_len > 0) {
        cp_length = cmp_rcv->req_len >= cmp_rcv->remain_len ? cmp_rcv->remain_len : cmp_rcv->req_len;
        memcpy(sq_buff + cmp_rcv->seq_len, cmp_rcv->rcv_top + sq_dlt_length, cp_length);
        cmp_rcv->remain_len -= cp_length;
        cmp_rcv->seq_len += cp_length;
        cmp_rcv->req_len -= cp_length;
        cmp_rcv->initial_read = false;
    }
    // 要求データ長を満たしたら受信終了
    if (cmp_rcv->req_len == 0) {
        cmp_rcv->rcv_status = e_cncl_rcv_idle;
        return e_cncl_rcv_cmpl;
    }
    // 残データがあるときはここにはこない。
    assert(cmp_rcv->remain_len == 0);
    Event_tag_t *event_socket = cncl_get_socket_tag(thread_rcv, socket, completion_trans_func);
    if (!event_socket) {
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    if (EXTRACEMODE) {
        trace_start(cmp_rcv->rcv_buff, DEF_TRACE_FILE_ID_PROC, cmp_rcv->transport_config->tcpip_prc_name, DEF_TRACE_IO_TYPE_RD, 0);
    }
//    cmp_rcv->rcv_status = e_cncl_rcv_inpg;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    ret = recv_nw(socket, cmp_rcv->rcv_top, sizeof(cmp_rcv->rcv_buff->data_info.rec_area), 0, (long)event_socket);
#pragma clang diagnostic pop
    if (ret) {
        FILE_GETINFO_(socket, &ret_last_error, no_param, no_param, no_param, no_param, no_param);
        cncl_remove_event(event_socket);
        // NOWAIT I/OのProcedure呼出時のエラーはprocedure_error
        cncl_ems_procedure_error("recv_nw", ret_last_error, DEF_NERR_RCV_ERR);
        cmp_rcv->rcv_status = e_cncl_rcv_fault;
        if (last_err) {
            *last_err = ret_last_error;
        }
        if (EXTRACEMODE) {
            trace_end(cmp_rcv->rcv_buff, DEF_TRACE_IO_TYPE_RD, 0, ret_last_error);
        }
        return e_cncl_rcv_fault;
    }
    Event_tag_t *event_rcv_timeout;

    // 必ずreq_len > 0でここにくる。seq_len > 0 ならばreq_lenに対して不足分データの受信中(e_cncl_rcv_inpg_more)、
    cmp_rcv->rcv_status    = cmp_rcv->seq_len != 0 ? e_cncl_rcv_inpg_more : e_cncl_rcv_inpg;
    // タイムアウト関数が設定されているかつ、後続データ受信中(e_cncl_rcv_inpg_more)または電文の最初の読出しではないinitial_read==false;
    if (timeout_trans_func && (cmp_rcv->rcv_status == e_cncl_rcv_inpg_more || !cmp_rcv->initial_read)) {
        event_rcv_timeout = cncl_get_timer_tag(thread_rcv, cmp_rcv->transport_config->recv_complete_timer, timeout_trans_func, DEF_RECEIVE_TIMER);
        if (!event_rcv_timeout) {
            cmp_rcv->rcv_status = e_cncl_rcv_fault;
            return e_cncl_rcv_fault;
        }
    }
    else if(timeout_trans_func && cmp_rcv->initial_read && cmp_rcv->transport_config->idle_mon_timer > 0){
        // 無通信監視タイマを追加。
        event_rcv_timeout = cncl_get_timer_tag(thread_rcv, cmp_rcv->transport_config->idle_mon_timer, timeout_trans_func, DEF_RECEIVE_TIMER);
        if (!event_rcv_timeout) {
            cmp_rcv->rcv_status = e_cncl_rcv_fault;
            return e_cncl_rcv_fault;
        }
    }

    return e_cncl_rcv_inpg;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_create_rcv                                         */
/*  CALLING SEQ.    :thread_object_t *cncl_create_rcv(                       */
/*                   thread_object_t *thread_trans)                          */
/*  ARGUMENT        :thread_trans:送受信の親スレッドオブジェクト             */
/*  RETURN CODE     :生成された受信制御スレッドへのポインタ                  */
/*  DESCRIPTION     :受信用スレッドを生成し、受信バッファ等を初期化          */
/*****************************************************************************/
thread_object_t *cncl_create_rcv(thread_object_t *thread_trans)
{
    thread_object_t *thread;
    cmp_rcv_t       *cmp_rcv;
    cmp_trans_t     *cmp_trans = (cmp_trans_t *)cncl_get_component(thread_trans);

    thread                     = cncl_create_thread(receiver);
    if (!thread) return NULL;
    cmp_rcv = (cmp_rcv_t *)cncl_get_component(thread);
    memset(cmp_rcv, 0, sizeof(cmp_rcv_t));
    cmp_rcv->rcv_buff = alloc_io_mem(&io_mem);
    if (!(cmp_rcv->rcv_buff)) {
        cncl_ems_procedure_error("cncl_create_rcv", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
        return NULL;
    }
    cmp_rcv->p_trans          = thread_trans;
    cmp_rcv->transport_config = &(cmp_trans->config);
    cmp_rcv->rcv_top          = cmp_rcv->rcv_buff->data_info.rec_area;
    cmp_rcv->initial_read     = true;
    return thread;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_delete_rcv                                         */
/*  CALLING SEQ.    :void cncl_delete_rcv(thread_object_t *thread_rcv)       */
/*  ARGUMENT        :thread_rcv:削除対象の受信用スレッド                     */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :受信制御スレッドを破棄しバッファを解放する              */
/*****************************************************************************/
void cncl_delete_rcv(thread_object_t *thread_rcv)
{
    if (!thread_rcv) return;
    cmp_rcv_t *cmp_rcv = (cmp_rcv_t *)cncl_get_component(thread_rcv);
    free_io_mem(&io_mem, cmp_rcv->rcv_buff);
    cncl_delete_thread(thread_rcv);
}
