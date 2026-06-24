/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX50                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               システム日時取得ジュール                    */
/*                                                                           */
/*                               現在時刻を取得し、テキスト形式(20byte)、    */
/*                               Binary形式(64bit)に変換し呼び出し元に返す。 */
/*                                  1.グリニッジ標準時                       */
/*                                  2.日本時間                               */
/*                                  3.中国時間                               */
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
#include "vproc.h"
#include "GFPCGX50.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_COM_SDT_jpn_time  32400000000  /* 9時間（マイクロ秒)  */
#define DEF_COM_SDT_chn_time  28800000000  /* 8時間（マイクロ秒)  */

/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_SDT                                        */
/*  CALLING SEQ.    : short COM_SDT(short, struct *, struct*, long long *)  */
/*  ARGUMENT        : 1.chg_type    (I)   変換タイプ                        */
/*                  : 2.datetime_c  (O)   日付(文字列形式)                  */
/*                  : 3.datetime_b  (O)   日付(Bynary形式)                  */
/*                  : 4.datetime_l  (O)   タイムスタンプ                    */
/*  RETURN CODE     : 0:正常 -1：パラメータエラー                           */
/*  DESCRIPTION     : システム日時の取得を行う                              */
/****************************************************************************/

short  COM_SDT(short              chg_type
             , COM_SDT_arg_2_def *datetime_c
             , COM_SDT_arg_3_def *datetime_b
             , long long         *datetime_l)
{
    char      wk_datetime_c[20+1];
    
    /* パラメータチェック */
    if( chg_type != DEF_COM_SDT_arg1_gmt && 
        chg_type != DEF_COM_SDT_arg1_jpn &&
          chg_type != DEF_COM_SDT_arg1_chn ) {
        return -1;
    }

    /* 変換タイプにより処理を決定 */
    *datetime_l = JULIANTIMESTAMP(); // GMTの取得
    switch( chg_type ) {
    case DEF_COM_SDT_arg1_gmt:
        break;
    case DEF_COM_SDT_arg1_jpn:
        *datetime_l = *datetime_l + DEF_COM_SDT_jpn_time;
        break;
    case DEF_COM_SDT_arg1_chn:
        *datetime_l = *datetime_l + DEF_COM_SDT_chn_time;
        break;
    }
    
    /* 第2アーギュメントへ設定 */
    INTERPRETTIMESTAMP(*datetime_l, (short *)datetime_b); 
    /* 第3アーギュメントへ設定 */
    sprintf(wk_datetime_c, "%04d%02d%02d%02d%02d%02d%03d%03d",
            datetime_b->yyyy, datetime_b->mm, datetime_b->dd, datetime_b->hh,
            datetime_b->md,  datetime_b->ss, datetime_b->ms, datetime_b->cc);
    memcpy(datetime_c,wk_datetime_c,sizeof(wk_datetime_c) - 1);
    return 0;
} /* end of COM_SDT */
