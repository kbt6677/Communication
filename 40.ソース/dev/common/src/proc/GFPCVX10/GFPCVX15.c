/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
/*                               OUTBOUND電文振分管理コンポーネント(ob)      */
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
/*  FUNCTION        : 1.1.0  CNSV_ob_manage                                 */
/*  CALLING SEQ.    : void CNSV_ob_manage ( Event_Node_def * )              */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : OUTBOUND電文振分管理処理                              */
/****************************************************************************/
void CNSV_ob_manage(Event_Node_def *Event)
{
    switch (Event->event)
    {
        case DEF_EV_text_send_req:      /*PATHSEND(受信)*/
            CNSV_send_req();
            break;
        case DEF_EV_connect_req:        /*C001 接続要求*/
            CNSV_outbound_open(Event->thread);
            break;
        case DEF_EV_disconnect_req:     /*C002 切断要求*/
            CNSV_outbound_close(Event->thread);
            break;
        case DEF_EV_io_timeout:         /*I/O完了待ちタイマー*/
            CNSV_ob_io_err();
            break;
        case DEF_EV_nw_open_timeout:    /*OPEN完了待ちタイマー*/
            CNSV_ob_open_timeout();
            break;
        case DEF_EV_S_retry_timeout:    /*ショートリトライタイマー*/
            CNSV_ob_short_retry_timeout();
            break;
        case DEF_EV_L_retry_timeout:     /*ロングリトライタイマー*/
            CNSV_ob_long_retry_timeout();
            break;
        case DEF_EV_cancel_msg:         /*キャンセル受信*/
            CNSV_outbound_cancel(Event->thread);
            break;
        case DEF_EV_sock_recv_err:      /*recvエラー*/
        case DEF_EV_sock_send_err:      /*sendエラー*/
            CNSV_ob_con_err(Event);
            break;
        case DEF_EV_open_comp:          /*Nowaitオープン完了*/
            CNSV_ob_open_comp();
            break;
        case DEF_EV_open_err:           /*オープンエラー*/
            CNSV_ob_open_err(Event->thread);
            break;
        case DEF_EV_con_state_notice:   /*R107:コネクション状態通知応答*/
            CNSV_ob_io_comp();
            break;
        case DEF_EV_outbound_io_err:    /*Outbound電文振分I/Oエラー*/
            CNSV_ob_io_err();
            break;
        case DEF_EV_tell_port_status:   /*ポートステータス送信*/
            CNSV_ob_send_port_status(Event->thread,Event->option1);
            break;
        default:
            AbNormal_End();
            break;
    }
} /*end of CNSV_ob_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_outbound_open                             */
/*  CALLING SEQ.    : void CNSV_outbound_open ( short )                     */
/*  ARGUMENT        : スレッド情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分オープン処理                          */
/****************************************************************************/
void CNSV_outbound_open(short thread)
{
short   s_err;
short   s_Timeout_tag;
unsigned long ul_tag;
char    ach_outbound_name[ZSYS_VAL_LEN_PROCESSDESCR+1];
_cc_status i_CC;

    if (ob_info[thread].ipc_mng.ob_fd != DEF_FILE_CLOSED) {     /*オープン中の場合はリターン*/
        return;
    }
    sprintf(ach_outbound_name,"%s.#CONN",cf[myinfo.cf_idx].ob_conf[thread].process_name);
    strcpy(ob_info[thread].ipc_mng.outbound_name,ach_outbound_name);
    if (ob_info[thread].ipc_mng.ob_status == 0) {
        ob_info[thread].ipc_mng.ob_status = 1;
        ob_info[thread].ipc_mng.retry_type = 0;
    }
    if (EXTRACEMODE) {
        memset((char *)&ob_info[thread].ipc_mng.trs,' ',sizeof(lk_trace_def));
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ob_info[thread].ipc_mng.trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    COM_TGM(DEF_Component_Outbound,thread,DEF_EV_nw_open_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(myinfo.nowait_open_timer
                       ,DEF_EV_nw_open_timeout
                       ,(__int32_t)ul_tag
                       ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    ob_info[thread].ipc_mng.timer_tag = s_Timeout_tag;
    s_err = FILE_OPEN_(ach_outbound_name
                      ,(short)strlen(ach_outbound_name)
                      ,&ob_info[thread].ipc_mng.ob_fd
                      ,ZSYS_VAL_OPENACC_READWRITE
                      ,ZSYS_VAL_OPENEXCL_SHARED
                      ,1
                      ,1
                      ,0x4000
                      );
    switch (s_err)
    {
    case 0:
        break;
    case 14:
    case 60:
    case 66:
        /*プロセスが起動されていない、再起動を待つためタイムアウトで再実施する*/
        ob_info[thread].ipc_mng.ob_fd = DEF_FILE_CLOSED;
        break;
    default:
        /*エラーメッセージ(※呼び出しパラメータ不良のためABEND相当)*/
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","FILE_OPEN_",s_err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ob_info[thread].ipc_mng.trs.func_flg = '1';
        memcpy(ob_info[thread].ipc_mng.trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(ob_info[thread].ipc_mng.trs.trace_info.prog_id));
        memcpy(ob_info[thread].ipc_mng.trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
        memcpy(ob_info[thread].ipc_mng.trs.trace_info.file_id,"PROCESS ",8);
        memcpy(ob_info[thread].ipc_mng.trs.trace_info.file_name,ach_outbound_name,strlen(ach_outbound_name));
        memcpy(ob_info[thread].ipc_mng.trs.trace_info.file_io_type,"OPEN(NW)",8);
        sprintf(trace_work,"%04d",s_err);
        memcpy(ob_info[thread].ipc_mng.trs.trace_info.guardian_errcode,trace_work,sizeof(ob_info[thread].ipc_mng.trs.trace_info.guardian_errcode));
        memset(ob_info[thread].ipc_mng.trs.data_info.rec_len,'0',sizeof(ob_info[thread].ipc_mng.trs.data_info.rec_len));
        TRACEOUT((char *)&ob_info[thread].ipc_mng.trs);
    }

} /*end of CNSV_outbound_open*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_outbound_close                            */
/*  CALLING SEQ.    : void CNSV_outbound_close ( short )                    */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分クローズ処理                          */
/****************************************************************************/
void CNSV_outbound_close(short thread)
{
short           s_lc,s_cnt;
buff_node_def *buff_node;
Event_Node_def Event;
_cc_status i_CC;
    for (s_lc = 0, s_cnt = 0;s_lc < cf[myinfo.cf_idx].sc_use; s_lc++ ) {
        if (sc_info[s_lc].sock_fd != DEF_FILE_CLOSED) {
            s_cnt++;
        }
    }
    if (s_cnt != 0) {       /*接続中コネクション有り*/
        return;
    }
    if (ob_info[thread].ipc_mng.ob_fd != DEF_FILE_CLOSED) {
        FILE_CLOSE_(ob_info[thread].ipc_mng.ob_fd);
        ob_info[thread].ipc_mng.ob_fd = DEF_FILE_CLOSED;
        if (EXTRACEMODE) {
            memset((char *)&ob_info[thread].ipc_mng.trs,' ',sizeof(lk_trace_def));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy((char *)ob_info[thread].ipc_mng.trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
            ob_info[thread].ipc_mng.trs.func_flg = '1';
            memcpy(ob_info[thread].ipc_mng.trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(ob_info[thread].ipc_mng.trs.trace_info.prog_id));
            memcpy(ob_info[thread].ipc_mng.trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
            memcpy(ob_info[thread].ipc_mng.trs.trace_info.file_id,"PROCESS ",8);
            memcpy(ob_info[thread].ipc_mng.trs.trace_info.file_name,ob_info[thread].ipc_mng.outbound_name,strlen(ob_info[thread].ipc_mng.outbound_name));
            memcpy(ob_info[thread].ipc_mng.trs.trace_info.file_io_type,"CLOSE   ",8);
            memset(ob_info[thread].ipc_mng.trs.trace_info.guardian_errcode,'0',sizeof(ob_info[thread].ipc_mng.trs.trace_info.guardian_errcode));
            memset(ob_info[thread].ipc_mng.trs.data_info.rec_len,'0',sizeof(ob_info[thread].ipc_mng.trs.data_info.rec_len));
            TRACEOUT((char *)&ob_info[thread].ipc_mng.trs);
        }
    }
    /*発行中のタイマーがあればキャンセル*/
    if (ob_info[thread].ipc_mng.timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(ob_info[thread].ipc_mng.timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        ob_info[thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    }
    /*バッファー開放*/
    if (ob_info[thread].ipc_mng.buf_p != 0) {
        buff_node = (buff_node_def *)(ob_info[thread].ipc_mng.buf_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
        ob_info[thread].ipc_mng.buf_p = 0;
    }
    ob_info[thread].ipc_mng.ob_status = 0;
    ob_info[thread].ipc_mng.retry_type = 0;
    while (ob_info[thread].ipc_mng.send_wait.head != NULL) {
        CNSV_remove_list(&ob_info[thread].ipc_mng.send_wait,&myinfo.free_list,&Event);
    }
} /*end of CNSV_outbound_close*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_outbound_cancel                           */
/*  CALLING SEQ.    : void CNSV_outbound_cancel ( short )                   */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分キャンセルメッセージ処理              */
/****************************************************************************/
void CNSV_outbound_cancel(short thread)
{
short s_Err;
_cc_status i_CC;

    i_CC = REPLYX(,,,ob_info[thread].ps_manage.reply_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    ob_info[thread].ps_manage.reply_tag = DEF_TAG_NULL;

} /*end of CNSV_outbound_cancel*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_send_req                                  */
/*  CALLING SEQ.    : void CNSV_send_req ( void )                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : C202:電文送信要求                                     */
/****************************************************************************/
void CNSV_send_req(void)
{
short   s_Thread;
short   s_Err;
short   s_compare_len;
r202_def r202;
_cc_status i_CC;

    /*インターフェース単位指定に基づき比較長を設定*/
    if (c202->text_send_info.recv_con_id.station_name[0] == '}') {              /*インターフェース単位指定*/
        s_compare_len = DEF_tbl_chk_len_interface;
    } else if (c202->text_send_info.recv_con_id.connection_name[0] == '}') {    /*ステーション単位指定*/
        s_compare_len = DEF_tbl_chk_len_station;
    } else {                                                                    /*コネクション単位*/
        s_compare_len = DEF_tbl_chk_len_connection;
    }
    s_Thread = CNSV_sc_table_search((char *)&c202->text_send_info.recv_con_id.site_name,s_compare_len);
    if (s_Thread == -1) {                                       /*該当コネクションなし*/
        message_output(DEF_EVT_DST_SHITEI_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_DST_SELECT_ERR,"@X@X@X@X","","",DEF_IPC_IFCD_DEN_SND_REQ,"not Found",DEF_VAR_STOP);
        memcpy(r202.common_header.interface_code,DEF_IPC_IFCD_DEN_SND_RSP,sizeof(DEF_IPC_IFCD_DEN_SND_RSP)-1); /*R202*/
        r202.common_header.error_code = 9;
        memcpy(r202.common_header.internal_error_code,DEF_NERR_DST_SELECT_ERR,sizeof(DEF_NERR_DST_SELECT_ERR)-1);
        memset(r202.common_header.filler_1,0x20,sizeof(r202.common_header.filler_1));
        r202.common_header.control_data_length = 0;
        i_CC = REPLYX((char *)&r202,sizeof(r202),,IOCMP.RINF.z_messagetag);
        if (_status_ne(i_CC)) {
            FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
            AbNormal_End();
        }
        return;
    } else {
        memcpy(r202.common_header.interface_code,DEF_IPC_IFCD_DEN_SND_RSP,sizeof(DEF_IPC_IFCD_DEN_SND_RSP)-1); /*R202*/
        if ((sc_info[s_Thread].sock_fd == DEF_FILE_CLOSED) || cf[myinfo.cf_idx].sc_conf[s_Thread].use_on_off == '1') {  /*未接続、送信不可*/
            message_output(DEF_EVT_DST_SHITEI_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_DST_SELECT_ERR,"@X@X@X@X","","",DEF_IPC_IFCD_DEN_SND_REQ,"Socket Disconnect",DEF_VAR_STOP);
            r202.common_header.error_code = 9;
            memcpy(r202.common_header.internal_error_code,DEF_NERR_DST_SELECT_ERR,sizeof(DEF_NERR_DST_SELECT_ERR)-1);
        } else {
            r202.common_header.error_code = 0;
            memcpy(r202.common_header.internal_error_code,DEF_NERR_NOMAL,sizeof(DEF_NERR_NOMAL)-1);
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,s_Thread,DEF_EV_text_send_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        }
        memset(r202.common_header.filler_1,0x20,sizeof(r202.common_header.filler_1));
        r202.common_header.control_data_length = 0;
        i_CC = REPLYX((char *)&r202,sizeof(r202_def),,IOCMP.RINF.z_messagetag);
        if (_status_ne(i_CC)) {
            FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
            AbNormal_End();
        }
    }

} /*end of CNSV_send_req*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_open_timeout                           */
/*  CALLING SEQ.    : void CNSV_ob_open_timeout ( void )                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分オープンタイムアウト処理              */
/****************************************************************************/
void CNSV_ob_open_timeout(void)
{
    /*Nowaitオープンタイムアウトメッセージ*/
    CNSV_ob_open_err_control();
} /*end of CNSV_ob_open_timeout*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_short_retry_timeout                    */
/*  CALLING SEQ.    : void CNSV_ob_short_retry_timeout ( void )             */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分短期タイムアウト処理                  */
/****************************************************************************/
void CNSV_ob_short_retry_timeout(void)
{
    ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    /*タイムアウトメッセージ*/
    CNSV_outbound_open(IOCMP.thread);
} /*end of CNSV_ob_short_retry_timeout*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_long_retry_timeot                      */
/*  CALLING SEQ.    : void CNSV_ob_long_retry_timeot ( void )               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分長期タイムアウト処理                  */
/****************************************************************************/
void CNSV_ob_long_retry_timeout(void)
{
    ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    /*タイムアウトメッセージ*/
    CNSV_outbound_open(IOCMP.thread);
} /*end of CNSV_ob_long_retry_timeot*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_open_comp                              */
/*  CALLING SEQ.    : void CNSV_ob_open_comp ( void )                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分オープン完了処理                      */
/****************************************************************************/
void CNSV_ob_open_comp(void)
{
short   s_lc;
_cc_status i_CC;
    ob_info[IOCMP.thread].ipc_mng.ob_status = 5;
    if (ob_info[IOCMP.thread].ipc_mng.timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(ob_info[IOCMP.thread].ipc_mng.timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    }
    /*ポート状態通知コマンド*/
    for (s_lc = 0;s_lc < cf[myinfo.cf_idx].sc_use;s_lc++) {
        if (s_lc == 0){
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,IOCMP.thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
        } else {
            CNSV_add_list(&myinfo.free_list,&ob_info[IOCMP.thread].ipc_mng.send_wait,DEF_Component_Outbound,IOCMP.thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
        }
    }

} /*end of CNSV_ob_open_comp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_open_err                               */
/*  CALLING SEQ.    : void CNSV_ob_open_err ( void )                        */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分オープンエラー処理                    */
/****************************************************************************/
void CNSV_ob_open_err(short thread)
{
    CNSV_ob_open_err_control();
} /*end of CNSV_ob_open_err*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_io_err                                 */
/*  CALLING SEQ.    : void CNSV_ob_io_err ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分I/Oエラー処理                         */
/****************************************************************************/
void CNSV_ob_io_err(void)
{
buff_node_def *buff_node;
    if (IOCMP.fd == 0 && IOCMP.fs_err == 6 && timer_expire_p->z_msgnumber == -22 && IOCMP.component == DEF_Component_Outbound) {
        ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    }
    message_output(DEF_EVT_PROC_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@5","","",cf[myinfo.cf_idx].ob_conf[IOCMP.thread].process_name,"WRITEREAD",IOCMP.fs_err,DEF_VAR_STOP);
    /*バッファー開放*/
    if (ob_info[IOCMP.thread].ipc_mng.buf_p != 0) {
        buff_node = (buff_node_def *)(ob_info[IOCMP.thread].ipc_mng.buf_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
        ob_info[IOCMP.thread].ipc_mng.buf_p = 0;
    }
    CNSV_ob_open_err_control();
} /*end of CNSV_ob_io_err*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_io_comp                                */
/*  CALLING SEQ.    : void CNSV_ob_io_comp ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分I/O完了処理                           */
/****************************************************************************/
void CNSV_ob_io_comp(void)
{
buff_node_def *buff_node;
Event_Node_def Event;
    /*バッファー開放*/
    buff_node = (buff_node_def *)(ob_info[IOCMP.thread].ipc_mng.buf_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    ob_info[IOCMP.thread].ipc_mng.buf_p = 0;

    if (ob_info[IOCMP.thread].ipc_mng.send_wait.head != NULL) {
        CNSV_remove_list(&ob_info[IOCMP.thread].ipc_mng.send_wait,&myinfo.free_list,&Event);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,Event.compo,Event.thread,Event.event,Event.option1,Event.option2,Event.len,(char *)&Event.text);
    }
} /*end of CNSV_ob_io_comp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_send_port_status                       */
/*  CALLING SEQ.    : void CNSV_ob_send_port_status ( short , short )       */
/*  ARGUMENT        : Outbound電文振分スレッド番号                          */
/*                  : コネクションスレッド番号                              */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分ポート状態通知処理                    */
/****************************************************************************/
void CNSV_ob_send_port_status(short ob_thread,short sc_thread)
{
short s_Err;
buff_node_def *buff_node;
c107_def *c107;
short    s_Timeout_tag;
unsigned long ul_tag;
_cc_status i_CC;

    if (ob_info[ob_thread].ipc_mng.ob_fd == DEF_FILE_CLOSED) return;    /*電文振替クローズ中*/

    if (ob_info[ob_thread].ipc_mng.timer_tag != DEF_TAG_NULL) {
        CNSV_add_list(&myinfo.free_list,&ob_info[ob_thread].ipc_mng.send_wait,DEF_Component_Outbound,ob_thread,DEF_EV_tell_port_status,sc_thread,DEF_EV_tell_port_status,0,"");
        return;
    }

    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    ob_info[ob_thread].ipc_mng.buf_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    c107 = (c107_def *)ob_info[ob_thread].ipc_mng.buf_p;
    if (EXTRACEMODE) {
        ipc_trs = (lk_zac2001i_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ipc_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    memset((char *)c107,0x20,sizeof(c107_def));
    memcpy(c107->common_header.interface_code,DEF_IPC_IFCD_CON_STS_NT_REQ,sizeof(DEF_IPC_IFCD_CON_STS_NT_REQ)-1);
    c107->common_header.error_code = 0;
    memcpy(c107->common_header.internal_error_code,"0000000",sizeof(c107->common_header.internal_error_code));
    c107->common_header.control_data_length = 158;
    memcpy((char *)&c107->line_info,(char *)&cf[myinfo.cf_idx].sc_conf[sc_thread].site_name,sizeof(c107->line_info));
    memcpy((char *)&c107->connection_status_info,(char *)&cf[myinfo.cf_idx].sc_conf[sc_thread].site_name,sizeof(c107->connection_status_info));
    memcpy((char *)&c107->connection_status_info.connection_status,(char *)&sc_info[sc_thread].status_info.connection_status,sizeof(sc_info[sc_thread].status_info.connection_status));
    memcpy((char *)&c107->connection_status_info.connection_status_time,(char *)&sc_info[sc_thread].status_info.connection_status_time,sizeof(sc_info[sc_thread].status_info.connection_status_time));
    memcpy((char *)&c107->process_state_info.process_status,(char *)&sc_info[sc_thread].process_info.process_status,sizeof(sc_info[sc_thread].process_info.process_status));
    memcpy((char *)&c107->process_state_info.process_status_time,(char *)&sc_info[sc_thread].process_info.process_status_time,sizeof(sc_info[sc_thread].process_info.process_status_time));
    memcpy((char *)&c107->connection_info.src_ip_address,(char *)&cf[myinfo.cf_idx].sc_conf[sc_thread].local_ipaddr,strlen(cf[myinfo.cf_idx].sc_conf[sc_thread].local_ipaddr));
    memcpy((char *)&c107->connection_info.src_port,(char *)&cf[myinfo.cf_idx].sc_conf[sc_thread].local_port_no,strlen(cf[myinfo.cf_idx].sc_conf[sc_thread].local_port_no));
    memcpy((char *)&c107->connection_info.dest_ip_address,(char *)&cf[myinfo.cf_idx].sc_conf[sc_thread].remote_ipaddr,strlen(cf[myinfo.cf_idx].sc_conf[sc_thread].remote_ipaddr));
    memcpy((char *)&c107->connection_info.dest_port,(char *)&cf[myinfo.cf_idx].sc_conf[sc_thread].remote_port_no,strlen(cf[myinfo.cf_idx].sc_conf[sc_thread].remote_port_no));
    memcpy((char *)&c107->connection_info.connection_error_code,(char *)&sc_info[sc_thread].connection_info.error_code,sizeof(c107->connection_info));
    memcpy((char *)&c107->connection_info.disconnect_reason,(char *)&sc_info[sc_thread].connection_info.disconnect_reason,sizeof(c107->connection_info));
    memcpy((char *)&c107->sockaddr_in,(char *)&sc_info[sc_thread].sock,sizeof(c107->sockaddr_in));
    COM_TGM(DEF_Component_Outbound,ob_thread,DEF_EV_io_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(myinfo.process_io_timer
                       ,DEF_EV_io_timeout
                       ,(__int32_t)ul_tag
                       ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    ob_info[ob_thread].ipc_mng.timer_tag = s_Timeout_tag;
    COM_TGM(DEF_Component_Outbound,ob_thread,DEF_EV_con_state_notice,&ul_tag);
    s_Err = 0;
    i_CC = WRITEREADX(ob_info[ob_thread].ipc_mng.ob_fd
                     ,(char *)c107
                     ,sizeof(c107_def)
                     ,MAX_TEXT_BUF_LEN
                     ,
                     ,(__int32_t)ul_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(ob_info[ob_thread].ipc_mng.ob_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","WRITEREADX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ipc_trs->func_flg = '1';
        memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
        memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
        memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
        memcpy(ipc_trs->trace_info.file_name,cf[myinfo.cf_idx].ob_conf[ob_thread].process_name,cf[myinfo.cf_idx].ob_conf[ob_thread].process_name_len);
        memcpy(ipc_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(c107_def));
        memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
        TRACEOUT((char *)ipc_trs);
    }

} /*end of CNSV_ob_send_port_status*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_con_err                                */
/*  CALLING SEQ.    : void CNSV_ob_con_err ( Event_Node_def * )             */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分コネクションエラー処理                */
/****************************************************************************/
void CNSV_ob_con_err(Event_Node_def *CurrentEvent)
{
short   s_idx;
    /*send/recv等コネクションのエラー時*/
    for ( s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
        CNSV_ob_send_port_status(s_idx,CurrentEvent->option1);
    }
} /*end of CNSV_ob_con_err*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_open_err_control                       */
/*  CALLING SEQ.    : void CNSV_ob_open_err_control ( void )                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Outbound電文振分オープンエラーコントロール処理        */
/****************************************************************************/
void CNSV_ob_open_err_control(void)
{
short   s_ev_type;
short   s_Timeout_tag;
unsigned long ul_tag;
Event_Node_def Event;
_cc_status i_CC;

    /*タイムアウト時はタイマータグを初期化*/
    if ((IOCMP.fs_err == 6) && (open_msg_p->u_z_msgnumber.z_msgnumber == ZSYS_VAL_SMSG_TIMESIGNAL) && (ob_info[IOCMP.thread].ipc_mng.timer_tag != DEF_TAG_NULL)) {
        ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    }
    /*発行中のタイマーがあればキャンセル*/
    if (ob_info[IOCMP.thread].ipc_mng.timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(ob_info[IOCMP.thread].ipc_mng.timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    }

    /*Outbound電文振分オープン中(Nowaitオープン未完了含む)の場合、クローズ*/
    if (ob_info[IOCMP.thread].ipc_mng.ob_fd != DEF_FILE_CLOSED) {
        FILE_CLOSE_(ob_info[IOCMP.thread].ipc_mng.ob_fd);
        ob_info[IOCMP.thread].ipc_mng.ob_fd = DEF_FILE_CLOSED;
        if (EXTRACEMODE) {
            memset((char *)&ob_info[IOCMP.thread].ipc_mng.trs,' ',sizeof(lk_trace_def));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy((char *)ob_info[IOCMP.thread].ipc_mng.trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
            ob_info[IOCMP.thread].ipc_mng.trs.func_flg = '1';
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.prog_id));
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.file_id,"PROCESS ",8);
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.file_name,ob_info[IOCMP.thread].ipc_mng.outbound_name,strlen(ob_info[IOCMP.thread].ipc_mng.outbound_name));
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.file_io_type,"CLOSE   ",8);
            memset(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.guardian_errcode,'0',sizeof(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.guardian_errcode));
            memset(ob_info[IOCMP.thread].ipc_mng.trs.data_info.rec_len,'0',sizeof(ob_info[IOCMP.thread].ipc_mng.trs.data_info.rec_len));
            TRACEOUT((char *)&ob_info[IOCMP.thread].ipc_mng.trs);
        }
    }

    while (ob_info[IOCMP.thread].ipc_mng.send_wait.head != NULL) {
        CNSV_remove_list(&ob_info[IOCMP.thread].ipc_mng.send_wait,&myinfo.free_list,&Event);
    }

    /*エラー処理動作の判定*/
    s_ev_type = 0;
    switch (ob_info[IOCMP.thread].ipc_mng.ob_status)
    {
    case 0:     /*クローズ中*/
        AbNormal_End();
        break;
    case 1:     /*オープン完了待ち*/
        s_ev_type = DEF_EV_S_retry_timeout;
        ob_info[IOCMP.thread].ipc_mng.ob_status = 2;
        ob_info[IOCMP.thread].ipc_mng.retry_type = 1;
        ob_info[IOCMP.thread].ipc_mng.retry_timer_value = cf[myinfo.cf_idx].st_conf[ob_info[IOCMP.thread].station_index].short_retry_timer;
        ob_info[IOCMP.thread].ipc_mng.retry_count = 0;
        ob_info[IOCMP.thread].ipc_mng.retry_max  = cf[myinfo.cf_idx].st_conf[ob_info[IOCMP.thread].station_index].short_retry_count;
        break;
    case 2:     /*ショートリトライ中*/
        ob_info[IOCMP.thread].ipc_mng.retry_count++;
        s_ev_type = DEF_EV_S_retry_timeout;
        if (ob_info[IOCMP.thread].ipc_mng.retry_count > ob_info[IOCMP.thread].ipc_mng.retry_max) {
            /*ショートリトライ、リトライオーバーメッセージ出力*/
            ob_info[IOCMP.thread].ipc_mng.ob_status = 3;
            ob_info[IOCMP.thread].ipc_mng.retry_type = 2;
            ob_info[IOCMP.thread].ipc_mng.retry_timer_value = cf[myinfo.cf_idx].st_conf[ob_info[IOCMP.thread].station_index].long_retry_timer;
            ob_info[IOCMP.thread].ipc_mng.retry_count = 0;
            ob_info[IOCMP.thread].ipc_mng.retry_max = cf[myinfo.cf_idx].st_conf[ob_info[IOCMP.thread].station_index].long_retry_count;
            s_ev_type = DEF_EV_L_retry_timeout;
        }
        break;
    case 3:     /*ロングリトライ中*/
        s_ev_type = DEF_EV_L_retry_timeout;
        ob_info[IOCMP.thread].ipc_mng.retry_count++;
        if (ob_info[IOCMP.thread].ipc_mng.retry_count > ob_info[IOCMP.thread].ipc_mng.retry_max) {
            /*ロングリトライ、リトライオーバーメッセージ出力*/
            ob_info[IOCMP.thread].ipc_mng.ob_status = 4;
            return;
        }
        break;
    case 4:     /*リトライオーバ*/
        /*内部矛盾、発生しない*/
        AbNormal_End();
        break;
    case 5:     /*オープン(online)中*/
        s_ev_type = DEF_EV_S_retry_timeout;
        ob_info[IOCMP.thread].ipc_mng.retry_type = 1;
        ob_info[IOCMP.thread].ipc_mng.retry_timer_value = cf[myinfo.cf_idx].st_conf[ob_info[IOCMP.thread].station_index].short_retry_timer;
        ob_info[IOCMP.thread].ipc_mng.retry_count = 0;
        ob_info[IOCMP.thread].ipc_mng.retry_max  = cf[myinfo.cf_idx].st_conf[ob_info[IOCMP.thread].station_index].short_retry_count;
        break;
    default:
        AbNormal_End();
        break;
    }

    if (s_ev_type) {
        COM_TGM(DEF_Component_Outbound,IOCMP.thread,s_ev_type,&ul_tag);
        i_CC = SIGNALTIMEOUT(ob_info[IOCMP.thread].ipc_mng.retry_timer_value
                            ,s_ev_type
                            ,(__int32_t)ul_tag
                            ,&s_Timeout_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        ob_info[IOCMP.thread].ipc_mng.timer_tag = s_Timeout_tag;
    }
    return;
} /*end of CNSV_ob_open_err_control*/
