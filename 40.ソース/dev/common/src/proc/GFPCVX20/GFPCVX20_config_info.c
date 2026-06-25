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
#include <stdlib.h>
#include <string.h> nolist
#include "GFPCGXB0.h"
/* USER HEADER     */
#include <errcd.h> nolist

#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_config_info.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_util.h" nolist
#include "common.h" nolist

/****************************************************************************/
/*   グローバル変数定義                                                     */
/****************************************************************************/
// コンフィグ管理
nw_conf_factory_t  my_nw_conf_factory;
nw_conf_factory_t *nw_conf_factory = &my_nw_conf_factory;

/*****************************************************************************/
/*  FUNCTION        :cncl_load_config                                        */
/*  CALLING SEQ.    :nw_conf_t *cncl_load_config(nw_conf_factory_t           */
/*                   *nw_conf_factory)                                       */
/*  ARGUMENT        :nw_conf_factory:NWコンフィグファクトリ情報              */
/*  RETURN CODE     :nw_conf_tへのポインタ, NULL(エラー時)                   */
/*  DESCRIPTION     :GFPHIファイル等を読み込みネットワーク設定を取得         */
/*****************************************************************************/
nw_conf_t *cncl_load_config(nw_conf_factory_t *nw_conf_factory)
{
    nw_conf_t     *current_conf;
    sel_conf_ind_t current_conf_ind = nw_conf_factory->active_conf;
    iom_params_def gfphi_io;
    Application   *app    = cncl_get_App();
    myinfo_def    *myInfo = &(app->my_info);
    db_gfphi_def   db_gfphi, *p_db_gfphi = (db_gfphi_def *)gfphi_io.arg6.rec_area;
    short          s_err;

    db_gfnwi_def  *gfnwi_work       = NULL;
    long           gfnwi_work_cnt   = 0;
    size_t         merged_gfnwi_cnt = DEF_MAX_GFNWI;

    switch (nw_conf_factory->active_conf) {
        case null_conf:
            nw_conf_factory->active_conf = primary_conf;
            break;
        case primary_conf:
            nw_conf_factory->active_conf = secondary_conf;
            break;
        case secondary_conf:
            nw_conf_factory->active_conf = primary_conf;
            break;
        default:
            break;
    }
    current_conf           = &(nw_conf_factory->nw_conf[nw_conf_factory->active_conf]);
    current_conf->conf_ind = nw_conf_factory->active_conf;
    while (1) {
        cncl_initial_ioparams(&gfphi_io);
        cncl_preset_ioparams(&gfphi_io, myInfo->GFPHI_name, DEF_GFPHI, DEF_FL_PHSIC_INFO);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_OPEN, no_param, no_param, no_param, no_param, no_param,
                              no_param, no_param, no_param, no_param, no_param, no_param, no_param, 0);
        s_err = COM_IOM_MAC(&gfphi_io);
        if (s_err) {
            cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }

        // NW情報ファイル GFNWI ファイル名取得
        memset(p_db_gfphi, ' ', sizeof(db_gfphi_def));
        cncl_gfphi_key(&db_gfphi, myInfo->srv_clsId.site, myInfo->srv_clsId.network, myInfo->srv_clsId.group,
                       DEF_SC_NAME_DEFAULT, DEF_SC_NUM_DEFAULT, DEF_SC_DUP_DEFAULT, DEF_FL_NW_INFO, DEF_ST_0000,
                       DEF_ST_0000);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)sizeof(db_gfphi.pri_key), DEF_COM_IOM_EXACT, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK) {
            if (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_EOF)
                cncl_ems_config_error(&db_gfphi, sizeof(db_gfphi.pri_key), DEF_FL_PHSIC_INFO, DEF_NERR_PRM_RD_ERR);
            else
                cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }
        memcpy(&(current_conf->gfnwi_info), p_db_gfphi, sizeof(db_gfphi_def));

        // 回線管理情報ファイル名 GFLIN ファイル名取得
        cncl_gfphi_key(&db_gfphi, no_param, no_param, no_param, no_param, no_param, no_param, DEF_FL_LIN_MG, no_param,
                       no_param);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)sizeof(db_gfphi.pri_key), DEF_COM_IOM_EXACT, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK) {
            if (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_EOF)
                cncl_ems_config_error(&db_gfphi, sizeof(db_gfphi.pri_key), DEF_FL_PHSIC_INFO, DEF_NERR_PRM_RD_ERR);
            else
                cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }
        memcpy(&(current_conf->gflin_info), p_db_gfphi, sizeof(db_gfphi_def));

        // 回線ステータスファイル名 GCLST ファイル名取得
        cncl_gfphi_key(&db_gfphi, no_param, no_param, no_param, no_param, no_param, no_param, DEF_FL_LIN_STS, no_param,
                       no_param);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)sizeof(db_gfphi.pri_key), DEF_COM_IOM_EXACT, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK) {
            if (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_EOF)
                cncl_ems_config_error(&db_gfphi, sizeof(db_gfphi.pri_key), DEF_FL_PHSIC_INFO, DEF_NERR_PRM_RD_ERR);
            else
                cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }
        memcpy(&(current_conf->gclst_info), p_db_gfphi, sizeof(db_gfphi_def));

        // pathmon名とサーバクラス名を取得
        cncl_gfphi_key(&db_gfphi, myInfo->srv_clsId.site, myInfo->srv_clsId.network, myInfo->srv_clsId.group,
                       myInfo->srv_clsId.server_class_name, myInfo->srv_clsId.server_class_num, DEF_ST_0000,
                       DEF_SC_NAME_DEFAULT, DEF_SC_NUM_DEFAULT, DEF_SC_DUP_DEFAULT);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)sizeof(db_gfphi.pri_key), DEF_COM_IOM_EXACT, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK) {
            if (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_EOF)
                cncl_ems_config_error(&db_gfphi, sizeof(db_gfphi.pri_key), DEF_FL_PHSIC_INFO, DEF_NERR_PRM_RD_ERR);
            else
                cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }
        memmove(current_conf->pathmon_name, p_db_gfphi->srv_cls_info.pathmon_name, sizeof(current_conf->pathmon_name));
        cncl_rm_trspc_wn(current_conf->pathmon_name, sizeof(current_conf->pathmon_name));
        memmove(current_conf->my_server_class, p_db_gfphi->srv_cls_info.srv_cls_name,
                sizeof(current_conf->my_server_class));
        cncl_rm_trspc_wn(current_conf->my_server_class, sizeof(current_conf->my_server_class));

        // コマンドI/Fサーバクラス
        cncl_gfphi_key(&db_gfphi, no_param, no_param, no_param, DEF_SC_CMD_SRV, DEF_ST_0000, DEF_ST_0000,
                       DEF_SC_NAME_DEFAULT, DEF_SC_NUM_DEFAULT, DEF_SC_DUP_DEFAULT);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)sizeof(db_gfphi.pri_key), DEF_COM_IOM_EXACT, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK) {
            if (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_EOF)
                cncl_ems_config_error(&db_gfphi, sizeof(db_gfphi.pri_key), DEF_FL_PHSIC_INFO, DEF_NERR_PRM_RD_ERR);
            else
                cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }
        memmove(&(current_conf->cmd_if_srvcls), p_db_gfphi, sizeof(db_gfphi_def));

        // 電文振り分(inboud)サーバクラス
        cncl_gfphi_key(&db_gfphi, no_param, no_param, no_param, DEF_SC_FURI_I, DEF_ST_0000, DEF_ST_0000,
                       DEF_SC_NAME_DEFAULT, DEF_SC_NUM_DEFAULT, DEF_SC_DUP_DEFAULT);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)sizeof(db_gfphi.pri_key), DEF_COM_IOM_EXACT, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK) {
            if (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_EOF)
                cncl_ems_config_error(&db_gfphi, sizeof(db_gfphi.pri_key), DEF_FL_PHSIC_INFO, DEF_NERR_PRM_RD_ERR);
            else
                cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
            break;
        }
        memmove(&(current_conf->in_dist_srvcls), p_db_gfphi, sizeof(db_gfphi_def));

        // 電文振り分(outbound)プロセス×n
        // 仮に100件以上あっても無視する。
        //srv_cls_kindまでキーを指定して検索し、prc_file_kindがDEF_PRC_FURI_Oのレコードのみ対象とする。
        cncl_gfphi_key(&db_gfphi, no_param, no_param, no_param, DEF_SC_FURI_O, DEF_ST_0000, DEF_ST_0000, DEF_PRC_FURI_O,
                       DEF_ST_0000, DEF_ST_0000);
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                              (char *)&(db_gfphi.pri_key), DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfphi.pri_key),
                              (short)offsetof(db_gfphi_def,pri_key.srv_cls_key.srv_cls_id.srv_cls_num), DEF_COM_IOM_GENERIC, DEF_COM_IOM_NOLOCK,
                              DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfphi_def));
        s_err = COM_IOM_MAC(&gfphi_io);
        while (gfphi_io.arg6.guardian_errcode == ZFIL_ERR_OK) {
            if(memcmp(p_db_gfphi->pri_key.prc_file_key.prc_file_id.prc_file_kind,DEF_PRC_FURI_O,sizeof(DEF_PRC_FURI_O) - 1) == 0){
                if (current_conf->out_dist_procs_count >= DEF_MAX_OUTP) break;
                current_conf->out_dist_procs_count++;
                memmove(&(current_conf->out_dist_procs[current_conf->out_dist_procs_count - 1]), p_db_gfphi,
                        sizeof(db_gfphi_def));
            }
            cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_NEXTREAD, no_param, no_param, no_param, no_param,
                                no_param, no_param, no_param, no_param, no_param, no_param, no_param, no_param,
                                sizeof(db_gfphi_def));
            s_err = COM_IOM_MAC(&gfphi_io);
        }
        if (gfphi_io.arg6.guardian_errcode != ZFIL_ERR_OK && gfphi_io.arg6.guardian_errcode != ZFIL_ERR_EOF) {
            cncl_ems_common_module_error("COM_IOM", gfphi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
        }
        // 物理情報ファイルCLOSE
        cncl_prepare_ioparams(&gfphi_io, DEF_COM_IOM_FUNC_CLOSE, no_param, no_param, no_param, no_param, no_param,
                              no_param, no_param, no_param, no_param, no_param, no_param, no_param, 0);
        COM_IOM_MAC(&gfphi_io);

        memmove(current_conf->GCLST_name, current_conf->gclst_info.prc_file_info.prc_file_name, sizeof(filename_p_t));
        cncl_rm_trspc_wn(current_conf->GCLST_name, sizeof(filename_p_t));
        current_conf->GCLST_name_len = (short)strlen(current_conf->GCLST_name);

        memmove(current_conf->GFLIN_name, current_conf->gflin_info.prc_file_info.prc_file_name, sizeof(filename_p_t));
        cncl_rm_trspc_wn(current_conf->GFLIN_name, sizeof(filename_p_t));
        current_conf->GFLIN_name_len = (short)strlen(current_conf->GFLIN_name);

        memmove(current_conf->GFNWI_name, current_conf->gfnwi_info.prc_file_info.prc_file_name, sizeof(filename_p_t));
        cncl_rm_trspc_wn(current_conf->GFNWI_name, sizeof(filename_p_t));
        current_conf->GFNWI_name_len = (short)strlen(current_conf->GFNWI_name);

        s_err = cncl_collect_gflin_recs(current_conf->gflin, DEF_MAX_GFLIN, &(current_conf->gflin_count),
                                        myInfo->srv_clsId.server_class_name, myInfo->srv_clsId.server_class_num,
                                        current_conf->GFLIN_name);
        if (s_err) {
            // 関数内でエラーを吐いているので中断のみ、ここでは何もしない
            break;
        }
        // 回線数にあわせてメモリを拡張する。(縮小はしない)
        // io_memがinitializeされていない場合は、trueを返す。(cncl_initialで初期化する)
        if(!reInitial_io_mem(&io_mem,current_conf->gflin_count))
        {
            cncl_ems_procedure_error("cncl_load_config", 0, DEF_NERR_CNCL_RES_XHAUST);
            break;
        }

        //        s_err = cncl_collect_gfnwi_recs(current_conf->gfnwi, DEF_MAX_GFNWI, &(current_conf->gfnwi_count),
        //                                        myInfo->srv_clsId.site, myInfo->srv_clsId.network,
        //                                        myInfo->srv_clsId.group, current_conf->GFNWI_name);
        gfnwi_work = calloc(DEF_MAX_GFNWI * 3, sizeof(db_gfnwi_def));
        if (!gfnwi_work) {
            cncl_ems_procedure_error("cncl_load_config", 0, DEF_NERR_CNCL_RES_XHAUST);
            break;
        }
        // 回線数200(CCC_MAX_connection)に対して、最大でステーションレコード200、インターフェースレコード200，グループレコード1の401レコードを
        // 読込む可能性がある。保険をかけて+100
        s_err = cncl_collect_gfnwi_recs(gfnwi_work, DEF_MAX_GFNWI * 3, &gfnwi_work_cnt, myInfo->srv_clsId.site,
                                        myInfo->srv_clsId.network, myInfo->srv_clsId.group, current_conf->GFNWI_name,
                                        current_conf->gflin, current_conf->gflin_count);
        if (s_err) {
            // 関数内でエラーを吐いているので中断のみ、ここでは何もしない
            break;
        }
        s_err = cncl_sort_marge(current_conf->gfnwi, &merged_gfnwi_cnt, gfnwi_work, gfnwi_work_cnt);
        if (s_err < 0) {
            cncl_ems_procedure_error("cncl_sort_marge", 0, DEF_NERR_CNCL_RES_XHAUST);
            break;
        }
        current_conf->gfnwi_count = (long)merged_gfnwi_cnt;
        free(gfnwi_work);
        return current_conf;
    }
    // エラー時の共通処理
    nw_conf_factory->active_conf = current_conf_ind;
    free(gfnwi_work);
    if (current_conf_ind == null_conf) {
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
    }
    return NULL;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_chk_gfnwi_key                                      */
/*  CALLING SEQ.    :bool cncl_chk_gfnwi_key(                                */
/*                   db_gfnwi_def *p_db_gfnwi,                               */
/*                   db_gflin_def *db_gflin_buff,                            */
/*                   long db_gflin_num)                                      */
/*  ARGUMENT        :p_db_gfnwi   : gfnwiレコードへのポインタ                */
/*                   db_gflin_buff: 回線管理ファイルのバッファ先頭ポインタ   */
/*                   db_gflin_num : 回線管理ファイルのレコード件数           */
/*  RETURN CODE     :true = キー一致あり, false = キー不一致                 */
/*  DESCRIPTION     :gfnwiの省略されていないキー部分が回線管理ファイルに     */
/*                   含まれているかを確認する。                              */
/*                   ・if_idレコードならstation_idまでのキーを比較           */
/*                   ・Stationレコードならconnect_idまでのキーを比較         */
/*                   ・Group IDは事前チェック済みのため常にtrueを返す        */
/*                   ・全レコードを走査し、キー一致ならtrue、不一致ならfalse */
/*****************************************************************************/
//from GFPCVX20_cncl_sort_marge.c
int is_if_id_rec(const db_gfnwi_def *r);
int is_station_rec(const db_gfnwi_def *r);
//* gfnwiの省略されていないキー部分が読込んだ回線管理ファイルの中に含まれていることを確認する */
bool cncl_chk_gfnwi_key(db_gfnwi_def *p_db_gfnwi, db_gflin_def *db_gflin_buff, long db_gflin_num)
{
    int           key_length = 0;
    db_gflin_def *gflin_rec;
    if (is_if_id_rec(p_db_gfnwi)) {
        key_length = (int)offsetof(db_gflin_def, pri_key.station_id);
    } else if (is_station_rec(p_db_gfnwi)) {
        key_length = (int)offsetof(db_gflin_def, pri_key.connect_id);
    } else {
        // グループIDはチェック済なので常に取得対象
        return true;
    }
    for (long gflin_index = 0; gflin_index < db_gflin_num; gflin_index++) {
        gflin_rec = db_gflin_buff + gflin_index;
        if (!memcmp(gflin_rec, p_db_gfnwi, key_length)) return true;
    }
    return false;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_collect_gfnwi_recs                                 */
/*  CALLING SEQ.    :short cncl_collect_gfnwi_recs(db_gfnwi_def *db_gfnwi_buff,*/
/*                   long buff_cnt,                                          */
/*                   long *read_count,                                       */
/*                   char site_id,                                           */
/*                   char nw_id,                                             */
/*                   group_t grp_id,                                         */
/*                   filename_p_t filename)                                  */
/*  ARGUMENT        :db_gfnwi_buff:読み込み先バッファ                        */
/*                  :buff_cnt     :最大レコード数                            */
/*                  :read_count   :実際に読み込んだ数                        */
/*                  :site_id      :サイトID                                  */
/*                  :nw_id        :NW ID                                     */
/*                  :grp_id       :グループID                                */
/*                  :filename     :GFNWIファイル名                           */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :GFNWIファイルから必要なレコードを読み込む               */
/*****************************************************************************/
short cncl_collect_gfnwi_recs(db_gfnwi_def *db_gfnwi_buff, long buff_cnt, long *read_count, char site_id, char nw_id,
                              group_t grp_id, filename_p_t filename,db_gflin_def *db_gflin_buff, long gflin_num)
{
    iom_params_def gfnwi_io;
    db_gfnwi_def   db_gfnwi, *p_db_gfnwi = (db_gfnwi_def *)gfnwi_io.arg6.rec_area;
    Application   *app              = cncl_get_App();
    myinfo_def    *myInfo           = &(app->my_info);
    short         *guardian_errcode = &(gfnwi_io.arg6.guardian_errcode);

    if (buff_cnt <= 0 || db_gfnwi_buff == NULL || read_count == NULL) return -1;
    *read_count = 0;

    // GFNWIファイルをOpen
    cncl_initial_ioparams(&gfnwi_io);
    cncl_preset_ioparams(&gfnwi_io, filename, DEF_GFNWI, DEF_FL_LIN_MG);
    cncl_prepare_ioparams(&gfnwi_io, DEF_COM_IOM_FUNC_OPEN, no_param, no_param, no_param, no_param, no_param, no_param,
                          no_param, no_param, no_param, no_param, no_param, no_param, 0);
    COM_IOM_MAC(&gfnwi_io);
    if (*guardian_errcode != ZFIL_ERR_OK) {
        cncl_ems_common_module_error("COM_IOM", gfnwi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_OPN_ERR);
        return *guardian_errcode;
    }

    // 読み込みKeyを設定して、GFNWIファイルの１件目をRead
    cncl_db_gfif_key(&db_gfnwi, site_id, nw_id, grp_id, no_param, no_param);
    cncl_prepare_ioparams(
        &gfnwi_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0, (char *)&(db_gfnwi.pri_key),
        DEF_COM_IOM_KEYTYPE_PRI, (short)sizeof(db_gfnwi.pri_key),
        (short)(sizeof(db_gfnwi.pri_key) - sizeof(db_gfnwi.pri_key.if_id) - sizeof(db_gfnwi.pri_key.station_id)),
        DEF_COM_IOM_GENERIC, DEF_COM_IOM_NOLOCK, DEF_COM_IOM_ASCEND, myInfo->file_io_timer, no_param, sizeof(db_gfnwi_def));
    COM_IOM_MAC(&gfnwi_io);
    if (*guardian_errcode != ZFIL_ERR_OK) {
        if (*guardian_errcode == ZFIL_ERR_EOF)
            cncl_ems_config_error(&db_gfnwi, sizeof(db_gfnwi.pri_key), DEF_FL_NW_INFO, DEF_NERR_PRM_RD_ERR);
        else
            cncl_ems_common_module_error("COM_IOM", gfnwi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
        return *guardian_errcode;
    }

    // GFNWIファイルの２件目以降をLoop。
    while (*guardian_errcode == ZFIL_ERR_OK) {
        if (*read_count >= buff_cnt) break;  // 上限到達チェック
        // gflinに存在するインタフェースか、ステーションのみ抽出
        if (cncl_chk_gfnwi_key(p_db_gfnwi, db_gflin_buff, gflin_num)) {
            memmove(db_gfnwi_buff, p_db_gfnwi, sizeof(db_gfnwi_def));
            db_gfnwi_buff++;
            (*read_count)++;
        }

        // GFNWIファイルの次レコードをRead。
        cncl_prepare_ioparams(&gfnwi_io, DEF_COM_IOM_FUNC_NEXTREAD, no_param, no_param, no_param, no_param, no_param,
                              no_param, no_param, no_param, no_param, no_param, no_param, no_param, sizeof(db_gfnwi_def));
        COM_IOM_MAC(&gfnwi_io);
    }
    if (*guardian_errcode != ZFIL_ERR_OK && *guardian_errcode != ZFIL_ERR_EOF) {
        cncl_ems_common_module_error("COM_IOM", gfnwi_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
        return *guardian_errcode;
    }

    // GFNWIファイルをClose
    cncl_prepare_ioparams(&gfnwi_io, DEF_COM_IOM_FUNC_CLOSE, no_param, no_param, no_param, no_param, no_param, no_param,
                          no_param, no_param, no_param, no_param, no_param, no_param, 0);
    COM_IOM_MAC(&gfnwi_io);
    return 0;
}

/****************************************************************************/
/*  FUNCTION        : cncl_chk_gflin_key                                    */
/*  CALLING SEQ.    : bool cncl_chk_gflin_key                               */
/*                           (db_gflin_def *db_gflin_key,                   */
/*                            char site_id,                                 */
/*                            char nw_id,                                   */
/*                            char grp_id[5])                               */
/*                                                                          */
/*  ARGUMENT        : db_gflin_key [in]                                     */
/*                      検査対象のGFLINキー構造体ポインタ                   */
/*                                             （db_gflin_def構造体）       */
/*                  : site_id [in]                                          */
/*                      サイト識別子                                        */
/*                  : nw_id [in]                                            */
/*                      ネットワーク識別子                                  */
/*                  : grp_id [in]                                           */
/*                      グループ識別子（5バイト配列）                       */
/*                                                                          */
/*  RETURN CODE     : true  - すべての項目が一致(対象レコード)              */
/*                  : false - 一部でも一致しない(対象外)                    */
/*                                                                          */
/*  DESCRIPTION     : GFLINの主キー情報（site_id, nw_id, grp_id, connect_id）*/
/*                    が一致し、処理対象であるか確認する。                  */
/*                    connect_id は "CC"（CNCL_CONNID_CLIENT）              */
/*                    で、コネクション制御クライアントのみを対象とする(固定)*/
/****************************************************************************/
bool cncl_chk_gflin_key(db_gflin_def *db_gflin_key, char site_id, char nw_id, char grp_id[5])
{
    bool result = true;
    result = result && (db_gflin_key->pri_key.site_id == site_id);
    result = result && (db_gflin_key->pri_key.nw_id == nw_id);
    result = result && !memcmp(db_gflin_key->pri_key.grp_id, grp_id, sizeof(db_gflin_key->pri_key.grp_id));
    result = result && !memcmp(db_gflin_key->pri_key.connect_id, CNCL_CONNID_CLIENT, sizeof(CNCL_CONNID_CLIENT) - 1);
    return result;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_collect_gflin_recs                                 */
/*  CALLING SEQ.    :short cncl_collect_gflin_recs(db_gflin_def *db_gflin_buf*/
/*                   size_t buff_cnt,                                        */
/*                   long *read_count,                                       */
/*                   char site,                                              */
/*                   char network,                                           */
/*                   group_t group,                                          */
/*                   filename_p_t filename)                                  */
/*  ARGUMENT        :db_gflin_buff:読み込み先バッファ                        */
/*                  :buff_cnt     :最大レコード数                            */
/*                  :read_count   :実際に読み込んだ数                        */
/*                  :server_class_name :                                     */
/*                  :server_class_num  :                                     */
/*                  :filename     :GFLINファイル名                           */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :GFLINファイルから必要なレコードを読み込む               */
/*****************************************************************************/
short cncl_collect_gflin_recs(db_gflin_def *db_gflin_buff, size_t buff_cnt, long *read_count, char *server_class_name,
                              char *server_class_num, filename_p_t filename)
{
    iom_params_def gflin_io;
    gflin_a1_key_t gflin_a1_key;
    db_gflin_def  *p_db_gflin       = (db_gflin_def *)gflin_io.arg6.rec_area;
    Application   *app              = cncl_get_App();
    myinfo_def    *myInfo           = &(app->my_info);
    short         *guardian_errcode = &(gflin_io.arg6.guardian_errcode);

    if (buff_cnt <= 0 || db_gflin_buff == NULL || read_count == NULL) return -1;
    *read_count = 0;

    // GFLINファイルのOpen
    cncl_initial_ioparams(&gflin_io);
    cncl_preset_ioparams(&gflin_io, filename, DEF_GFLIN, DEF_FL_LIN_MG);
    cncl_prepare_ioparams(&gflin_io, DEF_COM_IOM_FUNC_OPEN, no_param, no_param, no_param, no_param, no_param, no_param,
                          no_param, no_param, no_param, no_param, no_param, no_param, 0);
    COM_IOM_MAC(&gflin_io);
    if (*guardian_errcode != ZFIL_ERR_OK) {
        cncl_ems_common_module_error("COM_IOM", gflin_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_OPN_ERR);
        return *guardian_errcode;
    }

    // 読み込みKeyを設定して、GFLINファイルの１件目をRead。
    cncl_db_gflin_a1_key(&gflin_a1_key, server_class_name, server_class_num);
    cncl_prepare_ioparams(&gflin_io, DEF_COM_IOM_FUNC_STARTREAD, DEF_COM_IOM_PARTITION_KEY_NOT, 0, 0,
                          (char *)&gflin_a1_key, DEF_COM_IOM_KEYTYPE_A1, (short)sizeof(gflin_a1_key_t),
                          (short)sizeof(gflin_a1_key_t), DEF_COM_IOM_GENERIC, DEF_COM_IOM_NOLOCK, DEF_COM_IOM_ASCEND,
                          myInfo->file_io_timer, no_param, sizeof(db_gflin_def));
    COM_IOM_MAC(&gflin_io);
    if (*guardian_errcode != ZFIL_ERR_OK) {
        if (*guardian_errcode == ZFIL_ERR_EOF) {
            cncl_ems_config_error(p_db_gflin, sizeof(p_db_gflin->pri_key), DEF_FL_LIN_MG, DEF_NERR_PRM_RD_ERR);
            return *guardian_errcode;
        } else
            cncl_ems_common_module_error("COM_IOM", gflin_io.arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
        return *guardian_errcode;
    }

    // GFLINファイルの２件目以降をLoop。
    while (*guardian_errcode == ZFIL_ERR_OK) {
        // サイト識別、NW識別、グループ識別が一致するレコードのみ抽出
        if (cncl_chk_gflin_key(p_db_gflin,myInfo->srv_clsId.site,myInfo->srv_clsId.network,myInfo->srv_clsId.group)
            && p_db_gflin->invalid_flg == DEF_INVALID_FLG_OFF) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
            if (*read_count >= buff_cnt) break;  // 上限到達チェック
#pragma clang diagnostic pop
            memmove(db_gflin_buff, p_db_gflin, sizeof(db_gflin_def));
            db_gflin_buff++;
            (*read_count)++;
        }

        // GFLINファイルの次レコードをRead。
        cncl_prepare_ioparams(&gflin_io, DEF_COM_IOM_FUNC_NEXTREAD, no_param, no_param, no_param, no_param, no_param,
                              no_param, no_param, no_param, no_param, no_param, no_param, no_param, sizeof(db_gflin_def));
        COM_IOM_MAC(&gflin_io);
    }
    if (*guardian_errcode != ZFIL_ERR_OK && *guardian_errcode != ZFIL_ERR_EOF) {
        cncl_ems_common_module_error("COM_IOM", *guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
        return *guardian_errcode;
    }
    cncl_prepare_ioparams(&gflin_io, DEF_COM_IOM_FUNC_CLOSE, no_param, no_param, no_param, no_param, no_param, no_param,
                          no_param, no_param, no_param, no_param, no_param, no_param, 0);
    COM_IOM_MAC(&gflin_io);
    return 0;
}
/****************************************************************************/
/*  FUNCTION        :cncl_chk_nw_conf                                       */
/*  CALLING SEQ.    :bool cncl_chk_nw_conf(nw_conf_t *nw_conf)              */
/*  ARGUMENT        :*nw_conf:nw_conf_factory:NWコンフィグファクトリ情報へ  */
/*                                            のポインタ                    */
/*                  :                                                       */
/*  RETURN CODE     :true:エラーなし,false:チェック違反あり                 */
/*  DESCRIPTION     :コンフィグの妥当性チェックを行う                       */
/****************************************************************************/
bool cncl_chk_nw_conf(nw_conf_t *nw_conf)
{
    bool result = true;
    for (int i = 0; i < nw_conf->gfnwi_count; i++) {
        if (cncl_chk_gfnwi_rec(&nw_conf->gfnwi[i])) {
            result = false;
        }
    }

    for (int i = 0; i < nw_conf->gflin_count; i++) {
        if (cncl_chk_gflin_rec(&nw_conf->gflin[i])) {
            result = false;
        }
    }

    for (int i = 0; i < nw_conf->out_dist_procs_count; i++) {
        if (cncl_chk_gfphi_rec(&nw_conf->out_dist_procs[i])) {
            result = false;
        }
    }

    if (cncl_chk_gfphi_rec(&nw_conf->in_dist_srvcls)) {
        result = false;
    }

    if (cncl_chk_gfphi_rec(&nw_conf->cmd_if_srvcls)) {
        result = false;
    }

    //    物理名情報ファイル名を物理名情報ファイルから取得するという論理はありえないので
    //    やはりこのパラメータは使用していない。
    //    if (cncl_chk_gfphi_rec(&nw_conf->gfphi_info)) {
    //        result = false;
    //    }

    if (cncl_chk_gfphi_rec(&nw_conf->gfnwi_info)) {
        result = false;
    }

    if (cncl_chk_gfphi_rec(&nw_conf->gflin_info)) {
        result = false;
    }

    if (cncl_chk_gfphi_rec(&nw_conf->gclst_info)) {
        result = false;
    }
    return result;
}
