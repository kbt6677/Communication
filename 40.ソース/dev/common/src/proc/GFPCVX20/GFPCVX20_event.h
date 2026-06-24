#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
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
/*        WRITTEN-DATE      ････ 2025/01/08                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/01/08 新規作成                                      */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h> nolist
#include <stddef.h> nolist
/* USER HEADER     */
#include <limit.h> nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_sys.h" nolist
#include "GFPCVX20_thread_header.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
/*
    Eventタグの割り当て可能最大件数
*/
#define DEF_MAX_TAG                                                               \
    (CCC_MAX_connection                                                           \
     * (3 * 2                   /* (trans+rcv+snd) × (IO+Timer) */                \
        + CCC_MAX_sendqueue * 6 /* 送信キュー数 まあ300有ればなんとかなるかな？*/ \
        + 4                     /* 切断通知 + 接続通知 + タイマー × 2*/           \
        + 2                     /* cmd_if + タイマー */                           \
        ))                                                                        \
        + CCC_MAX_outbound * 2  /* outbound proc数 × (通知 + タイマー)*/          \
        + CCC_MAX_inbound       /* inbound pathsend */
/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
/**
* @brief 下記で使用するタイムスタンプ型(用途不明 $RECEIVEのトレースに使っているらしい)
    short TS_UNIQUE_CREATE_(short _far);
    void TS_UNIQUE_CONVERT_TO_JULIAN_(short _far);
*
* @note @@__TS_UNIQUE_128
*/
#ifndef __TS_UNIQUE_128__
#define __TS_UNIQUE_128__
#pragma fieldalign shared2 __TS_UNIQUE_128
typedef struct __TS_UNIQUE_128
{
    long long ts[2];
} TS_UNIQUE_128;
#endif

/* cncl_post_event: return codes */
typedef enum
{
    e_cncl_event_post_ok           = 0,
    e_cncl_event_post_war_dup      = 1,  /* duplicate post (already queued) */
    e_cncl_event_post_war_repaired = 2,  /* success but repaired */
    e_cncl_event_post_err_inval    = -1, /* invalid args / missing parent_list */
    e_cncl_event_post_err_corrupt  = -2  /* list corruption unrecoverable (tail/top inconsistent) */
} cncl_post_err_t;

typedef enum __Event_type_t
{
    e_undefined,
    e_socket_io,
    e_file_io,
    e_signal_tm,
    e_scheduled,
    e_pathsend
} Event_type_t;

/* イベントメッセージ */
#ifndef __type__serverclass_info_t
#define __type__serverclass_info_t
typedef struct __serverclass_info_t serverclass_info_t;
#endif

#ifndef __type__Event_tag_t
#define __type__Event_tag_t
typedef struct __Event_tag_t Event_tag_t;
#endif
typedef struct __Event_list_t      Event_list_t;
typedef struct __scheduled_event_t scheduled_event_t;
typedef struct __io_info_t         io_info_t;
typedef short (*event_func)(Event_tag_t *);
#pragma fieldalign shared2 __scheduled_event_t
#pragma fieldalign shared2 __io_info_t
#pragma fieldalign shared2 __Event_tag_t
#pragma fieldalign shared2 __signal_timeout_event_t
#pragma fieldalign shared2 __pathsend_event_t
struct __Event_tag_t
{
    Event_type_t event_type;
    struct __io_info_t
    {
        short                           fd;     /*ファイル番号*/
        short                           fs_err; /*エラーコード*/
        unsigned short                  len;    /*完了長 AWAITIOXの第3パラメータなので unsigned short*/
        io_trace_buf_t                 *data;   /*バッファーアドレス*/
        zsys_ddl_receiveinformation_def RINF;   /*$RECEIVE完了情報*/
        short                           cpu;
        char                            node_name[ZSYS_VAL_LEN_SYSTEMNAME + 1];
        char                            proc_name[ZSYS_VAL_LEN_PROCESSNAME + 1];
    } io_info;                                  /*I/O完了時に設定されるIO情報 */
    thread_object_t *thread;                    /*threadオブジェクトへのポインタ */
    struct __signal_timeout_event_t
    {
        short sparam;
    } signal_timeout_info;
    struct __scheduled_event_t
    {
        long long       scheduled_time;         /* スケジュール時刻 -1なら即時,0ならI/O */
        short           sparam;                 /* SIGNAL_TIMEOUTのParam1 */
        io_trace_buf_t *data;                   /* トレース付きデータ */
    } scheduled_event_info;                     /* 開始時刻を設定する場合 */
    struct __pathsend_event_t
    {
        short               pathsend_fd;        /*PATHSENDオペレーション番号*/
        io_trace_buf_t     *send_buffer;        /* pathsend data領域*/
        io_trace_buf_t     *org_msg;            /* 待避用 */
        short               send_len;           /* request length*/
        long                retry_count;        /* retry count*/
        serverclass_info_t *s9s_info;           /* server class info */
    } p6d_event_info;
    event_func    func;                         /* イベントを処理する関数 */
    TS_UNIQUE_128 TS128;                        /*I/O開始時間(トレース取得用)*/
    short         tm_tag;                       /* SIGNALTIMEOUT */
    bool          inuse;                        /* 割当済み */
    bool          queued;                       /* リスト連結済みか（重複ポスト防止）*/
    Event_tag_t  *prev;
    Event_tag_t  *next;
    Event_list_t *parent_list;
};

#pragma fieldalign shared2 __Event_list_t
struct __Event_list_t
{
    size_t       event_list_count;
    Event_tag_t *top;
    Event_tag_t *tail;
    Event_tag_t *event_tag_buff;
};

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
void initial_event_list(Event_list_t *event_list);
Event_tag_t *cncl_get_socket_tag(thread_object_t *thread, short socket, event_func func);
Event_tag_t *cncl_get_io_tag(thread_object_t *thread, short fd, event_func func);
Event_tag_t *cncl_get_timer_tag(thread_object_t *thread, long timer, event_func func, short sparam);
Event_tag_t *cncl_get_scheduled_tag(thread_object_t *thread, long long julian_time, event_func func, short sparam,
                                    io_trace_buf_t *data);
Event_tag_t *cncl_get_pathsend_tag(thread_object_t *thread, serverclass_info_t *s9s_info, event_func func,
                                   io_trace_buf_t *send_buffer, io_trace_buf_t *org_msg, short send_len,
                                   long num_retry);
void cncl_cancel_timer_tag(thread_object_t *thread);

cncl_post_err_t cncl_post_event(Event_tag_t *event);
Event_tag_t *cncl_remove_event(Event_tag_t *event);
short cncl_event_proc();
void cncl_post_event_error(const cncl_post_err_t post_err);
