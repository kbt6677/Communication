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
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#ifdef _TANDEM_SOURCE
#include <cextdecs.h(PROCESSHANDLE_NULLIT_, PROCESSHANDLE_GETMINE_)> nolist
#include <cextdecs.h(PROCESS_GETPAIRINFO_, PROCESSHANDLE_DECOMPOSE_)> nolist
#else
#include <cextdecs.h> nolist
#endif
/* USER HEADER     */
#include <GFPCGXD0.h> nolist
#include <common.h> nolist
#include <errcd.h> nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_util.h" nolist

/*****************************************************************************/
/*  FUNCTION        :cncl_initial_myinfo                                     */
/*  CALLING SEQ.    :short cncl_initial_myinfo(myinfo_def *myinfo)           */
/*  ARGUMENT        :myinfo:プロセス情報を格納する構造体                     */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :環境変数やプロセスハンドルなどを取得しmyinfoを初期化    */
/*                  :する                                                    */
/*****************************************************************************/
short cncl_initial_myinfo(myinfo_def *myinfo)
{
    short  s_err;
    short  s_count;
    char   node_name[ZSYS_VAL_LEN_SYSTEMNAME + 1];
    char  *pch_ptr;
    short  s_len;
    size_t sz_len;
    size_t r_len;
    long   ems_msg_psend_timer_val;
    enum
    {
        srv_logical_id,
        ems_msg_mon_name,
        ems_msg_srv_name,
        ems_msg_psend_timer,
        file_io_timer,
        psend_io_timer,
        proc_io_timer,
        socket_io_timer,
        psend_retry_cnt,
        so_rcvbuff,
        so_sndbuff,
        server_config_max
    } server_config                    = srv_logical_id;
    //パラメータ定義
    char  *server_config_param[]       = {DEF_SRV_LOGICAL_ID,
                                          DEF_MSG_MON_NAME,
                                          DEF_MSG_SRV_NAME,
                                          DEF_PSEND_TIMER_10MSECOND,
                                          DEF_FILE_IO_TIMER_10MSECOND,
                                          DEF_PSEND_TIMER_10MSECOND,
                                          DEF_PROC_IO_TIMER_10MSECOND,
                                          DEF_SOCKET_IO_TIMER_10MSECOND,
                                          DEF_PSEND_RETRY_CNT,
                                          "SO-RCVBUFF",
                                          "SO-SNDBUFF"};
    size_t server_config_param_len[]   = {sizeof(myinfo->srv_clsId) - 1,
                                          sizeof(myinfo->uytrmmon) - 1,
                                          sizeof(myinfo->uytrmsrv) - 1,
                                          sizeof(myinfo->uytrmtimer) - 1,
                                          10,
                                          10,
                                          10,
                                          10,
                                          6,
                                          10,
                                          10};
    void  *server_config_param_value[] = {(void *)&myinfo->srv_clsId,
                                          (void *)myinfo->uytrmmon,
                                          (void *)myinfo->uytrmsrv,
                                          (void *)myinfo->uytrmtimer,
                                          (void *)&(myinfo->file_io_timer),
                                          (void *)&(myinfo->pathsend_io_timer),
                                          (void *)&(myinfo->process_io_timer),
                                          (void *)&(myinfo->socket_io_timer),
                                          (void *)&(myinfo->psend_retry_cnt),
                                          (void *)&(myinfo->so_rcvbuff),
                                          (void *)&(myinfo->so_sndbuff)};

    memset(myinfo, 0, sizeof(myinfo_def));

    /*メッセージ出力用に初期値設定*/
    myinfo->pathsend_io_timer = 1000;
    sprintf(myinfo->uytrmtimer, "%04ld", myinfo->pathsend_io_timer / 100);

    PROCESSHANDLE_NULLIT_(myinfo->my_handle);
    PROCESSHANDLE_NULLIT_(myinfo->creator_phandle);
    PROCESSHANDLE_GETMINE_(myinfo->my_handle);
    PROCESSHANDLE_DECOMPOSE_(myinfo->my_handle, no_param, no_param, no_param, node_name, ZSYS_VAL_LEN_SYSTEMNAME,
                             &s_len, myinfo->my_name, ZSYS_VAL_LEN_PROCESSNAME, &s_count, no_param);
    node_name[s_len]         = 0;
    myinfo->my_name[s_count] = 0;
    myinfo->my_name_len      = s_count;
    s_err = PROCESS_GETPAIRINFO_(myinfo->my_handle, no_param, no_param, no_param, no_param, no_param, no_param,
                                 myinfo->creator_phandle, no_param, no_param, no_param, no_param, no_param, no_param,
                                 no_param);
    if (s_err != 4 && s_err != 5) {
        cncl_ems_procedure_error("PROCESS_GETPAIRINFO_", s_err, DEF_NERR_NOMAL);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_NOMAL);
    }
    PROCESSHANDLE_DECOMPOSE_(myinfo->creator_phandle, no_param, no_param, no_param, no_param, no_param, no_param,
                             myinfo->pathmon_name, ZSYS_VAL_LEN_PROCESSNAME, &s_count, no_param);
    myinfo->pathmon_name[s_count] = 0;

    while (server_config < server_config_max) {
        pch_ptr = getenv(server_config_param[server_config]);
        if (!pch_ptr) {
            switch (server_config) {
                case ems_msg_psend_timer:
                case psend_io_timer:
                case so_rcvbuff:
                case so_sndbuff:
                    break;
                case file_io_timer:
                case proc_io_timer:
                    *(long *)server_config_param_value[server_config] = 1000;
                    break;
                default:
                    cncl_ems_param_err(server_config_param[server_config], 0, DEF_NERR_PRM_RD_ERR);
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR);
                    break;
            }
            server_config++;
            continue;
        }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
        switch (server_config) {
            case ems_msg_mon_name:
            case ems_msg_srv_name:
            case srv_logical_id:
                sz_len = strlen(pch_ptr);
                if (sz_len > server_config_param_len[server_config]) {
                    cncl_ems_param_err(server_config_param[server_config], 0, DEF_NERR_PRM_RD_ERR_INV);
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
                }
                snprintf(server_config_param_value[server_config], server_config_param_len[server_config] + 1, "%-*s",
                         (int)server_config_param_len[server_config], pch_ptr);
                switch (server_config) {
                    case ems_msg_mon_name:
                        sprintf(myinfo->uytrmmonlen, "%02ld", (long)sz_len);
                        break;
                    case ems_msg_srv_name:
                        sprintf(myinfo->uytrmsrvlen, "%02ld", (long)sz_len);
                        break;
                }
#pragma clang diagnostic pop
                break;
            case ems_msg_psend_timer:
                sz_len                  = strlen(pch_ptr);
                ems_msg_psend_timer_val = str2ul_c(pch_ptr, sz_len, &r_len, &s_err);
                if (s_err || sz_len != r_len || sz_len > server_config_param_len[server_config]) {
                    cncl_ems_param_err(server_config_param[server_config], s_err, DEF_NERR_PRM_RD_ERR_INV);
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
                }
                snprintf(server_config_param_value[server_config], server_config_param_len[server_config] + 1, "%0*ld",
                         (int)server_config_param_len[server_config], ems_msg_psend_timer_val);
                break;
            case file_io_timer:
            case psend_io_timer:
            case proc_io_timer:
            case socket_io_timer:
            case so_rcvbuff:
            case so_sndbuff:
                sz_len                                            = strlen(pch_ptr);
                *(long *)server_config_param_value[server_config] = str2ul_c(pch_ptr, sz_len, &r_len, &s_err);
                if (s_err && r_len != sz_len) {
                    cncl_ems_param_err(server_config_param[server_config], s_err, DEF_NERR_PRM_RD_ERR_INV);
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
                }
                break;
            case psend_retry_cnt:
                sz_len                                             = strlen(pch_ptr);
                *(short *)server_config_param_value[server_config] = (short)str2ul_c(pch_ptr, sz_len, &r_len, &s_err);
                if (s_err && r_len != sz_len) {
                    cncl_ems_param_err(server_config_param[server_config], s_err, DEF_NERR_PRM_RD_ERR_INV);
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
                }
                break;
        }
        server_config++;
    }

    //サーバにならい、OpenTimerはprocess IOに同じ
    myinfo->nowait_open_timer = myinfo->process_io_timer;

    myinfo->srv_clsId.fil1 = '\0';
    myinfo->srv_clsId.fil2 = '\0';
    myinfo->srv_clsId.fil3 = '\0';
    myinfo->srv_clsId.fil4 = '\0';
    myinfo->srv_clsId.fil_null = '\0';

    char assign_name[DEF_COM_ASN_ID_LEN + 1];
    snprintf(assign_name, DEF_COM_ASN_ID_LEN, "%-*s", DEF_COM_ASN_ID_LEN, DEF_ASN_GFPHI);
    memset(myinfo->GFPHI_name, 0, sizeof(myinfo->GFPHI_name));
    COM_ASN(assign_name, myinfo->GFPHI_name, &myinfo->GFPHI_name_len);
    if (myinfo->GFPHI_name_len == 0) {
        cncl_ems_param_err(DEF_ASN_GFPHI, 0, DEF_NERR_PRM_RD_ERR_INV);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
    }
    return 0;
}
