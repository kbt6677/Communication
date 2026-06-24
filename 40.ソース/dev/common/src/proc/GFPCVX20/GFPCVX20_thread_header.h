#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/03/03＞         *
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
/*                               スレッドの共通実装                          */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/03/03                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/03/03 新規作成                                      */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
/* USER HEADER     */
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_proc_constant.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
typedef enum __component_id_t
{
    transport,             // ソケットの接続/切断を管理する。
    sender,                // transmissionの子スレッドとして送信を担当する。
    receiver,              // transmissionの子スレッドとして受信を担当する。
    notif_out_dist_procs,  // 1スレッド、outbaundに接続状態を通知する。transmissionスレッドにエラーを返す。
    ipc_interface,         // 1スレッド、$RECEIVEの監視
    out_cmd_srvcls,        // 1スレッド,開局コマンドを投げたりする。
    out_dist_proc          // 最大100スレッド
} component_id_t;

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
#ifndef __type__Event_tag_t
#define __type__Event_tag_t
typedef struct __Event_tag_t Event_tag_t;
#endif

typedef struct __thread_object_t thread_object_t;
#pragma fieldalign shared2 __thread_header_t
typedef struct __thread_header_t
{
    component_id_t   component_id;        /*コンポーネント番号*/
    short            thread;              /*コンポーネントのインデックスを渡す*/
    sel_conf_ind_t   selected_conf;       /*スレッドが参照しているconfig面 */
    bool             stopwait;            /*スレッド停止要求 */
    bool             allocated;           /*割当済み  */
    Event_tag_t     *io_ev_tag;           // 使用しているI/Oタグ
    Event_tag_t     *sig_ev_tag;          // 使用しているSIGNALTIMEOUTタグ
    Event_tag_t     *scheduled_ev_tag;    // 使用しているスケジュールイベントタグ
    io_trace_buf_t  *io_complete_buffer;  // 完了通知に乗せるデータ領域
    thread_object_t *prev;
    thread_object_t *next;
} thread_header_t;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
thread_object_t *cncl_create_thread(component_id_t cmp_id);
void cncl_delete_thread(thread_object_t *thread_object);
void *cncl_get_component(thread_object_t *thread_object);
thread_header_t *cncl_get_threadInfo(thread_object_t *thread_object);
