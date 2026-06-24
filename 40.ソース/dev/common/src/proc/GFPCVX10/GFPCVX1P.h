/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP制御                                     */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
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
#ifndef _GFPCVX1P_H_
#define _GFPCVX1P_H_
/* prototype     */
/* GFPCVX11 プロセス管理コンポーネント(sys) */
void CNSV_initialize(void);
void CNSV_get_param(void);
void CNSV_recovery_processing(void);
void CNSV_main_processing(void);
void CNSV_io_wait(void);
void CNSV_event_judgement(void);
void CNSV_sysmsg_manage(void);
void CNSV_open_msg(short *);
void CNSV_close_msg(void);
void CNSV_signal_msg(void);
void CNSV_end_processing(void);
void CNSV_cpu_down_msg(void);
void CNSV_r_cpu_down_msg(void);
void CNSV_node_down_msg(void);
void CNSV_cpu_up_msg(void);
void CNSV_r_cpu_up_msg(void);
void CNSV_node_up_msg(void);
void CNSV_cancel_msg(void);
void CNSV_unknown_system_msg(void);
void CNSV_msg_manage(void);
void CNSV_cmd_req(void);
void CNSV_unknown_req(void);
void CNSV_io_comp(void);
void CNSV_pathsend_comp(void);
void CNSV_sock_comp(void);
void CNSV_lsn_comp(void);
void CNSV_ob_comp(void);

/* GFPCVX12 コネクション管理コンポーネント(so) */
void CNSV_sock_manage(Event_Node_def *);
void CNSV_recv_complete(short);
void CNSV_remain_recv(short);
void CNSV_recv(short);
void CNSV_port_open(short,short);
void CNSV_port_close(short,short,short);
void CNSV_recv_error(Event_Node_def *);
void CNSV_send_complete(short);
void CNSV_remain_send(short);
void CNSV_send_error(Event_Node_def *);
void CNSV_recv_complete_reply(short);
void CNSV_send_request(short);
void CNSV_disconnect_reply();
void CNSV_connect_data_send(short);

/* GFPCVX13 リスナー管理コンポーネント(lsn) */
void CNSV_lsn_manage(Event_Node_def *);
void CNSV_lsn_io_err(short);
void CNSV_lsn_short_timeout(short);
void CNSV_lsn_long_timeout(short);
short CNSV_Listener_open(short);
void CNSV_Listener_close(short);
void CNSV_connect_notice(Event_Node_def *);
void CNSV_dis_reconnect_notice(Event_Node_def *);
void CNSV_request_complete_resp(short);
void CNSV_send_async(short);
void CNSV_lc_sock_err(Event_Node_def *);
void CNSV_lsn_unknown_resp(short);
void CNSV_lsn_excluded_req(Event_Node_def *);
void CNSV_lsn_tell_chg_sock(Event_Node_def *);
void CNSV_lsn_tell_port_status(Event_Node_def *);

/* GFPCVX14 INBOUND電文振分管理コンポーネント(ib) */
void CNSV_ib_manage(Event_Node_def *);
void CNSV_recv_msg_send(Event_Node_def *);
void CNSV_message_pathsend(Event_Node_def *);
void CNSV_recv_msg_resp(void);
void CNSV_in_unknown_resp(Event_Node_def *);
void CNSV_in_ps_err(Event_Node_def *);

/* GFPCVX15 OUTBOUND電文振分管理コンポーネント(ob) */
void CNSV_ob_manage(Event_Node_def *);
void CNSV_outbound_open(short);
void CNSV_outbound_close(short);
void CNSV_outbound_cancel(short);
void CNSV_ob_open_comp(void);
void CNSV_ob_io_err(void);
void CNSV_ob_io_comp(void);
void CNSV_send_req(void);
void CNSV_ob_open_err(short);
void CNSV_ob_open_timeout(void);
void CNSV_ob_short_retry_timeout(void);
void CNSV_ob_long_retry_timeout(void);
void CNSV_ob_con_err(Event_Node_def *);
void CNSV_ob_send_port_status(short,short);
void CNSV_ob_open_err_control(void);

/* GFPCVX16 コマンドサーバー管理コンポーネント(ci) */
void CNSV_ci_manage(Event_Node_def *);
void CNSV_ci_cancel(void);
void CNSV_connect_req(void);
void CNSV_disconnect_req(void);
void CNSV_reload_command(void);
void CNSV_cmd_resp(void);
void CNSV_ci_signon_req(Event_Node_def *);
void CNSV_ci_unknown_resp(void);
void CNSV_ci_ps_err(void);

/* GFPCVX17 プログラム内共通 */
short CNSV_LOAD(void);
short CNSV_GFPHI_open(void);
short CNSV_GFPHI_close(void);
short CNSV_GFPHI_load(void);
short CNSV_GFLIN_load(short);
short CNSV_GFNWI_load(short);
short CNSV_GCLST_open(void);
short CNSV_GCLST_load(short);
short CNSV_GCLST_update(short, short, short);
short CNSV_load_server_info(short);
short CNSV_sc_table_search(char *,short);
void CNSV_stop_check(short);
void AbNormal_End(void);
void CNSV_create_list(Event_List_def *, int);
void CNSV_add_list(Event_List_def *, Event_List_def *, short, short, short, short, short, short, char *);
void CNSV_remove_list(Event_List_def *, Event_List_def *,Event_Node_def *);
void CNSV_create_buff_list(buff_list_def *, int);
void message_output (short ,short ,char *,char *,...);
//-void BCD2CHAR(unsigned char *, char *,short);
short EBCNUM2CHAR(unsigned char *,char *,short);
void CNSV_set_null(char *,int);
short CNSV_GFLIN_Rebuild(short ,short );
short CNSV_GFLIN_key_read(char *, short ,short ,short ,short *);

/*ZAC20XX*/
short TRACEOUT( char* );

/*ZAD1001*/
//void EBCDIC2SJIS(char *,char *,short);
//void EBCDICKANA2SJIS(char *,char *,short);
//void EBCDIK2SJIS(char *,char *,short);
//void SJIS2EBCDIC(char *,char *,short);
//void SJIS2EBCDICKANA(char *,char *,short);
//void SJIS2EBCDIK (char *,char *,short);
//void BCD2CHAR(unsigned char *, char *, short);
//short CHAR2BCD(unsigned char *, short *);
//void HEX2CHAR(unsigned char *, char *, short);
//short CHAR2HEX(const char *, char *, short);

#endif
