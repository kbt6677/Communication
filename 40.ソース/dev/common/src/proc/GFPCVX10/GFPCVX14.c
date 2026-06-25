/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
/*                               INBOUND電文振分管理コンポーネント(ib)       */
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
/*  FUNCTION        : 1.1.0  CNSV_ib_manage                                 */
/*  CALLING SEQ.    : void CNSV_ib_manage ( Event_Node_def * )              */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : INBOUND電文振分管理処理                               */
/****************************************************************************/
void CNSV_ib_manage(Event_Node_def *Event)
{
    switch (Event->event)
    {
    case DEF_EV_sock_recv_comp:
        CNSV_recv_msg_send(Event);
        break;
    case DEF_EV_recv_text_send_req:
        CNSV_message_pathsend(Event);
        break;
    case DEF_EV_PS_comp:
    case DEF_EV_text_recv_rsp:
        CNSV_recv_msg_resp();
        break;
    case DEF_EV_unknown_resp:
        CNSV_in_unknown_resp(Event);
        break;
    case DEF_EV_PS_err:
    case DEF_EV_timeout:
        CNSV_in_ps_err(Event);
        break;
    default:
        AbNormal_End();
        break;
    }
} /*end of CNSV_ib_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recv_msg_send                             */
/*  CALLING SEQ.    : void CNSV_recv_msg_send ( Event_Node_def * )          */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : C201:電文受信通知送信                                 */
/****************************************************************************/
void CNSV_recv_msg_send(Event_Node_def *Event)
{
short   s_Err,s_FS_err,s_table,s_lc,s_idx;
short   thread;
short   s_Pathsend_err;
c201_def *c201;         /* C201 電文受信通知要求 */
unsigned long io_tag_value;
long    l_addr;
buff_node_def *buff_node;
char    ach_gflin_pkey[DEF_GFLIN_PKEY_LEN+1];

    thread = Event->thread;
    l_addr = (long)Event->text;
    c201 = (c201_def *)(l_addr - DEF_C201_hd_len - DEF_C201_msg_info_len - DEF_C201_txt_len);
    memcpy(c201->common_header.interface_code,DEF_IPC_IFCD_DEN_RCV_NT_REQ,sizeof(c201->common_header.interface_code));
    c201->common_header.error_code = 0;
    memcpy(c201->common_header.internal_error_code,"0000000",sizeof(c201->common_header.internal_error_code));
    memset(c201->common_header.filler_1,' ',sizeof(c201->common_header.filler_1));
    c201->common_header.control_data_length = Event->len + 150 + 2;
    c201->text_recv_notify.recv_con_id.site_name = cf[myinfo.cf_idx].sc_conf[Event->option1].site_name;
    c201->text_recv_notify.recv_con_id.nw_name = cf[myinfo.cf_idx].sc_conf[Event->option1].nw_name;
    memcpy(c201->text_recv_notify.recv_con_id.interface_name,cf[myinfo.cf_idx].sc_conf[Event->option1].interface_name,sizeof(c201->text_recv_notify.recv_con_id.interface_name));
    memcpy(c201->text_recv_notify.recv_con_id.group_name,cf[myinfo.cf_idx].sc_conf[Event->option1].group_name,sizeof(c201->text_recv_notify.recv_con_id.group_name));
    memcpy(c201->text_recv_notify.recv_con_id.station_name,cf[myinfo.cf_idx].sc_conf[Event->option1].station_name,sizeof(c201->text_recv_notify.recv_con_id.station_name));
    memcpy(c201->text_recv_notify.recv_con_id.connection_name,cf[myinfo.cf_idx].sc_conf[Event->option1].src_connection_name,sizeof(c201->text_recv_notify.recv_con_id.connection_name));
    memset(c201->text_recv_notify.recv_con_info.src_ip_address,' ',sizeof(c201->text_recv_notify.recv_con_info.src_ip_address));
    memcpy(c201->text_recv_notify.recv_con_info.src_ip_address,cf[myinfo.cf_idx].sc_conf[Event->option1].local_ipaddr,strlen(cf[myinfo.cf_idx].sc_conf[Event->option1].local_ipaddr));
    memcpy(c201->text_recv_notify.recv_con_info.src_port,cf[myinfo.cf_idx].sc_conf[Event->option1].local_port_no,sizeof(c201->text_recv_notify.recv_con_info.src_port));
    memset(c201->text_recv_notify.recv_con_info.dest_ip_address,' ',sizeof(c201->text_recv_notify.recv_con_info.dest_ip_address));
    memcpy(c201->text_recv_notify.recv_con_info.dest_ip_address,cf[myinfo.cf_idx].sc_conf[Event->option1].remote_ipaddr,strlen(cf[myinfo.cf_idx].sc_conf[Event->option1].remote_ipaddr));
    memcpy(c201->text_recv_notify.recv_con_info.dest_port,cf[myinfo.cf_idx].sc_conf[Event->option1].remote_port_no,sizeof(c201->text_recv_notify.recv_con_info.dest_port));
    memcpy(c201->text_recv_notify.recv_timestamp.time_stamp,(char *)(&sc_info[thread].ach_timestamp),sizeof(c201->text_recv_notify.recv_timestamp.time_stamp));
    memcpy(c201->text_recv_notify.recv_timestamp.ts_unique_data,(char *)&sc_info[thread].ach_uniq_ts,sizeof(c201->text_recv_notify.recv_timestamp.ts_unique_data));
    memset(c201->text_recv_notify.filler_1,' ',sizeof(c201->text_recv_notify.filler_1));
    c201->msg_info.msg_len = Event->len;

    /*PATHSENDテーブル取得*/
    s_table = -1;
    for (s_lc = 0 ,s_idx = ib_info->search_index;s_lc < DEF_MAX_INBOUND_PS;s_lc++, s_idx++) {
        if (s_idx == DEF_MAX_INBOUND_PS) {
            s_idx = 0;
        }
        if (ib_info->ps_req[s_idx].use_flag == DEF_TABLE_FREE) {  /*未使用*/
            s_table = s_idx;
            ib_info->search_index = s_idx + 1;
            ib_info->ps_req[s_idx].use_flag = DEF_TABLE_USE;
            if (ib_info->search_index == DEF_MAX_INBOUND_PS) {
                ib_info->search_index = 0;
            }
            s_lc = DEF_MAX_INBOUND_PS;
        }
    }
    if (s_table == -1) {    /*PATHSEND同時処理数超え、キューイングする、■最大数チェックが必要*/
        if (ib_info->send_wait.list_count >= DEF_MAX_RECV_QUEUE) {
            /**/
            memset(ach_gflin_pkey,'\0',sizeof(ach_gflin_pkey));
            memcpy(ach_gflin_pkey,(char *)&c201->text_recv_notify.recv_con_id,DEF_GFLIN_PKEY_LEN);
            message_output(DEF_EVT_PSEND_QUE_NON,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_NOMAL,"@X@X@A@P@X@X@5",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].tcpip_name,sc_info[thread].sock.sin_addr.s_addr,(unsigned short)sc_info[thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[thread].local_port_no,DEF_MAX_RECV_QUEUE,DEF_VAR_STOP);
            /*バッファー開放*/
            buff_node = (buff_node_def *)(l_addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
            myinfo.buffs.tail->next = buff_node;
            myinfo.buffs.tail = buff_node;
            myinfo.buffs.list_count++;
            buff_node->next = 0L;
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,Event->option1,DEF_EV_text_recv_rsp,0,0,IOCMP.len,(char *)IOCMP.addr);
        } else {
            CNSV_add_list(&myinfo.free_list,&ib_info->send_wait,DEF_Component_Inbound,Event->option1,DEF_EV_recv_text_send_req,Event->option1,DEF_EV_recv_text_send_req,Event->len,Event->text);
        }
        return;
    }

    ib_info->ps_req[s_table].use_flag = true;
    ib_info->ps_req[s_table].retry_cnt = 0;

    /*c201->msg_info.msg_dataは受信バッファーそのままなので編集不要*/
    ib_info->ps_req[s_table].ps_buf_p = (char *)c201;
    /*タグ取得*/
    /*◆他のコンポーネント違いスレッドはPATHSENDテーブル番号*/
    COM_TGM(DEF_Component_Inbound,s_table,IOCMP.event,&io_tag_value);
    ib_info->ps_req[s_table].ps_len = Event->len + DEF_C201_hd_len + DEF_C201_msg_info_len + DEF_C201_txt_len;
    ib_info->ps_req[s_table].sc_thread = thread;
    /*出来上がったデータは退避バッファにCOPYする*/
    memcpy(ib_info->ps_req[s_table].save_p,ib_info->ps_req[s_table].ps_buf_p,ib_info->ps_req[s_table].ps_len);
    ib_info->ps_req[s_table].save_len = ib_info->ps_req[s_table].ps_len;
    if (EXTRACEMODE) {
        ps_trs = (lk_zac2001p_arg_1_def *)((char *)c201 - DEF_IOCMP_TRS_HD_POS);
        memset((char *)&ps_trs->func_flg,' ',DEF_IOCMP_TRS_HD_POS);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ps_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }

    /*PATHSEND実施*/
    s_Err = SERVERCLASS_SEND_(ib_info->pathmon_name
                            ,ib_info->pathmon_name_len
                            ,ib_info->serverclass_name
                            ,ib_info->serverclass_name_len
                            ,ib_info->ps_req[s_table].ps_buf_p
                            ,ib_info->ps_req[s_table].ps_len
                            ,(short)sizeof(r201_def)
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
    ib_info->req_cnt++;         /*PATHSEND数カウントアップ*/
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ps_trs->func_flg = '1';
        memcpy(ps_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ps_trs->trace_info.prog_id));
        memcpy(ps_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
        memcpy(ps_trs->trace_info.file_id,"PATHSEND",8);
        sprintf(trace_work,"%s/%s",ib_info->pathmon_name,ib_info->serverclass_name);
        memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
        memcpy(ps_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ps_trs->trace_info.guardian_errcode,trace_work,sizeof(ps_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",ib_info->ps_req[s_table].ps_len);
        memcpy(ps_trs->data_info.rec_len,trace_work,sizeof(ps_trs->data_info.rec_len));
        TRACEOUT((char *)ps_trs);
    }
    if (ib_info->req_cnt != DEF_MAX_INBOUND_PS) {    /*コネクション管理に電文受信通知応答*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,ib_info->ps_req[s_table].sc_thread,DEF_EV_text_recv_rsp,0,0,IOCMP.len,(char *)IOCMP.addr);
    }

} /*end of CNSV_recv_msg_send*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_message_pathsend                          */
/*  CALLING SEQ.    : void CNSV_message_pathsend ( Event_Node_def * )       */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Inbound電文振分PATHSEND処理                           */
/****************************************************************************/
void CNSV_message_pathsend(Event_Node_def *Event)
{
short   s_Err,s_FS_err,s_table,s_idx,s_lc;
short   s_Pathsend_err;
c201_def *c201;         /* C201 電文受信通知要求 */
unsigned long io_tag_value;
long    l_addr;

    if (ib_info->req_cnt == DEF_MAX_INBOUND_PS) {   /*リクエスト数超過、キューイング*/
        CNSV_add_list(&myinfo.free_list,&ib_info->send_wait,Event->compo,Event->thread,Event->event,Event->option1,Event->option2,Event->len,(char *)Event->text);
        return;
    }
    s_table = -1;
    for (s_lc = 0 ,s_idx = ib_info->search_index;s_lc < DEF_MAX_INBOUND_PS;s_lc++, s_idx++) {
        if (s_idx == DEF_MAX_INBOUND_PS) {
            s_idx = 0;
        }
        if (ib_info->ps_req[s_idx].use_flag == DEF_TABLE_FREE) {  /*未使用*/
            s_table = s_idx;
            ib_info->search_index = s_idx + 1;
            ib_info->ps_req[s_idx].use_flag = DEF_TABLE_USE;
            if (ib_info->search_index == DEF_MAX_INBOUND_PS) {
                ib_info->search_index = 0;
            }
            s_lc = DEF_MAX_INBOUND_PS;
        }
    }
    if (s_table == -1) {    /*テーブル無し、リクエスト超過判定後のため論理矛盾で異常終了*/
        AbNormal_End();
    }

    l_addr = (long)Event->text;
    c201 = (c201_def *)(l_addr - DEF_C201_hd_len - DEF_C201_msg_info_len - DEF_C201_txt_len);
    ib_info->ps_req[s_table].use_flag = true;
    ib_info->ps_req[s_table].retry_cnt = 0;

    /*c201->msg_info.msg_dataは受信バッファーそのままなので編集不要*/
    ib_info->ps_req[s_table].ps_buf_p = (char *)c201;
    /*タグ取得*/
    /*◆他のコンポーネント違いスレッドはPATHSENDテーブル番号*/
    COM_TGM(DEF_Component_Inbound,s_table,DEF_EV_PS_comp,&io_tag_value);
    ib_info->ps_req[s_table].ps_len = Event->len + DEF_C201_hd_len + DEF_C201_msg_info_len + DEF_C201_txt_len;
    ib_info->ps_req[s_table].sc_thread = Event->option1;
    /*出来上がったデータは退避バッファにCOPYする*/
    memcpy(ib_info->ps_req[s_table].save_p,ib_info->ps_req[s_table].ps_buf_p,ib_info->ps_req[s_table].ps_len);
    ib_info->ps_req[s_table].save_len = ib_info->ps_req[s_table].ps_len;
    if (EXTRACEMODE) {
        ps_trs = (lk_zac2001p_arg_1_def *)((char *)c201 - DEF_IOCMP_TRS_HD_POS);
        memset((char *)&ps_trs->func_flg,' ',DEF_IOCMP_TRS_HD_POS);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ps_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    /*PATHSEND実施*/
    s_Err = SERVERCLASS_SEND_(ib_info->pathmon_name
                            ,ib_info->pathmon_name_len
                            ,ib_info->serverclass_name
                            ,ib_info->serverclass_name_len
                            ,ib_info->ps_req[s_table].ps_buf_p
                            ,ib_info->ps_req[s_table].ps_len
                            ,(short)sizeof(r201_def)
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
    ib_info->req_cnt++;         /*PATHSEND数カウントアップ*/
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ps_trs->func_flg = '1';
        memcpy(ps_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ps_trs->trace_info.prog_id));
        memcpy(ps_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
        memcpy(ps_trs->trace_info.file_id,"PATHSEND",8);
        sprintf(trace_work,"%s/%s",ib_info->pathmon_name,ib_info->serverclass_name);
        memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
        memcpy(ps_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ps_trs->trace_info.guardian_errcode,trace_work,sizeof(ps_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",ib_info->ps_req[Event->option1].ps_len);
        memcpy(ps_trs->data_info.rec_len,trace_work,sizeof(ps_trs->data_info.rec_len));
        TRACEOUT((char *)ps_trs);
    }

} /*end of CNSV_message_pathsend*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recv_msg_resp                             */
/*  CALLING SEQ.    : void CNSV_recv_msg_resp ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Inbound電文振分応答受信処理                           */
/****************************************************************************/
void CNSV_recv_msg_resp(void)
{
Event_Node_def CurrentEvent;
buff_node_def *buff_node;

    /*PATHSEND完了トレース出力はCNSV_pathsend_compで実施済*/
    /*完了コードからリトライ判定、リトライの場合SAVEから取り出しバッファーに上書き*/
    ib_info->req_cnt--;         /*PATHSEND数カウントダウン*/

    /*コネクション管理に電文受信通知応答*/
    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,ib_info->ps_req[IOCMP.thread].sc_thread,DEF_EV_text_recv_rsp,0,0,IOCMP.len,(char *)IOCMP.addr);
    ib_info->ps_req[IOCMP.thread].sc_thread = -1;
    ib_info->ps_req[IOCMP.thread].use_flag = DEF_TABLE_FREE;
    ib_info->ps_req[IOCMP.thread].ps_len = 0;
    ib_info->ps_req[IOCMP.thread].ps_buf_p = 0;
    ib_info->ps_req[IOCMP.thread].save_len = 0;

    /*バッファー開放*/
    buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;

    /*PATHSEND数超過判定*/
    if (ib_info->req_cnt < DEF_MAX_INBOUND_PS) {
    /*未送信有無チェック*/
        if (ib_info->send_wait.head) {
            /*滞留分の送信可能*/
            CNSV_remove_list(&ib_info->send_wait,&myinfo.free_list,&CurrentEvent);
            CNSV_message_pathsend(&CurrentEvent);
        }
    }

} /*end of CNSV_recv_msg_resp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_in_unknown_resp                           */
/*  CALLING SEQ.    : void CNSV_in_unknown_resp ( Event_Node_def * )        */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Inbound電文振分不明応答受信                           */
/****************************************************************************/
void CNSV_in_unknown_resp(Event_Node_def *Event)
{
Event_Node_def CurrentEvent;
buff_node_def *buff_node;
r201_def *dummy_ipc;

    /*■エラーメッセージ出力*/
    dummy_ipc = (r201_def *)ib_info->ps_req[Event->thread].ps_buf_p;
    message_output(DEF_EVT_RSP_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_IPC_SEISA_ERR,"@X@X@I","","",dummy_ipc->common_header.interface_code,DEF_VAR_STOP);

    ib_info->req_cnt--;         /*PATHSEND数カウントダウン*/

    /*コネクション管理に電文受信通知応答*/
    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,ib_info->ps_req[Event->thread].sc_thread,DEF_EV_text_recv_rsp,0,0,IOCMP.len,(char *)IOCMP.addr);
    ib_info->ps_req[Event->thread].sc_thread = -1;
    ib_info->ps_req[Event->thread].use_flag = DEF_TABLE_FREE;
    ib_info->ps_req[Event->thread].ps_len = 0;
    ib_info->ps_req[Event->thread].ps_buf_p = 0;
    ib_info->ps_req[Event->thread].save_len = 0;

    /*バッファー開放*/
    buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;

    /*PATHSEND数超過判定*/
    if (ib_info->req_cnt < DEF_MAX_INBOUND_PS) {
    /*未送信有無チェック*/
        if (ib_info->send_wait.head) {
            /*滞留分の送信可能*/
            CNSV_remove_list(&ib_info->send_wait,&myinfo.free_list,&CurrentEvent);
            CNSV_message_pathsend(&CurrentEvent);
        }
    }

} /*end of CNSV_in_unknown_resp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_in_ps_err                                 */
/*  CALLING SEQ.    : void CNSV_in_ps_err ( Event_Node_def * )              */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Inbound電文振分PATHSENDエラー処理                     */
/****************************************************************************/
void CNSV_in_ps_err(Event_Node_def *Event)
{
short s_Err,s_FS_err,s_Pathsend_err;
short s_retry_on_off;
unsigned long io_tag_value;
Event_Node_def CurrentEvent;
buff_node_def *buff_node;

    ib_info->req_cnt--;         /*PATHSEND数カウントダウン*/
    /*PATHSENDエラーメッセージ出力*/
    switch (Event->option1) {
        case 0:     /*Inbound電文振分からのエラー(リクエストエラー等))*/
            message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_OK,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,IOCMP.fs_err,DEF_VAR_STOP);
            s_retry_on_off = true;
            break;
        case 40:    /*タイムアウト*/
            message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_TIMEOUT,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,Event->option1,DEF_VAR_STOP);
            s_retry_on_off = false;
            break;
        case 201:   /*FEPATHDOWN*/
            message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_OK,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,Event->option1,DEF_VAR_STOP);
            s_retry_on_off = true;
            break;
        case 904:   /*FEScServerLinkConnect(サーバーへのリンクでエラーが発生)*/
        case 918:   /*FEScSendOperationAborted(サーバーへのsendがabortされた)*/
            switch (Event->option2) {
                case 40:
                    message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_TIMEOUT,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,Event->option2,DEF_VAR_STOP);
                    s_retry_on_off = false;
                    break;
                case 201:
                    message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_OK,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,Event->option2,DEF_VAR_STOP);
                    s_retry_on_off = true;
                    break;
                default:    /*PATHSENDエラー*/
                    message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_NG,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,Event->option2,DEF_VAR_STOP);
                    s_retry_on_off = false;
                    break;
            }
            break;
        case 905:   /*FEScNoServerLinkAvailable(サーバーとのリンクが取得できない)⇒リソース不足で代替不可のため停止*/
        case 913:   /*FEScServerClassFrozen(指定したサーバークラスがFrozen状態になっている)⇒片系の場合発生しない*/
        case 915:   /*FEScPathmonShutDown(指定したPATHMONがシャットダウンされている)⇒停止処理中のためリトライは行わない*/
            message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_NG,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,Event->option1,DEF_VAR_STOP);
            s_retry_on_off = false;
            break;
        default:    /*PATHSENDエラー*/
            message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_NG,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,IOCMP.fs_err,DEF_VAR_STOP);
            s_retry_on_off = false;
            break;
    }

    if (s_retry_on_off == false) {
        ib_info->ps_req[Event->thread].sc_thread = -1;
        ib_info->ps_req[Event->thread].use_flag = DEF_TABLE_FREE;
        ib_info->ps_req[Event->thread].ps_len = 0;
        ib_info->ps_req[Event->thread].ps_buf_p = 0;
        ib_info->ps_req[Event->thread].save_len = 0;

        /*バッファー開放*/
        buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;

        if (ib_info->send_wait.head) {  /*未送信有無チェック*/
            /*滞留分の送信可能*/
            CNSV_remove_list(&ib_info->send_wait,&myinfo.free_list,&CurrentEvent);
            CNSV_message_pathsend(&CurrentEvent);
        }
        return;
    }

    ib_info->ps_req[Event->thread].retry_cnt++;
    if (ib_info->ps_req[Event->thread].retry_cnt > myinfo.pathsend_retry_count) {
        /*リトライオーバー*/
        message_output(DEF_EVT_PSEND_ERR_DETECT,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PSEND_ERR_RE_OUT,"@X@X@X@5","",(char *)ib_info->pathmon_name,(char *)ib_info->serverclass_name,IOCMP.fs_err,DEF_VAR_STOP);
        /*コネクション管理に電文受信通知応答*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,ib_info->ps_req[Event->thread].sc_thread,DEF_EV_text_recv_rsp,0,0,IOCMP.len,(char *)IOCMP.addr);
        ib_info->ps_req[Event->thread].sc_thread = -1;
        ib_info->ps_req[Event->thread].use_flag = DEF_TABLE_FREE;
        ib_info->ps_req[Event->thread].ps_len = 0;
        ib_info->ps_req[Event->thread].ps_buf_p = 0;
        ib_info->ps_req[Event->thread].save_len = 0;

        /*バッファー開放*/
        buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;

        if (ib_info->send_wait.head) {  /*未送信有無チェック*/
            /*滞留分の送信可能*/
            CNSV_remove_list(&ib_info->send_wait,&myinfo.free_list,&CurrentEvent);
            CNSV_message_pathsend(&CurrentEvent);
        }

        return;
    } else {    /*リトライ*/
        /*退避バッファから復元、PATHSEND処理*/
        memcpy(ib_info->ps_req[Event->thread].ps_buf_p,ib_info->ps_req[Event->thread].save_p,ib_info->ps_req[Event->thread].save_len);
        ib_info->ps_req[Event->thread].ps_len = ib_info->ps_req[Event->thread].save_len;
        /*タグ取得*/
        /*◆他のコンポーネント違いスレッドはPATHSENDテーブル番号*/
        COM_TGM(DEF_Component_Inbound,Event->thread,IOCMP.event,&io_tag_value);
        if (EXTRACEMODE) {
            ps_trs = (lk_zac2001p_arg_1_def *)((char *)ib_info->ps_req[Event->thread].ps_buf_p - DEF_IOCMP_TRS_HD_POS);
            memset((char *)&ps_trs->func_flg,' ',DEF_IOCMP_TRS_HD_POS);     /*トレース情報初期化*/
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy((char *)ps_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
        }

        /*PATHSEND実施*/
        s_Err = SERVERCLASS_SEND_(ib_info->pathmon_name
                                ,ib_info->pathmon_name_len
                                ,ib_info->serverclass_name
                                ,ib_info->serverclass_name_len
                                ,ib_info->ps_req[Event->thread].ps_buf_p
                                ,ib_info->ps_req[Event->thread].ps_len
                                ,(short)sizeof(r201_def)
                                ,
                                ,myinfo.pathsend_io_timer
                                ,1
                                ,&myinfo.pathsend_fd
                                ,(__int32_t)io_tag_value
                                );
        if (s_Err) {
            s_Err = SERVERCLASS_SEND_INFO_(&s_Pathsend_err,&s_FS_err);
            if (s_Err) {
                message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SERVERCLASS_SEND_INFO_",s_Err,DEF_VAR_STOP);
                AbNormal_End();
            }
        }
        ib_info->req_cnt++;         /*PATHSEND数カウントアップ*/
        if (EXTRACEMODE) {
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            ps_trs->func_flg = '1';
            memcpy(ps_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
            memcpy(ps_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ps_trs->trace_info.prog_id));
            memcpy(ps_trs->trace_info.file_id,"PATHSEND",8);
            sprintf(trace_work,"%s/%s",ib_info->pathmon_name,ib_info->serverclass_name);
            memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
            memcpy(ps_trs->trace_info.file_io_type,"WRITE   ",8);
            sprintf(trace_work,"%04d",s_Err);
            memcpy(ps_trs->trace_info.guardian_errcode,trace_work,sizeof(ps_trs->trace_info.guardian_errcode));
            sprintf(trace_work,"%05d",ib_info->ps_req[Event->thread].ps_len);
            memcpy(ps_trs->data_info.rec_len,trace_work,sizeof(ps_trs->data_info.rec_len));
            TRACEOUT((char *)ps_trs);
        }
    }

} /*end of CNSV_in_ps_err*/
