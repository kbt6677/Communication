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
#include "GFPCVX20_cmp_odst_proc.h"
#include "GFPCVX20_cmp_trans.h"
#include "GFPCVX20_thread_factory.h"
#include <stdbool.h> nolist
#include <string.h> nolist
#include <assert.h> nolist
#ifdef _TANDEM_SOURCE
#include <tal.h> nolist
#include <cextdecs.h(FILE_GETINFO_)> nolist
#include <cextdecs.h(SIGNALTIMEOUT,FILE_OPEN_,WRITEREADX,FILE_CLOSE_,CANCELTIMEOUT)> nolist
#else
#include <cextdecs.h> nolist
#include <tal.h> nolist
#endif

/* USER HEADER     */
#include <errcd.h> nolist

#include "GFPCVX20_cmp_odst_proc.h" nolist
#include "GFPCVX20_cmp_ntf_odst_procs.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_util.h" nolist
#include "GFPCVX20_ipc_wrapper.h" nolist // IWYU pragma: keep

/*****************************************************************************/
/*  FUNCTION        :cncl_create_ntf_odst_procs                              */
/*  CALLING SEQ.    :thread_object_t *cncl_create_ntf_odst_procs(            */
/*                   db_gfphi_def *out_dist_procs,                           */
/*                   size_t out_dist_procs_count)                            */
/*  ARGUMENT        :out_dist_procs     :出力先のプロセス情報                */
/*                  :out_dist_procs_count:上記配列の要素数                   */
/*                  :unit               :1度に送信する処理単位               */
/*  RETURN CODE     :生成したスレッドオブジェクトのポインタ                  */
/*  DESCRIPTION     :通知先プロセスへの送信スレッドを作成し初期化            */
/*****************************************************************************/
thread_object_t *cncl_create_ntf_odst_procs(db_gfphi_def *out_dist_procs, size_t out_dist_procs_count)
{
    thread_object_t      *thread_ntf_odst = cncl_create_thread(notif_out_dist_procs);
    cmp_ntf_odst_procs_t *cmp_ntf_odst    = cncl_get_component(thread_ntf_odst);
    filename_p_t          out_dist_procs_name;
    memset(cmp_ntf_odst, 0, sizeof(cmp_ntf_odst_procs_t));
    while (out_dist_procs_count > 0) {
        if (out_dist_procs[out_dist_procs_count - 1].invalid_flg == ' ') {
            memcpy(out_dist_procs_name, out_dist_procs[out_dist_procs_count - 1].prc_file_info.prc_file_name,
                   sizeof(filename_p_t));
            cncl_rm_trspc_wn(out_dist_procs_name, sizeof(filename_p_t));
            cmp_ntf_odst->odst_procs[cmp_ntf_odst->out_dist_procs_count] =
                cncl_create_odst_proc(thread_ntf_odst, out_dist_procs_name, cncl_ntf_odst_procs_complete_callback);
            if (!(cmp_ntf_odst->odst_procs[cmp_ntf_odst->out_dist_procs_count])) {
                for (size_t i = 0; i < cmp_ntf_odst->out_dist_procs_count; i++) {
                    cncl_delete_thread(cmp_ntf_odst->odst_procs[i]);
                }
                cncl_delete_thread(thread_ntf_odst);
                return NULL;
            }
            cmp_ntf_odst->out_dist_procs_count++;
        }
        out_dist_procs_count--;
    }
    return thread_ntf_odst;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ntf_odst_procs_add_req                             */
/*  CALLING SEQ.    :short cncl_ntf_odst_procs_add_req(                      */
/*                   thread_object_t *thread_ntf_odst,                       */
/*                   io_trace_buf_t *request                                 */
/*                   bool open_request_required)                             */
/*  ARGUMENT        :thread_ntf_odst :通知用スレッドのポインタ               */
/*                  :request         :送信要求バッファ                       */
/*                  :open_request_required :開局要求要否                     */
/*  RETURN CODE     :0                                                       */
/*  DESCRIPTION     :ntf_odst_procsの管理台帳に要求を追加し、                */
/*                   全てのodst_procsに送信要求を渡す                        */
/*****************************************************************************/
short cncl_ntf_odst_procs_add_req(thread_object_t *thread_ntf_odst,thread_object_t *caller_transport, io_trace_buf_t *request, bool open_request_required)
{
    cmp_ntf_odst_procs_t *cmp_ntf_odst = cncl_get_component(thread_ntf_odst);
    iobuf_page_t         *work_req = (iobuf_page_t*)request;

    work_req->extra_info.owner_thread = caller_transport;
    work_req->extra_info.count = 0;
    work_req->extra_info.notify = open_request_required;
    enqueue_io_mem(&(cmp_ntf_odst->request_list), request, sizeof(c107_def));

    for(size_t i = 0; i < cmp_ntf_odst->out_dist_procs_count; i++)
        cncl_odst_proc_add_req(cmp_ntf_odst->odst_procs[i], request);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ntf_odst_procs_complete_callback                   */
/*  CALLING SEQ.    :void cncl_ntf_odst_procs_complete_callback(             */
/*                   thread_object_t *ntf_odst_procs_thread,                 */
/*                   iobuf_page_t *request)                                  */
/*  ARGUMENT        :ntf_odst_procs_thread: 通知管理(親)スレッド             */
/*                   request             : 通知要求(集計用拡張情報付き)      */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ODST送信完了の集計処理を行い、全宛先分が完了した場合に  */
/*                   後処理を実施する。                                      */
/*                   ・request->extra_info.count をインクリメント            */
/*                   ・全宛先(out_dist_procs_count)に達したら以下を実施      */
/*                     - notify==true の場合、送信元トランスポートを待機解除 */
/*                       (e_trans_idle)し、開局要求を送信                    */
/*                       (cncl_trans_post_connect_proc_sign_on)              */
/*                     - リクエストをリストから削除し、メモリを解放          */
/*****************************************************************************/
void cncl_ntf_odst_procs_complete_callback(thread_object_t *ntf_odst_procs_thread,iobuf_page_t *request)
{
    cmp_ntf_odst_procs_t * cmp_ntf_odst_procs = cncl_get_component(ntf_odst_procs_thread);
    cmp_trans_t          * cmp_trans = cncl_get_component(request->extra_info.owner_thread);
    request->extra_info.count++;
    if(request->extra_info.count >= cmp_ntf_odst_procs->out_dist_procs_count)
    {
        if(request->extra_info.notify)
        {
            cmp_trans->trans_status = e_trans_idle; // transport threadの待機を解除
            cncl_trans_post_connect_proc_sign_on(request->extra_info.owner_thread); //開局要求送信
            //開局出来なかった場合にあえて切断等はせず、手動で開局すれば良いと思う。
        }
        delete_queue_node_io_mem(&cmp_ntf_odst_procs->request_list, (io_trace_buf_t *)request);
        free_io_mem(&io_mem, (io_trace_buf_t *)request);
    }
}
