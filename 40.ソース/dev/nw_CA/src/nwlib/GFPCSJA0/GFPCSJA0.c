/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSJA0                                    */
/*        FUNCTION          ････ カット対象日付取得・更新                    */
/*                                                                           */
/*                               制御電文振り分けから受信したI/Fに従い、     */
/*                               カット日付制御を行う。                      */
/*                                                                           */
/*        AUTHER            ････ HAS                                         */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-03-06                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/03/31 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/* COMMON HEADER */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   nolist
#include <stdlib.h>   nolist
#include <ctype.h>    nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "common.h"
#include "ems.h"
#include "file.h"

#include "GFPCSJA0.h"                         /* カット対象日付取得・更新     */
#include "vproc.h"

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_INIT                                    */
/*  CALLING SEQ.    : void NWM_CTO_msg_check(char,db_gfnwi_def *             */
/*  ARGUMENT        : 1.pst_phys_name_inf   (I)   物理名情報ファイル情報     */
/*                  : 2.pst_cut_file_inf    (I)   カット対象日付管理情報     */
/*                  : 3.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 4.pch_module_id       (I)   モジュールID               */
/*                  : 5.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 6.pst_ems_add_inf     (I)   EMS追加情報                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : カット対象日付取得・更新用初期処理                     */
/*****************************************************************************/
short NWM_CTU_INIT(
    NWM_CTU_INI_arg_1_def   *pst_phys_name_inf, // 物理名情報ファイル情報
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf    // EMS追加情報
)
{
    short                   ret_val;        // 戻り値
    COM_IOM_arg_6_def       st_out_inf;     // 出力情報
    db_gfphi_def            *pgfphi;        // 物理名情報ファイルレコード

    memset(&st_out_inf, 0x00, sizeof(st_out_inf));

    // 出力引数設定（カット対象日付管理ファイルID）
    memcpy(pst_cut_file_inf->file_id, DEF_FL_CUT_DATE_MG, strlen(DEF_FL_CUT_DATE_MG));

    // カット対象日付管理ファイル情報レコード取得
    ret_val = NWM_CTU_gfphi_read(pst_phys_name_inf,
                                 pst_nw_inf,
                                 pst_cut_file_inf->file_id,
                                 pch_module_id,
                                 &st_out_inf,
                                 pst_ems_cmn_inf,
                                 pst_ems_add_inf);
    if(ret_val != 0){
        return -1;
    }
    pgfphi = (db_gfphi_def *)st_out_inf.rec_area;
    memcpy(pst_cut_file_inf->file_name, pgfphi->prc_file_info.prc_file_name, sizeof(pst_cut_file_inf->file_name));

    // カット対象日付管理ファイルオープン
    ret_val = NWM_CTU_gccut_open(pst_cut_file_inf,
                                 pst_nw_inf,
                                 pch_module_id,
                                 pst_ems_cmn_inf,
                                 pst_ems_add_inf);
    if(ret_val != 0){
        return -1;
    }

    // 正常終了
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_GET                                     */
/*  CALLING SEQ.    : void NWM_CTO_msg_check(char,db_gfnwi_def *             */
/*  ARGUMENT        : 1.pst_phys_name_inf   (I)   カット対象日付管理FILE情報 */
/*                  : 2.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 3.pch_module_id       (I)   モジュールID               */
/*                  : 4.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 5.pst_ems_add_inf     (I)   EMS追加情報                */
/*                  : 6.pst_ems_add_inf     (I)   カット対象日付管理レコード */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : カット対象日付取得処理                                 */
/*****************************************************************************/
short NWM_CTU_GET(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    char                    *pst_db_gccut_inf   // カット対象日付管理レコード
)
{


    short ret_val;  // リターンコード

    ret_val = NWM_CTU_gccut_read(pst_cut_file_inf,
                                 pst_nw_inf,
                                 pch_module_id,
                                 pst_ems_cmn_inf,
                                 pst_ems_add_inf,
                                 DEF_COM_IOM_NOLOCK,
                                 pst_db_gccut_inf);

    if(ret_val != 0){
        return -1;
    }

    return ret_val;

}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_INIT                                    */
/*  CALLING SEQ.    : void NWM_CTO_msg_check(char,db_gfnwi_def *             */
/*  ARGUMENT        : 1.pst_cut_file_inf    (I)   カット対象日付管理FILE情報 */
/*                  : 2.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 3.pch_module_id       (I)   モジュールID               */
/*                  : 4.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 5.pst_ems_add_inf     (I)   EMS追加情報                */
/*                  : 6.pch_cutdate         (I)   カット日付                 */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : カット対象日付更新処理                                 */
/*****************************************************************************/
short NWM_CTU_UPDATE(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    char                    *pch_cutdate        // カット日付
)
{

    short ret_val;  // リターンコード

    ret_val = NWM_CTU_gccut_update(pst_cut_file_inf,
                                   pst_nw_inf,
                                   pch_module_id,
                                   pst_ems_cmn_inf,
                                   pst_ems_add_inf,
                                   pch_cutdate);

    return ret_val;

}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_gfphi_read                              */
/*  CALLING SEQ.    : void NWM_CTU_gfphi_read(NWM_CTU_INI_arg_1_def,         */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pst_phys_name_inf   (I)   物理名情報ファイル情報     */
/*                  : 2.pst_cut_file_inf    (I)   カット対象日付管理情報     */
/*                  : 3.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 4.pch_module_id       (I)   モジュールID               */
/*                  : 5.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 6.pst_ems_add_inf     (I)   EMS追加情報                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : 物理名情報ファイルREAD処理                             */
/*****************************************************************************/
static short NWM_CTU_gfphi_read(
    NWM_CTU_INI_arg_1_def   *pst_pname_inf,     // 物理名情報ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_file_lgc_name, // ファイル論理名
    char                    *pch_module_id,     // モジュールID
    COM_IOM_arg_6_def       *pst_out_inf,       // 出力情報
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf    // EMS追加情報
)
{

    char  sub_status[2 +1];
    char  szbuf[256];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;

    short key_len;

    // 初期化
    memset(sub_status,  0x00, sizeof(sub_status));
    memset(&trace_inf,  0x00, sizeof(trace_inf));
    memset(&file_inf,   0x00, sizeof(file_inf));
    memset(&in_inf,     0x00, sizeof(in_inf));
    memset(pst_out_inf, 0x00, sizeof(COM_IOM_arg_6_def));

    key_len = sizeof(((db_gfphi_def *)in_inf.key_value)->pri_key);

    //------------------------------------------------------------------------//
    // 物理名情報ファイルよりレコードを取得                                   //
    //------------------------------------------------------------------------//
    // トレース情報・モジュールID
    memcpy(trace_inf.prog_id,   pch_module_id,            sizeof(trace_inf.prog_id));
    // トレース情報・ファイルID
    memcpy(trace_inf.file_id,   pst_pname_inf->file_id,   sizeof(trace_inf.file_id));
    // トレース情報・ファイル名
    memcpy(trace_inf.file_name, pst_pname_inf->file_name, sizeof(trace_inf.file_name));

    // ファイル情報・論理ファイル名
    memcpy(file_inf.file_id,   pst_pname_inf->file_id,   sizeof(file_inf.file_id));
    // ファイル情報・物理ファイル名
    memcpy(file_inf.file_name, pst_pname_inf->file_name, sizeof(file_inf.file_name));
    // ファイル情報・ファイル番号
    file_inf.file_no = pst_pname_inf->file_no;

    // 入力情報・パーティション識別
    in_inf.part_key_type     = DEF_COM_IOM_PARTITION_KEY_NOT;
    // 入力情報・パーティション情報開始位置
    in_inf.part_key_position = 0;
    // 入力情報・パーティション情報長
    in_inf.part_key_len      = 0;
    // 入力情報・KEY-VALUE
    snprintf(szbuf, sizeof(szbuf),
             "%c%c%5.5s%s%s%s%s%s%s",
             pst_nw_inf->site_id,
             pst_nw_inf->nw_id,
             pst_nw_inf->grp_id,
             DEF_SC_NAME_DEFAULT,
             DEF_SC_NUM_DEFAULT,
             DEF_SC_DUP_DEFAULT,
             pch_file_lgc_name,
             "0000",
             "0000");
    memcpy(in_inf.key_value, szbuf, strlen(szbuf));
    // 入力情報・KEY識別
    memcpy(in_inf.key_type,"10", 2);
    // 入力情報・KEY-LENGTH
    in_inf.key_len          = key_len;
    // 入力情報・COMPARE-LENGTH
    in_inf.compare_len      = key_len;
    // 入力情報・POSITIONNING-MODE
    in_inf.positioning_mode = DEF_COM_IOM_EXACT;
    // 入力情報・LOCKフラグ
    in_inf.lock_flg         = DEF_COM_IOM_NOLOCK;
    // 入力情報・昇順/降順識別
    in_inf.asc_desc_type    = DEF_COM_IOM_ASCEND;
    // 入力情報・I/Oタイマ
    in_inf.io_timer         = (long)pst_pname_inf->io_timer;
    // 入力情報・レコード長
    in_inf.rec_len          = sizeof(db_gfphi_def);

    // IOモジュール呼出
    COM_IOM(DEF_COM_IOM_FUNC_STARTREAD,
            sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            pst_out_inf);

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
    if(memcmp(sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        NWM_CTU_EMS_out(DEF_EVT_FILE_IO_ERR,
                        DEF_NWM_CTU_GYOM_ERR,
                        DEF_NWM_CTU_NERR_FILE_IO_ERR,
                        pst_ems_cmn_inf,
                        pst_ems_add_inf,
                        pst_pname_inf->file_id,
                        DEF_COM_IOM_FUNC_STARTREAD,
                        szbuf,
                        strlen(szbuf),
                        pst_out_inf->guardian_errcode);
        return -1;
    }

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;

}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_gccut_open                              */
/*  CALLING SEQ.    : void NWM_CTO_msg_check(char,db_gfnwi_def *             */
/*  ARGUMENT        : 1.pst_cut_file_inf    (I)   カット対象日付管理情報     */
/*                  : 2.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 3.pch_module_id       (I)   モジュールID               */
/*                  : 4.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 5.pst_ems_add_inf     (I)   EMS追加情報                */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : カット対象日付管理ファイルOPEN処理                     */
/*****************************************************************************/
static short NWM_CTU_gccut_open(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf    // EMS追加情報
)
{

    char  sub_status[2 +1];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;

    // 初期化
    memset(sub_status, 0x00, sizeof(sub_status));
    memset(&trace_inf, 0x00, sizeof(trace_inf));
    memset(&file_inf,  0x00, sizeof(file_inf));
    memset(&in_inf,    0x00, sizeof(in_inf));
    memset(&out_inf,   0x00, sizeof(out_inf));

    //------------------------------------------------------------------------//
    // 物理名情報ファイルよりレコードを取得                                   //
    //------------------------------------------------------------------------//
    // トレース情報・モジュールID
    memcpy(trace_inf.prog_id,      pch_module_id,               sizeof(trace_inf.prog_id));
    // トレース情報・ファイルID
    memcpy(trace_inf.file_id,      pst_cut_file_inf->file_id,   sizeof(trace_inf.file_id));
    // トレース情報・ファイル名
    memcpy(trace_inf.file_name,    pst_cut_file_inf->file_name, sizeof(trace_inf.file_name));
    // トレース情報・ファイルI/O種別
    memcpy(trace_inf.file_io_type, DEF_NWM_CTU_OPEN,            sizeof(trace_inf.file_io_type));

    // ファイル情報・論理ファイル名
    memcpy(file_inf.file_id,   pst_cut_file_inf->file_id,   sizeof(file_inf.file_id));
    // ファイル情報・物理ファイル名
    memcpy(file_inf.file_name, pst_cut_file_inf->file_name, sizeof(file_inf.file_name));

    // 入力情報・I/Oタイマ
    in_inf.io_timer =(long)pst_cut_file_inf->io_timer;

    // IOモジュール呼出
    COM_IOM(DEF_COM_IOM_FUNC_OPEN,
            sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
    if(memcmp(sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        NWM_CTU_EMS_out(DEF_EVT_FILE_IO_ERR,
                        DEF_NWM_CTU_GYOM_ERR,
                        DEF_NWM_CTU_NERR_FILE_IO_ERR,
                        pst_ems_cmn_inf,
                        pst_ems_add_inf,
                        DEF_FL_CUT_DATE_MG,
                        DEF_NWM_CTU_OPEN,
                        in_inf.key_value,
                        24,
                        out_inf.guardian_errcode);
        return -1;
    }

    pst_cut_file_inf->file_no = file_inf.file_no;

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_gccut_read                              */
/*  CALLING SEQ.    : void NWM_CTU_gccut_read(NWM_CTU_INI_arg_1_def,         */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pst_cut_file_inf    (I)   カット対象日付管理情報     */
/*                  : 2.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 3.pch_module_id       (I)   モジュールID               */
/*                  : 4.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 5.sh_lockmode         (I)   EMS追加情報                */
/*                  : 6.pst_ems_add_inf     (I)   ロックモード               */
/*                  : 7.pst_out_inf         (I)   出力情報                   */
/*  RETURN CODE     :  0：正常                                               */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : カット対象日付管理ファイルREAD処理                     */
/*****************************************************************************/
static short NWM_CTU_gccut_read(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    short                   sh_lockmode,        // ロックモード
    char                    *pst_out_inf        // 出力情報
)
{

    char  sub_status[2 +1];
    COM_IOM_arg_3_def trace_inf;
    COM_IOM_arg_4_def file_inf;
    COM_IOM_arg_5_def in_inf;
    COM_IOM_arg_6_def out_inf;
    short  key_len;

    // 初期化
    memset(sub_status,  0x00, sizeof(sub_status));
    memset(&trace_inf,  0x00, sizeof(trace_inf));
    memset(&file_inf,   0x00, sizeof(file_inf));
    memset(&in_inf,     0x00, sizeof(in_inf));
    memset(&out_inf,    0x00, sizeof(COM_IOM_arg_6_def));

    key_len = sizeof(((db_gccut_def *)in_inf.key_value)->pri_key);

    //------------------------------------------------------------------------//
    // 物理名情報ファイルよりレコードを取得                                   //
    //------------------------------------------------------------------------//
    // トレース情報・モジュールID
    memcpy(trace_inf.prog_id,      pch_module_id,                sizeof(trace_inf.prog_id));
    // トレース情報・ファイルID
    memcpy(trace_inf.file_id,      pst_cut_file_inf->file_id,   sizeof(trace_inf.file_id));
    // トレース情報・ファイル名
    memcpy(trace_inf.file_name,    pst_cut_file_inf->file_name, sizeof(trace_inf.file_name));
    // トレース情報・ファイルI/O種別
    memcpy(trace_inf.file_io_type, DEF_NWM_CTU_READ,             sizeof(trace_inf.file_io_type));

    // ファイル情報・論理ファイル名
    memcpy(file_inf.file_id,    pst_cut_file_inf->file_id,   sizeof(file_inf.file_id));
    // ファイル情報・物理ファイル名
    memcpy(file_inf.file_name,  pst_cut_file_inf->file_name, sizeof(file_inf.file_name));
    // ファイル情報・ファイル番号
    file_inf.file_no = pst_cut_file_inf->file_no;

    // 入力情報・パーティション識別
    in_inf.part_key_type =0;
    // 入力情報・パーティション情報開始位置
    in_inf.part_key_position =0;
    // 入力情報・パーティション情報長
    in_inf.part_key_len =0;
    // 入力情報・KEY-VALUE
    memcpy(in_inf.key_value, pst_nw_inf, key_len);
    // 入力情報・KEY識別
    memcpy(in_inf.key_type,"10", 2);
    // 入力情報・KEY-LENGTH
    in_inf.key_len =key_len;
    // 入力情報・COMPARE-LENGTH
    in_inf.compare_len =key_len;
    // 入力情報・POSITIONNING-MODE
    in_inf.positioning_mode =2;
    // 入力情報・LOCKフラグ
    in_inf.lock_flg =sh_lockmode;
    // 入力情報・昇順/降順識別
    in_inf.asc_desc_type =0;
    // 入力情報・I/Oタイマ
    in_inf.io_timer =(long)pst_cut_file_inf->io_timer;
    // 入力情報・レコード長
    in_inf.rec_len =sizeof(db_gccut_def);

    // IOモジュール呼出
    COM_IOM(DEF_COM_IOM_FUNC_STARTREAD,
            sub_status,
            &trace_inf,
            &file_inf,
            &in_inf,
            &out_inf);

    //------------------------------------------------------------------------//
    // 出力情報設定                                                           //
    //------------------------------------------------------------------------//
    if(memcmp(sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) != 0){
        // 異常終了
        NWM_CTU_EMS_out(DEF_EVT_FILE_IO_ERR,
                        DEF_NWM_CTU_GYOM_ERR,
                        DEF_NWM_CTU_NERR_FILE_IO_ERR,
                        pst_ems_cmn_inf,
                        pst_ems_add_inf,
                        DEF_FL_CUT_DATE_MG,
                        DEF_COM_IOM_FUNC_STARTREAD,
                        in_inf.key_value,
                        24,
                        out_inf.guardian_errcode);
        return -1;
    }

    memcpy(pst_out_inf, out_inf.rec_area, sizeof(db_gccut_def));

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return 0;


}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_gccut_update                            */
/*  CALLING SEQ.    : void NWM_CTU_gfphi_read(NWM_CTU_INI_arg_1_def,         */
/*                      NWM_CTU_INI_arg_3_def *,char *,char *,               */
/*                      COM_IOM_arg_6_def *)                                 */
/*  ARGUMENT        : 1.pst_cut_file_inf    (I)   カット対象日付管理情報     */
/*                  : 2.pst_nw_inf          (I)   ネットワーク特定情報       */
/*                  : 3.pch_module_id       (I)   モジュールID               */
/*                  : 4.pst_ems_cmn_inf     (I)   EMS共通情報                */
/*                  : 5.pst_ems_add_inf     (I)   EMS追加情報                */
/*                  : 6.pch_cutdate         (I)   カット日付                 */
/*  RETURN CODE     :  0：正常                                               */
/*                     1：更新無し                                           */
/*                    -1：初期化失敗                                         */
/*  DESCRIPTION     : カット対象日付管理ファイルUPDATE処理                   */
/*****************************************************************************/
static short NWM_CTU_gccut_update(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    char                    *pch_cutdate        // カット日付
)
{

    char  sub_status[2 +1];                 // ステータス
    COM_IOM_arg_3_def trace_inf;            // トレース情報
    COM_IOM_arg_4_def file_inf;             // ファイル情報
    COM_IOM_arg_5_def in_inf;               // 入力情報
    COM_IOM_arg_6_def out_inf;              // 出力情報
    db_gccut_def       gccu_read_inf;       // カット日付ファイル情報ポインタ
    db_gccut_def      *gccu_updatet_inf;    // カット日付ファイル情報ポインタ
    COM_SDT_arg_2_def  stdate_ch;
    COM_SDT_arg_3_def  stdate_sh;
    long long          lldate;
    short              ret_val;

    // 初期化
    memset(sub_status,  0x00, sizeof(sub_status));
    memset(&trace_inf,  0x00, sizeof(trace_inf));
    memset(&file_inf,   0x00, sizeof(file_inf));
    memset(&in_inf,     0x00, sizeof(in_inf));
    memset(&out_inf,    0x00, sizeof(out_inf));

    //------------------------------------------------------------------------------//
    // 基本的に更新しないものをあらかじめ設定
    //------------------------------------------------------------------------------//
    // 入力情報・パーティション識別
    in_inf.part_key_type =0;
    // 入力情報・パーティション情報開始位置
    in_inf.part_key_position =0;
    // 入力情報・パーティション情報長
    in_inf.part_key_len =0;
    // 入力情報・KEY-VALUE
    memcpy(in_inf.key_value, pst_nw_inf, sizeof(gccu_updatet_inf->pri_key));
    // 入力情報・KEY識別
    memcpy(in_inf.key_type,"10", 2);
    // 入力情報・KEY-LENGTH
    in_inf.key_len =sizeof(gccu_updatet_inf->pri_key);
    // 入力情報・COMPARE-LENGTH
    in_inf.compare_len =sizeof(gccu_updatet_inf->pri_key);
    // 入力情報・POSITIONNING-MODE
    in_inf.positioning_mode =2;
    // 入力情報・LOCKフラグ
    in_inf.lock_flg = DEF_COM_IOM_LOCK;
    // 入力情報・昇順/降順識別
    in_inf.asc_desc_type =0;
    // 入力情報・I/Oタイマ
    in_inf.io_timer =(long)pst_cut_file_inf->io_timer;
    // 入力情報・レコード長
    in_inf.rec_len =sizeof(db_gccut_def);

    // トレース情報・モジュールID
    memcpy(trace_inf.prog_id,      pch_module_id,               sizeof(trace_inf.prog_id));
    // トレース情報・ファイルID
    memcpy(trace_inf.file_id,      pst_cut_file_inf->file_id,   sizeof(trace_inf.file_id));
    // トレース情報・ファイル名
    memcpy(trace_inf.file_name,    pst_cut_file_inf->file_name, sizeof(trace_inf.file_name));

    // ファイル情報・論理ファイル名
    memcpy(file_inf.file_id,   pst_cut_file_inf->file_id,   sizeof(file_inf.file_id));
    // ファイル情報・物理ファイル名
    memcpy(file_inf.file_name, pst_cut_file_inf->file_name, sizeof(file_inf.file_name));
    // ファイル情報・ファイル番号
    file_inf.file_no = pst_cut_file_inf->file_no;

    //------------------------------------------------------------------------//
    // カット対象日付管理ファイルの読込                                       //
    //------------------------------------------------------------------------//
    ret_val = NWM_CTU_gccut_read(pst_cut_file_inf,
                                 pst_nw_inf,
                                 pch_module_id,
                                 pst_ems_cmn_inf,
                                 pst_ems_add_inf,
                                 DEF_NWM_CTU_LOCK,
                                 (char *)&gccu_read_inf);

    if(ret_val != 0){
        // 異常終了
        return -1;
    }

    //------------------------------------------------------------------------//
    // カット対象日付更新                                                     //
    //------------------------------------------------------------------------//
    // カット日付更新確認
    if(memcmp(gccu_read_inf.cut_date_info.cut_date, pch_cutdate,sizeof(gccu_read_inf.cut_date_info.cut_date)) == 0){
        // カット日に変更がない場合、更新無し
        ret_val =  1;

    }else{
        // カット日に変更がある場合、更新
        ret_val =  0;

        // 読込レコードを入力レコードにコピー
        memcpy(in_inf.rec_area, &gccu_read_inf, sizeof(gccu_read_inf));

        // システム日時取得処理システム日時取得(COM_SDT)を呼出し日付を取得する。
        memset(&stdate_ch, 0x00, sizeof(stdate_ch));
        memset(&stdate_sh, 0x00, sizeof(stdate_sh));
        COM_SDT(2, &stdate_ch, &stdate_sh, &lldate);

        // 入力情報・レコード
        gccu_updatet_inf = (db_gccut_def *)in_inf.rec_area;
        // 入力情報・レコード・カット対象日付ファイル・カット対象日付
        memcpy(gccu_updatet_inf->cut_date_info.cut_date,
               pch_cutdate,
               sizeof(gccu_updatet_inf->cut_date_info.cut_date));
        // 入力情報・レコード・カット対象日付ファイル・カット対象日付更新日時
        memcpy(gccu_updatet_inf->cut_date_info.cut_update_time,
               &stdate_ch,
               sizeof(gccu_updatet_inf->cut_date_info.cut_update_time));
        // 入力情報・レコード・カット対象日付ファイル・前回カット対象日付
        memcpy(gccu_updatet_inf->cut_date_info.last_cut_date,
               gccu_read_inf.cut_date_info.cut_date,
               sizeof(gccu_updatet_inf->cut_date_info.last_cut_date));
        // 入力情報・レコード・カット対象日付ファイル・前回カット対象日付更新日時
        memcpy(gccu_updatet_inf->cut_date_info.last_cut_update_time,
               gccu_read_inf.cut_date_info.cut_update_time,
               sizeof(gccu_updatet_inf->cut_date_info.last_cut_update_time));

        // トレース情報・ファイルI/O種別
        memcpy(trace_inf.file_io_type, DEF_NWM_CTU_WRITE,           sizeof(trace_inf.file_io_type));

        // IOモジュール呼出
        COM_IOM(DEF_COM_IOM_FUNC_UPDATE,
                sub_status,
                &trace_inf,
                &file_inf,
                &in_inf,
                &out_inf);

        //------------------------------------------------------------------------//
        // 出力情報設定                                                           //
        //------------------------------------------------------------------------//
        if(memcmp(sub_status, DEF_COM_IOM_NO_ERR, strlen(DEF_COM_IOM_NO_ERR)) == 0){
            // 更新正常
        }else{
            // 更新異常終了
            ret_val =  -1;
            NWM_CTU_EMS_out(DEF_EVT_FILE_IO_ERR,
                            DEF_NWM_CTU_GYOM_ERR,
                            DEF_NWM_CTU_NERR_FILE_IO_ERR,
                            pst_ems_cmn_inf,
                            pst_ems_add_inf,
                            DEF_FL_CUT_DATE_MG,
                            DEF_COM_IOM_FUNC_UPDATE,
                            in_inf.key_value,
                            sizeof(gccu_updatet_inf->pri_key),
                            out_inf.guardian_errcode);
        }
    }

    //------------------------------------------------------------------------//
    // 正常終了                                                               //
    //------------------------------------------------------------------------//
    return ret_val;


}
/*****************************************************************************/
/*  FUNCTION        : 1.0.0  CCUT_EMS_out                                    */
/*  CALLING SEQ.    : void NWM_CTU_EMS_out(short,char *,...)                 */
/*  ARGUMENT        : 1.msg_id         (I)   メッセージID                    */
/*                  : 2.msg_type       (I)   メッセージ通知区分              */
/*                  : 3.col_num        (I)   任意メッセージ設定項目数        */
/*                  : ...              (I)   任意メッセージ設定項目（可変長）*/
/*  RETURN CODE     : void                                                   */
/*  DESCRIPTION     : EMS出力処理                                            */
/*****************************************************************************/
void NWM_CTU_EMS_out(
    short msg_id,       /* メッセージID          */
    char  msg_type,     /* メッセージ通知区分    */
    char *inter_errcd,
    oggz1in_def             *pst_ems_cmn,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add,   // EMS追加情報
    char *filename,
    char *action,
    char *pKeyval,
    size_t key_len,
    short gurdian_cd
)
{
    oggz1in_def             ems;            /* EMS出力構造体                */
    char text[256];
    va_list ap;

    // EMS情報設定
    memset(&ems, 0x20, sizeof(ems));    //領域初期化
    // 運用監視端末出力サーバI/Oエラー
    ems.subrcd      = '1';

    // 運用監視端末出力情報
    memcpy(&ems.uytrminf,& pst_ems_cmn->uytrminf, sizeof(ems.uytrminf));

    // EMS出力情報・業務共通メッセージ情報
    memcpy(&ems.uytrminf,& pst_ems_cmn->emsinf.emsgkinf, sizeof(ems.uytrminf));

    // EMS出力情報・リターンコード ('0'固定)
    ems.emsinf.rcd  = '0';

    // EMS出力情報・メッセージID
    snprintf(text, sizeof(text), "%05.05d", msg_id);
    memcpy(ems.emsinf.msgid, text, strlen(text));

    // EMS出力情報・業務共通メッセージ・メッセージ通知区分
    ems.emsinf.emsgkinf.msgttkb = msg_type;

    // EMS出力情報・DTP内部エラーコード
    /*  GFP内部エラーコード       */
    memcpy ( ems.emsinf.emsgkinf.inter_errcd, inter_errcd, sizeof(ems.emsinf.emsgkinf.inter_errcd));

    // 任意メッセージ・サーバクラス論理ID
    memcpy(ems.emsinf.emsnninf.msgtbl[0].msgtbl_vl, pst_ems_add->srv_logical_id, 8);

    // 任意メッセージ・GFP内部LCN
    memcpy(ems.emsinf.emsnninf.msgtbl[1].msgtbl_vl, pst_ems_add->srv_logical_id, 15);

    // 任意メッセージ・接続先
    memcpy(ems.emsinf.emsnninf.msgtbl[2].msgtbl_vl, pst_ems_add->connect, 24);

    // 任意メッセージ・ファイル論理名
    memcpy(ems.emsinf.emsnninf.msgtbl[3].msgtbl_vl, filename, strlen(filename));

    // 任意メッセージ・アクション
    memcpy(ems.emsinf.emsnninf.msgtbl[4].msgtbl_vl, action, strlen(action));

    // 任意メッセージ・キー
    memcpy(ems.emsinf.emsnninf.msgtbl[5].msgtbl_vl, pKeyval, key_len);

    // 任意メッセージ・エラーコード
    sprintf(text, "%5.5d", gurdian_cd);
    memcpy(ems.emsinf.emsnninf.msgtbl[6].msgtbl_vl, text, strlen(text));

    /* EMS出力モジュールの呼び出し */
    GFPOGGZ1(&ems);

}
