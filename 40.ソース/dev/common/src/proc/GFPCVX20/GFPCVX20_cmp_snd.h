#pragma once
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
/*        WRITTEN-DATE      ････ 2025/01/20                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/01/20 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
/* USER HEADER     */
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_thread_header.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_SEND_TIMER 0x8002

typedef enum __snd_status_t
{
    e_cncl_snd_idle,  // 送信要求無し
    e_cncl_snd_inpg,  // 送信中
    e_cncl_snd_cmpl,  // 送信完了
    e_cncl_snd_fault  // 送信エラー
} snd_status_t;

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
#ifndef __type__transport_config_t
#define __type__transport_config_t
typedef struct __transport_config_t transport_config_t;
#endif
#pragma fieldalign shared2 __cmp_snd_t
typedef struct __cmp_snd_t
{
    thread_object_t    *p_trans;           ///< 親のtransportクラスのポインタ
    event_func          trans_notice;
    short               socket;
    io_mem_q_t          snd_req_q;         ///< 送信要求キュー
    size_t              snd_len;           ///< 送信要求電文長
    size_t              sold_len;          ///< 送信済電文長
    snd_status_t        snd_status;        ///< 送信スレッドステータス
    transport_config_t *transport_config;  ///< NW情報 transからの継承なのでポインタでよい。
} cmp_snd_t;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
thread_object_t *cncl_create_snd(thread_object_t *thread_trans, event_func trans_notice);
bool cncl_snd_request(thread_object_t *thread_snd, io_trace_buf_t *snd_request, size_t length);
snd_status_t cncl_snd_send(thread_object_t *thread_snd);
short cncl_snd_send_complete(Event_tag_t *event);
void cncl_delete_snd(thread_object_t *thread_snd);
short cncl_snd_send_timeout(Event_tag_t *event);
bool cncl_snd_request_post(thread_object_t *thread_snd, io_trace_buf_t *snd_request, size_t length);
short cncl_snd_send_post(Event_tag_t *event);
