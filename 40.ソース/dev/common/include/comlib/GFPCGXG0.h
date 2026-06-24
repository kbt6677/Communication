/*****************************************************************************/
/*****                    <<     HEADER PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCGXG0                                    */
/*        FUNCTION          ････ プロセス情報取得                            */
/*                                                                           */
/*        AUTHER            ････ ISYS Y.Kawasaki                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-11-27                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Y.Kawasaki 2024/11/27 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
#ifndef __IGFPCGXG0_H__
#define __IGFPCGXG0_H__

/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include  <zsysc>      nolist
/* USER HEADER */

/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/
#define     DEF_PROCEDURENAME_MAX          64

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/
// プロセス情報
typedef struct __procinfo
{
  short    my_phandle[ZSYS_VAL_PHANDLE_WLEN];           // 自プロセスハンドル
  char     my_nodename[8+1];                            // 自プロセスノード名
  short    my_nodename_len;                             // 自プロセスノード名長
  char     my_pname[ZSYS_VAL_LEN_PROCESSNAME+1];        // 自プロセス名
  short    my_pname_len;                                // 自プロセス名長
  short    ans_phandle[ZSYS_VAL_PHANDLE_WLEN];          // 親プロセスハンドル
  char     ans_pname[ZSYS_VAL_LEN_PROCESSNAME+1];       // 親プロセス名
  short    ans_pname_len;                               // 親プロセス名長
  char     err_pname[DEF_PROCEDURENAME_MAX];            // エラープロシージャ名
} procinfo_def;

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/
short COM_PRC(procinfo_def*);

#endif   // __IGFPCGXG0_H__
