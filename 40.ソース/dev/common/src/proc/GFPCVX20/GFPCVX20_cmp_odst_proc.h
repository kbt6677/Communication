#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/09/10＞         *
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
/*  1.0  ISYS 工藤   2025/09/10 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
/* USER HEADER     */
#include "GFPCVX20_fileIO.h"
#include "GFPCVX20_io_mem.h"
#include "GFPCVX20_thread_header.h" nolist

#define QUALIFIER_FOR_PSNMSDSO "#CONN"   // 電文振分(outbound)向けqualifier
#define DEF_ODST_CLOSED        -1

typedef void (*ntf_odst_complete_func)(thread_object_t *ntf_odst_procs_thread,iobuf_page_t *request);

typedef enum __odst_proc_status_t
{
    e_odst_proc_status_initial,          // 初期状態
    e_odst_proc_status_activating,       // Open中あるいはOpenリトライ中
    e_odst_proc_status_online,           // Open済
    e_odst_proc_status_offline           // Openリトライオーバーした。死んでいる。
} odst_proc_status_t;
typedef enum __odst_proc_send_t
{
    e_odst_proc_send_none,              // 送信可能。
    e_odst_proc_send_sending,           // 送信中
    e_odst_proc_send_retry              // retry
}odst_proc_send_t;
#pragma fieldalign shared2 __cmp_odst_proc_t
#pragma fieldalign shared2 __open_parameter
#pragma fieldalign shared2 __write_parameter
typedef struct __cmp_odst_proc_t
{
    thread_object_t       *parent_ntf_odst_procs;
    short                  out_dist_procs_f_num;
    filename_p_t           out_dist_procs_name;
    odst_proc_status_t     odst_proc_status;
    ntf_odst_complete_func complete_callback;
    iobuf_page_t          *current_request;
    iobuf_page_t           send_buffer;
    odst_proc_send_t       send_status;
    io_mem_q_t             send_queue;
    struct __open_parameter
    {
        long retry_count;                // リトライ回数
        long retry_max;                  // 最大Openリトライ回数
        long retry_interval;             // OpenまたはWriteがエラーの場合にretry_interval/0.1sec後に再オープンする
        long nowait_open_timer;          // Open待ち
    } open_param;
    struct __write_parameter
    {
        long retry_count;                // リトライ回数
        long retry_max;                  // 最大リトライ回数
        long process_io_timer;
    } write_param;
} cmp_odst_proc_t;


thread_object_t *cncl_create_odst_proc(thread_object_t *parent_ntf_odst_procs, filename_p_t out_dist_procs_name,
                                  ntf_odst_complete_func complete_callback);
short cncl_odst_proc_add_req(thread_object_t *thread_odst_proc, io_trace_buf_t *request);
short cncl_odst_proc_open(thread_object_t *thread_odst_proc);
void cncl_odst_proc_open_complete(thread_object_t *thread_odst_proc, short fs_err);
short cncl_odst_proc_open_timeout(Event_tag_t *event);
short cncl_odst_proc_open_retry_timeout(Event_tag_t *event);
short cncl_odst_proc_request_c107_send(thread_object_t *thread_odst_proc);
short cncl_odst_proc_request_c107_send_complete(Event_tag_t *event);
short cncl_odst_proc_request_c107_send_timeout(Event_tag_t *event);
