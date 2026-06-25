/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSV60                                    */
/*        FUNCTION          ････ NW個別(電文ヘッダ電文長編集）               */
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
#include  "msg_VI.h"
#include  "GFPCSV60.h"
#include  "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.1.0  NWM_HDL                                         */
/*  DESCRIPTION     : 電文ヘッダ電文長編集[VisaNet]                          */
/*                                                                           */
/*  CALLING SEQ.    : short  NWM_HDL()                                       */
/*  ARGUMENT        : *telegram_p      : 電文格納域（バッファ）アドレス      */
/*                    length           : ISO8583電文長                       */
/*                                                                           */
/*  RETURN CODE     : 0:正常終了  9:異常終了                                 */
/*****************************************************************************/
short NWM_HDL(char *telegram_p,short length)
{
    short    rtncd;
    short    *ptr;
    
    rtncd = DEF_RTN_NORMAL;
    
    MSG_HEADER_VISA_def    *msghead_p = (MSG_HEADER_VISA_def *) telegram_p;
    
    ptr = (short *) msghead_p->mh_tot_len;
    *ptr = (length + MSG_HEADER_VISA_def_Size);
    return(rtncd);
}


