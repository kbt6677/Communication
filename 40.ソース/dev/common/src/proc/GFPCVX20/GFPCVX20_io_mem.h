#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                      ＜コネクション制御(クライアント)＞                     *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/01/17＞         *
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
/*                               トレース管理情報付きI/O用メモリの割当て管理 */
/*                               トレース管理情報付きI/O用メモリキュー       */
/*                                                                           */
/*        AUTHOR            ････ ISYS Kudo                                   */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025/01/17                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/01/17 新規作成                                     */
/****************************************************************************/

#/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h> nolist
#include <stddef.h> nolist
/* USER HEADER     */
#ifdef _TANDEM_SOURCE
#include <GFPOGGZ4_traceout.h> nolist
#else
#ifndef __DDLCCLC_MF_H
#define __DDLCCLC_MF_H
#include <DDLCCLC.h> nolist
#endif
#endif
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
typedef lk_zac2001i_arg_1_def io_trace_buf_t;

/****************************************************************************/
/*   データ定義                                                         */
/****************************************************************************/
//トレース管理情報付きI/O用メモリ
typedef struct __iobuf_page_t iobuf_page_t;
typedef struct __extra_info_t extra_info_t;

#pragma fieldalign shared2 __iobuf_page_t
#pragma fieldalign shared2 __extra_info_t
struct __iobuf_page_t
{
    io_trace_buf_t val;  ///< lk_zac2001i_arg_1_defのalias長いので…
    size_t         data_length;
    bool           allocated;
    struct __extra_info_t{
        void           *owner_thread;
        size_t          count;
        bool            notify;
    }extra_info;
    iobuf_page_t  *next;
};

//トレース管理情報付きI/O用メモリ割当て管理
#pragma fieldalign shared2 __io_mem_t
typedef struct __io_mem_t
{
    iobuf_page_t *top;
    iobuf_page_t *origin; //for debug.
    size_t       current_pages;
    size_t       remain_pages;
    bool         initialize;
} io_mem_t;

#pragma fieldalign shared2 __io_mem_q_t
//トレース管理情報付きI/O用メモリキュー
typedef struct __io_mem_q_t
{
    iobuf_page_t *top;
    iobuf_page_t *tail;
    size_t       count;
} io_mem_q_t;

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
//トレース管理情報付きI/O用メモリ割当て管理
extern io_mem_t io_mem;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
bool initial_io_mem(io_mem_t *io_mem, size_t max_line);
bool reInitial_io_mem(io_mem_t *io_mem, size_t max_line);
size_t calc_memory(size_t thread_num);
io_trace_buf_t *alloc_io_mem(io_mem_t *io_mem);
void free_io_mem(io_mem_t *io_mem, io_trace_buf_t *page);

io_trace_buf_t *dequeue_io_mem(io_mem_q_t *queue, size_t *len);
void enqueue_io_mem(io_mem_q_t *queue, io_trace_buf_t *page, size_t len);
io_trace_buf_t *delete_queue_node_io_mem(io_mem_q_t *queue, io_trace_buf_t *node);
