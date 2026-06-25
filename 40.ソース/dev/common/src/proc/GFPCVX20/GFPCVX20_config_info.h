#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/03/01＞         *
 *        CODED                                 :＜ISYS＞                      *
 *                                                                             *
 *        MODIFY DATE                           :＜修正日 yyyy／mm／dd＞       *
 *        CODED                                 :＜修正者＞                    *
 *                                              :＜修正概要＞                  *
 ******************************************************************************/
/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
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
/*        WRITTEN-DATE      ････ 2025/03/01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/03/01 新規作成                                      */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h> nolist
#include <stddef.h> nolist
/* USER HEADER     */
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_proc_constant.h" nolist
#ifdef _TANDEM_SOURCE
#ifndef __db_gflin_def__
#define __db_gflin_def__
#include <file.h(db_gflin)> nolist
#endif
#ifndef __db_gfnwi_def__
#define __db_gfnwi_def__
#include <file.h(db_gfnwi)> nolist
#endif
#ifndef __db_gfphi_def__
#define __db_gfphi_def__
#include <file.h(db_gfphi)> nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h> nolist
#endif
#endif

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
typedef char group_t[5];             /*グループ識別*/
typedef char server_class_name_t[8]; /*サーバークラス論理名*/
typedef char server_class_num_t[4];  /*サーバークラス論理番号*/

#pragma fieldalign shared2 __nw_conf_t
typedef struct __nw_conf_t
{
    sel_conf_ind_t conf_ind;
    long           gfnwi_count;
    db_gfnwi_def   gfnwi[DEF_MAX_GFNWI];

    long           gflin_count;
    db_gflin_def   gflin[DEF_MAX_GFLIN];

    long           out_dist_procs_count;
    db_gfphi_def   out_dist_procs[DEF_MAX_OUTP];

    db_gfphi_def   in_dist_srvcls;

    db_gfphi_def   cmd_if_srvcls;

    db_gfphi_def   gfphi_info;       // 物理情報ファイル名(理論的に不要のはず)
    db_gfphi_def   gfnwi_info;       // NW情報ファイル名
    db_gfphi_def   gflin_info;       // 回線管理情報ファイル名
    db_gfphi_def   gclst_info;       // 回線ステータスファイル名

    /*物理名情報ファイルより取得*/
    char           pathmon_name[16];     // 物理名情報ファイルで設定されたPATHMON名
    char           my_server_class[16];  // 物理名情報ファイルで設定されたサーバークラス名

    short          GFNWI_name_len;       // 共通IOモジュール用にファイル名長を保持(空白埋めなので)
    filename_p_t   GFNWI_name;           // NW情報ファイル名
    short          GFLIN_name_len;       // 共通IOモジュール用にファイル名長を保持(空白埋めなので)
    filename_p_t   GFLIN_name;           // 回線管理ファイル名
    short          GCLST_name_len;       // 共通IOモジュール用にファイル名長を保持(空白埋めなので)
    filename_p_t   GCLST_name;           // 回線ステータスファイル名
    filename_p_t   trace_name;           // トレースファイル名
} nw_conf_t;

#pragma fieldalign shared2 __nw_info_key_t
typedef struct __nw_info_key_t
{
    char site_id;
    char nw_id;
    char grp_id[5];
    char if_id[5];
    char station_id[6];
    char connect_id[6];
} nw_info_key_t;

#pragma fieldalign shared2 __nw_conf_factory_t
typedef struct __nw_conf_factory_t
{
    sel_conf_ind_t active_conf;
    nw_conf_t      nw_conf[max_conf];
} nw_conf_factory_t;

extern nw_conf_factory_t *nw_conf_factory;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
//  gflinを列挙する。
//  gflinに紐付くgfnwiを返す。
//  1面と2面の選択が可能。

// コンフィグファイルを読直す。読めないとnullを返すよ。
nw_conf_t *cncl_load_config(nw_conf_factory_t *nw_conf_factory);
// 回線に対応するNW情報を返す。
db_gfnwi_def *cncl_get_gfnwi(nw_conf_t *nw_conf, db_gflin_def *line_info);
short cncl_collect_gfnwi_recs(db_gfnwi_def *db_gfnwi_buff, long buff_cnt, long *read_count, char site_id, char nw_id,
                              group_t grp_id, filename_p_t filename,db_gflin_def *db_gflin_buff, long gflin_num);
short cncl_collect_gflin_recs(db_gflin_def *db_gflin_buff, size_t buff_cnt, long *read_count, char *server_class_name,
                              char *server_class_num, filename_p_t filename);

int cncl_chk_gflin_rec(const db_gflin_def *db_gflin_rec);
int cncl_chk_gfnwi_rec(const db_gfnwi_def *db_gfnwi_rec);
int cncl_chk_gfphi_rec(const db_gfphi_def *db_gfphi_rec);
bool cncl_chk_nw_conf(nw_conf_t *nw_conf);
short cncl_sort_marge(db_gfnwi_def *dst, size_t *dst_cnt, const db_gfnwi_def *src, size_t src_cnt);
