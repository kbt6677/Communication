/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
/*                               コマンドサーバー管理コンポーネント(ci)      */
/*                                                                           */
/*        AUTHER            ････ HAS hashimoto                               */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2024-09-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  HAS 橋本   2029/09/25 (コネクション制御)新規作成                    */

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */

/* USER HEADER     */
#include "GFPCVX1G.h" nolist
#include "GFPCVX1E.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/* 関数のﾌﾟﾛﾄﾀｲﾌﾟ宣言 */
#include "GFPCVX1P.h"
/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ci_manage                                 */
/*  CALLING SEQ.    : void CNSV_ci_manage ( Event_Node_def * )              */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンドサーバー管理処理                              */
/****************************************************************************/
void CNSV_ci_manage(Event_Node_def *Event)
{
    switch (Event->event)
    {
        case DEF_EV_cancel_msg:
            CNSV_ci_cancel();
            break;
        case DEF_EV_connect_req:
            CNSV_connect_req();
            break;
        case DEF_EV_disconnect_req:
            CNSV_disconnect_req();
            break;
        case DEF_EV_file_reload:
            CNSV_reload_command();
            break;
        case DEF_EV_signon_req:
            CNSV_ci_signon_req(Event);
            break;
        case DEF_EV_signon_rsp:
            CNSV_cmd_resp();
            break;
        case DEF_EV_unknown_resp:
            CNSV_ci_unknown_resp();
            break;
        case DEF_EV_PS_err:
            CNSV_ci_ps_err();
            break;
        default:
            AbNormal_End();
           break;
    }
} /*end of CNSV_ci_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_connect_req                               */
/*  CALLING SEQ.    : void CNSV_connect_req ( void )                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : C502-1010:接続要求処理                                */
/****************************************************************************/
void CNSV_connect_req(void)
{
short s_Err;
short s_idx;
short s_thread;
r502_def command_reply;
buff_node_def *buff_node;
_cc_status i_CC;

//    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].lc_use; s_idx++ ) {
//        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_connect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
//    }
//    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {
//        if (memcmp(c502->command_info.connection_logical_name.group_name,cf[myinfo.cf_idx].sc_conf[s_idx].group_name,sizeof(cf[myinfo.cf_idx].sc_conf[s_idx].group_name)) == 0) {
//            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,s_idx,DEF_EV_port_open,0,0,IOCMP.len,(char *)IOCMP.addr);
//        }
//    }
//    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
//        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_connect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
//    }
    memcpy(&command_reply,(char *)c502,IOCMP.len);
    memcpy(&command_reply.common_header.interface_code,DEF_IPC_IFCD_CMD_PRC_RSP,sizeof(DEF_IPC_IFCD_CMD_PRC_RSP)-1);
    command_reply.common_header.error_code = DEF_IPC_ERRCD_OK;
    command_reply.common_header.control_data_length = 0;

    s_thread = CNSV_sc_table_search((char *)&c502->command_info.connection_logical_name,DEF_tbl_chk_len_connection);
    if (s_thread == DEF_Not_Found) {                                    /*存在しない*/
        command_reply.common_header.error_code = DEF_IPC_ERRCD_NG;
    } else if (cf[myinfo.cf_idx].sc_conf[s_thread].use_on_off == '1') { /*使用不可*/
        command_reply.common_header.error_code = DEF_IPC_ERRCD_NG;
    }
    if (command_reply.common_header.error_code == DEF_IPC_ERRCD_OK) {
        /*リスナー処理*/
        for (s_idx = 0; s_idx < myinfo.lp_info_use_cnt; s_idx++ ) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_connect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        }
        /*コネクション処理*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,s_thread,DEF_EV_port_open,0,0,IOCMP.len,(char *)IOCMP.addr);
        /*Outbound電文振分*/
        for (s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_connect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        }
    }

    i_CC = REPLYX((char *)&command_reply,sizeof(common_header_def),,IOCMP.RINF.z_messagetag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
} /*end of CNSV_connect_req*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_disconnect_req                            */
/*  CALLING SEQ.    : void CNSV_disconnect_req ( void )                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : C502-1020:切断要求処理                                */
/****************************************************************************/
void CNSV_disconnect_req(void)
{
short s_Err;
short s_idx;
short s_thread;
r502_def command_reply;
buff_node_def *buff_node;
_cc_status i_CC;
//    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].lc_use; s_idx++ ) {
//        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
//    }
//    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {
//        if (memcmp(c502->command_info.connection_logical_name.group_name,cf[myinfo.cf_idx].sc_conf[s_idx].group_name,sizeof(cf[myinfo.cf_idx].sc_conf[s_idx].group_name)) == 0) {
//            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,s_idx,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
//        }
//    }
//    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
//        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
//    }
    memcpy(&command_reply,(char *)c502,IOCMP.len);
    memcpy(&command_reply.common_header.interface_code,DEF_IPC_IFCD_CMD_PRC_RSP,sizeof(DEF_IPC_IFCD_CMD_PRC_RSP)-1);
    command_reply.common_header.error_code = DEF_IPC_ERRCD_OK;
    command_reply.common_header.control_data_length = 0;

    s_thread = CNSV_sc_table_search((char *)&c502->command_info.connection_logical_name,DEF_tbl_chk_len_connection);
    if (s_thread == DEF_Not_Found) {                                    /*存在しない*/
        command_reply.common_header.error_code = DEF_IPC_ERRCD_NG;
    } else if (cf[myinfo.cf_idx].sc_conf[s_thread].use_on_off == '1') { /*使用不可*/
        command_reply.common_header.error_code = DEF_IPC_ERRCD_NG;
    }
    if (command_reply.common_header.error_code == DEF_IPC_ERRCD_OK) {
        /*コネクション処理*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,s_thread,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        /*リスナー処理*/
        for (s_idx = 0; s_idx < cf[myinfo.cf_idx].lc_use; s_idx++ ) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        }
        /*Outbound電文振分*/
        for (s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        }
    }
    i_CC = REPLYX((char *)&command_reply,sizeof(common_header_def),,IOCMP.RINF.z_messagetag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
} /*end of CNSV_disconnect_req*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_reload_command                            */
/*  CALLING SEQ.    : void CNSV_reload_commandd ( void )                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : C502-4010:ファイル再読み込み処理                      */
/****************************************************************************/
// c502はGFPCVX11内CNSV_cmd_reqで設定済

void CNSV_reload_command(void)
{
short s_Err, s_result;
short s_idx, s_find, s_compare_len;
r502_def command_reply;
buff_node_def *buff_node;
_cc_status i_CC;

    memcpy(&command_reply,(char *)c502,IOCMP.len);
    memcpy(&command_reply.common_header.interface_code,DEF_IPC_IFCD_CMD_PRC_RSP,sizeof(DEF_IPC_IFCD_CMD_PRC_RSP)-1);
    if (cf[myinfo.cf_idx].connect_num_mng_lyr == 'S') { /*比較長設定*/
        s_compare_len = DEF_GROUP_len_station;
    } else {
        s_compare_len = DEF_GROUP_len_interface;
    }
    for ( s_idx = 0, s_find = false; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {
        if (memcmp((char *)&cf[myinfo.cf_idx].sc_conf[s_idx].site_name,(char *)&c502->command_info.connection_logical_name,s_compare_len) == 0) {
            if (sc_info[s_idx].sock_fd != DEF_FILE_CLOSED) {    /*接続中コネクション有無*/
                s_find = true;
            }
        }
    }
    if (s_find) {
        memcpy(&command_reply.common_header.internal_error_code,DEF_NERR_CON_STS_CHK_ERR,sizeof(DEF_NERR_CON_STS_CHK_ERR)-1);
        s_result = false;
    } else {
        s_result = CNSV_LOAD();
    }
    if (s_result == true) {
        command_reply.common_header.error_code = 0;
    } else {
        command_reply.common_header.error_code = 9;
    }
    command_reply.common_header.control_data_length = 0;

    i_CC = REPLYX((char *)&command_reply,sizeof(common_header_def),,IOCMP.RINF.z_messagetag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }

    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
} /*end of CNSV_reload_command*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_cmd_resp                                  */
/*  CALLING SEQ.    : void CNSV_cmd_resp ( void )                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : R501:コマンド応答                                     */
/****************************************************************************/
void CNSV_cmd_resp(void)
{
buff_node_def *buff_node;
Event_Node_def Event;
    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    ci_info->ps_manage.buf_p = NULL;
    if (ci_info->ps_manage.send_wait.head != NULL) {
        CNSV_remove_list(&ci_info->ps_manage.send_wait,&myinfo.free_list,&Event);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,Event.compo,Event.thread,Event.event,Event.option1,Event.option2,Event.len,(char *)&Event.text);
    }
} /*end of CNSV_cmd_resp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ci_signon_req                             */
/*  CALLING SEQ.    : void CNSV_ci_signon_req ( Event_Node_def * )          */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : サインオン要求処理                                    */
/****************************************************************************/
void CNSV_ci_signon_req(Event_Node_def *Event)
{
short   s_Err,s_FS_err;
short   s_Pathsend_err;
c501_def *c501;         /* C201 電文受信通知要求 */
unsigned long io_tag_value;
buff_node_def *buff_node;

    if (ci_info->ps_manage.buf_p != NULL) {
        CNSV_add_list(&myinfo.free_list,&ci_info->ps_manage.send_wait,Event->compo,Event->thread,Event->event,Event->option1,Event->option2,0,"");
        return;
    }
    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    ci_info->ps_manage.buf_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    c501 = (c501_def *)ci_info->ps_manage.buf_p;
    memset(c501,' ',sizeof(c501_def));
    memcpy(c501->common_header.interface_code,DEF_IPC_IFCD_CMD_REQ,sizeof(c501->common_header.interface_code));
    c501->common_header.error_code = 0;
    memcpy(c501->common_header.internal_error_code,DEF_NERR_NOMAL,sizeof(c501->common_header.internal_error_code));
    c501->common_header.control_data_length = 70;       /*コマンド情報部(68)+レコード数(2)の長さ)*/

    memcpy(c501->command_info.command_name,DEF_IPC_CMD_CNT_OPN_AUT_CON,sizeof(c501->command_info.command_name));
    memcpy((char *)&c501->command_info.connection_logical_name,(char *)&cf[myinfo.cf_idx].sc_conf[Event->thread].site_name,sizeof(c501->command_info.connection_logical_name));
    memcpy(c501->command_info.interface_ext_name,cf[myinfo.cf_idx].st_conf[sc_info[Event->thread].station_index].nw_if,sizeof(c501->command_info.interface_ext_name));
    memcpy(c501->command_info.station_ext_name,cf[myinfo.cf_idx].st_conf[sc_info[Event->thread].station_index].nw_station,sizeof(c501->command_info.station_ext_name));
    c501->record_count = 0;
    COM_TGM(DEF_Component_Command,Event->thread,DEF_EV_PS_comp,&io_tag_value);
    if (EXTRACEMODE) {
        ps_trs = (lk_zac2001p_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ps_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ps_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }

    /*PATHSEND実施*/
    s_Err = SERVERCLASS_SEND_(ci_info->ps_manage.pathmon_name
                             ,ci_info->ps_manage.pathmon_name_len
                             ,ci_info->ps_manage.server_class
                             ,ci_info->ps_manage.server_class_len
                             ,ci_info->ps_manage.buf_p
                             ,(short)sizeof(c501_def)
                             ,(short)sizeof(r501_def)
                             ,
                             ,myinfo.pathsend_io_timer
                             ,1
                             ,&myinfo.pathsend_fd
                             ,(__int32_t)io_tag_value
                             );
    if (s_Err) {
        s_Err = SERVERCLASS_SEND_INFO_(&s_Pathsend_err,&s_FS_err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SERVERCLASS_SEND_",s_Pathsend_err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ps_trs->func_flg = '1';
        memcpy(ps_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
        memcpy(ps_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ps_trs->trace_info.prog_id));
        memcpy(ps_trs->trace_info.file_id,"PATHSEND",8);
        sprintf(trace_work,"%s/%s",ci_info->ps_manage.pathmon_name,ci_info->ps_manage.server_class);
        memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
        memcpy(ps_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ps_trs->trace_info.guardian_errcode,trace_work,sizeof(ps_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(c501_def));
        memcpy(ps_trs->data_info.rec_len,trace_work,sizeof(ps_trs->data_info.rec_len));
        TRACEOUT((char *)ps_trs);
    }

} /*end of CNSV_ci_signon_req*/
/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ci_unknown_resp                           */
/*  CALLING SEQ.    : void CNSV_ci_unknown_resp ( void )                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンドサーバー不明応答処理                          */
/****************************************************************************/
void CNSV_ci_unknown_resp(void)
{
buff_node_def *buff_node;
Event_Node_def Event;
    /*◆エラーメッセージ出力*/
    message_output(DEF_EVT_RSP_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_IPC_SEISA_ERR,"@X@X@I","","",r501->common_header.interface_code,DEF_VAR_STOP);
    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    ci_info->ps_manage.buf_p = NULL;
    if (ci_info->ps_manage.send_wait.head != NULL) {
        CNSV_remove_list(&ci_info->ps_manage.send_wait,&myinfo.free_list,&Event);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,Event.compo,Event.thread,Event.event,Event.option1,Event.option2,Event.len,(char *)&Event.text);
    }
} /*end of CNSV_ci_unknown_resp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ci_ps_err                                 */
/*  CALLING SEQ.    : void CNSV_ci_ps_err ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンドサーバーPATHSENDエラー                        */
/****************************************************************************/
void CNSV_ci_ps_err(void)
{
buff_node_def *buff_node;
Event_Node_def Event;
    message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_NG,"@X@X@X@5","",(char *)ci_info->ps_manage.pathmon_name,(char *)ci_info->ps_manage.server_class,IOCMP.fs_err,DEF_VAR_STOP);
    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    ci_info->ps_manage.buf_p = NULL;
    if (ci_info->ps_manage.send_wait.head != NULL) {
        CNSV_remove_list(&ci_info->ps_manage.send_wait,&myinfo.free_list,&Event);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,Event.compo,Event.thread,Event.event,Event.option1,Event.option2,Event.len,(char *)&Event.text);
    }
} /*end of CNSV_ci_ps_err*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ci_cancel                                 */
/*  CALLING SEQ.    : void CNSV_ci_cancel ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンドキャンセルメッセージ処理                      */
/****************************************************************************/
void CNSV_ci_cancel(void)
{
short s_Err;
_cc_status i_CC;

    i_CC = REPLYX(,,,ci_info->cmd_rcv_manage.reply_tag);
    if (_status_ne(i_CC)) {
        /*■REPLYエラーメッセージ*/
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
    }
    ci_info->cmd_rcv_manage.reply_tag = DEF_TAG_NULL;
} /*end of CNSV_ci_cancel*/

