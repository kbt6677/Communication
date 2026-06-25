/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX90                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               ユニーク日時取得モジュール                  */
/*                                                                           */
/*                               現在時刻(ユニーク日時128bit)を取得し、      */
/*                               テキスト形式(20byte)、16進数文字(16byte)    */
/*                               へ変換する                                  */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist
#include <clurdec.h>  nolist

/* USER HEADER     */
#include "GFPCGX90.h"

/* vproc関数の宣言 */
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_UNQ                                        */
/*  CALLING SEQ.    : short COM_UNQ(struct *, char *)                       */
/*  ARGUMENT        : 1.datetime    (O)   日付(文字列形式)                  */
/*                  : 2.datetime_l  (O)   タイムスタンプ(後半64bit)         */
/*  RETURN CODE     : 0:正常                                                */
/*  DESCRIPTION     : ユニーク日時取得を行う                                */
/****************************************************************************/

short  COM_UNQ(COM_UNQ_arg_1_def *datetime,char *datetime_hex)
{
    short     ts[8];
    long long julian;        /* 内部用ユリウス暦タイムスタンプ */
    long long timestamp;
    short     date_and_time[8];
    char      wk_datetime[20+1];
    char      wk_datetime_hex[16+1];
    
    /* ユニークタイムスタンプ(128bit)の取得 */
    TS_UNIQUE_CREATE_(ts);
    /* ユリウス暦タイムスタンプ(64bit)の取得 */
    TS_UNIQUE_CONVERT_TO_JULIAN_(ts,&julian); 
    /* 日本時間(LCT)へ変換 */
    timestamp = CONVERTTIMESTAMP(julian);
    /* グレゴリオ日時形式に変換 */
    INTERPRETTIMESTAMP(timestamp,(short _near *)date_and_time);
    /*グレゴリオ日時形式を第1アーギュメント・日付に テキスト形式(20byte)で設定 */
    sprintf(wk_datetime,"%04d%02d%02d%02d%02d%02d%03d%03d"
            , date_and_time[0],date_and_time[1],date_and_time[2]
            , date_and_time[3],date_and_time[4],date_and_time[5]
            , date_and_time[6],date_and_time[7] );
    /* ユニークタイムスタンプ（後半64bit)を16進数文字列に変換し
                                   第2アーギュメント・タイスタンプに設定 */
    sprintf((char *)wk_datetime_hex,"%04x%04x%04x%04x", ts[4],ts[5],ts[6],ts[7]);
    memcpy((char *)datetime,wk_datetime,sizeof(COM_UNQ_arg_1_def));
    memcpy(datetime_hex,wk_datetime_hex,sizeof(wk_datetime_hex) - 1);
    return 0;
} /* end of COM_UNQ */
