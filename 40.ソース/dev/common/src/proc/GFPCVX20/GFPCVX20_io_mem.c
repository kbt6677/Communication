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
#include <stddef.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
/* USER HEADER     */
#include "GFPCVX20_io_mem.h" nolist
#include <limit.h>

/****************************************************************************/
/*   内部データ定義                                                         */
/****************************************************************************/
io_mem_t io_mem;

/*****************************************************************************/
/*  FUNCTION        :enqueue_io_mem                                          */
/*  CALLING SEQ.    :void enqueue_io_mem(io_mem_q_t *queue,                  */
/*                   io_trace_buf_t *page, size_t len)                       */
/*  ARGUMENT        :queue:送受信キュー管理構造体                            */
/*                  :page :キューに追加するバッファ                          */
/*                  :len  :バッファに格納されたデータ長                      */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :指定バッファをキュー末尾に連結し待機列に入れる          */
/*****************************************************************************/
void enqueue_io_mem(io_mem_q_t *queue, io_trace_buf_t *page, size_t len)
{
    iobuf_page_t *temp = (iobuf_page_t *)page;
    temp->data_length  = len;
    if (queue->top == NULL) {
        queue->top = queue->tail = temp;
        temp->next               = NULL;
    } else {
        queue->tail->next = temp;
        queue->tail       = temp;
        temp->next        = NULL;
    }
    queue->count++;
}
/*****************************************************************************/
/*  FUNCTION        :dequeue_io_mem                                          */
/*  CALLING SEQ.    :io_trace_buf_t* dequeue_io_mem(io_mem_q_t *queue,       */
/*                   size_t *len)                                            */
/*  ARGUMENT        :queue:送受信キュー管理構造体                            */
/*                  :len  :取得データの長さを返すポインタ                    */
/*  RETURN CODE     :キュー先頭のバッファへのポインタ, NULL(空時)            */
/*  DESCRIPTION     :キュー先頭からバッファを取り出し、切り離す              */
/*****************************************************************************/
io_trace_buf_t *dequeue_io_mem(io_mem_q_t *queue, size_t *len)
{
    iobuf_page_t *temp;
    if (queue->top == NULL) return NULL;
    temp       = (queue->top);
    queue->top = queue->top->next;
    *len       = temp->data_length;
    queue->count--;
    return &(temp->val);
}
/*****************************************************************************/
/*  FUNCTION        :delete_queue_node_io_mem                                */
/*  CALLING SEQ.    :io_trace_buf_t *delete_queue_node_io_mem                */
/*                            (io_mem_q_t *queue, io_trace_buf_t *node)      */
/*  ARGUMENT        :queue:送受信キュー管理構造体                            */
/*                  :node  :キューから削除するノード                         */
/*  RETURN CODE     :削除したノード, NULL(空時)                              */
/*  DESCRIPTION     :キュー先頭から検索し、所定のノードを切り離す            */
/*****************************************************************************/
io_trace_buf_t *delete_queue_node_io_mem(io_mem_q_t *queue, io_trace_buf_t *node)
{
    //キューがNULLまたはキューが空の場合NULLを返す。
    if(!queue || !queue->top) return NULL;

    iobuf_page_t *cur = queue->top;
    iobuf_page_t *prev = NULL;

    while(cur)
    {
        //削除対象ノードを検索する
        if(cur == (iobuf_page_t*)node)
        {
            if(prev)
                prev->next = cur->next;
            else
                queue->top = cur->next; //一つ前がNULLなのでcurは先頭
            if(cur == queue->tail){ //削除対象がtailの場合
                queue->tail = prev;
                if(prev) prev->next = NULL; //prev == NULLならばキューは空になる
            }
            queue->count--;
            return (io_trace_buf_t*)cur;
        }
        prev = cur;
        cur = cur->next;
    }
    return NULL;
}
/*****************************************************************************/
/*  FUNCTION        :initial_io_mem                                          */
/*  CALLING SEQ.    :bool initial_io_mem(                                    */
/*                   io_mem_t *io_mem, size_t max_line)                      */
/*  ARGUMENT        :io_mem  : I/Oメモリ管理構造体                           */
/*                   max_line: 割当てる回線数                                */
/*  RETURN CODE     :true=初期化成功, false=メモリ確保失敗                   */
/*  DESCRIPTION     :I/Oメモリを初期化し、必要なページ数分のバッファを確保する。*/
/*                   ・calc_memoryで必要ページ数を算出                       */
/*                   ・callocでページ分のメモリを確保                        */
/*                   ・リスト構造としてnextポインタを接続                    */
/*                   ・origin/topを設定し、initializeをtrueにする            */
/*                   ・確保失敗時はfalseを返却                               */
/*****************************************************************************/
bool initial_io_mem(io_mem_t *io_mem, size_t max_line)
{
    size_t pages = calc_memory(max_line);
    iobuf_page_t *whole = (iobuf_page_t *)calloc(pages,sizeof(iobuf_page_t));
    iobuf_page_t *current;
    if (!whole) return false;
    io_mem->origin = whole;
    io_mem->top = whole;
    io_mem->current_pages = io_mem->remain_pages = pages;
    current     = whole;
    while ((pages - 1) > 0) {
        current->next = ++whole;
        current       = current->next;
        pages--;
    }
    io_mem->initialize = true;
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :reInitial_io_mem                                        */
/*  CALLING SEQ.    :bool reInitial_io_mem(                                  */
/*                   io_mem_t *io_mem, size_t max_line)                      */
/*  ARGUMENT        :io_mem  : I/Oメモリ管理構造体                           */
/*                   max_line: 割当てる回線数                                */
/*  RETURN CODE     :true=再初期化成功, false=メモリ確保失敗                 */
/*  DESCRIPTION     :I/Oメモリを再初期化し、必要ページ数に満たない場合は     */
/*                   追加で確保する。                                        */
/*                   ・現在のページ数が十分またはinitialize未設定なら終了    */
/*                   ・不足分をcallocで追加確保                              */
/*                   ・追加確保したページをfree_io_memにより解放して再利用可能 */
/*                     状態にする                                            */
/*                   ・確保に失敗した場合はfalseを返却                       */
/*****************************************************************************/
bool reInitial_io_mem(io_mem_t *io_mem, size_t max_line)
{
    size_t pages = calc_memory(max_line);
    size_t delta;
    iobuf_page_t *additional, *current;;
    if(io_mem->current_pages >= pages || !io_mem->initialize) return true;
    delta = pages - io_mem->current_pages;
    additional = calloc(delta ,sizeof(iobuf_page_t));
    if(!additional) return false;
    current = additional;
    while ((delta - 1) > 0) {
        current = ++additional;
        current->allocated = true;
        free_io_mem(io_mem, (io_trace_buf_t*)current);
        delta--;
    }
    return true;
}
/*****************************************************************************/
/*  FUNCTION        :alloc_io_mem                                            */
/*  CALLING SEQ.    :io_trace_buf_t* alloc_io_mem(io_mem_t *io_mem)          */
/*  ARGUMENT        :io_mem:バッファ管理構造体                               */
/*  RETURN CODE     :使用可能なバッファのポインタ,NULL(枯渇時)               */
/*  DESCRIPTION     :空きバッファリストから1ページを切り出して返す           */
/*****************************************************************************/
io_trace_buf_t *alloc_io_mem(io_mem_t *io_mem)
{
    iobuf_page_t *ret_ptr;

    if (io_mem->top == NULL) return NULL;
    ret_ptr              = io_mem->top;
    io_mem->top          = io_mem->top->next;
    ret_ptr->next        = NULL;
    ret_ptr->data_length = 0;
    ret_ptr->allocated   = true;
    io_mem->remain_pages--;
    return &(ret_ptr->val);
}
/*****************************************************************************/
/*  FUNCTION        :free_io_mem                                             */
/*  CALLING SEQ.    :void free_io_mem(io_mem_t *io_mem,                      */
/*                   io_trace_buf_t *page)                                   */
/*  ARGUMENT        :io_mem:バッファ管理構造体                               */
/*                  :page :解放対象のバッファ                                */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :使用済みバッファページを再度空きリストへ戻す            */
/*****************************************************************************/
void free_io_mem(io_mem_t *io_mem, io_trace_buf_t *page)
{
    iobuf_page_t *c_page = (iobuf_page_t *)page;
    iobuf_page_t *temp;
    if (!c_page) return;
    if (!(c_page->allocated)) return;
    memset(c_page, 0, sizeof(iobuf_page_t));
    if (io_mem->top == NULL) {
        io_mem->top = c_page;
    } else {
        temp         = io_mem->top;
        io_mem->top  = c_page;
        c_page->next = temp;
    }
    io_mem->remain_pages++;
}
/*****************************************************************************/
/*  FUNCTION        :calc_memory                                             */
/*  CALLING SEQ.    :size_t calc_memory(size_t thread_num)                   */
/*  ARGUMENT        :thread_num: スレッド数                                  */
/*  RETURN CODE     :必要ページ数                                            */
/*  DESCRIPTION     :I/Oメモリとして必要となるページ数を算出する。           */
/*                   ・スレッド数に応じてtrans/rcv/snd, 受信/送信キュー数、  */
/*                     接続状態通知、cmd_ifを加算                            */
/*                   ・さらにsend_nw滞留分やinbound pathsend分を加算         */
/*                   ・算出した総ページ数を返却する                          */
/*****************************************************************************/
size_t calc_memory(size_t thread_num)
{
    size_t pages = (thread_num
                    * (3                          /* trans+rcv+snd */
                       + CCC_MAX_recvqueue        /* 受信キュー数 */
                       + CCC_MAX_sendqueue        /* 送信キュー数*/
                       + 1 /* 接続状態通知 */ * 2 /* 切断通知 + 接続通知 */
                       + 1                        /* cmd_if */
                       ))
                   + CCC_MAX_sendqueue * 6        /* 少なくとも1スレッド分はsend_nwの滞留に備える */
                   + CCC_MAX_inbound * 2;         /* inbound pathsend */
    return pages;
}
