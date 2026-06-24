/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSVC0                                    */
/*        FUNCTION          ････ NW個別(通知電文個別）                       */
/*                                                                           */
/*        AUTHER            ････ HAS K.F                                     */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-04-18                                  */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.F        2025/04/18 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                            */
/*****************************************************************************/
#pragma ENV COMMON

/*****************************************************************************/
/*   INCLUDE定義                                                             */
/*****************************************************************************/
/* SYSTEM HEADER   */
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <stdbool.h> nolist

/* COMMON HEADER   */
#include "common.h"
#include "ems.h"
#include "file.h"

/* COMMON HEADER   */
#include "NWM_MKM.h"
#include "NWM_NTC.h"
#include  "msg_VI.h"
#include "GFPCSVC0.h"
#include "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  通知電文精査処理                                */
/*                                                                           */
/*  CALLING SEQ.    : short NWM_NTC_msg_check()                              */
/*  ARGUMENT        : 1.*ctltyp_p      (I)   制御電文種別(4桁)格納域アドレス */
/*                  : 2.*reqdat_p      (I)   リクエストデータ格納域アドレス  */
/*                    3.reqdatasize    (I)   リクエストデータのバイトサイズ  */
/*                    4.rcvdatasize    (I)   接続先からの受信電文バイトサイズ*/
/*                    5.*nwinfo_p      (I/O) ネットワーク情報格納域アドレス  */
/*                                                                           */
/*  RETURN CODE     : 1: 通知電文                                            */
/*                    2: 要求応答型要求電文                                  */
/*                    3: 要求応答型応答電文                                  */
/*                    9: 精査異常                                            */
/*  DESCRIPTION     :                                                        */
/*****************************************************************************/
short  NWM_NTC_msg_check(char         *ctltyp_p          // 制御電文種別(4桁)格納域アドレス
                        ,char         *reqdat_p          // リクエストデータ格納域アドレス
                        ,short int    reqdatasize        // リクエストデータのバイトサイズ(固定バッファサイズ)
                        ,short int    rcvdatasize        // 接続先からの受信電文バイトサイズ
                        ,db_gfnwi_def *nwinfo_p )        // ネットワーク情報格納域アドレス
{
    short        rtncd;

    /* 電文精査処理 - NONE - */

    rtncd = (short) DEF_NOTICE;

    return(rtncd);
}

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  通知電文編集                                    */
/*                                                                           */
/*  CALLING SEQ.    : short NWM_NTC_msg_edit()                               */
/*  ARGUMENT        : 1.*notice_p      (I)     通知電文格納域アドレス        */
/*                    2.noticesize     (I)     通知電文バイトサイズ(固定長)  */
/*                    3.*nwinfo_p      (I)     NW情報格納域アドレス          */
/*                    4.*eanser_p      (I/O)   編集対象NW情報格納域アドレス  */
/*                    5.eansersize     (I)     編集応答電文バイトサイズ      */
/*                    6.*eanserlen     (I/O)   編集した応答電文バイトサイズ  */
/*                  : 7.*mti_p         (I/O)   MTI (ASCII 4桁)               */
/*                                                                           */
/*  RETURN CODE     : 0: 応答電文なし                                        */
/*                    1: 応答電文あり                                        */
/*                    2: 編集エラー                                          */
/*  DESCRIPTION     :                                                        */
/*****************************************************************************/
short  NWM_NTC_msg_edit(char         *notice_p           // 通知電文格納域アドレス
                       ,short int    noticesize          // 通知電文バイトサイズ(固定バッファサイズ)
                       ,db_gfnwi_def *nwinfo_p           // NW情報格納域アドレス
                       ,char         *eanser_p           // 編集対象NW情報格納域アドレス
                       ,short int    eansersize          // 編集応答電文バイトサイズ(固定バッファサイズ)
                       ,short int    *eanserlen          // 編集した応答電文バイトサイズ
                       ,char         *mti_p    )         // MTI (ASCII 4桁)
{
    short        rtncd;

    /* 応答電文編集要否判定 - none - */

    /* 応答電文編集 -none            */

    rtncd = (short)  DEF_NOTANSER;

    return(rtncd);
}

