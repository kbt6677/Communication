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
/* USER HEADER     */
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#ifdef _TANDEM_SOURCE
#ifndef __db_gfnwi_def__
#define __db_gfnwi_def__
#include <file.h(db_gfnwi)> nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h> nolist
#endif
#endif

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_RECEIVE_TIMER 0x8001

typedef enum __rcv_status_t
{
    e_cncl_rcv_idle,       // 受信要求無し
    e_cncl_rcv_inpg,       // 受信中
    e_cncl_rcv_inpg_more,  // 追加データ受信中
    e_cncl_rcv_cmpl,       // 受信完了
    e_cncl_rcv_close,      // fin受信
    e_cncl_rcv_fault = -1  // 受信失敗
} rcv_status_t;

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
#ifndef __type__transport_config_t
#define __type__transport_config_t
typedef struct __transport_config_t transport_config_t;
#endif
#pragma fieldalign shared2 __cmp_rcv_t
typedef struct __cmp_rcv_t
{
    thread_object_t    *p_trans;           ///< 親のtransportクラスのポインタ
    event_func          complete_notice;   ///< 完了通知関数
    short               socket;            ///< ソケット
    io_trace_buf_t     *sq_buff;           ///< 呼出元が提供するバッファ
    size_t              req_len;           ///< 受信要求電文長
    size_t              seq_len;           ///< 受信済み
    io_trace_buf_t     *rcv_buff;          ///< 受信バッファ
    char               *rcv_top;           ///< 受信バッファ先頭
    size_t              total_len;         ///< 受信したデータ長
    size_t              remain_len;        ///< 未使用データ
    transport_config_t *transport_config;  ///< NW情報 transからの継承なのでポインタでよい。
    rcv_status_t        rcv_status;        ///< 受信スレッドステータス
    short               sig_tm_hdl;        ///< 受信タイマーのキャンセルハンドル
    bool                initial_read;      ///< 最初の1バイトの読込
} cmp_rcv_t;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
rcv_status_t cncl_rcv_receive(thread_object_t *cmp_rcv, short socket, char *sq_buff, size_t req_len,
                              event_func completion_trans_func, event_func timeout_trans_func,short * last_err);
thread_object_t *cncl_create_rcv(thread_object_t *cmp_trans);
void cncl_delete_rcv(thread_object_t *thread);
