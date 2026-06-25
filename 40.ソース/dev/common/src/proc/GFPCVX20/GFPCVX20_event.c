/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                                *
 *                                                                             *
 *                       ＜コネクション制御(クライアント)＞                    *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/01/08＞         *
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
/*                               スレッド制御/アプリメイン                   */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/01/08                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/01/08 新規作成                                     */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <assert.h> nolist
#include <stdint.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#ifdef _TANDEM_SOURCE
#include <tal.h> nolist

#include <cextdecs.h(AWAITIOX, FILE_GETINFO_, PROCESS_DELAY_, TS_UNIQUE_CREATE_)> nolist
#include <cextdecs.h(FILE_GETRECEIVEINFO_, PROCESSHANDLE_DECOMPOSE_)> nolist
#include <cextdecs.h(JULIANTIMESTAMP, SIGNALTIMEOUT, CANCELTIMEOUT)> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif
/* USER HEADER     */
#include <errcd.h> nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_cmp_odst_proc.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_thread_factory.h" nolist
#include "GFPCVX20_util.h" nolist

/****************************************************************************/
/*   Event list integrity helpers                                           */
/****************************************************************************/
/*****************************************************************************/
/*  FUNCTION        :__cncl_list_recount_and_repair                           */
/*  CALLING SEQ.    :static int __cncl_list_recount_and_repair(               */
/*                   Event_list_t *event_list)                                */
/*  ARGUMENT        :event_list:イベントリスト構造体                          */
/*  RETURN CODE     :リスト内のイベント件数                                   */
/*  DESCRIPTION     :イベントリストの整合性を再構築し、件数を再集計する。      */
/*                   ・全ノードを走査し、prevポインタを再設定                 */
/*                   ・リスト末尾ノードをtailに再設定                         */
/*                   ・正しいイベント件数をevent_list_countに格納              */
/*                   ・件数を戻り値として返却                                 */
/*****************************************************************************/
static int __cncl_list_recount_and_repair(Event_list_t *event_list)
{
    size_t       cnt  = 0;
    Event_tag_t *prev = NULL;
    Event_tag_t *last = NULL;
    for (Event_tag_t *p = event_list->top; p; p = p->next) {
        p->prev = prev;
        prev    = p;
        last    = p;
        cnt++;
    }
    event_list->event_list_count = cnt;
    event_list->tail             = last;
    return (int)cnt;
}
/*****************************************************************************/
/*  FUNCTION        :__cncl_list_detect_and_break_cycle                      */
/*  CALLING SEQ.    :static bool __cncl_list_detect_and_break_cycle(         */
/*                   Event_list_t *event_list)                               */
/*  ARGUMENT        :event_list:イベントリスト構造体                         */
/*  RETURN CODE     :true = ループ検出および修復実施, false = ループなし     */
/*  DESCRIPTION     :イベントリスト内の循環(ループ)を検出し、存在する場合は  */
/*                   ループを切断してリスト整合性を修復する。                */
/*                   ・Floydの循環検出アルゴリズム(二重ポインタ法)を使用     */
/*                   ・ループ検出時はループ開始ノードを特定                  */
/*                   ・ループ終端ノードを探索してnextをNULLに設定            */
/*                   ・リストを再集計し(__cncl_list_recount_and_repair呼出)  */
/*                   ・修復が行われた場合はtrue、検出されなければfalseを返却 */
/*****************************************************************************/
static bool __cncl_list_detect_and_break_cycle(Event_list_t *event_list)
{
    if (!event_list || !event_list->top) return false;
    Event_tag_t *slow = event_list->top;
    Event_tag_t *fast = event_list->top;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            /* Cycle detected. Find loop start. */
            slow = event_list->top;
            while (slow != fast) {
                slow = slow->next;
                fast = fast->next;
            }
            /* 'slow' is at the start of the loop. Find last node in the loop. */
            Event_tag_t *last = slow;
            while (last->next != slow) {
                last = last->next;
            }
            /* Break the cycle and repair list invariants. */
            last->next = NULL;
            __cncl_list_recount_and_repair(event_list);
            return true;
        }
    }
    return false;
}
/****************************************************************************/
/*   内部関数定義                                                           */
/****************************************************************************/
Event_tag_t *cncl_get_event_tag(Event_list_t *event_list);

/*****************************************************************************/
/*  FUNCTION        :cncl_event_proc                                         */
/*  CALLING SEQ.    :short cncl_event_proc(void)                             */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :AWAITIOXでI/O完了を待ち、完了イベントを処理する         */
/*****************************************************************************/
short cncl_event_proc()
{
    Application   *app       = cncl_get_App();
    short          ret_event = 0;
    static int32_t timelimit = 1;

    // AWAITIOXでI/O完了を待ち、完了したI/Oをイベントリストに記載する
    cncl_handle_await_io(timelimit);
    // イベントリストをすべて処理する。
    ret_event = cncl_process_event_list();
    // イベントリストの残件数をチェックし、次回実行時のI/O待ち時間を決定する。
    if (app->event_list.event_list_count > 0) {
        // イベントリストに残りがあるのであれば、AWAITIOXを挟んで処理
        timelimit = 0;
    } else {
        // イベントリストに残りがなければ、次のIOまで待ち。
        timelimit = -1;
    }
    return ret_event;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_process_event_list                                 */
/*  CALLING SEQ.    :short cncl_process_event_list(void)                     */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :イベントリストを走査し、実行時刻到来またはPOST済み      */
/*                   (scheduled_time==-1)のイベントを順次処理する。          */
/*                   ・scheduled_time < 現在時刻 または -1 のイベントを実行  */
/*                   ・イベント関数の戻り(ret_event)を取得                   */
/*                   ・処理済みイベントはリストから削除                      */
/*                   ・未到来のイベントは次ノードへ進む                      */
/*                   ・残数カウンタで走査回数を制限し無限ループを防止        */
/*                   ・最後に処理したイベント関数の戻り値を返却              */
/*****************************************************************************/
short cncl_process_event_list()
{
    Application *app                   = cncl_get_App();
    Event_tag_t *processing_evt_tag    = NULL;
    size_t       remaining_event_count = 0;
    short        ret_event             = 0;

    // イベントリストの先頭から処理を行う。
    processing_evt_tag                 = app->event_list.top;
    remaining_event_count              = app->event_list.event_list_count;
    while (processing_evt_tag && remaining_event_count) {
        // 時間指定されているか、されていない=0=I/Oイベント、POSTされたイベント=-1ならば
        if (processing_evt_tag->scheduled_event_info.scheduled_time
                < JULIANTIMESTAMP(no_param, no_param, no_param, no_param)
            || processing_evt_tag->scheduled_event_info.scheduled_time == -1) {
            //
            ret_event = processing_evt_tag->func(processing_evt_tag);
            if (ret_event != 0) {
                // Todo:当該スレッドの終了を計画する。
            }
            // 処理したイベントは削除する。
            processing_evt_tag = cncl_remove_event(processing_evt_tag);
        } else {
            processing_evt_tag = processing_evt_tag->next;
        }
        remaining_event_count--;
    }
    return ret_event;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_handle_await_io                                    */
/*  CALLING SEQ.    :void cncl_handle_await_io(                              */
/*                   int32_t timelimit)                                      */
/*  ARGUMENT        :timelimit:I/O完了待ちのタイムリミット                   */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :AWAITIOXでI/O完了を待ち、完了結果に応じた後処理を行う。 */
/*                   ・OPEN完了(ZSYS_VAL_AWAITIOTAG_OPEN)は対応スレッドを検索*/
/*                     し、cncl_odst_proc_open_completeを呼出す              */
/*                   ・通常I/O完了はイベントタグへ完了情報(fd/len/fs_err等)を*/
/*                     設定し、cncl_post_eventでイベントキューへ追加         */
/*                   ・$RECEIVE(fd==0)時はFILE_GETRECEIVEINFO_および         */
/*                     PROCESSHANDLE_DECOMPOSE_で送信元情報を取得            */
/*                   ・AWAITIOXタイムアウト(fs_err==ZFIL_ERR_TIMEDOUT)は無処理*/
/*                   ・その他のエラーはEMS出力後に異常終了                   */
/*****************************************************************************/
void cncl_handle_await_io(int32_t timelimit)
{
    short          ret           = 0;
    short          file          = -1;
    short          fs_err        = 0;
    unsigned short len           = 0;
    int32_t        completed_tag = -1;
    int32_t        io_buffer     = 0;
    Event_tag_t   *io_evt_tag    = NULL;
    // I/O完了待ち
    AWAITIOX(&file, &io_buffer, &len, &completed_tag, timelimit, no_param);
    FILE_GETINFO_(file, &fs_err, no_param, no_param, no_param, no_param, no_param);

    // I/O完了時はIOのイベント情報一式を取りだす。第４引数のtagにイベントのポインタアドレスを格納している。
    if (file > -1) {
        // completed_tagが-30Lの場合はNOWAIT I/OのFILE_OPEN_完了通知なので別途処理を行う。
        if (completed_tag == ZSYS_VAL_AWAITIOTAG_OPEN) {
            thread_object_t *odst_proc_thread = cncl_search_odst_proc_thread(file);
            if (odst_proc_thread) {
                cncl_odst_proc_open_complete(odst_proc_thread, fs_err);
            }
            return;
        }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
        io_evt_tag = (Event_tag_t *)completed_tag;
        // タイムスタンプ取得(用途不明恐らくトレース)
        TS_UNIQUE_CREATE_((short *)&io_evt_tag->TS128);
        // イベントタグに完了したI/Oの情報を設定する。
        io_evt_tag->io_info.fd     = file;
        io_evt_tag->io_info.data   = container_of(io_buffer, io_trace_buf_t, data_info.rec_area);
        io_evt_tag->io_info.len    = len;
        io_evt_tag->io_info.fs_err = fs_err;
#pragma clang diagnostic pop
        // $RECEIVEの追加情報(用途不明)
        if (io_evt_tag->io_info.fd == 0) {
            // OPEN, CLOSE, CONTROL, SETMODE, SETPARAM, RESETSYNC, CONTROLBUF 以外の場合はNULLハンドルを返す。
            ret = FILE_GETRECEIVEINFO_((short *)&io_evt_tag->io_info.RINF, no_param);
            if (ret != 0) {
                cncl_ems_procedure_error("FILE_GETRECEIVEINFO_", ret, DEF_NERR_FILE_IO_ERR);
                cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
            }
            short s_name_len;
            short s_node_len;
            short s_ret;
            short ret_event;
            s_ret = PROCESSHANDLE_DECOMPOSE_(
                (short *)&io_evt_tag->io_info.RINF.z_sender, (short *)&io_evt_tag->io_info.cpu, no_param, no_param,
                io_evt_tag->io_info.node_name, ZSYS_VAL_LEN_SYSTEMNAME, &s_node_len, io_evt_tag->io_info.proc_name,
                ZSYS_VAL_LEN_PROCESSNAME, &s_name_len, no_param);
            if (s_ret != ZFIL_ERR_OK) {
                s_node_len = s_name_len = 0;
            }
            io_evt_tag->io_info.node_name[s_node_len] = 0;
            io_evt_tag->io_info.proc_name[s_name_len] = 0;
            // トランザクション内でNOWAITIOが発生しないように$RECEIVEからの要求は直ちに実行する。
            ret_event = io_evt_tag->func(io_evt_tag);
            if (ret_event != 0) {
                // 必要であればここでプロセスの終了を計画するが現状必要ない。
            }
            // イベントリスト外だけど初期化してもらう。
            cncl_remove_event(io_evt_tag);
            return;
        }
        // イベントリストに完了したI/Oのイベントタグを追加する。
        cncl_post_event(io_evt_tag);
        return;
    }
    if (fs_err != ZFIL_ERR_TIMEDOUT) {
        // その他エラーは想定外。Abendする。
        cncl_ems_procedure_error("AWAITIOX", fs_err, DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
}
/*****************************************************************************/
/*  FUNCTION        :initial_event_list                                      */
/*  CALLING SEQ.    :void initial_event_list(Event_list_t *event_list)       */
/*  ARGUMENT        :event_list:イベントリスト管理構造体                     */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :イベントリストを初期化し使用可能な状態にする            */
/*****************************************************************************/
void initial_event_list(Event_list_t *event_list)
{
    memset(event_list, 0, sizeof(Event_list_t));
    event_list->event_tag_buff = calloc(DEF_MAX_TAG, sizeof(Event_tag_t));
    if (!event_list->event_tag_buff) {
        cncl_ems_procedure_error("initial_event_list", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_event_tag                                      */
/*  CALLING SEQ.    :Event_tag_t* cncl_get_event_tag(                        */
/*                   Event_list_t *event_list)                               */
/*  ARGUMENT        :event_list:イベントリスト管理構造体                     */
/*  RETURN CODE     :使用可能なEvent_tagポインタ,                            */
/*                  :またはNULL(バッファ不足時)                              */
/*  DESCRIPTION     :未使用のイベントタグを1つ取得する                       */
/*****************************************************************************/
Event_tag_t *cncl_get_event_tag(Event_list_t *event_list)
{
    size_t event_index = 0;
    while (event_index < DEF_MAX_TAG) {
        if (event_list->event_tag_buff[event_index].inuse == false) {
            memset(&(event_list->event_tag_buff[event_index]), 0, sizeof(Event_tag_t));
            event_list->event_tag_buff[event_index].inuse       = true;
            event_list->event_tag_buff[event_index].parent_list = event_list;
            return &(event_list->event_tag_buff[event_index]);
        }
        event_index++;
    }
    return NULL;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_post_event                                         */
/*  CALLING SEQ.    :void cncl_post_event(Event_tag_t *event)                */
/*  ARGUMENT        :event:発生させたイベント情報                            */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :イベントをリストへ追加し処理待ちにする                  */
/*****************************************************************************/
#ifndef NODEBUG
cncl_post_err_t test_cncl_post_event(Event_tag_t *event);
cncl_post_err_t cncl_post_event(Event_tag_t *event){

    cncl_post_err_t post_err;
    post_err = test_cncl_post_event(event);
    if(post_err) cncl_post_event_error(post_err);
    return post_err;
}
cncl_post_err_t test_cncl_post_event(Event_tag_t *event)
#else
cncl_post_err_t cncl_post_event(Event_tag_t *event)
#endif
{
    bool repaired =false;
    if (!event || !event->parent_list) return e_cncl_event_post_err_inval;
    Event_list_t *event_list = event->parent_list;

    /* すでにリスト連結済みなら成功扱い（冪等） */
    if (event->queued) return e_cncl_event_post_war_dup;

    /* 既存リストの健全性を確認・修復 */
    __cncl_list_detect_and_break_cycle(event_list);

    /* 念のため：queued==false だが既にリストに存在するケース（破損/外部改変）を検出 */
    if (event_list->top) {
        size_t steps = 0, cap = event_list->event_list_count ? event_list->event_list_count : 4096;
        for (Event_tag_t *p = event_list->top; p && steps++ < cap; p = p->next) {
            if (p == event) {
                /* リスト内に既に存在 → 状態を整えて成功扱い */
                event->queued = true;
                return e_cncl_event_post_war_dup;
            }
        }
    }

    /* O(1) 末尾連結 */
    if (event_list->top == NULL) {
        event_list->top  = event;
        event_list->tail = event;
        event->prev      = NULL;
        event->next      = NULL;
    } else {
        Event_tag_t *tail = event_list->tail;
        if (!tail) {
            /* 不整合：再構築を試みる */
            __cncl_list_recount_and_repair(event_list);
            tail = event_list->tail;
            repaired = true;
        }
        if (!tail && event_list->top) {
            /* 依然修復できない（top はあるのに tail が得られない）→ 破損扱い */
            return e_cncl_event_post_err_corrupt;
        }
        if (!event_list->top && !tail) {
            /* top/tail ともに欠落していれば空リストとして扱う */
            event_list->top  = event;
            event_list->tail = event;
            event->prev      = NULL;
            event->next      = NULL;
        } else {
            tail->next       = event;
            event->prev      = tail;
            event->next      = NULL;
            event_list->tail = event;
        }
    }

    event->queued = true;
    event_list->event_list_count++;
    return repaired ? e_cncl_event_post_war_repaired : e_cncl_event_post_ok;
}

/*****************************************************************************/
/*  FUNCTION        :cncl_remove_event                                       */
/*  CALLING SEQ.    :Event_tag_t* cncl_remove_event(Event_tag_t *event)      */
/*  ARGUMENT        :event:削除したいイベントタグ                            */
/*  RETURN CODE     :次のイベントタグへのポインタ                            */
/*  DESCRIPTION     :イベントリストから指定タグを除去して再リンク            */
/*****************************************************************************/
Event_tag_t *cncl_remove_event(Event_tag_t *event)
{
    if (!event) return NULL;
    Event_list_t    *event_list = event->parent_list;
    thread_header_t *hd         = cncl_get_threadInfo(event->thread);

    /* イベントタグを開放（再利用可能） */
    event->inuse                = false;
    event->queued               = false;

    /* スレッドヘッダのタグ情報を削除する。 */
    if (hd) {
        if (event == hd->io_ev_tag) {
            hd->io_ev_tag = NULL;
        }
        if (event == hd->sig_ev_tag) {
            hd->sig_ev_tag = NULL;
        }
        if (event == hd->scheduled_ev_tag) {
            hd->scheduled_ev_tag = NULL;
        }
    }

    /* イベントリスト外のイベント */
    if ((!event_list)
        || (event->prev == NULL && event->next == NULL && (event != event_list->top))) {
        event->prev = event->next = NULL;
        return event_list ? event_list->top : NULL;
    }

    /* 削除準備：返却用 next を退避 */
    Event_tag_t *ret_next = event->next;

    /* リンク張替え */
    if (event->prev)
        event->prev->next = event->next;
    else
        event_list->top = event->next;

    if (event->next)
        event->next->prev = event->prev;
    else
        event_list->tail = event->prev;

    /* カウント調整 */
    if (event_list->event_list_count > 0) event_list->event_list_count--;

    /* 自ノードのリンク情報をクリア */
    event->prev = event->next = NULL;

    return ret_next;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_socket_tag                                     */
/*  CALLING SEQ.    :Event_tag_t* cncl_get_socket_tag(                       */
/*                   thread_object_t *thread, short socket,                  */
/*                   event_func func)                                        */
/*  ARGUMENT        :thread:紐づくスレッドオブジェクト                       */
/*                  :socket:ソケット番号                                     */
/*                  :func  :I/O完了時に呼ばれる関数                          */
/*  RETURN CODE     :生成したイベントタグへのポインタ                        */
/*  DESCRIPTION     :ソケットI/O用のイベントタグを取得し紐づける             */
/*****************************************************************************/
Event_tag_t *cncl_get_socket_tag(thread_object_t *thread, short socket, event_func func)
{
    thread_header_t *hd      = cncl_get_threadInfo(thread);
    Event_list_t    *ev_list = &(cncl_get_App()->event_list);
    Event_tag_t     *ev_tag;

    ev_tag = cncl_get_event_tag(ev_list);
    if (!ev_tag) {
        cncl_ems_procedure_error("cncl_get_event_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    ev_tag->event_type = e_socket_io;
    ev_tag->io_info.fd = socket;
    ev_tag->func       = func;
    ev_tag->thread     = thread;
    hd->io_ev_tag      = ev_tag;
    return ev_tag;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_io_tag                                         */
/*  CALLING SEQ.    :Event_tag_t* cncl_get_io_tag(                           */
/*                   thread_object_t *thread, short fd, event_func func)     */
/*  ARGUMENT        :thread:紐づくスレッド                                   */
/*                  :fd    :ファイルディスクリプタ                           */
/*                  :func  :I/O完了時に呼ばれる関数                          */
/*  RETURN CODE     :生成したイベントタグへのポインタ                        */
/*  DESCRIPTION     :ファイルI/O用のイベントタグを取得し紐づける             */
/*****************************************************************************/
Event_tag_t *cncl_get_io_tag(thread_object_t *thread, short fd, event_func func)
{
    thread_header_t *hd      = cncl_get_threadInfo(thread);
    Event_list_t    *ev_list = &(cncl_get_App()->event_list);
    Event_tag_t     *ev_tag;

    ev_tag = cncl_get_event_tag(ev_list);
    if (!ev_tag) {
        cncl_ems_procedure_error("cncl_get_event_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    ev_tag->event_type = e_file_io;
    ev_tag->io_info.fd = fd;
    ev_tag->func       = func;
    ev_tag->thread     = thread;
    hd->io_ev_tag      = ev_tag;
    return ev_tag;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_pathsend_tag                                   */
/*  CALLING SEQ.    :Event_tag_t* cncl_get_pathsend_tag(                     */
/*                   thread_object_t *thread, event_func func)               */
/*  ARGUMENT        :thread:紐づくスレッド                                   */
/*                  :func  :Pathsend完了時に呼ばれる関数                     */
/*  RETURN CODE     :生成したイベントタグへのポインタ                        */
/*  DESCRIPTION     :Pathsend I/O用のイベントタグを取得し紐づける            */
/*****************************************************************************/
Event_tag_t *cncl_get_pathsend_tag(thread_object_t *thread, serverclass_info_t *s9s_info, event_func func,
                                   io_trace_buf_t *send_buffer, io_trace_buf_t *org_msg, short send_len,long num_retry)
{
    Event_list_t *ev_list = &(cncl_get_App()->event_list);
    Event_tag_t  *ev_tag;

    ev_tag = cncl_get_event_tag(ev_list);
    if (!ev_tag) {
        cncl_ems_procedure_error("cncl_get_event_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    ev_tag->event_type                 = e_pathsend;
    ev_tag->io_info.fd                 = -1;
    ev_tag->func                       = func;
    ev_tag->thread                     = thread;
    ev_tag->p6d_event_info.send_buffer = send_buffer;
    ev_tag->p6d_event_info.org_msg     = org_msg;
    ev_tag->p6d_event_info.send_len    = send_len;
    ev_tag->p6d_event_info.retry_count = num_retry;
    ev_tag->p6d_event_info.s9s_info    = s9s_info;
    return ev_tag;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_timer_tag                                      */
/*  CALLING SEQ.    :Event_tag_t* cncl_get_timer_tag(                        */
/*                   thread_object_t *thread, long timer,                    */
/*                   event_func func, short sparam)                          */
/*  ARGUMENT        :thread:紐づくスレッド                                   */
/*                  :timer :タイムアウト値(ミリ秒or秒)                       */
/*                  :func  :タイムアウト時の処理関数                         */
/*                  :sparam:追加パラメータ                                   */
/*  RETURN CODE     :生成したイベントタグへのポインタ,                       */
/*                  :NULL(生成失敗時)                                        */
/*  DESCRIPTION     :SIGNALTIMEOUTをコールしタイマー満了イベントを取得       */
/*****************************************************************************/
Event_tag_t *cncl_get_timer_tag(thread_object_t *thread, long timer, event_func func, short sparam)
{
    thread_header_t *hd      = cncl_get_threadInfo(thread);
    Event_list_t    *ev_list = &(cncl_get_App()->event_list);
    Event_tag_t     *ev_tag;
    _cc_status       cc;
    short            timer_tag;

    assert(!hd->sig_ev_tag);
    if (hd->sig_ev_tag) return NULL;

    ev_tag = cncl_get_event_tag(ev_list);
    if (!ev_tag) {
        cncl_ems_procedure_error("cncl_get_event_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
    cc = SIGNALTIMEOUT(timer, sparam, (long)ev_tag, &timer_tag);
#pragma clang diagnostic pop
    if (_status_ne(cc)) {
        cncl_ems_procedure_error("SIGNALTIMEOUT", (short)cc, DEF_NERR_CNCL_TIMER_ERR);
        cncl_remove_event(ev_tag);
        return NULL;
    }
    ev_tag->event_type = e_signal_tm;
    ev_tag->func       = func;
    ev_tag->thread     = thread;
    ev_tag->tm_tag     = timer_tag;
    hd->sig_ev_tag     = ev_tag;
    return ev_tag;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_cancel_timer_tag                                   */
/*  CALLING SEQ.    :void cncl_cancel_timer_tag(thread_object_t *thread)     */
/*  ARGUMENT        :thread:キャンセル対象のスレッド                         */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :指定スレッドに紐づくタイマーをCANCELTIMEOUTする         */
/*****************************************************************************/
void cncl_cancel_timer_tag(thread_object_t *thread)
{
    thread_header_t *hd = cncl_get_threadInfo(thread);
    if (hd->sig_ev_tag) {
        CANCELTIMEOUT(hd->sig_ev_tag->tm_tag);
        cncl_remove_event(hd->sig_ev_tag);
        hd->sig_ev_tag = NULL;
    }
}
/*****************************************************************************/
/*  FUNCTION        :cncl_get_scheduled_tag                                  */
/*  CALLING SEQ.    :Event_tag_t* cncl_get_scheduled_tag(                    */
/*                   thread_object_t *thread, long long julian_time,         */
/*                   event_func func, short sparam,                          */
/*                   io_trace_buf_t *data)                                   */
/*  ARGUMENT        :thread       :紐づくスレッド                            */
/*                  :julian_time  :実行予定のジュリアン時刻                  */
/*                  :func         :実行時に呼ばれる関数                      */
/*                  :sparam       :追加パラメータ                            */
/*                  :data         :関連するデータへのポインタ                */
/*  RETURN CODE     :生成したイベントタグへのポインタ                        */
/*  DESCRIPTION     :指定時刻や即時(-1)で動作するイベントタグを取得          */
/*****************************************************************************/
Event_tag_t *cncl_get_scheduled_tag(thread_object_t *thread, long long julian_time, event_func func, short sparam,
                                    io_trace_buf_t *data)
{
    thread_header_t *hd      = cncl_get_threadInfo(thread);
    Event_list_t    *ev_list = &(cncl_get_App()->event_list);
    Event_tag_t     *ev_tag;
    ev_tag = cncl_get_event_tag(ev_list);
    if (!ev_tag) {
        cncl_ems_procedure_error("cncl_get_event_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    ev_tag->event_type                          = e_scheduled;
    ev_tag->func                                = func;
    ev_tag->scheduled_event_info.scheduled_time = julian_time;
    ev_tag->scheduled_event_info.data           = data;
    ev_tag->scheduled_event_info.sparam         = sparam;
    ev_tag->thread                              = thread;
    hd->scheduled_ev_tag                        = ev_tag;
    return ev_tag;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_post_event_error                                   */
/*  CALLING SEQ.    :void cncl_post_event_error(                             */
/*                   const cncl_post_err_t post_err)                         */
/*  ARGUMENT        :post_err:イベントポスト処理結果コード                   */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :イベントポスト処理(cncl_post_event)で発生したエラーを   */
/*                   判定し、EMS出力および異常終了処理を行う。               */
/*                   ・post_errが負値の場合は致命的エラーとして異常終了      */
/*                   ・post_errが正値の場合はエラーをEMSへ出力のみ行う       */
/*****************************************************************************/
void cncl_post_event_error(const cncl_post_err_t post_err)
{

    if(post_err < 0)
    {
        cncl_ems_procedure_error("cncl_post_event", (short)post_err, DEF_NERR_CNCL_EVLST_BROKEN);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_EVLST_BROKEN);
    }
    else {
        cncl_ems_procedure_error("cncl_post_event", (short)post_err, DEF_NERR_CNCL_EVLST_BROKEN);
    }
}
