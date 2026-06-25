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
/*  1.0  ISYS 工藤   2025/03/01 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <string.h> nolist
/* USER HEADER     */
#include <GFPCGXB0.h>  nolist
#include <common.h> nolist
#include <errcd.h> nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_config_info.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_gclst_info.h" nolist
#include "GFPCVX20_sys.h" nolist
#include "GFPCVX20_util.h" nolist

/*****************************************************************************/
/*  FUNCTION        :cncl_gclst_info_open                                    */
/*  CALLING SEQ.    :short cncl_gclst_info_open(gclst_info_t *gclst_info)    */
/*  ARGUMENT        :gclst_info:回線ステータス管理オブジェクト               */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :GCLSTファイルをOPENしアクセス可能にする                 */
/*****************************************************************************/
short cncl_gclst_info_open(gclst_info_t *gclst_info)
{
    iom_params_def *gclst_io = &(gclst_info->gclst_io);
    nw_conf_t      *nw_conf  = cncl_get_nwcnf();
    filename_p_t    filename;
    short           s_err;

    cncl_initial_ioparams(gclst_io);
    memcpy(filename, nw_conf->gclst_info.prc_file_info.prc_file_name, sizeof(filename_p_t));
    cncl_rm_trspc_wn(filename, sizeof(filename_p_t));
    cncl_preset_ioparams(gclst_io, filename, DEF_GCLST, DEF_FL_LIN_STS);
    cncl_prepare_ioparams(gclst_io, DEF_COM_IOM_FUNC_OPEN, no_param, no_param, no_param, no_param, no_param, no_param,
                          no_param, no_param, no_param, no_param, no_param, no_param, 0);
    s_err = COM_IOM_MAC(gclst_io);
    if (gclst_io->arg6.guardian_errcode != ZFIL_ERR_OK) {
        cncl_ems_common_module_error("COM_IOM", gclst_io->arg6.guardian_errcode, no_param, DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
    }
    gclst_info->conf_ind = nw_conf->conf_ind;
    gclst_info->opened   = true;
    gclst_io->info       = gclst_info;
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_gclst_info_close                                   */
/*  CALLING SEQ.    :short cncl_gclst_info_close(gclst_info_t *gclst_info)   */
/*  ARGUMENT        :gclst_info:回線ステータス管理オブジェクト               */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :GCLSTファイルをCLOSEしリソースを解放する                */
/*****************************************************************************/
short cncl_gclst_info_close(gclst_info_t *gclst_info)
{
    short           s_err;
    iom_params_def *gclst_io = &(gclst_info->gclst_io);
    cncl_prepare_ioparams(gclst_io, DEF_COM_IOM_FUNC_CLOSE, no_param, no_param, no_param, no_param, no_param, no_param,
                          no_param, no_param, no_param, no_param, no_param, no_param, 0);
    s_err = COM_IOM_MAC(gclst_io);
    memset(gclst_info, 0, sizeof(gclst_info_t));
    return s_err;
}
