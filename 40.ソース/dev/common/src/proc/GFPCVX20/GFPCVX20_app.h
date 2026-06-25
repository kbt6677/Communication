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
#include <GFPCGXC0.h> nolist
#include "GFPCVX20_config_info.h"  nolist
#include "GFPCVX20_event.h"  nolist
#include "GFPCVX20_gclst_info.h"  nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_ipc_wrapper.h"  nolist // IWYU pragma: keep

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_MY_PROGID      DEF_GFPCVX20
#define CNCL_CONNID_CLIENT "CC" /* コネクション制御(クライアント)   */

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
#pragma fieldalign shared2 __Application
typedef struct __Application
{
    myinfo_def        my_info;
    nw_conf_t*        nw_conf;
    Event_list_t      event_list;
    bool              stopwait;      // プロセス停止待ち状態
    size_t           *activethread;  // 動作中のスレッドが0かつ、プロセス停止待ち状態ならばプロセスは終了する。
    gclst_info_t      gclst_info;
    COM_STP_arg_1_def openers;
} Application;

extern Application App;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/

Application* cncl_get_App();
nw_conf_t* cncl_get_nwcnf();
Event_list_t* cncl_get_eventlist();
bool cncl_initialize_application();
short cncl_initial();
short cncl_business();
short cncl_finish();
short cncl_load_config_request(gflin_pkey_def* gflin_pkey, thread_object_t* thread_trans_old); //廃止
size_t cncl_get_gflin_part_key_length(gflin_pkey_def *gflin_pkey);
short cncl_load_config_collection_request(gflin_pkey_def *gflin_pkey);
const db_gfnwi_def* cncl_search_gfnwi(const db_gflin_def * gflin);
