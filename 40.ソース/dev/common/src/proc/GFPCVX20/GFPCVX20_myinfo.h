#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/01/28＞         *
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
/*        WRITTEN-DATE      ････ 2025/01/28                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/01/28 新規作成                                     */
/****************************************************************************/

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stddef.h> nolist
/* USER HEADER     */
#include <GFPCGX50.h> nolist
#include "GFPCVX20_sys.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_COM_ASN_ID_LEN 32

/****************************************************************************/
/*   外部データ定義                                                             */
/****************************************************************************/
#pragma fieldalign shared2 __srv_clsId_t
typedef struct __srv_clsId_t
{
    char site;                 /*サイト識別*/
    char fil1;
    char network;              /*N/W識別*/
    char fil2;
    char group[5];             /*グループ識別*/
    char fil3;
    char server_class_name[8]; /*サーバークラス論理名*/
    char fil4;
    char server_class_num[4];  /*サーバークラス論理番号*/
    char fil_null;
} srv_clsId_t;                 /*サーバクラス論理ID*/

/*自プロセス情報*/
#pragma fieldalign shared2 __myinfo_def
typedef struct __myinfo_def
{
    long        pathsend_io_timer;                      /*PATHSEND完了待ちタイマー*/
    long        process_io_timer;                       /*プロセスI/O完了待ちタイマー*/
    long        file_io_timer;                          /*ファイルI/O完了待ちタイマー*/
    long        nowait_open_timer;                      /*Nowaitオープン完了待ちタイマー*/
    long        socket_io_timer;
    short       psend_retry_cnt;                        /*pathsend retry count */
    short       my_handle[ZSYS_VAL_PHANDLE_WLEN];       /*自プロセスハンドル*/
    short       creator_phandle[ZSYS_VAL_PHANDLE_WLEN]; /*クリエータープロセスハンドル*/
    /*PARAM(PRM-SC-ID)より取得*/
    srv_clsId_t srv_clsId;
    /*運用監視端末情報*/
    char        uytrmmon[13 + 1];  /*運用監視端末出力PATHMON名*/
    char        uytrmmonlen[2 + 1];
    char        uytrmsrv[12 + 1];  /*運用監視端末出力サーバ名*/
    char        uytrmsrvlen[2 + 1];
    char        uytrmtimer[4 + 1]; /*運用監視端末出力I/Oタイマー*/
    /*ファイル名情報*/
    short       GFPHI_name_len;
    char        GFPHI_name[ZSYS_VAL_LEN_FILENAME + 1]; /*物理名情報ファイル*/
    /*トレース情報*/
    char        pathmon_name[16]; /*物理名情報ファイルで設定されたPATHMON名*/
    /*自プロセス名*/
    char        my_name[ZSYS_VAL_LEN_PROCESSNAME + 1]; /*自プロセス名*/
    short       my_name_len;                           /*自プロセス名長*/
                                                       //    /*障害調査用*/
    //    long long       start_time;                             /*プロセス起動タイムスタンプ*/
    //    COM_SDT_arg_2_def   ts_char;                            /*システム日時取得*/
    //    COM_SDT_arg_3_def   ts_short;                           /*システム日時取得*/
    //    long long           ts_64;                              /*システム日時取得*/
    int         so_rcvbuff;
    int         so_sndbuff;
} myinfo_def;

/****************************************************************************/
/*   関数定義                                                              */
/****************************************************************************/
short cncl_initial_myinfo(myinfo_def* myinfo);
