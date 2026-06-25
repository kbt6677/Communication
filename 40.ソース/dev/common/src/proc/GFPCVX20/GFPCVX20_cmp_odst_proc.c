#/******************************************************************************
*                                                                             *
*                               ＜GFP通信制御＞                               *
*                                                                             *
*                               ＜コネクション制御(クライアント)＞            *
*                                                                             *
*        VERSION                               :＜1.0.0＞                     *
*                                                                             *
*        CREATE DATE                           :＜作成日 2025/09/16＞         *
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
/*        AUTHER            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/09/16                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/09/16 新規作成                                      */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdio.h> nolist
#include <string.h> nolist

#ifdef _TANDEM_SOURCE
#include <tal.h> nolist
#include <cextdecs.h(FILE_OPEN_,WRITEREADX,FILE_CLOSE_,FILE_GETINFO_,CANCEL)> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif

/* USER HEADER     */
#include <GFPCGX50.h> nolist
#include <common.h> nolist
#include <errcd.h> nolist
#include <limit.h> nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_cmp_odst_proc.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_sys.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_util.h" nolist
#include "ipc.h" nolist

/*****************************************************************************/
/*  FUNCTION        :cncl_create_odst_proc                                   */
/*  CALLING SEQ.    :thread_object_t *cncl_create_odst_proc(                 */
/*                   thread_object_t *parent_ntf_odst_procs,                 */
/*                   filename_p_t out_dist_procs_name,                       */
/*                   ntf_odst_complete_func complete_callback)               */
/*  ARGUMENT        :parent_ntf_odst_procs: 通知管理(親)スレッド             */
/*                   out_dist_procs_name : 宛先プロセスのプロセス名          */
/*                   complete_callback   : 完了通知コールバック              */
/*  RETURN CODE     :作成したODSTプロセス用スレッドオブジェクトのポインタ    */
/*  DESCRIPTION     :ODSTプロセス通信用のスレッドを生成・初期化する。        */
/*                   ・コンポーネント領域のゼロクリア                        */
/*                   ・プロセス名(QUALIFIER付与)の組立て                     */
/*                   ・OPEN/WRITEの各種リトライ/タイマ値を設定               */
/*                   ・送信用バッファをスレッドに関連付け                    */
/*****************************************************************************/
thread_object_t *cncl_create_odst_proc(thread_object_t *parent_ntf_odst_procs, filename_p_t out_dist_procs_name,
                                  ntf_odst_complete_func complete_callback)
{
    thread_object_t *odst_proc_thread = cncl_create_thread(out_dist_proc);
    cmp_odst_proc_t *cmp_odst_proc    = cncl_get_component(odst_proc_thread);
    thread_header_t *hd               = cncl_get_threadInfo(odst_proc_thread);
    myinfo_def      *myInfo           = &cncl_get_App()->my_info;

    memset(cmp_odst_proc, 0, sizeof(cmp_odst_proc_t));
    cmp_odst_proc->parent_ntf_odst_procs = parent_ntf_odst_procs;
    cmp_odst_proc->out_dist_procs_f_num  = DEF_ODST_CLOSED;
    snprintf(cmp_odst_proc->out_dist_procs_name, sizeof(filename_p_t), "%s.%s", out_dist_procs_name,
             QUALIFIER_FOR_PSNMSDSO);
    cmp_odst_proc->odst_proc_status             = e_odst_proc_status_initial;
    cmp_odst_proc->complete_callback            = complete_callback;
    cmp_odst_proc->send_status                  = e_odst_proc_send_none;

    cmp_odst_proc->open_param.retry_max         = 6;    // TODO:パラメータをとる？ 6回リトライする。
    cmp_odst_proc->open_param.retry_interval    = 200;  // TODO:パラメータをとる？
    cmp_odst_proc->open_param.nowait_open_timer = myInfo->nowait_open_timer;

    cmp_odst_proc->write_param.retry_max        = 2;    // これは現行のCOM_PSDに準拠
    cmp_odst_proc->write_param.process_io_timer = myInfo->process_io_timer;

    hd->io_complete_buffer                      = &cmp_odst_proc->send_buffer.val;
    return odst_proc_thread;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_add_req                                  */
/*  CALLING SEQ.    :short cncl_odst_proc_add_req(                           */
/*                   thread_object_t *thread_odst_proc,                      */
/*                   io_trace_buf_t *request,                                */
/*                   thread_object_t *caller_transport)                      */
/*  ARGUMENT        :thread_odst_proc : ODSTプロセス用スレッド               */
/*                   request         : 送信要求(C107)のトレース付きバッファ  */
/*                   caller_transport: 起票元トランスポートスレッド          */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :送信要求をキューへ積み、状態に応じて送信/OPENを行う。   */
/*                   ・extra_infoに起票元スレッドを保持(集計用)              */
/*                   ・Onlineなら即送信                                      */
/*                   ・Initial/OfflineならOPEN処理を開始                     */
/*****************************************************************************/
short cncl_odst_proc_add_req(thread_object_t *thread_odst_proc, io_trace_buf_t *request)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(thread_odst_proc);
    // どの接続が通知を送信しているのか集計のため呼出しもとtransportのスレッドを渡す。
    // 送信待ちキューに格納
    enqueue_io_mem(&cmp_odst_proc->send_queue, request, sizeof(c107_def));
    // ステータスがOnlineならば送信する。
    if (cmp_odst_proc->odst_proc_status == e_odst_proc_status_online) {
        return cncl_odst_proc_request_c107_send(thread_odst_proc);
    }
    // 切断中であればOpenする。
    if (cmp_odst_proc->odst_proc_status == e_odst_proc_status_initial
        || cmp_odst_proc->odst_proc_status == e_odst_proc_status_offline) {
        return cncl_odst_proc_open(thread_odst_proc);
    }
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_open                                      */
/*  CALLING SEQ.    :short cncl_odst_proc_open(thread_object_t *thread_odst_proc)*/
/*  ARGUMENT        :thread_odst_proc: ODSTプロセス用スレッド                */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :ODSTプロセスをFILE_OPEN_し、非同期完了を待つ。          */
/*                   ・失敗時(未起動/停止中等)はタイムアウトでリトライ       */
/*                   ・致命的エラーはEMS出力後に異常終了                     */
/*                   ・nowait open タイマをセットし完了を待機                */
/*****************************************************************************/
short cncl_odst_proc_open(thread_object_t *thread_odst_proc)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(thread_odst_proc);
    short            s_err;
    io_trace_buf_t  *trace = &(cmp_odst_proc->send_buffer.val);
    char             proc_name[ZSYS_VAL_LEN_FILENAME + 1];
    Event_tag_t     *file_open_tm;
    cmp_trans_t     *cmp_trans;

    cmp_odst_proc->odst_proc_status = e_odst_proc_status_activating;
    if (EXTRACEMODE) {
        if (cmp_odst_proc->send_status == e_odst_proc_send_none) {
            memset(&trace->trace_info, ' ', sizeof(trace->trace_info));
        }
        memset(proc_name, 0, sizeof(proc_name));
        memcpy(proc_name, cmp_odst_proc->out_dist_procs_name,
               strnlen_isys(cmp_odst_proc->out_dist_procs_name, sizeof(cmp_odst_proc->out_dist_procs_name)) - sizeof(QUALIFIER_FOR_PSNMSDSO));
        trace_start(trace, DEF_TRACE_FILE_ID_PROC, proc_name, DEF_TRACE_IO_TYPE_OP, 0);
    }
    s_err = FILE_OPEN_(cmp_odst_proc->out_dist_procs_name,
                       (short)strnlen_isys(cmp_odst_proc->out_dist_procs_name, sizeof(filename_p_t)),
                       &(cmp_odst_proc->out_dist_procs_f_num), ZSYS_VAL_OPENACC_READWRITE, ZSYS_VAL_OPENEXCL_SHARED, 1,
                       1, 0x4000, no_param, no_param, no_param, no_param);
    switch (s_err) {
        case 0:
            break;
        case ZFIL_ERR_NOSUCHDEV:
        case ZFIL_ERR_WRONGID:
        case ZFIL_ERR_DEVDOWN:
            /*プロセスが起動されていない、再起動を待つためタイムアウトで再実施する*/
            cmp_trans = cncl_get_component(cmp_odst_proc->send_queue.top->extra_info.owner_thread);
            cmp_odst_proc->out_dist_procs_f_num = DEF_ODST_CLOSED;
            cncl_ems_procio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin), DEF_PRC_FURI_O, "FILE_OPEN_",
                                  (short)s_err, DEF_NERR_PROC_OPN_ERR);
            break;
        default:
            /*エラーメッセージ(※呼び出しパラメータ不良のためABEND相当)*/
            cmp_trans = cncl_get_component(cmp_odst_proc->send_queue.top->extra_info.owner_thread);
            cncl_ems_procio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin), DEF_PRC_FURI_O, "FILE_OPEN_",
                                  (short)s_err, DEF_NERR_PROC_OPN_ERR);
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PROC_OPN_ERR);
    }
    file_open_tm = cncl_get_timer_tag(thread_odst_proc, cmp_odst_proc->open_param.nowait_open_timer,
                                      cncl_odst_proc_open_timeout, !s_err ? 40 : s_err);
    if (!file_open_tm) {
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_TIMER_ERR);
    }
    if (EXTRACEMODE) {
        trace_end(trace, DEF_TRACE_IO_TYPE_OP, 0, s_err);
    }
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_open_complete                             */
/*  CALLING SEQ.    :void cncl_odst_proc_open_complete(                       */
/*                   thread_object_t *thread_odst_proc, short fs_err)         */
/*  ARGUMENT        :thread_odst_proc: ODSTプロセス用スレッド                 */
/*                   fs_err         : ファイルシステム戻り(補足トレース用)    */
/*  RETURN CODE     :なし                                                     */
/*  DESCRIPTION     :OPEN完了処理を行う。                                     */
/*                   ・タイマ解除、状態Online化、リトライ回数リセット        */
/*                   ・保留要求の送信を再開                                   */
/*****************************************************************************/
void cncl_odst_proc_open_complete(thread_object_t *thread_odst_proc, short fs_err)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(thread_odst_proc);
    io_trace_buf_t  *trace         = &(cmp_odst_proc->send_buffer.val);

    cncl_cancel_timer_tag(thread_odst_proc);
    if (EXTRACEMODE) {
        trace_end(trace, DEF_TRACE_IO_TYPE_OPCMP, 0, fs_err);
    }
    if (fs_err) {
        // ステータスはactivatingのまま
        cmp_trans_t *cmp_trans = cncl_get_component(cmp_odst_proc->send_queue.top->extra_info.owner_thread);
        cncl_ems_procio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin), DEF_PRC_FURI_O, "FILE_OPEN_",
                              (short)fs_err, DEF_NERR_PROC_OPN_ERR);
        cmp_odst_proc->out_dist_procs_f_num = DEF_ODST_CLOSED;
        Event_tag_t *file_open = cncl_get_timer_tag(thread_odst_proc, cmp_odst_proc->open_param.nowait_open_timer,
                                                    cncl_odst_proc_open_timeout, fs_err);
        if (!file_open) {
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_TIMER_ERR);
        }
        return;
    }
    cmp_odst_proc->odst_proc_status       = e_odst_proc_status_online;
    cmp_odst_proc->open_param.retry_count = 0;
    cncl_odst_proc_request_c107_send(thread_odst_proc);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_open_timeout                              */
/*  CALLING SEQ.    :short cncl_odst_proc_open_timeout(Event_tag_t *event)    */
/*  ARGUMENT        :event: タイムアウトイベントタグ                           */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :nowait OPENのタイムアウト処理。リトライ/断念を判定。     */
/*                   ・一旦クローズして再試行(規定回数まで)                   */
/*                   ・リトライ超過時はOffline化し保留要求をすべて破棄通知    */
/*                   ・当該接続情報を添付してEMS出力                           */
/*****************************************************************************/
short cncl_odst_proc_open_timeout(Event_tag_t *event)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(event->thread);
    iobuf_page_t    *abadone;
    size_t           len;
    filename_p_t     work;

    memset(work, 0, sizeof(filename_p_t));

    FILE_CLOSE_(cmp_odst_proc->out_dist_procs_f_num, no_param);
    cmp_odst_proc->out_dist_procs_f_num = DEF_FILENO_CLOSED;

    if (++(cmp_odst_proc->open_param.retry_count) < cmp_odst_proc->open_param.retry_max) {
        // リトライする。
        switch (event->signal_timeout_info.sparam) {
            case ZFIL_ERR_NOSUCHDEV:
            case ZFIL_ERR_WRONGID:
            case ZFIL_ERR_DEVDOWN:
            case ZFIL_ERR_TIMEDOUT:;
                Event_tag_t *retry_timer_tag = cncl_get_timer_tag(
                    event->thread, cmp_odst_proc->open_param.retry_interval, cncl_odst_proc_open_retry_timeout, 40);
                if (!retry_timer_tag) {
                    // SIGNALTIMEOUTが設定できない場合システム異常が考えられるのでABENDする。
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
                }
                return 0;
        }
    }
    // リトライオーバーorリトライ無し
    // 当該odstは再通知要求が発生するまで捨てる。
    cmp_odst_proc->odst_proc_status = e_odst_proc_status_offline;
    // Open要求が行われたということは、少なくともどちらか一方にc107要求が入っている。
    if (cmp_odst_proc->send_status != e_odst_proc_send_none)
        abadone = cmp_odst_proc->current_request;
    else
        abadone = (iobuf_page_t *)dequeue_io_mem(&(cmp_odst_proc->send_queue), &len);

    // 送信バッファを未使用に
    cmp_odst_proc->send_status     = e_odst_proc_send_none;
    cmp_odst_proc->current_request = NULL;

    // 当該回線のIDを添付してEMS出力
    cmp_trans_t *cmp_trans = cncl_get_component((thread_object_t *)((iobuf_page_t *)abadone)->extra_info.owner_thread);
    memcpy(work, cmp_odst_proc->out_dist_procs_name,
           strnlen_isys(cmp_odst_proc->out_dist_procs_name, sizeof(filename_p_t)) - sizeof(QUALIFIER_FOR_PSNMSDSO));
    cncl_ems_procio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin), work, DEF_TRACE_IO_TYPE_OP,
                          event->signal_timeout_info.sparam, DEF_NERR_PROC_OPN_ERR);
    // バックログに全て終了通知
    while (abadone) {
        // 通知要求の解放はcmp_ntf_odst_procsの方で解除する。
        cmp_odst_proc->complete_callback(cmp_odst_proc->parent_ntf_odst_procs, abadone);
        abadone = (iobuf_page_t *)dequeue_io_mem(&(cmp_odst_proc->send_queue), &len);
    }
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_open_retry_timeout                        */
/*  CALLING SEQ.    :short cncl_odst_proc_open_retry_timeout(                 */
/*                   Event_tag_t *event)                                      */
/*  ARGUMENT        :event: タイマイベントタグ                                */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :OPEN再試行のタイマ満了処理。即座にOPEN処理を再実行。     */
/*****************************************************************************/
short cncl_odst_proc_open_retry_timeout(Event_tag_t *event)
{
    return cncl_odst_proc_open(event->thread);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_request_c107_send                         */
/*  CALLING SEQ.    :short cncl_odst_proc_request_c107_send(                  */
/*                   thread_object_t *thread_odst_proc)                       */
/*  ARGUMENT        :thread_odst_proc: ODSTプロセス用スレッド                 */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :C107通知をODSTプロセスに送信(WRITEREADX)する。           */
/*                   ・送信中でなければキューから1件取り出し送信              */
/*                   ・IOタグを取得し非同期I/Oを発行                           */
/*                   ・システムエラー時はEMS出力の上で異常終了                */
/*****************************************************************************/
short cncl_odst_proc_request_c107_send(thread_object_t *thread_odst_proc)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(thread_odst_proc);
    io_trace_buf_t  *trace         = &(cmp_odst_proc->send_buffer.val);
    iobuf_page_t    *send_request;
    char             proc_name[ZSYS_VAL_LEN_FILENAME + 1];
    size_t           send_req_len;
    Event_tag_t     *event_odst_proc_io;
    _cc_status       cc;
    short            wr_err;

    // 現在送信中でなければキューから要求を取出す。送信中ならば抜ける。リトライ中ならそのまま再送する。
    switch (cmp_odst_proc->send_status) {
        case e_odst_proc_send_none:
            send_request = (iobuf_page_t *)dequeue_io_mem(&(cmp_odst_proc->send_queue), &send_req_len);
            if (!send_request) {
                return 0;
            } else {
                cmp_odst_proc->current_request = send_request;
                memcpy(&cmp_odst_proc->send_buffer, send_request, sizeof(iobuf_page_t));
            }
            break;
        case e_odst_proc_send_sending:
            return 0;
        case e_odst_proc_send_retry:
            // エラー応答でリトライしている場合は電文エリアが書き換えられているので書き戻す。
            memcpy(&cmp_odst_proc->send_buffer.val.data_info, &cmp_odst_proc->current_request->val.data_info,
                   sizeof(cmp_odst_proc->current_request->val.data_info));
            break;
    }

    // トレース処理
    if (EXTRACEMODE) {
        if (cmp_odst_proc->send_status == e_odst_proc_send_none) {
            memset(&trace->trace_info, ' ', sizeof(trace->trace_info));
        }
        memset(proc_name, 0, sizeof(proc_name));
        memcpy(proc_name, cmp_odst_proc->out_dist_procs_name,
               strnlen_isys(cmp_odst_proc->out_dist_procs_name, sizeof(cmp_odst_proc->out_dist_procs_name)) - sizeof(QUALIFIER_FOR_PSNMSDSO));
        trace_start(trace, DEF_TRACE_FILE_ID_PROC, proc_name, DEF_TRACE_IO_TYPE_WR, sizeof(c107_def));
    }
    cmp_odst_proc->send_status = e_odst_proc_send_sending;
    event_odst_proc_io         = cncl_get_io_tag(thread_odst_proc, cmp_odst_proc->out_dist_procs_f_num,
                                                 cncl_odst_proc_request_c107_send_complete);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    cc = WRITEREADX(cmp_odst_proc->out_dist_procs_f_num, cmp_odst_proc->send_buffer.val.data_info.rec_area,
                    sizeof(c107_def), MAX_TEXT_BUF_LEN, no_param, (long)event_odst_proc_io);
#pragma clang diagnostic pop
    if (_status_ne(cc)) {
        cmp_trans_t *cmp_trans = cncl_get_component(send_request->extra_info.owner_thread);
        FILE_GETINFO_(cmp_odst_proc->out_dist_procs_f_num, &wr_err, no_param, no_param, no_param, no_param, no_param);
        cncl_ems_procio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin), DEF_PRC_FURI_O, "WRITEREADX", wr_err,
                              DEF_NERR_SYSIF_LGC_ERR);
        // TODO:サーバはここでABENDしているけどどうしよう？
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_SYSIF_LGC_ERR);
    }
    if (EXTRACEMODE) {
        trace_end(trace, DEF_TRACE_FILE_ID_PROC, 0, cc);
    }
    Event_tag_t *file_write_tm = cncl_get_timer_tag(thread_odst_proc, cmp_odst_proc->write_param.process_io_timer,
                                                    cncl_odst_proc_request_c107_send_timeout, 0);
    if (!file_write_tm) {
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_TIMER_ERR);
    }
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_request_c107_send_complete                */
/*  CALLING SEQ.    :short cncl_odst_proc_request_c107_send_complete(         */
/*                   Event_tag_t *event)                                      */
/*  ARGUMENT        :event: I/O完了イベントタグ                                */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :C107送信後の応答受信完了処理。                           */
/*                   ・成功かつ応答のerror_code==0で次処理へ                  */
/*                   ・完了コールバックを起票元へ通知                         */
/*                   ・次の要求があれば続けて送信                             */
/*                   ・失敗時は送信タイムアウト処理へ                          */
/*****************************************************************************/
short cncl_odst_proc_request_c107_send_complete(Event_tag_t *event)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(event->thread);
    r107_def        *r107_resp;
    if (EXTRACEMODE) {
        trace_end(&cmp_odst_proc->send_buffer.val, DEF_TRACE_IO_TYPE_RD, event->io_info.len, event->io_info.fs_err);
    }
    // 送信タイマー停止
    cncl_cancel_timer_tag(event->thread);
    // 処理結果判定
    do {
        if (event->io_info.fs_err == 0) {
            r107_resp = (r107_def *)cmp_odst_proc->send_buffer.val.data_info.rec_area;
            if (r107_resp->common_header.error_code != 0) {
                break;
            }
            cmp_odst_proc->complete_callback(cmp_odst_proc->parent_ntf_odst_procs, cmp_odst_proc->current_request);
            cmp_odst_proc->send_status             = e_odst_proc_send_none;  // バッファは送信済
            cmp_odst_proc->write_param.retry_count = 0;
            cmp_odst_proc->current_request         = NULL;
            // 次要求の送信(非同期I/OなのでPostする必要はない)
            return cncl_odst_proc_request_c107_send(event->thread);
        }
    } while (0);
    // リトライ
    return cncl_odst_proc_request_c107_send_timeout(event);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_odst_proc_send_timeout                              */
/*  CALLING SEQ.    :short cncl_odst_proc_send_timeout(Event_tag_t *event)    */
/*  ARGUMENT        :event: 送信失敗を受けたイベントタグ                      */
/*  RETURN CODE     :0／エラーコード                                         */
/*  DESCRIPTION     :送信(又は応答)失敗時のリカバリ。                         */
/*                   ・ファイルを閉じ、規定回数までOPENからやり直し           */
/*                   ・リトライ超過時はOffline化し、保留要求を破棄通知        */
/*                   ・EMSへエラー出力                                        */
/*****************************************************************************/
short cncl_odst_proc_request_c107_send_timeout(Event_tag_t *event)
{
    cmp_odst_proc_t *cmp_odst_proc = cncl_get_component(event->thread);
    iobuf_page_t    *abadone;
    size_t           len;
    r107_def        *r107_resp;

    CANCEL(cmp_odst_proc->out_dist_procs_f_num);
    FILE_CLOSE_(cmp_odst_proc->out_dist_procs_f_num, no_param);
    cmp_odst_proc->out_dist_procs_f_num = DEF_FILENO_CLOSED;
    if (++(cmp_odst_proc->write_param.retry_count) < cmp_odst_proc->write_param.retry_max) {
        cmp_odst_proc->odst_proc_status = e_odst_proc_status_activating;
        cmp_odst_proc->send_status      = e_odst_proc_send_retry;
        Event_tag_t *file_open_tm       = cncl_get_timer_tag(event->thread, cmp_odst_proc->open_param.nowait_open_timer,
                                                             cncl_odst_proc_open_retry_timeout, event->io_info.fs_err);
        if (!file_open_tm) {
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_TIMER_ERR);
        }
    } else {
        cmp_odst_proc->odst_proc_status = e_odst_proc_status_offline;
        cmp_odst_proc->send_status      = e_odst_proc_send_none;
        abadone                         = cmp_odst_proc->current_request;
        cmp_odst_proc->current_request  = NULL;
        short        disp_err;
        cmp_trans_t *cmp_trans = cncl_get_component(abadone->extra_info.owner_thread);
        r107_resp              = (r107_def *)cmp_odst_proc->send_buffer.val.data_info.rec_area;
        if (event->io_info.fs_err != 0) {
            disp_err = event->io_info.fs_err;
        } else if (r107_resp->common_header.error_code != 0) {
            disp_err = r107_resp->common_header.error_code;
        } else {
            disp_err = 40;
        }
        cncl_ems_procio_error(no_param, (gflin_pkey_def *)&(cmp_trans->gflin), DEF_PRC_FURI_O, "WRITEREADX", disp_err,
                              DEF_NERR_FILE_IO_ERR);
        while (abadone) {
            // 通知要求の解放はcmp_ntf_odst_procsの方で解除する。
            cmp_odst_proc->complete_callback(cmp_odst_proc->parent_ntf_odst_procs, abadone);
            abadone = (iobuf_page_t *)dequeue_io_mem(&(cmp_odst_proc->send_queue), &len);
        }
    }
    return 0;
}
