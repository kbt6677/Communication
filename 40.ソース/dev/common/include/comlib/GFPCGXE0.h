/**
 * @brief 共通（タイムアウト時刻算出）・ヘッダファイル
 *
 * @date 2025/04/07 新規作成 by tatsuya.sugisaki
*/
#ifndef _GFPCGXE0_H
#define _GFPCGXE0_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>   nolist
#include <ctype.h>    nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist
#include <time.h>     nolist
#include <sys/time.h> nolist

/* USER HEADER     */


/* アクションコード */
#define DEF_COM_DTC_YYYY_DIFF        1900       /* 年差分    */
#define DEF_COM_DTC_1_100            100        /* 業務共通・ヘッダータイプ           */
#define DEF_COM_DTC_BASE10MILI_SEC   10         /* 10ミリ秒単位                       */
#define DEF_COM_DTC_YYYY_LEN         4          /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_OTHR_LEN         2          /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_YYYY_PTR         0          /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_MM_PTR           4          /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_DD_PTR           6          /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_HH_PTR           8          /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_MI_PTR           10         /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_SS_PTR           12         /* 業務共通・電文種別コード           */
#define DEF_COM_DTC_100_PTR          14         /* 業務共通・電文種別コード           */

/*----------------------------------------------------------------------------*/
/* 公開モジュール                                                             */
/*----------------------------------------------------------------------------*/
void
COM_DTC(
    char         *pch_base_time,    /* 基準時刻 */
    long         lg_timer,          /* タイマー値 */
    char         *pch_timeout       /* タイマ満了時刻 */
);

#endif
