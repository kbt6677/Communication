/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXE0                                    */
/*        FUNCTION          ････ 共通（タイムアウト時刻算出）                */
/*                                                                           */
/*                               基本時刻とタイマ値よりタイマ満了時刻を      */
/*                               編集して返す。                              */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-07                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/04/07 新規作成                                      */
/*                                                                           */
/*****************************************************************************/

#include "GFPCGXE0.h"                /* 共通（タイムアウト時刻算出）ヘッダ   */

/* vproc関数の宣言 */
#include "vproc.h"

/*----------------------------------------------------------------------------*/
/* 非公開モジュール                                                           */
/*----------------------------------------------------------------------------*/
static short COM_DTC_atoi(
    char *num_str,
    short len
);

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_DTC                                         */
/*  CALLING SEQ.    : void COM_DTC(char,long,char)                           */
/*  ARGUMENT        : 1.pch_base_time   (I)   基準時刻                       */
/*                  : 2.lg_timer        (I)   タイマー値                     */
/*                  : 3.pch_timeout     (O)   タイマ満了時刻                 */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : タイムアウト時刻算出                                   */
/*****************************************************************************/
void
COM_DTC(
    char         *pch_base_time,    /* 基準時刻 */
    long         lg_timer,          /* タイマー値 */
    char         *pch_timeout       /* タイマ満了時刻 */
)
{
    typedef struct
    {
        short   Gregorian_year;
        short   Gregorian_mon;
        short   Gregorian_day;
        short   hour;
        short   minute;
        short   second;
        short   millisecond;
        short   microsecond;
    } Gregorian_time;

    Gregorian_time      tm_val;           // ワーク・基準時間
    Gregorian_time      tm_val_to;        // ワーク・タイマ満了時刻
    long long           julian_timestamp; // ワーク・時間変換

    // YYYYMMDDhhmissの数値化設定準備
    // 引数No.1の基準時刻をグレゴリオ日時形式変換しグレゴリオ日時配列に設定する。
    memset(&tm_val, 0x00, sizeof(tm_val));
    tm_val.Gregorian_year = COM_DTC_atoi(&pch_base_time[0],  DEF_COM_DTC_YYYY_LEN);
    tm_val.Gregorian_mon  = COM_DTC_atoi(&pch_base_time[DEF_COM_DTC_MM_PTR], DEF_COM_DTC_OTHR_LEN);
    tm_val.Gregorian_day  = COM_DTC_atoi(&pch_base_time[DEF_COM_DTC_DD_PTR], DEF_COM_DTC_OTHR_LEN);
    tm_val.hour           = COM_DTC_atoi(&pch_base_time[DEF_COM_DTC_HH_PTR], DEF_COM_DTC_OTHR_LEN);
    tm_val.minute         = COM_DTC_atoi(&pch_base_time[DEF_COM_DTC_MI_PTR], DEF_COM_DTC_OTHR_LEN);
    tm_val.second         = COM_DTC_atoi(&pch_base_time[DEF_COM_DTC_SS_PTR], DEF_COM_DTC_OTHR_LEN);
    tm_val.millisecond    = COM_DTC_atoi(&pch_base_time[DEF_COM_DTC_100_PTR], DEF_COM_DTC_OTHR_LEN)*DEF_COM_DTC_BASE10MILI_SEC;

    // グレゴリ日時配列をJULIANTIMESTAP形式に変換しタイマ満了時刻（JULIANTIMESTAP）に設定する。（COMPUTETIMESTAMP）
    julian_timestamp = COMPUTETIMESTAMP((short *)&tm_val);

    // タイマ満了時刻（JULIANTIMESTAP）に引数No.2のタイマー値を加算する。
    julian_timestamp += (long long)(lg_timer*DEF_COM_DTC_1_100*DEF_COM_DTC_1_100);

    // タイマ満了時刻（JULIANTIMESTAP）をグレゴリオ日時形式に変換しグレゴリオ日時配列に設定する。（INERPRETTIMESTAMP)
    INTERPRETTIMESTAMP(julian_timestamp,(short *)&tm_val_to);

    // グレゴリオ日時配列をテキスト形式(20Bye)で文字列引数No.3タイマ満了時刻に格納する。
    snprintf(pch_timeout, 16 + 1,
             "%04d%02d%02d%02d%02d%02d%02d",tm_val_to.Gregorian_year,
                                            tm_val_to.Gregorian_mon ,
                                            tm_val_to.Gregorian_day ,
                                            tm_val_to.hour          ,
                                            tm_val_to.minute        ,
                                            tm_val_to.second        ,
                                            tm_val_to.millisecond / DEF_COM_DTC_BASE10MILI_SEC);

    /* 正常終了 */
    return;
}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_DTC_atoi                                    */
/*  CALLING SEQ.    : void COM_DTC_msg_check(char,db_gfnwi_def *             */
/*  ARGUMENT        : 1.num_str         (I)   数値文字列（最大4桁）          */
/*                  : 2.lg_timer        (I)   文字列長（1～4）               */
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : 数値文字列変換（最大4桁）                              */
/*****************************************************************************/
static short COM_DTC_atoi(
    char *num_str,
    short len
)
{
    char szbuf[8];
    memset(szbuf, 0x00, sizeof(szbuf));
    memcpy(szbuf, num_str, len);

    return (short)atoi(szbuf);
}
