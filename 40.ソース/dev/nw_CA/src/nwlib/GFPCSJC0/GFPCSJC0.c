/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJC0                                    */
/*        FUNCTION          ････ 通知電文個別処理(CARDNET)                   */
/*                                                                           */
/*                               通知電文精査は、接続先からの                */
/*                               通知電文を精査する。                        */
/*                               通知電文編集は、対象とする電文が            */
/*                               無いため無編集とする。                      */
/*                                                                           */
/*        AUTHER            ････ HAS Yumoto                                  */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-04-14                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 湯本   2025/04/14 (通知電文個別処理(CARDNET))新規作成           */
/*****************************************************************************/
/*   INCLUDE定義                                                             */
/* STANDARD HEADER */
#include <netdb.h>    nolist
#include <stdarg.h>   nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "file.h"
#include "GFPCVXZ0.h"
#include "GFPCVXZ2.h"
#include "GFPCGX90.h"
#include "GFPCVXA0.h"
#include "GFPCVXC0.h"                    /* 通知電文制御ヘッダーファイル     */
#include "NWM_NTC.h"                     /* NW個別モジュールヘッダーファイル */
#include "vproc.h"

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/* 関数のﾌﾟﾛﾄﾀｲﾌﾟ宣言 */

/****************************************************************************/
/*  FUNCTION        : 3.1  NWM_NTC_msg_check                                */
/*  CALLING SEQ.    : short NWM_NTC_msg_check( char *,                      */
/*                                             char *,                      */
/*                                             short,                       */
/*                                             short,                       */
/*                                             db_gfnwi_def * )             */
/*  ARGUMENT        : char *:         制御電文種別                          */
/*                    char *:         リクエストデータ                      */
/*                    short:          リクエストデータ長                    */
/*                    short:          受信電文長                            */
/*                    db_gfnwi_def *: N/W情報ファイル                       */
/*  RETURN CODE     : 1：通知電文                                           */
/*                    2：要求応答型要求電文                                 */
/*                    3：要求応答型応答電文                                 */
/*                    9：精査異常                                           */
/*  DESCRIPTION     : 通知電文精査                                          */
/*                  : 通知電文内容を精査する。                              */
/****************************************************************************/
short NWM_NTC_msg_check( char *pch_control_kind,
                         char *pch_request,
                         short ps_request_len,
                         short ps_denbun_len,
                         db_gfnwi_def *p_db_gfnwi ) 
{

    /* (1)電文精査処理 */
    /* ・共通制御ヘッダ項目の精査を行わない。 */
    /* ・業務共通ヘッダ項目の精査を行わない。 */
    /* ・データ部項目の精査を行わない。       */

    /* (2)処理結果に「1：通知電文」を設定してリターンする。 */
    return DEF_RTN_RECEIVE_NOTICE;

} /*end of NWM_NTC_msg_check*/

/****************************************************************************/
/*  FUNCTION        : 3.2  NWM_NTC_msg_edit                                 */
/*  CALLING SEQ.    : short NWM_NTC_msg_edit ( char *,                      */
/*                                             short,                       */
/*                                             db_gfnwi_def *,              */
/*                                             char *,                      */
/*                                             short,                       */
/*                                             short *,                     */
/*                                             char *,                      */
/*  ARGUMENT        : char *:           通知電文                            */
/*                    short:            通知電文長                          */
/*                    db_gfnwi_def *:   N/W情報ファイル                     */
/*                    char *:           応答電文                            */
/*                    short:            応答電文バッファサイズ              */
/*                    short *:          応答電文長                          */
/*                    char *:           MTI                                 */
/*  RETURN CODE     : 1：応答電文なし                                       */
/*                    2：応答電文あり                                       */
/*                    9：編集エラー                                         */
/*  DESCRIPTION     : 通知電文編集                                          */
/*                  : 通知電文の編集は無く、呼出し元に正常の結果を戻す。    */
/****************************************************************************/
short NWM_NTC_msg_edit ( char *pch_ntf_msg,
                         short ps_ntf_msg_len,
                         db_gfnwi_def *p_db_gfnwi,
                         char *pch_resp_msg,
                         short ps_resp_msg_size,
                         short *ps_resp_msg_len,
                         char *pch_mti )
{

    /* (1) 応答電文編集要否判定 */
    /*   ① 応答不要と判定する。 */

    /* (2) 応答電文編集 */
    /* ・応答不要のため編集処理は行わない。 */

    /* (3) 処理結果に「1：応答電文なし」を設定してリターンする。 */
    *ps_resp_msg_len = 0;
    return DEF_RTN_NO_RESPONSE_MSG;

} /*end of NWM_NTC_msg_edit*/
