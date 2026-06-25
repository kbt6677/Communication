/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
/*                               コネクション管理コンポーネント(so)          */
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
/*  FUNCTION        : 1.1.0  CNSV_sock_manage                               */
/*  CALLING SEQ.    : void CNSV_sock_manage ( Event_Node_def * )            */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション管理処理                                  */
/****************************************************************************/
void CNSV_sock_manage(Event_Node_def *Event)
{
    switch (Event->event)
    {
        case DEF_EV_port_open:
            if (sc_info[Event->thread].sock_fd == DEF_FILE_CLOSED) {    /*クローズ中の場合、コネクション状態更新*/
                memcpy(sc_info[Event->thread].connection_info.error_code,"0000",4);
                memcpy(sc_info[Event->thread].connection_info.disconnect_reason,"00",2);
                CNSV_GCLST_update(Event->thread,DEF_GCLST_con_state_listen,DEF_GCLST_proc_state_open);
            }
            break;
        case DEF_EV_connect_notice:
            CNSV_port_open(Event->thread,false);
            break;
//      case DEF_EV_port_close:
//          CNSV_port_close(Event->thread,DEF_GCLST_proc_state_open,DEF_EV_sock_recv_err);
//          break;
        case DEF_EV_text_recv_rsp:
        case DEF_EV_unknown_resp:
        case DEF_EV_PS_err:
            CNSV_recv_complete_reply(Event->thread);
            break;
        case DEF_EV_text_send_req:
            CNSV_send_request(Event->thread);
            break;
        case DEF_EV_sock_recv_comp:
            CNSV_recv_complete(Event->thread);
            break;
        case DEF_EV_idle_timeout:
        case DEF_EV_recv_timeout:
        case DEF_EV_sock_recv_err:
            CNSV_recv_error(Event);
            break;
        case DEF_EV_sock_send_comp:
            CNSV_send_complete(Event->thread);
            break;
        case DEF_EV_send_timeout:
        case DEF_EV_sock_send_err:
            CNSV_send_error(Event);
            break;
        case DEF_EV_chg_sock_close:
            CNSV_port_close(Event->thread,DEF_GCLST_proc_state_nocgange,DEF_EV_chg_sock_close);
            break;
        case DEF_EV_chg_sock_open:
            CNSV_port_open(Event->thread,true);
            break;
        case DEF_EV_disconnect_req:
            CNSV_port_close(Event->thread,DEF_GCLST_proc_state_close,DEF_EV_sock_recv_err);
            break;
        default:
            AbNormal_End();
            break;
    }
} /*end of CNSV_sock_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recv                                      */
/*  CALLING SEQ.    : void CNSV_recv ( short )                              */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : RECV処理                                              */
/****************************************************************************/
void CNSV_recv(short thread)
{
buff_node_def *buff_node;
short s_Err;
unsigned long ul_tag;
    if (sc_info[thread].sock_fd == DEF_FILE_CLOSED) {
        return;
    }
    buff_node = myinfo.buffs.head;
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    sc_info[thread].recv_p = (char *)buff_node->dt + (DEF_BUF_ADJUST) + DEF_C201_hd_len + DEF_C201_msg_info_len + DEF_C201_txt_len;

    if (cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_id == DEF_datalen_inside) {
        sc_info[thread].hdlen = (cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_pos - 1) + cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_size;
    } else {
        sc_info[thread].hdlen = (cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].text_pos - 1);
    }
    sc_info[thread].textlen_pos = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_pos - 1;
    sc_info[thread].textlen_size = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_size;
    sc_info[thread].textlen_type = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_attr;
    sc_info[thread].textlen_include = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].length_id;
    sc_info[thread].recv_len = sc_info[thread].hdlen;
    sc_info[thread].recv_comp_len = 0;
    COM_TGM(DEF_Component_socket,thread,DEF_EV_sock_recv_comp,&ul_tag);
    s_Err = (short)recv_nw(sc_info[thread].sock_fd,sc_info[thread].recv_p,sc_info[thread].recv_len,0,(__int32_t)ul_tag);
    if (s_Err == -1) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_RCV_ERR,"@X@5","recv_nw",errno,DEF_VAR_STOP);
        AbNormal_End();
    }

} /*end of CNSV_recv*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recv_complete                             */
/*  CALLING SEQ.    : void CNSV_recv_complete ( short )                     */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : RECV完了処理                                          */
/****************************************************************************/
void CNSV_recv_complete(short thread)
{
short           s_idx;
short           s_Len;
short           s_chk_len;
short           s_val;
char            ach_Buf[DEF_MAX_textlen_size+1];
char            ach_text[DEF_MAX_textlen_size+1];
char            ach_var[6];
char            ach_gflin_pkey[DEF_GFLIN_PKEY_LEN+1];
unsigned long   ul_tag;
short           s_Timeout_tag;
_cc_status      i_CC;
    memset(ach_var,'\0',sizeof(ach_var));
    if (sc_info[thread].recv_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].recv_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].recv_timer_tag = DEF_TAG_NULL;
    }
    if (sc_info[thread].idle_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].idle_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].idle_timer_tag = DEF_TAG_NULL;
    }
    sc_info[thread].recv_comp_len += IOCMP.len;
    if (sc_info[thread].recv_comp_len == sc_info[thread].hdlen) {    /*ヘッダー読込完了チェック*/
        memset(ach_gflin_pkey,'\0',sizeof(ach_gflin_pkey));
        memcpy(ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].site_name,DEF_GFLIN_PKEY_LEN);
        memset(ach_Buf,'\0',sizeof(ach_Buf));
        memcpy(ach_Buf,(char *)&sc_info[thread].recv_p[sc_info[thread].textlen_pos],sc_info[thread].textlen_size);
        memset(ach_text,'\0',sizeof(ach_text));
        switch (sc_info[thread].textlen_type) {
            case DEF_textlen_type_BIN:
                memcpy((char *)&s_val,ach_Buf,2);
                sprintf(ach_text,"%04d",s_val);
                s_chk_len = 4;
                break;
            case DEF_textlen_type_BCD:
                BCD2CHAR(ach_Buf,ach_text,sc_info[thread].textlen_size);
                s_chk_len = sc_info[thread].textlen_size * 2;   /*BCD(Half-Byte)をASCII(Byte)に変換しているためチェック長を2倍にする*/
                break;
            case DEF_textlen_type_ASCII:
                memcpy(ach_text,ach_Buf,sc_info[thread].textlen_size);
                s_chk_len = sc_info[thread].textlen_size;
                break;
            case DEF_textlen_type_EBCDIC:
                EBCNUM2CHAR(ach_Buf,ach_text,sc_info[thread].textlen_size);
                s_chk_len = sc_info[thread].textlen_size;
                break;
            default:
                /*データレングス不正*/
                memcpy(ach_var,cf[myinfo.cf_idx].sc_conf->local_port_no,5);
                message_output(DEF_EVT_MSG_LEN_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_RCV_ERR,"@X@X@A@P@X@X@X",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].tcpip_name,sc_info[thread].sock.sin_addr.s_addr,(unsigned short)sc_info[thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[thread].local_port_no,ach_var,DEF_VAR_STOP);
                /*コネクション切断*/
                CNSV_port_close(thread,DEF_GCLST_proc_state_nocgange,DEF_EV_sock_recv_err);
                return;
        }
        for (s_idx = 0; s_idx < s_chk_len; s_idx++ ) {
            if (!isdigit(ach_text[s_idx])) {
                /*データレングス不正*/
                memcpy(ach_var,cf[myinfo.cf_idx].sc_conf->local_port_no,5);
                message_output(DEF_EVT_MSG_LEN_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_RCV_ERR,"@X@X@A@P@X@X@X",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].tcpip_name,sc_info[thread].sock.sin_addr.s_addr,(unsigned short)sc_info[thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[thread].local_port_no,ach_var,DEF_VAR_STOP);
                /*コネクション切断*/
                CNSV_port_close(thread,DEF_GCLST_proc_state_nocgange,DEF_EV_sock_recv_err);
                return;
            }
        }
        s_Len = (short)atoi(ach_text);
        if (s_Len > MAX_TEXT_BUF_LEN || s_Len < sc_info[thread].hdlen) {    /*length err*/
                /*データレングス不正*/
                memcpy(ach_var,cf[myinfo.cf_idx].sc_conf->local_port_no,5);
                message_output(DEF_EVT_MSG_LEN_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_RCV_ERR,"@X@X@A@P@X@X@X",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].tcpip_name,sc_info[thread].sock.sin_addr.s_addr,(unsigned short)sc_info[thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[thread].local_port_no,ach_var,DEF_VAR_STOP);
                /*コネクション切断*/
                CNSV_port_close(thread,DEF_GCLST_proc_state_nocgange,DEF_EV_sock_recv_err);
                return;
        }
        if (sc_info[thread].textlen_include == DEF_datalen_inside) {
            sc_info[thread].recv_len = s_Len;   /*■ヘッダー長を含む場合、CARDNETは含む*/
        } else {
            sc_info[thread].recv_len += s_Len;  /*■ヘッダー長を含まない場合*/
        }
        CNSV_remain_recv(thread);
        return;
    } else if (sc_info[thread].recv_len != sc_info[thread].recv_comp_len) {
        CNSV_remain_recv(thread);   /*受信残あり*/
        return;
    } else {                        /*1電文受信完了*/
        COM_UNQ(&sc_info[thread].ach_timestamp,(char *)&sc_info[thread].ach_uniq_ts);
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Inbound,thread,DEF_EV_sock_recv_comp,thread,0,sc_info[thread].recv_comp_len,(char *)sc_info[thread].recv_p);
        sc_info[thread].recv_p = NULL;
    }
    if ((sc_info[thread].idle_timer_value != 0) && (sc_info[thread].idle_timer_tag == DEF_TAG_NULL)) {
        COM_TGM(DEF_Component_socket,thread,DEF_EV_idle_timeout,&ul_tag);
        i_CC = SIGNALTIMEOUT(sc_info[thread].idle_timer_value
                            ,DEF_EV_recv_timeout
                            ,(__int32_t)ul_tag
                            ,&s_Timeout_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].idle_timer_tag = s_Timeout_tag;
    }
} /*end of CNSV_recv_complete*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_remain_recv                               */
/*  CALLING SEQ.    : void CNSV_remain_recv ( short )                       */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 残データrecv発行処理                                  */
/****************************************************************************/
void CNSV_remain_recv(short thread)
{
short   s_remain;
short   s_Err;
short   s_Timeout_tag;
unsigned long    ul_tag;
char    *ptr;
_cc_status i_CC;

    COM_TGM(DEF_Component_socket,thread,DEF_EV_recv_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(sc_info[thread].recv_timer_value
                        ,DEF_EV_recv_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    sc_info[thread].recv_timer_tag = s_Timeout_tag;
    ptr = sc_info[thread].recv_p + sc_info[thread].recv_comp_len;
    s_remain = sc_info[thread].recv_len - sc_info[thread].recv_comp_len;
    COM_TGM(DEF_Component_socket,thread,DEF_EV_sock_recv_comp,&ul_tag);
    s_Err = (short)recv_nw(sc_info[thread].sock_fd,ptr,s_remain,0,(__int32_t)ul_tag);
    if (s_Err == -1) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","recv_nw",errno,DEF_VAR_STOP);
        AbNormal_End();
    }

} /*end of CNSV_remain_recv*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_port_close                                */
/*  CALLING SEQ.    : void CNSV_port_close ( short , short , short )        */
/*  ARGUMENT        : スレッド番号                                          */
/*                  : プロセスステート                                      */
/*                  : イベント                                              */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : データポート解放要求処理                              */
/****************************************************************************/
void CNSV_port_close(short thread,short porcstate,short event)
{
short   s_idx;
short   s_state;
char    ach_text[6];
buff_node_def *buff_node;
_cc_status i_CC;

    if (sc_info[thread].recv_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].recv_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].recv_timer_tag = DEF_TAG_NULL;
    }
    if (sc_info[thread].idle_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].idle_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].idle_timer_tag = DEF_TAG_NULL;
    }
    if (sc_info[thread].send_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].send_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].send_timer_tag = DEF_TAG_NULL;
    }
    if (sc_info[thread].sock_fd != DEF_FILE_CLOSED) {
        FILE_CLOSE_(sc_info[thread].sock_fd);
        sc_info[thread].sock_fd = DEF_FILE_CLOSED;
    }
    if (sc_info[thread].recv_p) {
        buff_node = (buff_node_def *)(sc_info[thread].recv_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST - DEF_C201_hd_len - DEF_C201_msg_info_len - DEF_C201_txt_len);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
        sc_info[thread].recv_p = NULL;
    }
    if (sc_info[thread].send_p) {
        buff_node = (buff_node_def *)(sc_info[thread].send_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST - DEF_c202_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
        sc_info[thread].send_p = NULL;
    }

    sprintf(ach_text,"%04d",IOCMP.fs_err);
    memcpy(sc_info[thread].connection_info.error_code,ach_text,sizeof(sc_info[thread].connection_info.error_code));
    s_state = DEF_GCLST_con_state_listen;       /*回線状態デフォルト値、他の値にする場合次の処理で更新*/
    switch (porcstate)
    {
    case DEF_GCLST_proc_state_open:
        s_state = DEF_GCLST_con_state_open;
        memcpy(sc_info[thread].connection_info.disconnect_reason,"00",2);
        break;
    case DEF_GCLST_proc_state_close:
        s_state = DEF_GCLST_con_state_close;
        memcpy(sc_info[thread].connection_info.disconnect_reason,"00",2);
        break;
    case DEF_GCLST_proc_state_nocgange:
        if ((IOCMP.fs_err != 0) || (IOCMP.fs_err == 0 && IOCMP.len == 0)) { /*エラーまたはrecvの0バイト完了*/
            memcpy(sc_info[thread].connection_info.disconnect_reason,"91",2);
        }
        break;
    default:
        break;
    }
    /*コネクション状態更新*/
    CNSV_GCLST_update(thread,s_state,porcstate);
    /*Outbound電文振分に通知*/
    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,sc_info[thread].outbound_index,DEF_EV_sock_recv_err,thread,0,IOCMP.len,(char *)IOCMP.addr);
    switch (event) {
        case 0:                     /*リスナーにC105コネクション切断完了通知 (全リスナー)*/
            for ( s_idx = 0;s_idx < myinfo.lp_info_use_cnt; s_idx++ ) {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_sock_recv_err,thread,0,IOCMP.len,(char *)IOCMP.addr);
            }
            break;
        case DEF_EV_chg_sock_close: /*リスナー管理でコントロールしているため無処理*/
            break;
        default:                    /*その他、指定されたイベントを通知(全リスナー)*/
            for ( s_idx = 0;s_idx < myinfo.lp_info_use_cnt; s_idx++ ) {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,event,thread,0,IOCMP.len,(char *)IOCMP.addr);
            }
            break;
    }
} /*end of CNSV_port_close*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_port_open                                 */
/*  CALLING SEQ.    : void CNSV_port_open ( short, short )                  */
/*  ARGUMENT        : スレッド番号                                          */
/*                  : 切り替え有無                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : データポートオープン処理                              */
/****************************************************************************/
void CNSV_port_open(short thread,short change)
{
short   s_Err,s_idx;
int     i_Value;
short   s_Timeout_tag;
unsigned long    ul_tag;
char    *pch_edit_ptr;
struct  sockaddr_in sock;
char    ach_num_edit[6];
char    ach_gflin_pkey[DEF_GFLIN_PKEY_LEN+1];
_cc_status i_CC;

    if (sc_info[thread].sock_fd != DEF_FILE_CLOSED) {
        CNSV_port_close(thread,DEF_GCLST_proc_state_nocgange,DEF_EV_sock_recv_err);
    }
    sc_info[thread].recv_timer_value = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].recv_wait_timer;
    sc_info[thread].send_timer_value = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].send_wait_timer;
    sc_info[thread].idle_timer_value = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].idle_timer;
    memset(cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,'\0',sizeof(cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr));
    memcpy(cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,(char *)&n101->connection_info.src_ip_address,sizeof(n101->connection_info.src_ip_address));
    CNSV_set_null(cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,sizeof(cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr));
    memset(cf[myinfo.cf_idx].sc_conf[thread].local_port_no,'\0',sizeof(cf[myinfo.cf_idx].sc_conf[thread].local_port_no));
    memcpy(cf[myinfo.cf_idx].sc_conf[thread].local_port_no,(char *)&n101->connection_info.src_port,sizeof(n101->connection_info.src_port));
    CNSV_set_null(cf[myinfo.cf_idx].sc_conf[thread].local_port_no,sizeof(cf[myinfo.cf_idx].sc_conf[thread].local_port_no));
    memcpy(&sc_info[thread].sock,(char *)&n101->socket_info,sizeof(sc_info[thread].sock));
    pch_edit_ptr = inet_ntoa(sc_info[thread].sock.sin_addr);
    memset(sc_info[thread].remote_ipaddr,' ',sizeof(sc_info[thread].remote_ipaddr));
    memcpy(sc_info[thread].remote_ipaddr,pch_edit_ptr,strlen(pch_edit_ptr));
    CNSV_set_null(sc_info[thread].remote_ipaddr,sizeof(sc_info[thread].remote_ipaddr));
    sprintf(ach_num_edit,"%05u",sc_info[thread].sock.sin_port);
    memcpy(sc_info[thread].remote_port_no,ach_num_edit,sizeof(sc_info[thread].remote_port_no));
    CNSV_set_null(sc_info[thread].remote_port_no,sizeof(sc_info[thread].remote_port_no));
    socket_set_inet_name(cf[myinfo.cf_idx].sc_conf[thread].tcpip_name);
    sc_info[thread].sock_fd = (short)socket_nw(AF_INET,SOCK_STREAM,0,2,0);
    if (sc_info[thread].sock_fd == DEF_FILE_CLOSED) {
        FILE_GETINFO_(sc_info[thread].sock_fd,&s_Err);
        /*エラーメッセージ*/
        switch (s_Err) {
            case 60:
            case 66:
                break;
            default:
                message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","socket_nw",s_Err,DEF_VAR_STOP);
                return;
        }
    }
    if (sc_info[thread].sock_fd == DEF_FILE_CLOSED) {
        sc_info[thread].sock_fd = (short)socket_nw(AF_INET,SOCK_STREAM,0,2,0);
        if (sc_info[thread].sock_fd == -1) {
            FILE_GETINFO_(sc_info[thread].sock_fd,&s_Err);
            /*エラーメッセージ*/
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","socket_nw",s_Err,DEF_VAR_STOP);
            return;
        }
    }
    memcpy(&sock,&sc_info[thread].sock,sizeof(sock));
    s_Err = (short)accept_nw2(sc_info[thread].sock_fd
                     ,(struct sockaddr *)&sc_info[thread].sock,DEF_TAG_SOCKET);
    if (s_Err == -1) {
        /**/
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","accept_nw2",errno,DEF_VAR_STOP);
        AbNormal_End();
    }
    AWAITIOX(&sc_info[thread].sock_fd,,,,myinfo.socket_io_timer);
    FILE_GETINFO_(sc_info[thread].sock_fd,&s_Err);
    if (s_Err) {
        FILE_CLOSE_(sc_info[thread].sock_fd);
        sc_info[thread].sock_fd = DEF_FILE_CLOSED;
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","accept_nw2",s_Err,DEF_VAR_STOP);
        return;
    }

    i_Value = 1;
    s_Err = (short)setsockopt_nw(sc_info[thread].sock_fd,SOL_SOCKET,SO_KEEPALIVE
                        ,(char *)&i_Value,sizeof(i_Value),DEF_TAG_SOCKET);
    if (s_Err == -1) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","setsockopt_nw",errno,DEF_VAR_STOP);
        AbNormal_End();
    }
    AWAITIOX(&sc_info[thread].sock_fd,,,,myinfo.socket_io_timer);
    FILE_GETINFO_(sc_info[thread].sock_fd,&s_Err);
    if (s_Err) {
        FILE_CLOSE_(sc_info[thread].sock_fd);
        sc_info[thread].sock_fd = DEF_FILE_CLOSED;
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","setsockopt_nw",s_Err,DEF_VAR_STOP);
        return;
    }
    /*■接続メッセージ*/
    /*■回線ステータスファイル更新*/
    memcpy(cf[myinfo.cf_idx].sc_conf[thread].remote_ipaddr,sc_info[thread].remote_ipaddr,sizeof(cf[myinfo.cf_idx].sc_conf[thread].remote_ipaddr));
    memcpy(cf[myinfo.cf_idx].sc_conf[thread].remote_port_no,sc_info[thread].remote_port_no,sizeof(cf[myinfo.cf_idx].sc_conf[thread].remote_port_no));
    if (change == true) {
        memcpy(sc_info[thread].connection_info.disconnect_reason,"10",2);
    }
    CNSV_GCLST_update(thread,DEF_GCLST_con_state_open,DEF_GCLST_proc_state_open);
    /*コネクション確率、メッセージ出力*/
    memset(ach_gflin_pkey,'\0',sizeof(ach_gflin_pkey));
    memcpy(ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].site_name,DEF_GFLIN_PKEY_LEN);
    message_output(DEF_EVT_CONN_CONNECTED,DEF_MSGTTKB_NORMAL,DEF_NERR_NOMAL,"@X@X@A@P@X@X",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[thread].tcpip_name,sc_info[thread].sock.sin_addr.s_addr,(unsigned short)sc_info[thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[thread].local_port_no,DEF_VAR_STOP);
    /*■recv発行*/
    CNSV_recv(thread);
    /*コネクション後処理*/
    /*■無通信監視指定有りの場合、無通信状態監視タイマーを発行する*/
    if (sc_info[thread].idle_timer_value != 0) {
        COM_TGM(DEF_Component_socket,thread,DEF_EV_idle_timeout,&ul_tag);
        i_CC = SIGNALTIMEOUT(sc_info[thread].idle_timer_value
                            ,DEF_EV_recv_timeout
                            ,(__int32_t)ul_tag
                            ,&s_Timeout_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].idle_timer_tag = s_Timeout_tag;
    }
    if        (memcmp(cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].connect_after,"SO",2) == 0) {
        /*■自動サインオン有無チェック、有りの場合イベントを生成*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,thread,DEF_EV_signon_req,0,0,IOCMP.len,(char *)IOCMP.addr);
    } else if (memcmp(cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].connect_after,"DT",2) == 0) {
        /*■特定データ有無チェック、有りの場合送信電文を編集しsend*/
        CNSV_connect_data_send(thread);
    };
    /* C107 コネクション状態通知要求 */
    /*■スレッド番号は0ではなく、該当するOutbound電文振分のスレッド番号を設定する*/
    /*Outbound電文振分*/
    for (s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_tell_port_status,thread,DEF_EV_tell_port_status,0,"");
    }
} /*end of CNSV_port_open*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recv_error                                */
/*  CALLING SEQ.    : void CNSV_recv_error ( Event_Node_def * )             */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : RECVエラー処理                                        */
/****************************************************************************/
void CNSV_recv_error(Event_Node_def *Event)
{
char    ach_gflin_pkey[DEF_GFLIN_PKEY_LEN+1];
    if (Event->event == DEF_EV_recv_timeout) {
        sc_info[Event->thread].recv_timer_tag = DEF_TAG_NULL;
    }
    if (Event->event == DEF_EV_idle_timeout) {
        sc_info[Event->thread].idle_timer_tag = DEF_TAG_NULL;
    }
    /*エラーメッセージ出力*/
    memset(ach_gflin_pkey,'\0',sizeof(ach_gflin_pkey));
    memcpy(ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[Event->thread].site_name,DEF_GFLIN_PKEY_LEN);
    message_output(DEF_EVT_CONN_SYOGAI,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_RCV_ERR,"@X@X@A@P@X@X@5",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[Event->thread].tcpip_name,sc_info[Event->thread].sock.sin_addr.s_addr,(unsigned short)sc_info[Event->thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[Event->thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[Event->thread].local_port_no,IOCMP.fs_err,DEF_VAR_STOP);
    CNSV_port_close(Event->thread,DEF_GCLST_proc_state_nocgange,DEF_EV_sock_recv_err);
} /*end of CNSV_recv_error*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_send_complete                             */
/*  CALLING SEQ.    : void CNSV_send_complete ( short )                     */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : SEND完了処理                                          */
/****************************************************************************/
void CNSV_send_complete(short thread)
{
short           s_len;
buff_node_def  *buff_node;
Event_Node_def  CurrentEvent;
unsigned long   ul_tag;
short           s_Timeout_tag;
_cc_status      i_CC;
    /*タイマー発行時、キャンセル処理*/
    if (sc_info[thread].send_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].send_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].send_timer_tag = DEF_TAG_NULL;
    }
    /*完了長取得*/
    s_len = (short)socket_get_len(sc_info[thread].sock_fd);
    /*送信完了判定*/
    sc_info[thread].send_comp_len += s_len;
    if (sc_info[thread].send_total_len == sc_info[thread].send_comp_len) {
        /*バッファー開放*/
        buff_node = (buff_node_def *)(sc_info[thread].send_p - DEF_BUF_CTL_HD_SIZE - DEF_BUF_ADJUST - DEF_c202_ADJUST);
        myinfo.buffs.tail->next = buff_node;
        myinfo.buffs.tail = buff_node;
        myinfo.buffs.list_count++;
        buff_node->next = 0L;
        if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
        sc_info[thread].send_p = NULL;
        if (sc_info[thread].send_wait.head) {
            CNSV_remove_list(&sc_info[thread].send_wait,&myinfo.free_list,&CurrentEvent);
            c202 = (c202_def *)CurrentEvent.text;   /*アドレス設定*/
            CNSV_send_request(thread);              /*次データ送信*/
        } else {
            if ((sc_info[thread].idle_timer_value != 0) && (sc_info[thread].idle_timer_tag == DEF_TAG_NULL)) {
                COM_TGM(DEF_Component_socket,thread,DEF_EV_idle_timeout,&ul_tag);
                i_CC = SIGNALTIMEOUT(sc_info[thread].idle_timer_value
                                    ,DEF_EV_recv_timeout
                                    ,(__int32_t)ul_tag
                                    ,&s_Timeout_tag);
                if (_status_ne(i_CC)) {
                    message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
                    AbNormal_End();
                }
                sc_info[thread].idle_timer_tag = s_Timeout_tag;
            }
        }
    } else {
        CNSV_remain_send(thread);                   /*残データ送信*/
    }

} /*end of CNSV_send_complete*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_remain_send                               */
/*  CALLING SEQ.    : void CNSV_remain_send ( short )                       */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 残データsend発行処理                                  */
/****************************************************************************/
void CNSV_remain_send(short thread)
{
short   s_Err;
short   s_Remain;
char    *pch_ptr;
short   s_Timeout_tag;
unsigned long ul_tag;
_cc_status i_CC;

    COM_TGM(DEF_Component_socket,thread,DEF_EV_send_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(sc_info[thread].send_timer_value
                        ,DEF_EV_send_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    sc_info[thread].send_timer_tag = s_Timeout_tag;

    s_Remain = sc_info[thread].send_total_len - sc_info[thread].send_comp_len;
    pch_ptr = sc_info[thread].send_p + sc_info[thread].send_comp_len;
    COM_TGM(DEF_Component_socket,thread,DEF_EV_sock_send_comp,&ul_tag);
    s_Err = (short)send_nw2(sc_info[thread].sock_fd,pch_ptr,s_Remain,0,(__int32_t)ul_tag);
    if (s_Err == -1) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","send_nw2",errno,DEF_VAR_STOP);
        AbNormal_End();
    }

} /*end of CNSV_remain_send*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_send_error                                */
/*  CALLING SEQ.    : void CNSV_send_error ( Event_Node_def * )             */
/*  ARGUMENT        : イベント情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : SENDエラー処理                                        */
/****************************************************************************/
void CNSV_send_error(Event_Node_def *Event)
{
char ach_var[6];
char ach_gflin_pkey[DEF_GFLIN_PKEY_LEN+1];
    if (Event->event == DEF_EV_send_timeout) {
        sc_info[Event->thread].send_timer_tag = DEF_TAG_NULL;
    }
    memset(ach_var,'\0',sizeof(ach_var));
    memcpy(ach_var,cf[myinfo.cf_idx].sc_conf->local_port_no,5);
    /*エラーメッセージ出力*/
    memset(ach_gflin_pkey,'\0',sizeof(ach_gflin_pkey));
    memcpy(ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[Event->thread].site_name,DEF_GFLIN_PKEY_LEN);
    message_output(DEF_EVT_CONN_SYOGAI,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SEND_ERR,"@X@X@A@P@X@X@5",ach_gflin_pkey,(char *)&cf[myinfo.cf_idx].sc_conf[Event->thread].tcpip_name,sc_info[Event->thread].sock.sin_addr.s_addr,(unsigned short)sc_info[Event->thread].sock.sin_port,cf[myinfo.cf_idx].sc_conf[Event->thread].local_ipaddr,cf[myinfo.cf_idx].sc_conf[Event->thread].local_port_no,IOCMP.fs_err,DEF_VAR_STOP);
    CNSV_port_close(Event->thread,DEF_GCLST_proc_state_nocgange,DEF_EV_sock_recv_err);
} /*end of CNSV_send_error*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recv_complete_reply                       */
/*  CALLING SEQ.    : void CNSV_recv_complete_reply ( short )               */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 電文受信応答処理                                      */
/****************************************************************************/
void CNSV_recv_complete_reply(short thread)
{
    /*コネクションがクローズ済の場合return*/
    if (sc_info[thread].sock_fd == DEF_FILE_CLOSED) return;
    /*recv未発行の場合、recvを発行する*/
    if (!sc_info[thread].recv_p) {
        CNSV_recv(thread);
    }
} /*end of CNSV_recv_complete_reply*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_send_request                              */
/*  CALLING SEQ.    : void CNSV_send_request ( short )                      */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 電文送信要求処理                                      */
/****************************************************************************/
void CNSV_send_request(short thread)
{
short   s_Err;
short   s_Timeout_tag;
unsigned long    ul_tag;
_cc_status i_CC;
    /*コネクションがクローズ済の場合*/
    if (sc_info[thread].sock_fd == DEF_FILE_CLOSED) {
        /*Outbound電文振分制御でコネクション状態をチェック済でクローズ中は有りえず、論理矛盾で異常終了*/
        AbNormal_End();
    }
    /*送信中の場合、待ちリストに登録してリターン*/
    if (sc_info[thread].send_timer_tag != DEF_TAG_NULL) {
        CNSV_add_list(&myinfo.free_list,&sc_info[thread].send_wait,DEF_Component_socket,thread,DEF_EV_text_send_req,0,0,IOCMP.len,(char *)IOCMP.addr);
        return;
    }
    if (sc_info[thread].idle_timer_tag != DEF_TAG_NULL) {       /*無通信状態監視タイマー*/
        i_CC = CANCELTIMEOUT(sc_info[thread].idle_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].idle_timer_tag = DEF_TAG_NULL;
    }
    /*■c202はI/O完了時と送信待ちリストから取り出し時にアドレス設定済、送信データはmsg_data部、send_pに設定する*/
    /*タイマー発行*/
    COM_TGM(DEF_Component_socket,thread,DEF_EV_send_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(sc_info[thread].send_timer_value
                        ,DEF_EV_send_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    sc_info[thread].send_timer_tag = s_Timeout_tag;
    /*send_nw発行*/
    sc_info[thread].send_comp_len = 0;
    sc_info[thread].send_total_len = c202->msg_info.msg_len;
    sc_info[thread].send_p = (char *)&c202->msg_info.msg_data[0];
    COM_TGM(DEF_Component_socket,thread,DEF_EV_sock_send_comp,&ul_tag);
    s_Err = (short)send_nw2(sc_info[thread].sock_fd,sc_info[thread].send_p,sc_info[thread].send_total_len,0,(__int32_t)ul_tag);
    if (s_Err == -1) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","send_nw2",errno,DEF_VAR_STOP);
        AbNormal_End();
    }

} /*end of CNSV_send_request*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_disconnect_reply                          */
/*  CALLING SEQ.    : void CNSV_disconnect_reply ( void )                   */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : データポート切断応答処理                              */
/****************************************************************************/
//void CNSV_disconnect_reply(void)
//{
//buff_node_def *buff_node;
//    /*バッファー開放*/
//    buff_node = (buff_node_def *)(sc_info[IOCMP.thread].send_p - DEF_BUF_ADJUST);
//    myinfo.buffs.tail->next = buff_node;
//    myinfo.buffs.tail = buff_node;
//    myinfo.buffs.list_count++;
//    buff_node->next = 0L;
//    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
//
//} /*end of CNSV_disconnect_reply*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_connect_data_send                         */
/*  CALLING SEQ.    : void CNSV_connect_data_send ( short )                 */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 特定データ送信処理                                    */
/****************************************************************************/
void CNSV_connect_data_send(short thread)
{
short   s_Err;
short   s_Timeout_tag;
unsigned long    ul_tag;
buff_node_def *buff_node;
_cc_status i_CC;

    if (sc_info[thread].idle_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[thread].idle_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[thread].idle_timer_tag = DEF_TAG_NULL;
    }

    buff_node = myinfo.buffs.head;
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;

    sc_info[thread].send_comp_len = 0;
    sc_info[thread].send_total_len = cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].connect_data_len;
    /*送信データ格納アドレスはc202と同じにする*/
    sc_info[thread].send_p = (char *)buff_node->dt + (DEF_BUF_ADJUST) + DEF_c202_ADJUST;
    memcpy(sc_info[thread].send_p
          ,cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].connect_data
          ,cf[myinfo.cf_idx].st_conf[sc_info[thread].station_index].connect_data_len);

    COM_TGM(DEF_Component_socket,thread,DEF_EV_send_timeout,&ul_tag);
    i_CC = SIGNALTIMEOUT(sc_info[thread].send_timer_value
                        ,DEF_EV_send_timeout
                        ,(__int32_t)ul_tag
                        ,&s_Timeout_tag);
    if (_status_ne(i_CC)) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
        AbNormal_End();
    }
    sc_info[thread].send_timer_tag = s_Timeout_tag;

    /*send_nw発行*/
    COM_TGM(DEF_Component_socket,thread,DEF_EV_sock_send_comp,&ul_tag);
    s_Err = (short)send_nw2(sc_info[thread].sock_fd,sc_info[thread].send_p,sc_info[thread].send_total_len,0,(__int32_t)ul_tag);
    if (s_Err == -1) {
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","send_nw2",errno,DEF_VAR_STOP);
        AbNormal_End();
    }

} /*end of CNSV_connect_data_send*/

