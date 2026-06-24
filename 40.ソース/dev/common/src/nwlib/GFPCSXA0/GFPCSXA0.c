/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCSXA0                                    */
/*        FUNCTION          ････ NW個別 カット対象日付取得[CARDNET以外]      */
/*                                                                           */
/*                               制御電文振り分けから受信したI/Fに従い、     */
/*                               カット日付制御を行う。                      */
/*                                                                           */
/*        AUTHER            ････ HAS K.F                                     */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-21                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS        2025/04/21 新規作成                                      */
/*                                                                           */
/*****************************************************************************/
/* SYSTEM HEADER   */
#include <stdio.h>
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <ctype.h>    nolist
#include <stdbool.h>  nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist

/* COMMON HEADER   */
#include "common.h"
#include "ems.h"
#include "file.h"

/* USER HEADER     */
#include "NWM_CTU.h"                            /* カット対象日付取得・更新  */
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
    // カット対象日付管理ファイル情報．ファイルIDに、スペースを設定。
    memset(pst_cut_file_inf->file_id,0x20,sizeof(pst_cut_file_inf->file_id));

    // カット対象日付管理ファイル情報．ファイル名に、スペースをを設定。
    memset(pst_cut_file_inf->file_name,0x20,sizeof(pst_cut_file_inf->file_name));

    // カット対象日付管理ファイル情報．ファイル番号に、-1を設定。
    pst_cut_file_inf->file_no = -1;

    // 正常終了
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_GET                                     */
/*  CALLING SEQ.    : void NWM_CTU_GET(NWM_CTU_INI_arg_2_def *,              */
/*                                     NWM_CTU_INI_arg_3_def *,              */
/*                                     char *,oggz1in_def *,                 */
/*                                     NWM_CTU_INI_arg_6_def *,char *)       */
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
    /* VISA-NET では未処理 */
    return 0;
}

/*****************************************************************************/
/*  FUNCTION        : 1.0.0  NWM_CTU_UPDATE                                  */
/*  CALLING SEQ.    : short NWM_CTU_UPDATE(NWM_CTU_INI_arg_2_def *,          */
/*                                         NWM_CTU_INI_arg_3_def *,          */
/*                                         char *,oggz1in_def *,             */
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
    /* VISA-NET では未処理 */
    return 0;
}


