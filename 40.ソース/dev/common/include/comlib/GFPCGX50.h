/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGX50                                    */
/*        FUNCTION          ････ 共通モジュール・ヘッダー                    */
/*                               システム日時取得ジュール                    */
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
#ifndef _GFPCGX50_H
#define _GFPCGX50_H

#define DEF_COM_SDT_arg1_gmt  1            /* 標準時              */
#define DEF_COM_SDT_arg1_jpn  2            /* 日本時間            */
#define DEF_COM_SDT_arg1_chn  3            /* 中国時間            */

/* typedef定義 */
#pragma fieldalign shared2 __COM_SDT_arg_2
typedef struct __COM_SDT_arg_2
{
     char    yyyy[4];
     char    mm[2];
     char    dd[2];
     char    hh[2];
     char    md[2];
     char    ss[2];
     char    ms[3];
     char    cc[3];
} COM_SDT_arg_2_def;

#pragma fieldalign shared2 __COM_SDT_arg_3
typedef struct __COM_SDT_arg_3
{
     short   yyyy;
     short   mm;
     short   dd;
     short   hh;
     short   md;
     short   ss;
     short   ms;
     short   cc;
} COM_SDT_arg_3_def;


/* プロトタイプ宣言 */
short COM_SDT(short, COM_SDT_arg_2_def *, COM_SDT_arg_3_def *, long long *);

#endif

