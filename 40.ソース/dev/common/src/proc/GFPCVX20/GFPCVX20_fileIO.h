#pragma once
/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/01/29＞         *
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
/*        WRITTEN-DATE      ････ 2025/01/29                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤  2025/01/29 新規作成                                      */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
/* USER HEADER     */
#include <GFPCGXB0.h> nolist
#include "GFPCVX20_sys.h" nolist
#ifdef _TANDEM_SOURCE
#ifndef __db_gfphi_def__
#define __db_gfphi_def__
#include <file.h(db_gfphi)> nolist
#endif
#ifndef __db_gfnwi_def__
#define __db_gfnwi_def__
#include <file.h(db_gfnwi)> nolist
#endif
#else
#ifndef __file_h__
#define __file_h__
#include <file.h> nolist
#endif
#endif

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_FILENO_CLOSED -1
#define DEF_WC_CH         '}'
#define DEF_ST_0000       "0000"

/****************************************************************************/
/*   マクロ定義                                                             */
/****************************************************************************/
#define COM_IOM_MAC(iom_params)                                                                                \
    COM_IOM((iom_params)->func_type, (iom_params)->sub_prog_sts, &((iom_params)->arg3), &((iom_params)->arg4), \
            &((iom_params)->arg5), &((iom_params)->arg6));

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
typedef char module_id_t[8 + 1];
typedef char filename_l_t[8 + 1];
typedef char filename_p_t[ZSYS_VAL_LEN_FILENAME + 1];
typedef char sub_prog_sts_t[2 + 1];
typedef char func_type_t[4 + 1];

#pragma fieldalign shared2 __iom_params_def
typedef struct __iom_params_def
{
    //    filename_l_t       filename_l;              ///< I   制御電文エレメント情報ファイル論理名
    //    filename_p_t       filename_p;              ///< I   制御電文エレメント情報ファイル物理名
    func_type_t       func_type;     ///< arg1
    sub_prog_sts_t    sub_prog_sts;  ///< arg2
    COM_IOM_arg_3_def arg3;          ///< I   トレース情報
    COM_IOM_arg_4_def arg4;          ///< I/O ファイル情報
    COM_IOM_arg_5_def arg5;          ///< I   入力情報(主として読込関連)
    COM_IOM_arg_6_def arg6;          ///< O   出力情報(主として書込関連)
    void             *info;          ///< 拡張用
} iom_params_def;

typedef struct __gflin_a1_key_t
{
    char srv_cls_kind[8];
    char srv_cls_num[4];
} gflin_a1_key_t;

/****************************************************************************/
/*   関数定義                                                               */
/****************************************************************************/
void cncl_initial_ioparams(iom_params_def *iom_params);
void cncl_preset_ioparams(iom_params_def *iom_params, filename_p_t filename, filename_l_t fileId,
                          filename_l_t fileType);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_prepare_ioparams(iom_params_def *iom_params, func_type_t func_type, short part_key_type,
                          short part_key_position, short part_key_len, char key_value[70], char key_type[2],
                          short key_len, short compare_len, short positioning_mode, short lock_flg, short asc_desc_type,
                          long io_timer, void *rec_area, short rec_len);

#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_gfphi_key(db_gfphi_def *db_gfphi, char site_id, char nw_id, char grp_id[5], char srv_cls_kind[8],
                   char srv_cls_num[4], char srv_cls_mlt_num[4], char prc_file_kind[8], char prc_file_num[4],
                   char prc_file_mlt_num[4]);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_db_gfif_key(void *db_gfif_key, char site_id, char nw_id, char grp_id[5], char if_id[5], char station_id[6]);
void cncl_db_gflin_a1_key(gflin_a1_key_t *gflin_a1_key, char server_class_name[8], char server_class_num[4]);
