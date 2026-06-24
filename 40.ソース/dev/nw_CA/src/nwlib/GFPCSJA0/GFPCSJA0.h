/**
 * @brief カット対象日付取得・更新[CARDNET]・・ヘッダファイル
 *
 * @date 2025/03/31 新規作成 by tatsuya.sugisaki
*/
#ifndef _NWM_CTU_H
#define _NWM_CTU_H

#include "GFPCGX50.h"
#include "GFPCGXB0.h"
#include "NWM_CTU.h"

#define     DEF_NWM_CTU_GYOM_ERR       'E'
#define     DEF_NWM_CTU_NERR_FILE_IO_ERR                "SCAA001"            /* ファイルIOエラー                           */

/*----------------------------------------------------------------------------*/
/* 非公開モジュール                                                           */
/*----------------------------------------------------------------------------*/
static short NWM_CTU_gfphi_read(
    NWM_CTU_INI_arg_1_def   *pst_pname_inf,     // 物理名情報ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_file_lgc_name, // ファイル論理名
    char                    *pch_module_id,     // モジュールID
    COM_IOM_arg_6_def       *pst_out_inf,       // 出力情報
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf    // EMS追加情報
);
static short NWM_CTU_gccut_open(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf    // EMS追加情報
);

static short NWM_CTU_gccut_read(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    short                   sh_lockmode,        // ロックモード
    char                    *pst_out_inf        // 出力情報
);

static short NWM_CTU_gccut_update(
    NWM_CTU_INI_arg_2_def   *pst_cut_file_inf,  // カット対象日付管理ファイル情報
    NWM_CTU_INI_arg_3_def   *pst_nw_inf,        // ネットワーク特定情報
    char                    *pch_module_id,     // モジュールID
    oggz1in_def             *pst_ems_cmn_inf,   // EMS共通情報
    NWM_CTU_INI_arg_6_def   *pst_ems_add_inf,   // EMS追加情報
    char                    *pch_cutdate        // カット日付
);
static void NWM_CTU_EMS_out(
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
);

#endif
