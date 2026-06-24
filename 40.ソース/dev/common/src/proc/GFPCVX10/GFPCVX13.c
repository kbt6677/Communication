/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GGFP通信制御                                */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
/*                               リスナー管理コンポーネント(lsn)             */
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
/*  FUNCTION        : 1.1.0  CNSV_lsn_manage                                */
/*  CALLING SEQ.    : void CNSV_lsn_manage ( Event_Node_def * )             */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナー管理処理                                      */
/****************************************************************************/
void CNSV_lsn_manage(Event_Node_def *Event)
{
short   s_Err;
short   s_Timeout_tag;
unsigned long ul_tag;
_cc_status i_CC;
    switch (Event->event)
    {
        case DEF_EV_io_timeout:                 /*I/O完了待ちタイマー*/
            CNSV_lsn_io_err(Event->thread);
            break;
        case DEF_EV_S_retry_timeout:            /*ショートリトライタイマー*/
            CNSV_lsn_short_timeout(Event->thread);
            break;
        case DEF_EV_L_retry_timeout:            /*ロングリトライタイマー*/
            CNSV_lsn_long_timeout(Event->thread);
            break;
        case DEF_EV_sock_recv_err:              /*recvエラー*/
        case DEF_EV_sock_send_err:              /*sendエラー*/
            /*R105:コネクション切断完了通知応答を該当リスナーに送る*/
            CNSV_lc_sock_err(Event);
            break;
        case DEF_EV_connect_notice:             /*N101:コネクション接続通知*/
            CNSV_connect_notice(Event);
            break;
        case DEF_EV_dis_reconnect_notice:       /*N102:コネクション入替・切断指示通知*/
            CNSV_dis_reconnect_notice(Event);
            break;
        case DEF_EV_connect_start_resp:         /*R103:コネクション接続開始応答*/
        case DEF_EV_connect_complete_resp:      /*R104:コネクション接続完了通知応答*/
        case DEF_EV_discinnect_complete_resp:   /*R105:コネクション切断完了通知応答*/
        case DEF_EV_chg_disconnect_notice:      /*R106:コネクション入替・切断完了通知応答*/
            CNSV_request_complete_resp(Event->thread);
            break;
        case DEF_EV_unknown_resp:               /*不明応答処理*/
            CNSV_lsn_unknown_resp(Event->thread);
            break;
        case DEF_EV_Listener_io_err:            /*リスナーI/Oエラー*/
            CNSV_lsn_io_err(Event->thread);
            break;
        case DEF_EV_connect_req:                /*C001 接続要求*/
            s_Err = CNSV_Listener_open(Event->thread);
            if (s_Err) {
                lp_info[Event->thread].retry_type = 1;
                lp_info[Event->thread].retry_timer_value = cf[myinfo.cf_idx].st_conf[lc_info[myinfo.cf_idx].station_index].short_retry_timer;
                lp_info[Event->thread].retry_count = 0;
                lp_info[Event->thread].retry_max = cf[myinfo.cf_idx].st_conf[lc_info[myinfo.cf_idx].station_index].short_retry_count;
                COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_S_retry_timeout,&ul_tag);
                i_CC = SIGNALTIMEOUT(lp_info[Event->thread].retry_timer_value
                                   ,DEF_EV_S_retry_timeout
                                   ,(__int32_t)ul_tag
                                   ,&s_Timeout_tag);
                if (_status_ne(i_CC)) {
                    message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
                    AbNormal_End();
                }
                lp_info[Event->thread].timer_tag = s_Timeout_tag;
            }
            break;
        case DEF_EV_disconnect_req:             /*C002 切断要求*/
            CNSV_Listener_close(Event->thread);
            break;
        case DEF_EV_tell_port_status:           /*Listenerポート状態送信*/
            CNSV_lsn_tell_port_status(Event);
            break;
        case DEF_EV_chg_sock_close:             /*コネクション入替・切断完了通知(切断のみ)*/
            CNSV_lsn_tell_chg_sock(Event);
            break;
        case DEF_EV_chg_sock_open:              /*コネクション入替・切断完了通知(切替)*/
            CNSV_lsn_tell_chg_sock(Event);
            break;
        case DEF_EV_excluded_req:
            CNSV_lsn_excluded_req(Event);
            break;
        default:
            AbNormal_End();
            break;
    }
} /*end of CNSV_lsn_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_io_err                                */
/*  CALLING SEQ.    : void CNSV_lsn_io_err ( short )                        */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナーI/Oエラー処理                                 */
/****************************************************************************/
void CNSV_lsn_io_err(short thread)
{
short   s_Timeout_tag;
unsigned long ul_tag;
Event_Node_def  Event_Node;
buff_node_def *buff_node;
_cc_status i_CC;
lk_trace_def trs;
    if (IOCMP.fd == 0 && IOCMP.fs_err == 6 && timer_expire_p->z_msgnumber == -22) {
        lp_info[thread].timer_tag = DEF_TAG_NULL;
    }
    if (lp_info[thread].timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(lp_info[thread].timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        lp_info[thread].timer_tag = DEF_TAG_NULL;
    }
    /*エラーメッセージ出力*/
    message_output(DEF_EVT_PROC_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@5","","",(char *)&cf[myinfo.cf_idx].lc_conf[thread].process_name,"WRITEREAD",IOCMP.fs_err,DEF_VAR_STOP);
    if (lp_info[thread].listner_fd != DEF_FILE_CLOSED) {
        FILE_CLOSE_(lp_info[thread].listner_fd);
        if (EXTRACEMODE) {
            memset((char *)&trs.func_flg,' ',sizeof(lk_trace_def));     /*トレース情報初期化*/
            trs.func_flg = '1';
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy((char *)trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(trs.trace_info.shori_start_time));
            memcpy(trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(trs.trace_info.shori_end_time));
            memcpy(trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(trs.trace_info.prog_id));
            memcpy(trs.trace_info.file_id,"PROCESS ",8);
            memcpy(trs.trace_info.file_name,lp_info[thread].listner_name,strlen(lp_info[thread].listner_name));
            memcpy(trs.trace_info.file_io_type,"CLOSE   ",8);
            memset(trs.trace_info.guardian_errcode,'0',sizeof(trs.trace_info.guardian_errcode));
            memset(trs.data_info.rec_len,'0',sizeof(trs.data_info.rec_len));
            TRACEOUT((char *)&trs);
        }
    }
    lp_info[thread].listner_fd = DEF_FILE_CLOSED;
    if (lp_info[thread].async_p != NULL) {
        buff_node = (buff_node_def *)(lp_info[thread].async_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        lp_info[thread].async_p = NULL;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    }
    if (lp_info[thread].buf_p != NULL) {
        buff_node = (buff_node_def *)(lp_info[thread].buf_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        lp_info[thread].buf_p = NULL;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    }
    while (lp_info[thread].send_wait.head) {    /*未処理イベントの開放*/
        CNSV_remove_list(&lp_info[thread].send_wait,&myinfo.free_list,&Event_Node);
    }
    /*該当するネットワークのショートリトライ情報をテーブルに設定*/
    if (lp_info[thread].retry_type == 0) {
        lp_info[thread].retry_type = 1;
        lp_info[thread].retry_timer_value = cf[myinfo.cf_idx].st_conf[lc_info[myinfo.cf_idx].station_index].short_retry_timer;
        lp_info[thread].retry_count = 0;
        lp_info[thread].retry_max = cf[myinfo.cf_idx].st_conf[lc_info[myinfo.cf_idx].station_index].short_retry_count;
    }
    COM_TGM(DEF_Component_Listener,thread,DEF_EV_S_retry_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(lp_info[thread].retry_timer_value
                       ,DEF_EV_S_retry_timeout
                       ,(__int32_t)ul_tag
                       ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[thread].timer_tag = s_Timeout_tag;
} /*end of CNSV_lsn_io_err*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_short_timeout                         */
/*  CALLING SEQ.    : void CNSV_lsn_short_timeout ( short )                 */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナー短期タイムアウト処理                          */
/****************************************************************************/
void CNSV_lsn_short_timeout(short thread)
{
short   s_Err;
short   s_Timeout_tag;
short   s_Event;
unsigned long ul_tag;
_cc_status i_CC;
    /*該当リスナーのオープン*/
    s_Err = CNSV_Listener_open(thread);
    if (s_Err == 0) {
        /*オープン正常の場合呼び出し元にリターンする*/
        return;
    }
    /*該当するネットワークのショートリトライ情報をテーブルに設定*/
    lp_info[thread].listner_fd = DEF_FILE_CLOSED;
    lp_info[thread].retry_count++;
    s_Event = DEF_EV_S_retry_timeout;
    if (lp_info[thread].retry_count >= lp_info[thread].retry_max) {
        /*エラーメッセージ出力*/
        lp_info[thread].retry_type = 2;
        lp_info[thread].retry_timer_value = cf[myinfo.cf_idx].st_conf[lc_info[myinfo.cf_idx].station_index].long_retry_timer;
        lp_info[thread].retry_count = 0;
        lp_info[thread].retry_max = cf[myinfo.cf_idx].st_conf[lc_info[myinfo.cf_idx].station_index].long_retry_count;
        s_Event = DEF_EV_L_retry_timeout;
    }
    COM_TGM(DEF_Component_Listener,thread,s_Event,&ul_tag);
    i_CC = SIGNALTIMEOUT(lp_info[thread].retry_timer_value
                        ,s_Event
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[thread].timer_tag = s_Timeout_tag;
} /*end of CNSV_lsn_short_timeout*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_long_timeout                          */
/*  CALLING SEQ.    : void CNSV_lsn_long_timeout ( short )                  */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナー長期タイムアウト処理                          */
/****************************************************************************/
void CNSV_lsn_long_timeout(short thread)
{
short   s_Err;
short   s_Timeout_tag;
short   s_Event;
unsigned long ul_tag;
_cc_status i_CC;
    /*該当リスナーのオープン*/
    s_Err = CNSV_Listener_open(thread);
    if (s_Err == 0) {
        return;
    }
    lp_info[thread].listner_fd = DEF_FILE_CLOSED;
    lp_info[thread].retry_count++;
    s_Event = DEF_EV_L_retry_timeout;
    if (lp_info[thread].retry_count >= lp_info[thread].retry_max) {
        /*■エラーメッセージ出力*/
        return;
    }
    COM_TGM(DEF_Component_Listener,thread,s_Event,&ul_tag);
    i_CC = SIGNALTIMEOUT(lp_info[thread].retry_timer_value
                        ,s_Event
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[thread].timer_tag = s_Timeout_tag;
} /*end of CNSV_lsn_long_timeout*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_Listener_open                             */
/*  CALLING SEQ.    : short CNSV_Listener_open ( short )                    */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : 処理結果                                              */
/*  DESCRIPTION     : リスナーオープン処理                                  */
/****************************************************************************/
// コマンドが送ってくるコネクト要求はlpのスレッド番号
// リカバリーで送ってくるコネクト要求はlpのスレッド番号
short CNSV_Listener_open(short thread)
{
short   s_Err;
short   s_lc,s_1st;
char    ach_1st_qualifier[8+1];
char    ach_2nd_qualifier[8+1];
char    ach_listner_name[ZSYS_VAL_LEN_PROCESSDESCR+1];
//buff_node_def *buff_node;
lk_trace_def trs;
_cc_status i_CC;

    if (lp_info[thread].listner_fd != DEF_FILE_CLOSED) {
        /*既にOPEN中の場合にもリスナーに対してコネクション接続開始要求(C103)を送信する*/
        for ( s_lc = 0, s_1st = 0; s_lc < cf[myinfo.cf_idx].sc_use; s_lc++ ) {
            if ( lc_info[sc_info[s_lc].listner_index].mng_no == thread ) {
                if (s_1st == 0) {
                    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
                    s_1st = 1;
                } else {
                    CNSV_add_list(&myinfo.free_list,&lp_info[thread].send_wait,DEF_Component_Listener,thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
                }
            }
        }
        return (0);
    }
    memset(ach_1st_qualifier,'\0',sizeof(ach_1st_qualifier));
    memset(ach_2nd_qualifier,'\0',sizeof(ach_2nd_qualifier));
    memcpy(ach_1st_qualifier,myinfo.server_class_name,6);
    memcpy(ach_2nd_qualifier,(char *)&myinfo.server_class_name[6],2);
    memcpy((char *)&ach_2nd_qualifier[2],myinfo.server_class_num,4);
    sprintf(ach_listner_name,"%s.#%s.%s",lp_info[thread].process_name,ach_1st_qualifier,ach_2nd_qualifier);
    strcpy(lp_info[thread].listner_name,ach_listner_name);
    if (EXTRACEMODE) {
        memset((char *)&trs.func_flg,' ',sizeof(lk_trace_def));     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(trs.trace_info.shori_start_time));
    }
    s_Err = FILE_OPEN_(ach_listner_name
                      ,(short)strlen(ach_listner_name)
                      ,&lp_info[thread].listner_fd
                      ,ZSYS_VAL_OPENACC_READWRITE
                      ,ZSYS_VAL_OPENEXCL_SHARED
                      ,DEF_LISTENER_NOWAITDEPTH);
    if (EXTRACEMODE) {
        trs.func_flg = '1';
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy(trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(trs.trace_info.shori_end_time));
        memcpy(trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(trs.trace_info.prog_id));
        memcpy(trs.trace_info.file_id,"PROCESS ",8);
        memcpy(trs.trace_info.file_name,lp_info[thread].listner_name,strlen(lp_info[thread].listner_name));
        memcpy(trs.trace_info.file_io_type,"OPEN    ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(trs.trace_info.guardian_errcode,trace_work,sizeof(trs.trace_info.guardian_errcode));
        memset(trs.data_info.rec_len,'0',sizeof(trs.data_info.rec_len));
        TRACEOUT((char *)&trs);
    }
    if (s_Err) {
        /*■オープンエラー*/
        return (s_Err);
    }
    i_CC = SETMODE(lp_info[thread].listner_fd
                  ,DEF_ALLOW_NOWAIT_OPERATIONS
                  ,DEF_COMPLETE_OPERATIONS_ANY_ORDER
                  );
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(lp_info[thread].listner_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SETMODE",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[thread].retry_type = 0; /*リトライタイプ初期化*/
    CNSV_send_async(thread);    /*非同期発行*/
    /*該当リスナーのコネクション分、通知を積む*/
//    for ( s_lc = 0, s_1st = 0; s_lc < myinfo.lp_info_use_cnt; s_lc++ ) {
//        if (s_1st == 0) {
//            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
//            s_1st = 1;
//        } else {
//            CNSV_add_list(&myinfo.free_list,&lp_info[thread].send_wait,DEF_Component_Listener,thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
//        }
//    }
    for ( s_lc = 0, s_1st = 0; s_lc < cf[myinfo.cf_idx].sc_use; s_lc++ ) {
        if ( lc_info[sc_info[s_lc].listner_index].mng_no == thread ) {
            if (s_1st == 0) {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
                s_1st = 1;
            } else {
                CNSV_add_list(&myinfo.free_list,&lp_info[thread].send_wait,DEF_Component_Listener,thread,DEF_EV_tell_port_status,s_lc,DEF_EV_tell_port_status,0,"");
            }
        }
    }
    return (s_Err);
} /*end of CNSV_Listener_open*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_Listener_close                            */
/*  CALLING SEQ.    : void CNSV_Listener_close ( short )                    */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナークローズ処理                                  */
/****************************************************************************/
// コマンドが送ってくる要求はコネクション定義(sc)単位のスレッド番号
void CNSV_Listener_close(short event_thread)
{
short           thread; /*対象となるリスナープロセス管理情報テーブル番号*/
short           s_lc,s_cnt;
Event_Node_def  Event_Node;
buff_node_def *buff_node;
lk_trace_def trs;
_cc_status i_CC;

    thread = lc_info[event_thread].mng_no;      /*スレッド番号設定*/

    for (s_lc = 0, s_cnt= 0;s_lc < cf[myinfo.cf_idx].sc_use;s_lc++ ) {
        if ((sc_info[s_lc].listner_index == thread) && sc_info[s_lc].sock_fd != DEF_FILE_CLOSED) {
            s_cnt++;
        }
    }
    if (s_cnt != 0) {       /*接続中コネクション有り*/
        return;
    }
    if (lp_info[thread].timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(lp_info[thread].timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        lp_info[thread].timer_tag = DEF_TAG_NULL;
    }
    if (lp_info[thread].listner_fd != DEF_FILE_CLOSED) {
        FILE_CLOSE_(lp_info[thread].listner_fd);
        if (EXTRACEMODE) {
            memset((char *)&trs.func_flg,' ',sizeof(lk_trace_def));     /*トレース情報初期化*/
            trs.func_flg = '1';
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy((char *)trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(trs.trace_info.shori_start_time));
            memcpy(trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(trs.trace_info.shori_end_time));
            memcpy(trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(trs.trace_info.prog_id));
            memcpy(trs.trace_info.file_id,"PROCESS ",8);
            memcpy(trs.trace_info.file_name,lp_info[thread].listner_name,strlen(lp_info[thread].listner_name));
            memcpy(trs.trace_info.file_io_type,"CLOSE   ",8);
            memset(trs.trace_info.guardian_errcode,'0',sizeof(trs.trace_info.guardian_errcode));
            memset(trs.data_info.rec_len,'0',sizeof(trs.data_info.rec_len));
            TRACEOUT((char *)&trs);
        }
    }
    lp_info[thread].listner_fd = DEF_FILE_CLOSED;
    if (lp_info[thread].async_p != NULL) {
        buff_node = (buff_node_def *)(lp_info[thread].async_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        lp_info[thread].async_p = NULL;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    }
    if (lp_info[thread].buf_p != NULL) {
        buff_node = (buff_node_def *)(lp_info[thread].buf_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        lp_info[thread].buf_p = NULL;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    }
    while (lp_info[thread].send_wait.head) {    /*未処理イベントの開放*/
        CNSV_remove_list(&lp_info[thread].send_wait,&myinfo.free_list,&Event_Node);
    }
} /*end of CNSV_Listener_close*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_connect_notice                            */
/*  CALLING SEQ.    : void CNSV_connect_notice ( Event_Node_def * )         */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : N101:コネクション接続通知処理                         */
/****************************************************************************/
// Event->thread lpのスレッド番号
// Event->option1 scのスレッド番号
// バッファ開放しているがポインターはそのままのためn102の情報はアクセスできる。
void CNSV_connect_notice(Event_Node_def *Event)
{
buff_node_def *buff_node;

buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    lp_info[Event->thread].async_p = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,Event->option1,DEF_EV_connect_notice,Event->thread,0,IOCMP.len,(char *)IOCMP.addr);
    CNSV_send_async(Event->thread);
} /*end of CNSV_connect_notice*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_dis_reconnect_notice                      */
/*  CALLING SEQ.    : void NSV_dis_reconnect_notice ( Event_Node_def * )    */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : N102:コネクション入替・切断指示通知                   */
/****************************************************************************/
// Event->thread lpのスレッド番号
// Event->option1 scのスレッド番号または-1(クローズのみ)
// バッファ開放しているがポインターはそのままのためn102の情報はアクセスできる。
void CNSV_dis_reconnect_notice(Event_Node_def *Event)
{
buff_node_def *buff_node;
short s_idx,s_compare_len,s_option;

    buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    lp_info[Event->thread].async_p = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;

    if (n102->disconnect_info.disconnect_info == 'S') { /*比較長設定(cf[myinfo.cf_idx].connect_num_mng_lyrと同じ)*/
        s_compare_len = DEF_GROUP_len_station;
    } else {
        s_compare_len = DEF_GROUP_len_interface;
    }
    if (n102->line_info.site_name != 0x20) {        /*接続指示あり*/ /*再接続の場合要求元リスナーのみ*/
        s_option = false;
    } else {                                        /*再接続なしの場合、入換応答を各リスナーに通知する*/
        s_option = true;
    }
    CNSV_send_async(Event->thread);
    /*ソケット管理へのイベント発行*/
    for ( s_idx = 0; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {  /*対象検索、イベント登録*/
        if ((memcmp((char *)&n102->listen_info,(char *)&sc_info[s_idx].site_name,s_compare_len) == 0) &&
            (sc_info[s_idx].sock_fd != DEF_FILE_CLOSED)) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,s_idx,DEF_EV_chg_sock_close,Event->thread,s_option,IOCMP.len,(char *)n101);
        }
    }
    if (n102->line_info.site_name != 0x20) {        /*接続指示あり*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket,Event->option1,DEF_EV_chg_sock_open,0,0,IOCMP.len,(char *)n101);
        for (s_idx = 0; s_idx < myinfo.lp_info_use_cnt; s_idx++ ) { /*全リスナーに通知*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_chg_sock_open ,Event->option1,true,IOCMP.len,(char *)n101);
        }
    } else {    /*クローズのみ*/
        for (s_idx = 0; s_idx < myinfo.lp_info_use_cnt; s_idx++ ) { /*全リスナーに通知*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_chg_sock_close,-1,true,IOCMP.len,(char *)n101);
        }
    }
} /*end of CNSV_dis_reconnect_notice*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_excluded_req                          */
/*  CALLING SEQ.    : void CNSV_lsn_excluded_req ( Event_Node_def *thread ) */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 対象外コネクションに対する要求処理                    */
/****************************************************************************/
// Event->thread lpのスレッド番号
// n102はCNSV_lsn_compで設定済
void CNSV_lsn_excluded_req(Event_Node_def *Event)
{
short s_Err;
_cc_status i_CC;
buff_node_def *buff_node;
c106_def *c106;
short   s_Timeout_tag;
unsigned long ul_tag;

    buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    lp_info[Event->thread].buf_p = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;

    CNSV_send_async(Event->thread);

    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    lp_info[Event->thread].buf_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    if (EXTRACEMODE) {
        ipc_trs = (lk_zac2001i_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ipc_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    c106 = (c106_def *)lp_info[Event->thread].buf_p;
    memset((char *)c106,0x20,sizeof(c106_def));
    memcpy(c106->common_header.interface_code,DEF_IPC_IFCD_CON_SW_NT_REQ,sizeof(DEF_IPC_IFCD_CON_SW_NT_REQ)-1);
    c106->common_header.error_code = 0;
    memcpy((char *)&c106->common_header.internal_error_code,"0000000",sizeof(c106->common_header.internal_error_code));
    memset((char *)&c106->common_header.filler_1,' ',sizeof(c106->common_header.filler_1));
    c106->common_header.control_data_length = 150;
    memcpy((char *)&c106->line_info,(char *)&n102->listen_info,sizeof(c106->line_info));

    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_io_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(myinfo.process_io_timer
                        ,DEF_EV_io_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[Event->thread].timer_tag = s_Timeout_tag;
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_connect_start_resp,&ul_tag);
    s_Err = 0;
    i_CC = WRITEREADX(lp_info[Event->thread].listner_fd
                     ,(char *)c106
                     ,sizeof(c106_def)
                     ,MAX_TEXT_BUF_LEN
                     ,,(__int32_t)ul_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(lp_info[Event->thread].listner_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","WRITEREADX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ipc_trs->func_flg = '1';
        memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
        memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
        memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
        memcpy(ipc_trs->trace_info.file_name,lp_info[Event->thread].process_name,lp_info[Event->thread].process_name_len);
        memcpy(ipc_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(c106_def));
        memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
        TRACEOUT((char *)ipc_trs);
    }
} /*end of CNSV_lsn_excluded_req*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_request_complete_resp                     */
/*  CALLING SEQ.    : void CNSV_request_complete_resp ( short )             */
/*  ARGUMENT        : スレッド番号 (I/O完了のためlpのスレッド番号)          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リクエスト完了処理                                    */
/****************************************************************************/
void CNSV_request_complete_resp(short thread)
{
buff_node_def  *buff_node;
Event_Node_def  InEvent;

    buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    lp_info[thread].buf_p = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    if (lp_info[thread].send_wait.head != 0) {
        CNSV_remove_list(&lp_info[thread].send_wait,&myinfo.free_list,&InEvent);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,InEvent.compo,InEvent.thread,InEvent.event,InEvent.option1,InEvent.option2,InEvent.len,(char *)&InEvent.text);
    }
} /*end of CNSV_request_complete_resp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_send_async                                */
/*  CALLING SEQ.    : void CNSV_send_async ( short )                        */
/*  ARGUMENT        : スレッド番号 (lpのスレッド番号)                       */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 非同期送信処理                                        */
/****************************************************************************/
void CNSV_send_async(short thread)
{
short           s_Err;
_cc_status      i_CC;
buff_node_def   *buff_node;
a001_def    *a001;
unsigned long ul_tag;

    /*##バッファ取得*/
    buff_node = myinfo.buffs.head;
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    lp_info[thread].async_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    if (EXTRACEMODE) {
        ipc_trs = (lk_zac2001i_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ipc_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    a001 = (a001_def *)lp_info[thread].async_p;
    memset((char *)a001,0x20,sizeof(a001_def));
    memcpy(a001->common_header.interface_code,DEF_IPC_IFCD_CON_NT_ACTL_REQ,sizeof(DEF_IPC_IFCD_CON_NT_ACTL_REQ)-1);
    a001->common_header.error_code = 0;
    memcpy(a001->common_header.internal_error_code,"0000000",sizeof(a001->common_header.internal_error_code));
    a001->common_header.control_data_length = 0;
    s_Err = 0;
    COM_TGM(DEF_Component_Listener,thread,DEF_EV_async_req,&ul_tag);
    i_CC = WRITEREADX(lp_info[thread].listner_fd
                     ,(char *)a001
                     ,sizeof(a001_def)
                     ,MAX_TEXT_BUF_LEN
                     ,
                     ,(__int32_t)ul_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(lp_info[thread].listner_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","WRITEREADX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ipc_trs->func_flg = '1';
        memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
        memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
        memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
        memcpy(ipc_trs->trace_info.file_name,lp_info[thread].process_name,lp_info[thread].process_name_len);
        memcpy(ipc_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(a001_def));
        memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
        TRACEOUT((char *)ipc_trs);
    }

} /*end of CNSV_send_async*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lc_sock_err                               */
/*  CALLING SEQ.    : void CNSV_lc_sock_err ( Event_Node_def * )            */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ソケットI/Oエラー処理                                 */
/****************************************************************************/
// Event->thread 全リスナーブロードキャスト、scでlp数で設定
void CNSV_lc_sock_err(Event_Node_def *Event)
{
short s_Err;
buff_node_def *buff_node;
c105_def *c105;
short   s_Timeout_tag;
unsigned long ul_tag;
_cc_status i_CC;

    if (lp_info[Event->thread].listner_fd == DEF_FILE_CLOSED) return;    /*リスナークローズ中*/
    if (lp_info[Event->thread].buf_p != NULL) {
        CNSV_add_list(&myinfo.free_list,&lp_info[Event->thread].send_wait,DEF_Component_Listener,Event->thread,Event->event,Event->option1,Event->option2,0,"");
        return;
    }
    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    lp_info[Event->thread].buf_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    /*C105 コネクション切断完了通知*/
    c105 = (c105_def *)lp_info[Event->thread].buf_p;
    if (EXTRACEMODE) {
        ipc_trs = (lk_zac2001i_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ipc_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    memset((char *)c105,0x20,sizeof(c105_def));
    memcpy(c105->common_header.interface_code,DEF_IPC_IFCD_DISCON_NT_REQ,sizeof(DEF_IPC_IFCD_DISCON_NT_REQ)-1);
    c105->common_header.error_code = 0;
    memcpy(c105->common_header.internal_error_code,"0000000",sizeof(c105->common_header.internal_error_code));
    c105->common_header.control_data_length = 150;
    memcpy((char *)&c105->line_info,(char *)&cf[myinfo.cf_idx].sc_conf[Event->option1].site_name,sizeof(c105->line_info));
    memcpy((char *)&c105->status_info,(char *)&sc_info[Event->option1].status_info,sizeof(c105->status_info));
    memcpy((char *)&c105->process_info,(char *)&sc_info[Event->option1].process_info,sizeof(c105->process_info));
    memcpy((char *)&c105->connection_info,(char *)&sc_info[Event->option1].connection_info,sizeof(c105->connection_info));
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_io_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(myinfo.process_io_timer
                        ,DEF_EV_recv_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[Event->thread].timer_tag = s_Timeout_tag;
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_discinnect_complete_resp,&ul_tag);
    s_Err = 0;
    i_CC = WRITEREADX(lp_info[Event->thread].listner_fd
                     ,(char *)c105
                     ,sizeof(c105_def)
                     ,MAX_TEXT_BUF_LEN
                     ,
                     ,(__int32_t)ul_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(lp_info[Event->thread].listner_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","WRITEREADX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ipc_trs->func_flg = '1';
        memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
        memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
        memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
        memcpy(ipc_trs->trace_info.file_name,lp_info[Event->thread].process_name,lp_info[Event->thread].process_name_len);
        memcpy(ipc_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(c105_def));
        memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
        TRACEOUT((char *)ipc_trs);
    }

} /*end of CNSV_lc_sock_err*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_unknown_resp                          */
/*  CALLING SEQ.    : void CNSV_lsn_unknown_resp ( short )                  */
/*  ARGUMENT        : スレッド番号 (I/O完了のためlpのスレッド番号)          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 不明応答処理                                          */
/****************************************************************************/
void CNSV_lsn_unknown_resp(short thread)
{
Event_Node_def InEvent;
buff_node_def *buff_node;
a001_def *dummy_ipc;

    /*エラーメッセージ出力*/
    if (IOCMP.event == DEF_EV_async_req) {
        dummy_ipc = (a001_def *)lp_info[thread].async_p;
        lp_info[thread].async_p = 0;
    } else {
        dummy_ipc = (a001_def *)lp_info[thread].buf_p;
        lp_info[thread].buf_p = 0;
    }
    message_output(DEF_EVT_RSP_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_IPC_SEISA_ERR,"@X@X@I","","",dummy_ipc->common_header.interface_code,DEF_VAR_STOP);
    buff_node = (buff_node_def *)(IOCMP.addr - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST);   /*バッファ開放##*/
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
    if (lp_info[thread].send_wait.head != 0) {
        CNSV_remove_list(&lp_info[thread].send_wait,&myinfo.free_list,&InEvent);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,InEvent.compo,InEvent.thread,InEvent.event,InEvent.option1,InEvent.option2,InEvent.len,(char *)&InEvent.text);
    }

} /*end of CNSV_lsn_unknown_resp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_tell_chg_sock                         */
/*  CALLING SEQ.    : void CNSV_lsn_tell_chg_sock ( Event_Node_def * )      */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション入替・切断完了通知要求                    */
/****************************************************************************/
// Event->thread lpのスレッド番号
// Event->option1 scのスレッド番号または-1(クローズのみ)
void CNSV_lsn_tell_chg_sock(Event_Node_def *Event)
{
short s_Err;
_cc_status i_CC;
buff_node_def *buff_node;
c106_def *c106;
short   s_Timeout_tag;
unsigned long ul_tag;

    if (lp_info[Event->thread].listner_fd == DEF_FILE_CLOSED) return;    /*リスナークローズ中*/
    if (lp_info[Event->thread].buf_p != NULL) {
        CNSV_add_list(&myinfo.free_list,&lp_info[Event->thread].send_wait,DEF_Component_Listener,Event->thread,Event->event,Event->option1,Event->option2,0,"");
        return;
    }
    n102 = (n102_def *)Event->text;
    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    lp_info[Event->thread].buf_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    if (EXTRACEMODE) {
        ipc_trs = (lk_zac2001i_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ipc_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    c106 = (c106_def *)lp_info[Event->thread].buf_p;
    memset((char *)c106,0x20,sizeof(c106_def));
    memcpy(c106->common_header.interface_code,DEF_IPC_IFCD_CON_SW_NT_REQ,sizeof(DEF_IPC_IFCD_CON_SW_NT_REQ)-1);
    c106->common_header.error_code = 0;
    memcpy((char *)&c106->common_header.internal_error_code,"0000000",sizeof(c106->common_header.internal_error_code));
    memset((char *)&c106->common_header.filler_1,' ',sizeof(c106->common_header.filler_1));
    c106->common_header.control_data_length = 150;
    if (Event->option1 != -1) { /*スレッド番号有り*/
        memcpy((char *)&c106->line_info,(char *)&cf[myinfo.cf_idx].sc_conf[Event->option1].site_name,sizeof(c106->line_info));
        memcpy((char *)&c106->status_info,(char *)&sc_info[Event->option1].status_info,sizeof(c106->status_info));
        memcpy((char *)&c106->process_info,(char *)&sc_info[Event->option1].process_info,sizeof(c106->process_info));
    } else {                    /*スレッド番号なしの場合はリスンポート識別情報からコネクション識別情報を設定*/
        memcpy((char *)&c106->line_info,(char *)&n102->listen_info,DEF_tbl_chk_len_station);
    }

    /*■IPC内容に回線のステータスを追加*/
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_io_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(myinfo.process_io_timer
                        ,DEF_EV_io_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[Event->thread].timer_tag = s_Timeout_tag;
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_connect_start_resp,&ul_tag);
    s_Err = 0;
    i_CC = WRITEREADX(lp_info[Event->thread].listner_fd
                     ,(char *)c106
                     ,sizeof(c106_def)
                     ,MAX_TEXT_BUF_LEN
                     ,,(__int32_t)ul_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(lp_info[Event->thread].listner_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","WRITEREADX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ipc_trs->func_flg = '1';
        memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
        memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
        memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
        memcpy(ipc_trs->trace_info.file_name,lp_info[Event->thread].process_name,lp_info[Event->thread].process_name_len);
        memcpy(ipc_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(c106_def));
        memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
        TRACEOUT((char *)ipc_trs);
    }
} /*end of CNSV_lsn_tell_chg_sock*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_tell_port_status                      */
/*  CALLING SEQ.    : void CNSV_lsn_tell_port_status ( Event_Node_def * )   */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : Listenerポート状態送信                                */
/****************************************************************************/
// Event->thread lpのスレッド番号
// Event->option1 scのスレッド番号
void CNSV_lsn_tell_port_status(Event_Node_def *Event)
{
short s_Err;
_cc_status i_CC;
buff_node_def *buff_node;
c103_def *c103;
short   s_Timeout_tag;
unsigned long ul_tag;

    if (lp_info[Event->thread].listner_fd == DEF_FILE_CLOSED) return;    /*リスナークローズ中*/
    if (lp_info[Event->thread].buf_p != NULL) {
        CNSV_add_list(&myinfo.free_list,&lp_info[Event->thread].send_wait,DEF_Component_Listener,Event->thread,Event->event,Event->option1,Event->option2,0,"");
        return;
    }
    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    lp_info[Event->thread].buf_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    if (EXTRACEMODE) {
        ipc_trs = (lk_zac2001i_arg_1_def *)&buff_node->dt[1];
        memset((char *)&ipc_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    c103 = (c103_def *)lp_info[Event->thread].buf_p;
    memset((char *)c103,0x20,sizeof(c103_def));
    memcpy(c103->common_header.interface_code,DEF_IPC_IFCD_CON_START_REQ,sizeof(DEF_IPC_IFCD_CON_START_REQ)-1);
    c103->common_header.error_code = 0;
    memcpy((char *)&c103->common_header.internal_error_code,"0000000",sizeof(c103->common_header.internal_error_code));
    memset((char *)&c103->common_header.filler_1,' ',sizeof(c103->common_header.filler_1));
    c103->common_header.control_data_length = 150;
    memcpy((char *)&c103->line_info,(char *)&cf[myinfo.cf_idx].sc_conf[Event->option1].site_name,sizeof(c103->line_info));
    memcpy((char *)&c103->status_info,(char *)&sc_info[Event->option1].status_info,sizeof(c103->status_info));
    memcpy((char *)&c103->process_info,(char *)&sc_info[Event->option1].process_info,sizeof(c103->process_info));

    /*■IPC内容に回線のステータスを追加*/
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_io_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(myinfo.process_io_timer
                        ,DEF_EV_io_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    lp_info[Event->thread].timer_tag = s_Timeout_tag;
    COM_TGM(DEF_Component_Listener,Event->thread,DEF_EV_connect_start_resp,&ul_tag);
    s_Err = 0;
    i_CC = WRITEREADX(lp_info[Event->thread].listner_fd
                     ,(char *)c103
                     ,sizeof(c103_def)
                     ,MAX_TEXT_BUF_LEN
                     ,,(__int32_t)ul_tag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(lp_info[Event->thread].listner_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","WRITEREADX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (EXTRACEMODE) {
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        ipc_trs->func_flg = '1';
        memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
        memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
        memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
        memcpy(ipc_trs->trace_info.file_name,lp_info[Event->thread].process_name,lp_info[Event->thread].process_name_len);
        memcpy(ipc_trs->trace_info.file_io_type,"WRITE   ",8);
        sprintf(trace_work,"%04d",s_Err);
        memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
        sprintf(trace_work,"%05d",sizeof(c103_def));
        memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
        TRACEOUT((char *)ipc_trs);
    }
} /*end of CNSV_lsn_tell_port_status*/

