/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/01/20＞         *
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
/*        WRITTEN-DATE      ････ 2025/01/20                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/03/01 新規作成                                     */
/*                   2025/05/08 SS内結合テスト不具合 No.91                   */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#ifdef _TANDEM_SOURCE
#include <tal.h> nolist
#include <cextdecs.h(FILE_GETINFO_,CANCELREQ)> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif
#include <string.h> nolist
/* USER HEADER     */
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_cmp_snd.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_util.h" nolist
#include "GFPCVX20_ipc_wrapper.h" nolist // IWYU pragma: keep
#include <errcd.h> nolist
/****************************************************************************/
/*  FUNCTION        :cncl_create_snd                                        */
/*  CALLING SEQ.    :thread_object_t* cncl_create_snd(                      */
/*                   thread_object_t *thread_trans,                         */
/*                   event_func trans_notice)                               */
/*  ARGUMENT        :thread_trans    :送受信制御スレッド                    */
/*                  :trans_notice    :送信完了後に呼ばれる関数              */
/*  RETURN CODE     :生成した送信用スレッドのポインタ                       */
/*  DESCRIPTION     :送信用スレッドを生成し初期化する                       */
/****************************************************************************/
thread_object_t *cncl_create_snd(thread_object_t *thread_trans, event_func trans_notice)
{
    thread_object_t *thread_snd = cncl_create_thread(sender);
    if (thread_snd == NULL) return NULL;
    cmp_snd_t   *snd          = (cmp_snd_t *)cncl_get_component(thread_snd);
    cmp_trans_t *cmp_trans    = cncl_get_component(thread_trans);
    /* ここはスレッド作成に含めてもよいかもしれない */
    snd->p_trans              = thread_trans;
    snd->socket               = cmp_trans->socket;
    snd->trans_notice         = trans_notice;
    snd->transport_config     = &cmp_trans->config;
    return thread_snd;
}
/****************************************************************************/
/*  FUNCTION        :cncl_snd_request                                       */
/*  CALLING SEQ.    :bool cncl_snd_request(thread_object_t *thread_snd,     */
/*                   io_trace_buf_t *snd_request,                           */
/*                   size_t length)                                         */
/*  ARGUMENT        :thread_snd :送信用スレッド                             */
/*                  :snd_request:送信データを格納したバッファ               */
/*                  :length    :送信データ長                                */
/*  RETURN CODE     :true,false(送信キューへの追加可否)                     */
/*  DESCRIPTION     :送信用リクエストをキューに登録し、必要に応じ送信       */
/****************************************************************************/
bool cncl_snd_request(thread_object_t *thread_snd, io_trace_buf_t *snd_request, size_t length)
{
    cmp_snd_t *snd = (cmp_snd_t *)cncl_get_component(thread_snd);
    enqueue_io_mem(&(snd->snd_req_q), snd_request, length);
    if (snd->snd_status == e_cncl_snd_idle) {
        cncl_snd_send(thread_snd);
    }
    return true;  // voidで良かった？
}
/****************************************************************************/
/*  FUNCTION        :cncl_snd_request                                       */
/*  CALLING SEQ.    :bool cncl_snd_request_post(thread_object_t *thread_snd,*/
/*                   io_trace_buf_t *snd_request,                           */
/*                   size_t length)                                         */
/*  ARGUMENT        :thread_snd :送信用スレッド                             */
/*                  :snd_request:送信データを格納したバッファ               */
/*                  :length    :送信データ長                                */
/*  RETURN CODE     :true,false(送信キューへの追加可否)                     */
/*  DESCRIPTION     :送信用リクエストをキューに登録する                     */
/****************************************************************************/
bool cncl_snd_request_post(thread_object_t *thread_snd, io_trace_buf_t *snd_request, size_t length)
{
    cmp_snd_t *snd = (cmp_snd_t *)cncl_get_component(thread_snd);
    enqueue_io_mem(&(snd->snd_req_q), snd_request, length);
    if (snd->snd_status == e_cncl_snd_idle) {
        Event_tag_t *send_post = cncl_get_scheduled_tag(thread_snd, -1, cncl_snd_send_post, 0, NULL);
        cncl_post_event(send_post);
    }
    return true;  // voidで良かった？
}
/*****************************************************************************/
/*  FUNCTION        :cncl_snd_send_post                                      */
/*  CALLING SEQ.    :short cncl_snd_send_post(                               */
/*                   Event_tag_t *event)                                     */
/*  ARGUMENT        :event:送信要求イベントタグ                              */
/*  RETURN CODE     :0                                                       */
/*  DESCRIPTION     :送信要求イベントを受け取り、対応するスレッドの送信処理を*/
/*                   実行する。                                              */
/*                   ・event->threadを指定してcncl_snd_sendを呼び出す        */
/*                   ・常に0を返す                                           */
/*****************************************************************************/
short cncl_snd_send_post(Event_tag_t *event)
{
    cncl_snd_send(event->thread);
    return 0;
}
/****************************************************************************/
/*  FUNCTION        :cncl_snd_send                                          */
/*  CALLING SEQ.    :snd_status_t cncl_snd_send(thread_object_t *thread_snd */
/*  ARGUMENT        :thread_snd:送信用スレッド                              */
/*  RETURN CODE     :e_cncl_snd_inpg,e_cncl_snd_idle,e_cncl_snd_fault等     */
/*  DESCRIPTION     :キューからデータを取り出して送信処理を行う             */
/****************************************************************************/
snd_status_t cncl_snd_send(thread_object_t *thread_snd)
{
    //    cmp_trans_t * trans = (cmp_trans_t *)cncl_get_component(thread_snd);
    cmp_snd_t       *cmp_snd = (cmp_snd_t *)cncl_get_component(thread_snd);
    thread_header_t *hd  = cncl_get_threadInfo(thread_snd);
    int              ret;
    io_trace_buf_t  *send_req;
    size_t           send_req_len;
    c202_def        *c202_req;
    int              send_len;
    char            *send_data;
    short            s_err = 0;

    if (cmp_snd->snd_status != e_cncl_snd_idle) return cmp_snd->snd_status;
    send_req = dequeue_io_mem(&(cmp_snd->snd_req_q), &send_req_len);
    if (!send_req) {
        return e_cncl_snd_idle;
    }
    c202_req  = (c202_def *)send_req->data_info.rec_area;
    send_len  = c202_req->msg_info.msg_len;
    send_data = c202_req->msg_info.msg_data;

    Event_tag_t *event_socket;
    cmp_snd->snd_len                    = send_len;
    cmp_snd->sold_len                   = 0;
    hd->io_complete_buffer          = send_req;
    event_socket                    = cncl_get_socket_tag(thread_snd, cmp_snd->socket, cncl_snd_send_complete);
    if (!event_socket) {
        cncl_ems_procedure_error("cncl_get_socket_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    Event_tag_t *event_send_timeout = cncl_get_timer_tag(thread_snd, cmp_snd->transport_config->send_complete_timer,
                                                         cncl_snd_send_timeout, DEF_SEND_TIMER);
    if (!event_send_timeout) {
        cmp_snd->snd_status                    = e_cncl_snd_fault;
        event_send_timeout                 = cncl_get_scheduled_tag(thread_snd, -1, cmp_snd->trans_notice, 0, NULL);
        cncl_post_event(event_send_timeout);
        return e_cncl_snd_fault;
    }
    if (EXTRACEMODE) {
        trace_start(send_req, DEF_TRACE_FILE_ID_PROC, cmp_snd->transport_config->tcpip_prc_name, DEF_TRACE_IO_TYPE_WR, 0);
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    ret = send_nw2(cmp_snd->socket, send_data, send_len, 0, (long)event_socket);
#pragma clang diagnostic pop
    if (ret != 0) {
        // このエラーは完了通知側で処理する。
        // cncl_remove_event(event_socket);
        FILE_GETINFO_(cmp_snd->socket, &s_err, no_param, no_param, no_param, no_param, no_param);
        event_socket->io_info.fs_err = s_err;
        event_socket->io_info.data   = send_req;
        event_socket->io_info.len    = 0;
        event_socket->thread         = thread_snd;
        event_socket->func           = cmp_snd->trans_notice;
        cncl_post_event(event_socket);
        if (EXTRACEMODE) {
            trace_end(send_req, DEF_TRACE_IO_TYPE_WR, 0, s_err);
        }
    }
    cmp_snd->snd_status = e_cncl_snd_inpg;
    //    if (EXTRACEMODE) {
    //        ;
    //    }

    return e_cncl_snd_inpg;
}
/****************************************************************************/
/*  FUNCTION        :cncl_snd_send_complete                                 */
/*  CALLING SEQ.    :short cncl_snd_send_complete(Event_tag_t *event)       */
/*  ARGUMENT        :event:ソケット送信完了のイベント情報                   */
/*  RETURN CODE     :0,エラーコード                                         */
/*  DESCRIPTION     :送信完了時または残データ送信時の処理を行う             */
/*  MODIFIED        :2025/05/08 SS内結合テスト不具合 No.91                  */
/****************************************************************************/
short cncl_snd_send_complete(Event_tag_t *event)
{
    thread_object_t *thread_snd = event->thread;
    cmp_snd_t       *cmp_snd    = (cmp_snd_t *)cncl_get_component(thread_snd);
    thread_header_t *hd         = cncl_get_threadInfo(thread_snd);
    int              ret;
    c201_def        *c201_req;
    char            *send_data;
    short            s_err;
    io_trace_buf_t   trace;

    cncl_cancel_timer_tag(thread_snd);
    event->io_info.len = (unsigned short)socket_get_len(cmp_snd->socket);
    if (EXTRACEMODE) {
        memset(&trace, 0,sizeof(io_trace_buf_t));
        trace.trace_info = hd->io_complete_buffer->trace_info;
        c201_req  = (c201_def *)hd->io_complete_buffer->data_info.rec_area;
        send_data = c201_req->msg_info.msg_data;
        memcpy(&(trace.data_info.rec_area) ,send_data + cmp_snd->sold_len, event->io_info.len);
        trace_end(&trace, DEF_TRACE_IO_TYPE_WR, event->io_info.len, event->io_info.fs_err);
    }
    if (event->io_info.fs_err == 0) {
        if (event->io_info.len + cmp_snd->sold_len < cmp_snd->snd_len) {
            // 残データ有り 再送
            Event_tag_t *event_socket = cncl_get_socket_tag(thread_snd, cmp_snd->socket, cncl_snd_send_complete);
            if (!event_socket) {
                cncl_ems_procedure_error("cncl_get_socket_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
                cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
            }
            cmp_snd->sold_len += event->io_info.len;

            Event_tag_t *event_send_timeout = cncl_get_timer_tag(
                thread_snd, cmp_snd->transport_config->send_complete_timer, cncl_snd_send_timeout, DEF_SEND_TIMER);
            if (!event_send_timeout) {
                cmp_snd->snd_status = e_cncl_snd_fault;
                event_send_timeout  = cncl_get_scheduled_tag(thread_snd, -1, cmp_snd->trans_notice, 0, NULL);
                cncl_post_event(event_send_timeout);
                return e_cncl_snd_fault;
            }
            if (EXTRACEMODE) {
                trace_start(hd->io_complete_buffer, DEF_TRACE_FILE_ID_PROC, cmp_snd->transport_config->tcpip_prc_name,
                            DEF_TRACE_IO_TYPE_WR, 0);
            }
            c201_req  = (c201_def *)hd->io_complete_buffer->data_info.rec_area;
            send_data = c201_req->msg_info.msg_data;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
            ret = send_nw2(cmp_snd->socket, send_data + cmp_snd->sold_len,
                           (int)(cmp_snd->snd_len > cmp_snd->sold_len ? cmp_snd->snd_len - cmp_snd->sold_len : 0), 0,
                           (long)event_socket);
#pragma clang diagnostic pop
            if (ret != 0) {
                FILE_GETINFO_(cmp_snd->socket, &s_err, no_param, no_param, no_param, no_param, no_param);
                event_socket->io_info        = event->io_info;
                event_socket->io_info.fs_err = s_err;
                event_socket->thread         = thread_snd;
                event_socket->func           = cmp_snd->trans_notice;
                cncl_post_event(event_socket);
                if (EXTRACEMODE) {
                    trace_end(hd->io_complete_buffer, DEF_TRACE_IO_TYPE_WR, 0, s_err);
                }
                return 0;
            }
        } else {
            // 送信完了
            cmp_snd->snd_status = e_cncl_snd_idle;
            free_io_mem(&io_mem, hd->io_complete_buffer);
            hd->io_complete_buffer = NULL;
            cncl_snd_send(thread_snd);
        }
        return 0;
    }
    // 送信エラー
    cmp_snd->snd_status       = e_cncl_snd_fault;
    // ここでTransThreadにイベントを渡すのは違反なので、sndthreadをtransハンドラに渡す。
    Event_tag_t *event_socket = cncl_get_socket_tag(thread_snd, cmp_snd->socket, cmp_snd->trans_notice);
    event_socket->io_info     = event->io_info;
    cncl_post_event(event_socket);
    return 0;
}
/****************************************************************************/
/*  FUNCTION        :cncl_snd_cancel                                        */
/*  CALLING SEQ.    :void cncl_snd_cancel(thread_object_t *thread_snd)      */
/*  ARGUMENT        :thread_snd:送信用スレッド                              */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :送信待ちキューをクリアし、登録済みデータを解放         */
/*  MODIFIED        :2025/05/08 SS内結合テスト不具合 No.91                  */
/****************************************************************************/
void cncl_snd_cancel(thread_object_t *thread_snd)
{
    if (!thread_snd) return;
    cmp_snd_t       *snd = (cmp_snd_t *)cncl_get_component(thread_snd);
    thread_header_t *hd  = cncl_get_threadInfo(thread_snd);
    io_trace_buf_t  *tmp;
    size_t           data_length;
    tmp = dequeue_io_mem(&(snd->snd_req_q), &data_length);
    while (tmp) {
        free_io_mem(&io_mem, tmp);
        tmp = dequeue_io_mem(&(snd->snd_req_q), &data_length);
    }
    free_io_mem(&io_mem, hd->io_complete_buffer);
    hd->io_complete_buffer = NULL;
}
/****************************************************************************/
/*  FUNCTION        :cncl_delete_snd                                        */
/*  CALLING SEQ.    :void cncl_delete_snd(thread_object_t *thread_snd)      */
/*  ARGUMENT        :thread_snd:削除する送信用スレッド                      */
/*  RETURN CODE     :なし                                                   */
/*  DESCRIPTION     :送信用スレッドを破棄しバッファを解放する               */
/*  MODIFIED        :2025/05/08 SS内結合テスト不具合 No.91                  */
/****************************************************************************/
void cncl_delete_snd(thread_object_t *thread_snd)
{
    size_t          data_length;
    io_trace_buf_t *io_trace_buf;
    if (!thread_snd) return;
    cmp_snd_t       *cmp_snd = cncl_get_component(thread_snd);
    thread_header_t *hd      = cncl_get_threadInfo(thread_snd);
    do {
        io_trace_buf = dequeue_io_mem(&(cmp_snd->snd_req_q), &data_length);
        free_io_mem(&io_mem, io_trace_buf);
    } while (io_trace_buf != NULL);
    free_io_mem(&io_mem, hd->io_complete_buffer);
    cncl_delete_thread(thread_snd);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_snd_send_timeout                                   */
/*  CALLING SEQ.    :short cncl_snd_send_timeout(Event_tag_t *event)         */
/*  ARGUMENT        :event:受信タイムアウトイベント                          */
/*  RETURN CODE     :0                                                       */
/*  DESCRIPTION     :送信タイムアウト発生時にソケットをキャンセルし回線終了  */
/*                  :処理を行う                                              */
/*****************************************************************************/
short cncl_snd_send_timeout(Event_tag_t *event)
{
    cmp_snd_t       *cmp_snd       = cncl_get_component(event->thread);
    cmp_trans_t     *cmp_trans     = cncl_get_component(cmp_snd->p_trans);
    thread_header_t *snd_thread_hd = cncl_get_threadInfo(event->thread);
    thread_object_t *thread_trans  = cmp_snd->p_trans;
    Event_tag_t     *close_event   = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close, event->io_info.fs_err, NULL);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    CANCELREQ(cmp_trans->socket, (long)snd_thread_hd->io_ev_tag);
#pragma clang diagnostic pop
    if (EXTRACEMODE) {
        trace_end(snd_thread_hd->io_complete_buffer, DEF_TRACE_IO_TYPE_WR, 0, event->io_info.fs_err);
    }
    cncl_ems_nw_error(transport_gflin_key(cmp_trans), cmp_trans->config.tcpip_prc_name, &(cmp_trans->config.remote_host.sin_addr),
                      cmp_trans->config.remote_host.sin_port, &(cmp_trans->config.local_if.sin_addr),
                      cmp_trans->config.local_if.sin_port, event->io_info.fs_err, DEF_NERR_SEND_ERR);
    cncl_remove_event(snd_thread_hd->io_ev_tag);
    cmp_snd->snd_status = e_cncl_snd_fault;
    cncl_post_event(close_event);
    return 0;
}
