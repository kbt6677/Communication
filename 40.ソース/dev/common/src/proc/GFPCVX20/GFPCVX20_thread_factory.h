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
/*                               コンポーネントスレッドの生成・管理・操作    */
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
/*  1.0  ISYS 工藤  2025/03/01 新規作成                                      */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h> nolist
/* USER HEADER     */
#include "GFPCVX20_cmp_odst_proc.h" nolist
#include "GFPCVX20_cmp_ipc_interface.h" nolist
#include "GFPCVX20_cmp_ntf_odst_procs.h" nolist
#include "GFPCVX20_cmp_outcmd_srvcls.h" nolist
#include "GFPCVX20_cmp_rcv.h" nolist
#include "GFPCVX20_cmp_snd.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_thread_header.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_MAX_TRANSMISSION CCC_MAX_connection  // パラメータ化が必要DEF_MAX_GFLINの回線数と合わせてスレッド数とする
#define DEF_MAX_THREAD       (DEF_MAX_TRANSMISSION * 3 /* trans+rcv+snd */                                 \
                                 + CCC_MAX_outbound    /* cmp_odst_proc(outbound process数)*/              \
                                 + 1                   /* cmp_ipc_interface */                             \
                                 + 1                   /* cmp_ntf_odst_procs */                            \
                             )

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
#pragma fieldalign shared2 __thread_container
//各コンポーネントスレッドの管理台帳
typedef struct __thread_container
{
    size_t           count;
    thread_object_t *top;
} thread_container;

#pragma fieldalign shared2 __thread_object_t
//コンポーネントスレッドオブジェクト
struct __thread_object_t
{
    thread_header_t   header;                     //スレッドの共通機能部
    thread_container *container;                  //自身のコンポーネントスレッド管理台帳へのポインタ
    union
    {
        cmp_trans_t          cmp_trans;           //トランスポートコンポーネント
        cmp_snd_t            cmp_snd;             //送信コンポーネント
        cmp_rcv_t            cmp_rcv;             //受信コンポーネント
        cmp_ntf_odst_procs_t cmp_ntf_obound;      //電文振分(outbound)コンポーネント(接続通知・電文受信通知)
        cmp_ipc_interface_t  cmp_ipc_interface;   //システムIPCインタフェースコンポーネント($RECEIVE)
        cmp_odst_proc_t      cmp_odst_proc;       //cmp_ntf_odst_procs_tのサブスレッド
        cmp_outcmd_srvcls_t  cmp_otg_cmd;         //コマンドサーバI/F(未使用・未実装)
    } comp;                                       //コンポーネント毎の固有の機能実装
};

#pragma fieldalign shared2 __cncl_thread_factory_t
//コンポーネントスレッドファクトリ
typedef struct __cncl_thread_factory_t
{
    sel_conf_ind_t   crt_cfg_ind;                   //スレッドを生成したコンフィグのインジケータ(未使用)
    thread_container trans;                         //トランスポートコンポーネントスレッド管理台帳
    thread_container snd;                           //送信コンポーネントスレッド管理台帳
    thread_container rcv;                           //受信コンポーネントスレッド管理台帳
    thread_container ntf_obound;                    //電文振分(outbound)コンポーネントスレッド管理台帳
    thread_container ipc_if;                        //システムIPCインタフェースコンポーネントスレッド管理台帳
    thread_container odst_proc;                     //電文振分プロセススレッドの管理台帳
    thread_container otg_cmd;                       //コマンドサーバI/F(未使用・未実装)
    thread_object_t  thread_buffer[DEF_MAX_THREAD]; //スレッドオブジェクトプール(スレッドはこの配列から割当てる)
    size_t           activethread;
} cncl_thread_factory_t;

extern cncl_thread_factory_t cncl_thread_factory; //唯一のスレッドファクトリインスタンス

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
thread_object_t *cncl_create_thread(component_id_t cmp_id);
void cncl_delete_thread(thread_object_t *thread_object);
void *cncl_get_component(thread_object_t *thread_object);
thread_object_t *cncl_search_trans_thread(gflin_pkey_def *recv_con_id);
void cncl_set_conf_ind_thread_factory(sel_conf_ind_t ind);
void cncl_initial_thread_factory();
#ifdef _TANDEM_SOURCE
_extensible
#endif
    size_t
    cncl_max_transport(size_t new_max);
size_t cncl_current_transport();
thread_object_t *cncl_get_ntf_odst();
bool cncl_col_trans_thread(const gflin_pkey_def *recv_con_id, size_t key_length,thread_object_t **collection, long *item);
thread_object_t *cncl_search_odst_proc_thread(short file);
