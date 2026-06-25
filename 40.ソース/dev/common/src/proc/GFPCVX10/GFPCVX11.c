/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCVX10                                    */
/*        FUNCTION          ････ コネクション制御(サーバ)                    */
/*                               プロセス管理コンポーネント(sys)             */
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
#include "GFPCVX1D.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/* 関数のﾌﾟﾛﾄﾀｲﾌﾟ宣言 */
#include "GFPCVX1P.h"

/* vproc関数の宣言 */
#include "vproc.h"

/****************************************************************************/
/*  FUNCTION        : 1.0.0  main                                           */
/*  CALLING SEQ.    : int main (int, char[] *)                              */
/*  ARGUMENT        : int argc, char *argv[]                                */
/*  RETURN CODE     : int 0                                                 */
/*  DESCRIPTION     : コネクション制御(サーバー)の処理全体の制御を行う      */
/****************************************************************************/
int main( int argc, char *argv[] )
{
    /*------------------------------------------------*/
    /*    初期処                                      */
    /*------------------------------------------------*/
    CNSV_initialize();

    /*------------------------------------------------*/
    /*   主処理                                       */
    /*------------------------------------------------*/
    CNSV_main_processing();

    /*------------------------------------------------*/
    /*    終了処理                                    */
    /*------------------------------------------------*/
    CNSV_end_processing();

}

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_initialize                                */
/*  CALLING SEQ.    : void CNSV_initialize ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御(サーバー)の初期処理を行う            */
/****************************************************************************/
void CNSV_initialize(void)
{
short s_Err;
short s_Count;
short s_idx1,s_idx2;
//-char node_name[ZSYS_VAL_LEN_SYSTEMNAME+1];
char *pch_ptr;
short s_len;
typedef struct __param_def  /*サーバクラス論理ID*/
{
    char  site;
    char  fil1;
    char  network;
    char  fil2;
    char  group[5];
    char  fil3;
    char  server_class_name[8];
    char  fil4;
    char  server_class_num[4];
} param_def;
#define DEF_param_def_len 23
param_def *params;
lk_zac2001t_arg_1_def Trace_on;
char text_work[48];
procinfo_def procinfo;
_cc_status i_CC;

    EXTRACEFILENO = -1;                 /* トレース取得モジール用のファイル番号を初期化 */
    memset(&myinfo,'\0',sizeof(myinfo_def));
    myinfo.rcv_fd = DEF_FILE_CLOSED;
    myinfo.pathsend_fd = DEF_FILE_CLOSED;
    myinfo.cf_idx = 0;    /*コンフィグテーブルインデックス*/
    myinfo.end_flag = 0;
    PROCESSHANDLE_NULLIT_(myinfo.my_handle);
    PROCESSHANDLE_NULLIT_(myinfo.creator_phandle);
    s_Err = COM_PRC(&procinfo);             /*プロセス情報取得*/
    if (s_Err) {
        AbNormal_End();
    }
    memcpy(myinfo.my_handle,procinfo.my_phandle,ZSYS_VAL_PHANDLE_WLEN*2);
    memcpy(myinfo.creator_phandle,procinfo.ans_phandle,ZSYS_VAL_PHANDLE_WLEN*2);
    myinfo.my_name_len = procinfo.my_pname_len;
    memcpy(myinfo.my_name,procinfo.my_pname,procinfo.my_pname_len);
    myinfo.my_name[myinfo.my_name_len] = 0;
    myinfo.my_node_name_len = procinfo.my_nodename_len;
    memcpy(myinfo.my_node,procinfo.my_nodename,procinfo.my_nodename_len);
    memcpy(myinfo.pathmon_name,procinfo.ans_pname,procinfo.ans_pname_len);
    myinfo.pathmon_name[procinfo.ans_pname_len] = 0;
//-    PROCESSHANDLE_GETMINE_(myinfo.my_handle);
//-    PROCESSHANDLE_DECOMPOSE_(myinfo.my_handle
//-                            ,,,,node_name
//-                            ,ZSYS_VAL_LEN_SYSTEMNAME
//-                            ,&s_len
//-                            ,myinfo.my_name
//-                            ,ZSYS_VAL_LEN_PROCESSNAME
//-                            ,&s_Count);
//-    node_name[s_len] = 0;
//-    myinfo.my_name[s_Count] = 0;
//-    myinfo.my_name_len = s_Count;
//-    s_Err = PROCESS_GETPAIRINFO_(myinfo.my_handle
//-                                ,,,,,,,myinfo.creator_phandle);
//-    if (s_Err != 4 && s_Err != 5) {
//-        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_NOMAL,"@X@5","PROCESS_GETPAIRINFO_",s_Err,DEF_VAR_STOP);
//-        AbNormal_End();
//-    }
//-    PROCESSHANDLE_DECOMPOSE_(myinfo.creator_phandle
//-                            ,,,,,,,myinfo.pathmon_name,ZSYS_VAL_LEN_PROCESSNAME
//-                            ,&s_Count);
//-    myinfo.pathmon_name[s_Count] = 0;
    myinfo.GFPHI_fd = DEF_FILE_CLOSED;
    myinfo.GCLST_fd = DEF_FILE_CLOSED;
    myinfo.start_time = JULIANTIMESTAMP();

    myinfo.pathsend_io_timer = 1000;    /*メッセージ出力用に初期値設定*/
    sprintf(text_work,"%05d",myinfo.pathsend_io_timer/100);
    memcpy(myinfo.uytrmtimer,text_work,sizeof(myinfo.uytrmtimer));

    pch_ptr = getenv(DEF_MSG_MON_NAME);
    if (!pch_ptr) {     /*パラメータなし*/
        message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR,"@X@5",DEF_MSG_MON_NAME,1,DEF_VAR_STOP);
        exit(0);
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > sizeof(myinfo.uytrmmon)) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_MSG_MON_NAME,21,DEF_VAR_STOP);
            exit(0);
        }
        memset(myinfo.uytrmmon,' ',sizeof(myinfo.uytrmmon));
        memcpy(myinfo.uytrmmon,pch_ptr,strlen(pch_ptr));
        sprintf(text_work,"%02d",s_Count);
        memcpy(myinfo.uytrmmonlen,text_work,sizeof(myinfo.uytrmmonlen));
    }

    pch_ptr = getenv(DEF_MSG_SRV_NAME);
    if (!pch_ptr) {     /*パラメータなし*/
        message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR,"@X@5",DEF_MSG_SRV_NAME,1,DEF_VAR_STOP);
        exit(0);
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > sizeof(myinfo.uytrmsrv)) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_MSG_SRV_NAME,21,DEF_VAR_STOP);
            exit(0);
        }
        memset(myinfo.uytrmsrv,' ',sizeof(myinfo.uytrmsrv));
        memcpy(myinfo.uytrmsrv,pch_ptr,strlen(pch_ptr));
        sprintf(text_work,"%02d",s_Count);
        memcpy(myinfo.uytrmsrvlen,text_work,sizeof(myinfo.uytrmsrvlen));
    }

    pch_ptr = getenv(DEF_SRV_LOGICAL_ID);
    if (!pch_ptr) {     /*パラメータなし*/
       message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR,"@X@5",DEF_SRV_LOGICAL_ID,1,DEF_VAR_STOP);
       exit(0);
    }
    s_len = (short)strlen(pch_ptr);
    if (s_len != DEF_param_def_len)     /*サイズ正しくない*/
    {
        message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_SRV_LOGICAL_ID,21,DEF_VAR_STOP);
        exit(0);
    }
    params = (param_def *)pch_ptr;
    myinfo.site = params->site;
    myinfo.network = params->network;
    memcpy(myinfo.group,params->group,sizeof(myinfo.group));
    memcpy(myinfo.server_class_name,params->server_class_name,sizeof(myinfo.server_class_name));
    memcpy(myinfo.server_class_num,params->server_class_num,sizeof(myinfo.server_class_num));

    pch_ptr = getenv(DEF_FILE_IO_TIMER_10MSECOND);
    if (!pch_ptr) {     /*パラメータなし、デフォルト設定で動作*/
        myinfo.file_io_timer = 1000;
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > 8) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_FILE_IO_TIMER_10MSECOND,21,DEF_VAR_STOP);
            exit(0);
        }
        myinfo.file_io_timer = atol(pch_ptr);
    }

    pch_ptr = getenv(DEF_PSEND_TIMER_10MSECOND);
    if (!pch_ptr) {     /*パラメータなし、デフォルト設定で動作*/
        myinfo.pathsend_io_timer = 1000;
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > 8) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_PSEND_TIMER_10MSECOND,21,DEF_VAR_STOP);
            exit(0);
        }
        myinfo.pathsend_io_timer = atol(pch_ptr);
    }

    /*外部パラメータ確定、EMSのタイマー値も更新*/
    sprintf(text_work,"%04d",myinfo.pathsend_io_timer/100);
    memcpy(myinfo.uytrmtimer,text_work,sizeof(myinfo.uytrmtimer));

    pch_ptr = getenv(DEF_PSEND_RETRY_CNT);
    if (!pch_ptr) {     /*パラメータなし、デフォルト設定で動作*/
        myinfo.pathsend_retry_count = 1;
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > 4) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_PSEND_RETRY_CNT,21,DEF_VAR_STOP);
            exit(0);
        }
        myinfo.pathsend_retry_count = (short)atoi(pch_ptr);
    }

    pch_ptr = getenv(DEF_SOCKET_IO_TIMER_10MSECOND);
    if (!pch_ptr) {     /*パラメータなし、デフォルト設定で動作*/
        myinfo.socket_io_timer = DEF_SOCKET_TIMER;
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > 8) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_SOCKET_IO_TIMER_10MSECOND,21,DEF_VAR_STOP);
            exit(0);
        }
        myinfo.socket_io_timer = atol(pch_ptr);
    }

    pch_ptr = getenv(DEF_PROC_IO_TIMER_10MSECOND);
    if (!pch_ptr) {     /*パラメータなし、デフォルト設定で動作*/
        myinfo.process_io_timer = 1000;
    } else {
        s_Count = (short)strlen(pch_ptr);
        if (s_Count > 8) {
            message_output(DEF_EVT_PARAM_GET_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@5",DEF_PROC_IO_TIMER_10MSECOND,21,DEF_VAR_STOP);
            exit(0);
        }
        myinfo.process_io_timer = atol(pch_ptr);
    }
    myinfo.nowait_open_timer = myinfo.process_io_timer;    /*Nowaitオープン待ちタイマーもプロセスI/Oと同じにする*/

    COM_STP_INIT(&COM_STP_arg);
//-    /*オープナー管理テーブル*/
//-    for ( s_idx1 = 0; s_idx1 < DEF_MAX_NODE; s_idx1++ ) {
//-        memset(openers[s_idx1].node_name,'\0',sizeof(openers[s_idx1].node_name));
//-        for ( s_idx2 = 0; s_idx2 < 16; s_idx2++ ) {
//-            openers[s_idx1].cpus[s_idx2].opener_count = 0;
//-        }
//-    }
//-    memcpy(openers[0].node_name,node_name,strlen(node_name));   /*自ノードをテーブルに登録*/

    /*外部・内部イベント用空きリスト*/
    myinfo.failed_analyze_info.alloc_list_count = DEF_MAX_Event_list;
    CNSV_create_list(&myinfo.free_list,DEF_MAX_Event_list);
    myinfo.failed_analyze_info.event_node_top = (char *)myinfo.free_list.head;
    myinfo.failed_analyze_info.event_node_tail = (char *)myinfo.free_list.tail;
    CNSV_create_list(&myinfo.send_wait,0);

    /*管理テーブル*/
    for ( s_idx1 = 0; s_idx1 < 2; s_idx1++ ) {
        cf[s_idx1].sc_use = 0;
        cf[s_idx1].sc_conf = (sc_conf_def *)calloc(DEF_MAX_CONNECTION,sizeof(sc_conf_def));
        cf[s_idx1].if_use = 0;
        cf[s_idx1].st_conf = (st_conf_def *)calloc(DEF_MAX_interface,sizeof(st_conf_def));
        memset(cf[s_idx1].nw_kubun,' ',sizeof(cf[s_idx1].nw_kubun));
        cf[s_idx1].lc_use = 0;
        cf[s_idx1].lc_conf = (lc_conf_def *)calloc(DEF_MAX_LISTNER,sizeof(lc_conf_def));
        cf[s_idx1].ob_use = 0;
        cf[s_idx1].ob_conf = (ob_conf_def *)calloc(DEF_MAX_OUTBOUND,sizeof(ob_conf_def));
    }

    /*コネクション管理*/
    sc_info = (sc_info_def *)calloc(DEF_MAX_CONNECTION,sizeof(sc_info_def));
    sc_info2 = (sc_info_def *)calloc(DEF_MAX_CONNECTION,sizeof(sc_info_def));
    for ( s_idx2 = 0; s_idx2 < DEF_MAX_CONNECTION; s_idx2++ ) {
        sc_info[s_idx2].sock_fd = DEF_FILE_CLOSED;
        sc_info[s_idx2].recv_p = NULL;
        sc_info[s_idx2].send_p = NULL;
        sc_info[s_idx2].recv_timer_tag = DEF_TAG_NULL;
        sc_info[s_idx2].send_timer_tag = DEF_TAG_NULL;
        sc_info[s_idx2].idle_timer_tag = DEF_TAG_NULL;
        CNSV_create_list(&sc_info[s_idx2].send_wait,0);
        sc_info2[s_idx2].sock_fd = DEF_FILE_CLOSED;
        sc_info2[s_idx2].recv_p = NULL;
        sc_info2[s_idx2].send_p = NULL;
        sc_info2[s_idx2].recv_timer_tag = DEF_TAG_NULL;
        sc_info2[s_idx2].send_timer_tag = DEF_TAG_NULL;
        sc_info2[s_idx2].idle_timer_tag = DEF_TAG_NULL;
        CNSV_create_list(&sc_info2[s_idx2].send_wait,0);
    }
    /*リスナー管理*/
    lc_info = (lc_info_def *)calloc(DEF_MAX_LISTNER,sizeof(lc_info_def));
    for ( s_idx2 = 0; s_idx2 < DEF_MAX_LISTNER; s_idx2++ ) {
        lc_info[s_idx2].mng_no = DEF_FILE_CLOSED;
        lc_info[s_idx2].manage_count = 0;
        lc_info[s_idx2].station_index = 0;
    }
    /*リスナープロセス管理*/
    lp_info = (lp_info_def *)calloc(DEF_MAX_LISTNER,sizeof(lp_info_def));
    for ( s_idx2 = 0; s_idx2 < DEF_MAX_LISTNER; s_idx2++ ) {
        lp_info[s_idx2].listner_fd = DEF_FILE_CLOSED;
        lp_info[s_idx2].status = 0;
        lp_info[s_idx2].retry_type = 0;
        lp_info[s_idx2].async_p = NULL;
        lp_info[s_idx2].buf_p = NULL;
        lp_info[s_idx2].timer_tag = DEF_TAG_NULL;
        CNSV_create_list(&lp_info[s_idx2].send_wait,0);
    }
    /*Inbound電文振分管理*/
    ib_info = (ib_info_def *)calloc(1,sizeof(ib_info_def));
    CNSV_create_list(&ib_info->send_wait,0);
    for ( s_idx2 = 0; s_idx2 < DEF_MAX_INBOUND_PS; s_idx2++ ) {
        ib_info->ps_req[s_idx2].use_flag = DEF_TABLE_FREE;
        ib_info->ps_req[s_idx2].ps_buf_p = NULL;
        ib_info->ps_req[s_idx2].save_p = malloc(sizeof(c201_def));
    }
    /*Outbound電文振分管理*/
    ob_info = (ob_info_def *)calloc(DEF_MAX_OUTBOUND,sizeof(ob_info_def));
    for ( s_idx2 = 0; s_idx2 < DEF_MAX_OUTBOUND; s_idx2++ ) {
        ob_info[s_idx2].ps_manage.reply_tag = DEF_TAG_NULL;
        ob_info[s_idx2].ipc_mng.ob_fd = DEF_FILE_CLOSED;
        ob_info[s_idx2].ipc_mng.ob_status = 0;
        ob_info[s_idx2].ipc_mng.timer_tag = DEF_TAG_NULL;
        ob_info[s_idx2].ipc_mng.buf_p = NULL;
        ob_info[s_idx2].ipc_mng.retry_type = 0;
        CNSV_create_list(&ob_info[s_idx2].ipc_mng.send_wait,0);
    }
    /*コマンドサーバー管理*/
    ci_info = (ci_info_def *)calloc(1,sizeof(ci_info_def));
    ci_info->cmd_rcv_manage.reply_tag = DEF_TAG_NULL;
    CNSV_create_list(&ci_info->ps_manage.send_wait,0);

    /*バッファー$RECEIVE用と各I/O用バッファーをアロケート*/
    myinfo.failed_analyze_info.alloc_buffs_count = DEF_MAX_BUFFERS;
    CNSV_create_buff_list(&myinfo.buffs,DEF_MAX_BUFFERS);
    myinfo.failed_analyze_info.buff_list_top = (char *)myinfo.buffs.head;
    myinfo.failed_analyze_info.buff_list_tail = (char *)myinfo.buffs.tail;

    Trace_on.func_flg = '0';    /*トレース初期処理*/
    TRACEOUT((char *)&Trace_on);
    pch_ptr = getenv("PM-TRACE-FLG");
    if (pch_ptr) {
        if (memcmp(pch_ptr,"00",2)==0) {
            EXTRACEMODE = 0;        /*トレース指定がOFFの場合、EXTRACEMODEもOFFにする*/
        }
    }

    CNSV_get_param();   /*パラメータ取得*/

    /*$RECEIVEのオープン*/
    s_Err = FILE_OPEN_(DEF_RECEIVE_FILENAME
                      ,(short)strlen(DEF_RECEIVE_FILENAME)
                      ,&myinfo.rcv_fd
                      ,ZSYS_VAL_OPENACC_READWRITE
                      ,ZSYS_VAL_OPENEXCL_SHARED
                      ,DEF_RCV_NOWAITDEPTH
                      ,DEF_RECVDEPTH);
    if (s_Err != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_OPN_ERR,"@X@X@X@X@X@5","","","$RECEIVE","OPEN","",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }

    i_CC = SETMODE(myinfo.rcv_fd
                  ,30
                  ,3
                  );
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SETMODE",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }

//-    MONITORCPUS(0xffff);    /*monitor all cpus*/
//-    MONITORNET(1);          /*Enable receipt of messages.*/

    /*プロセス起動メッセージ*/
    message_output(DEF_EVT_PROC_START,DEF_MSGTTKB_NORMAL,DEF_NERR_NOMAL,"@X",myinfo.my_name,DEF_VAR_STOP);

    CNSV_recovery_processing();     /*リカバリー処理*/

} /*end of CNSV_initialize*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_get_param                                 */
/*  CALLING SEQ.    : void CNSV_get_param ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得                                        */
/****************************************************************************/
void CNSV_get_param(void)
{
short   s_rc;
short   s_gfphi_name_len;
char    ach_gfphi_name[ZSYS_VAL_LEN_FILENAME+1];
short   s_idx1,s_idx2;
short s_grp_id;
char  cha_group_info[DEF_GROUP_len_station];

    /*物理名情報ファイル名取得*/
    memset(ach_gfphi_name,' ',sizeof(ach_gfphi_name));
    memcpy(ach_gfphi_name,DEF_ASN_GFPHI,sizeof(DEF_ASN_GFPHI)-1);
    COM_ASN(ach_gfphi_name,myinfo.GFPHI_name,&s_gfphi_name_len);
    if (s_gfphi_name_len == 0) {
        AbNormal_End();
    }
    myinfo.GFPHI_name_len = s_gfphi_name_len;
    myinfo.GFPHI_name[s_gfphi_name_len] = 0;

    s_rc = CNSV_GFPHI_open();       /*物理名情報ファイルオープン*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_GFPHI_load();       /*物理名情報ファイル読込*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_GFLIN_load(myinfo.cf_idx);       /*回線管理ファイル*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_GCLST_open();       /*回線ステータスファイルオープン*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_GCLST_load(myinfo.cf_idx);       /*回線ステータスファイル読み込み*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_GFNWI_load(myinfo.cf_idx);       /*N/W情報ファイル読み込み*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_load_server_info(myinfo.cf_idx); /*サーバー情報取得*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }
    s_rc = CNSV_GFPHI_close();      /*物理名情報ファイルクローズ*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        AbNormal_End();
    }

    if (cf[myinfo.cf_idx].sc_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","LINE DEF",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (cf[myinfo.cf_idx].if_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI DEF",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (cf[myinfo.cf_idx].lc_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","Listener DEF",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (cf[myinfo.cf_idx].ob_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_PHSIC_INFO,"","Outbound DEF",DEF_VAR_STOP);
        AbNormal_End();
    }
    for (s_idx1 = 0; s_idx1 < cf[myinfo.cf_idx].sc_use; s_idx1++ ) {
        /*インターフェースorステーション管理テーブル、ステーション名で紐づけ*/
        for (s_idx2 = 0; s_idx2 < cf[myinfo.cf_idx].if_use; s_idx2++ ) {
            if (memcmp(cf[myinfo.cf_idx].sc_conf[s_idx1].station_name,cf[myinfo.cf_idx].st_conf[s_idx2].station_name,sizeof(cf[myinfo.cf_idx].sc_conf[s_idx1].station_name)) == 0) {
                sc_info[s_idx1].station_index = s_idx2;
                s_idx2 = cf[myinfo.cf_idx].if_use;
            }
        }
        /*リスナー管理テーブル、リスナーコネクション識別で紐づけ*/
        for (s_idx2 = 0; s_idx2 < cf[myinfo.cf_idx].lc_use; s_idx2++ ) {
            if (memcmp(cf[myinfo.cf_idx].sc_conf[s_idx1].lc_sc_sign,cf[myinfo.cf_idx].lc_conf[s_idx2].lc_sc_sign,sizeof(cf[myinfo.cf_idx].sc_conf[s_idx1].lc_sc_sign)) == 0) {
                sc_info[s_idx1].listner_index = s_idx2;
                s_idx2 = cf[myinfo.cf_idx].lc_use;
            }
        }
        /*Outbound電文振分管理テーブル、*/
        /*※コネクションテーブルとの紐づけは不要*/
    }
    for (s_idx1 = 0; s_idx1 < cf[myinfo.cf_idx].lc_use; s_idx1++ ) {
        /*インターフェースorステーション管理テーブル、ステーション名で紐づけ*/
        for (s_idx2 = 0; s_idx2 < cf[myinfo.cf_idx].if_use; s_idx2++ ) {
            if (memcmp(cf[myinfo.cf_idx].lc_conf[s_idx1].interface_name,cf[myinfo.cf_idx].st_conf[s_idx2].interface_name,sizeof(cf[myinfo.cf_idx].lc_conf[s_idx1].interface_name)) == 0) {
                lc_info[s_idx1].station_index = s_idx2;
                s_idx2 = cf[myinfo.cf_idx].if_use;
            }
        }
    }

    /*グルーピング設定*/
    memcpy(cha_group_info,(char *)&cf[myinfo.cf_idx].sc_conf[0].site_name,DEF_GROUP_len_station);   /*先頭のテーブルの値で初期化*/
    if (cf[myinfo.cf_idx].connect_num_mng_lyr == 'I') {            /*コネクション数管理単位 "I"：インタフェース単位*/
        for ( s_idx1 = 0, s_grp_id = 0; s_idx1 < cf[myinfo.cf_idx].sc_use; s_idx1++ ) {
            if (memcmp((char *)&cf[myinfo.cf_idx].sc_conf[s_idx1].site_name,cha_group_info,DEF_GROUP_len_interface) == 0) {
                sc_info[s_idx1].group_no = s_grp_id;
            } else {
                memcpy(cha_group_info,(char *)&cf[myinfo.cf_idx].sc_conf[s_idx1].site_name,DEF_GROUP_len_interface);
                s_grp_id++;
                sc_info[s_idx1].group_no = s_grp_id;
            }
        }
    } else if (cf[myinfo.cf_idx].connect_num_mng_lyr == 'S') {     /*コネクション数管理単位 "S"：ステーション単位*/
        for ( s_idx1 = 0, s_grp_id = 0; s_idx1 < cf[myinfo.cf_idx].sc_use; s_idx1++ ) {
            if (memcmp((char *)&cf[myinfo.cf_idx].sc_conf[s_idx1].site_name,cha_group_info,DEF_GROUP_len_station) == 0) {
                sc_info[s_idx1].group_no = s_grp_id;
            } else {
                memcpy(cha_group_info,(char *)&cf[myinfo.cf_idx].sc_conf[s_idx1].site_name,DEF_GROUP_len_station);
                s_grp_id++;
                sc_info[s_idx1].group_no = s_grp_id;
            }
        }
    } else {                                                /*コネクション数管理単位 その他(スペース)：管理対象外*/
    }

    for ( s_idx1 = 0; s_idx1 < cf[myinfo.cf_idx].lc_use; s_idx1++ ) {   /*従属コネクション数クリア*/
        lc_info[s_idx1].manage_count = 0;
    }
    for ( s_idx1 = 0; s_idx1 < cf[myinfo.cf_idx].sc_use; s_idx1++ ) {   /*従属コネクション数取得*/
        lc_info[sc_info[s_idx1].listner_index].manage_count++;
    }


} /*end of CNSV_get_param*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_recovery_processing                       */
/*  CALLING SEQ.    : void CNSV_recovery_processing ( void )                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リカバリー処理                                        */
/****************************************************************************/
void CNSV_recovery_processing(void)
{
short   s_idx;
short   s_find;
unsigned long ul_tag;
_cc_status i_CC;

    /*コネクション状態を確認、
    コマンドによりオープン状態のレコードがあればリスナーとOutbound電文振分に状態通知を行う*/
    for ( s_find = false,s_idx = 0; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {
        if (memcmp(sc_info[s_idx].process_info.process_status,DEF_PROC_STS_OPN,sizeof(DEF_PROC_STS_OPN)-1) == 0) {
            s_find = true;
        }
    }
    if (s_find == true) {
        /*コネクションステータスを再接続処理中に変更*/
        for ( s_idx = 0; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {
            if (memcmp(sc_info[s_idx].process_info.process_status,DEF_PROC_STS_OPN,sizeof(DEF_PROC_STS_OPN)-1) == 0) {
                CNSV_GCLST_update(s_idx,DEF_GCLST_con_state_listen,DEF_GCLST_proc_state_nocgange);
            }
        }
        /*リスナー、Outboundコンポーネントにコマンドを作成、サーバーのためコネクションは初期状態*/
        for ( s_idx = 0; s_idx < myinfo.lp_info_use_cnt; s_idx++ ) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Listener,s_idx,DEF_EV_connect_req,0,0,0,"");
        }
//        for ( s_idx = 0; s_idx < cf[myinfo.cf_idx].sc_use; s_idx++ ) {
//            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_socket  ,s_idx,DEF_EV_port_open,0,0,0,"");
//        }
        for ( s_idx = 0; s_idx < cf[myinfo.cf_idx].ob_use; s_idx++ ) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_connect_req,0,0,0,"");
        }
        COM_TGM(DEF_Component_system,0,DEF_EV_io_timeout,&ul_tag);
        i_CC = SIGNALTIMEOUT(1
                            ,DEF_EV_io_timeout
                            ,(__int32_t)ul_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","SIGNALTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        /*タイマーは掛け捨てのためタグの保持は行わない*/
    }

} /*end of CNSV_recovery_processing*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_main_processing                           */
/*  CALLING SEQ.    : void CNSV_main_processing ( void )                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御(サーバー)の主処理を行う              */
/****************************************************************************/
void CNSV_main_processing(void)
{
short s_Err;
_cc_status i_CC;
Event_Node_def InEvent;
buff_node_def *buff_node;

    buff_node = myinfo.buffs.head;        /*##バッファ取得*/
    myinfo.buffs.head = buff_node->next;
    myinfo.buffs.list_count--;
    buff_node->next = 0L;
    myinfo.rcv_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
    if (EXTRACEMODE) {
        rcv_trs = (lk_zac2001r_arg_1_def *)&buff_node->dt[1];
        memset((char *)&rcv_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
        COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
        memcpy((char *)rcv_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
    }
    i_CC = READUPDATEX(myinfo.rcv_fd
                      ,(char *)myinfo.rcv_p
                      ,sizeof(struct __c202_def)
                      ,,DEF_TAG_RECV);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","","$RECEIVE","READUPDATEX","",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    TS_UNIQUE_CREATE_((short *)&myinfo.TS128);

    for ( myinfo.end_flag = 0; myinfo.end_flag != -1;) {
        CNSV_io_wait();
        CNSV_event_judgement();
        while (myinfo.send_wait.head != NULL) {
            CNSV_remove_list(&myinfo.send_wait,&myinfo.free_list,&InEvent);
            switch (InEvent.compo) {
                case DEF_Component_system:
                    break;
                case DEF_Component_socket:
                    CNSV_sock_manage(&InEvent);
                    break;
                case DEF_Component_Listener:
                    CNSV_lsn_manage(&InEvent);
                    break;
                case DEF_Component_Inbound:
                    CNSV_ib_manage(&InEvent);
                    break;
                case DEF_Component_Outbound:
                    CNSV_ob_manage(&InEvent);
                    break;
                case DEF_Component_Command:
                    CNSV_ci_manage(&InEvent);
                    break;
                default:
                    AbNormal_End();
                    break;
            } /*end of switch compo*/
        } /*end of while*/
        if (IOCMP.fd == 0) {
            if (!myinfo.buffs.head) {   /*バッファ枯渇判定*/
                AbNormal_End();
            }
            buff_node = myinfo.buffs.head;        /*##バッファ取得*/
            myinfo.buffs.head = buff_node->next;
            buff_node->next = 0L;
            myinfo.buffs.list_count--;
            myinfo.rcv_p = (char *)buff_node->dt + (DEF_BUF_ADJUST);
            if (EXTRACEMODE) {
                rcv_trs = (lk_zac2001r_arg_1_def *)&buff_node->dt[1];
                memset((char *)&rcv_trs->func_flg,' ',DEF_TRS_HD_SIZE);     /*トレース情報初期化*/
                COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
                memcpy((char *)rcv_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
            }
            i_CC = READUPDATEX(myinfo.rcv_fd
                              ,(char *)myinfo.rcv_p
                              ,sizeof(struct __c202_def)
                              ,,DEF_TAG_RECV);
            if (_status_ne(i_CC)) {
                FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
                message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","","$RECEIVE","READUPDATEX","",s_Err,DEF_VAR_STOP);
                AbNormal_End();
            }
            TS_UNIQUE_CREATE_((short *)&myinfo.TS128);
        }
    }
} /*end of CNSV_main_processing*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_io_wait                                   */
/*  CALLING SEQ.    : void CNSV_io_wait ( void )                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : I/O完了待ち処理                                       */
/****************************************************************************/
void CNSV_io_wait(void)
{
_cc_status i_CC;

    IOCMP.fd = -1;
    IOCMP.fs_err = 0;
    IOCMP.len = 0;
    IOCMP.component = 0;
    IOCMP.thread = -1;
    IOCMP.event = 0;
    IOCMP.tag = 0L;
    IOCMP.addr = 0L;
    IOCMP.RINF.z_iotype = 0;
    IOCMP.RINF.z_maxreplycount = 0;
    IOCMP.RINF.z_messagetag = 0;
    IOCMP.RINF.z_filenum = 0;
    IOCMP.RINF.z_openlabel = 0;
    IOCMP.RINF.z_syncid = 0L;

    i_CC = AWAITIOX(&IOCMP.fd
                   ,&IOCMP.addr
                   ,(unsigned short *)&IOCMP.len
                   ,&IOCMP.tag
                   ,-1L);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(IOCMP.fd,&IOCMP.fs_err);
    }
    TS_UNIQUE_CREATE_((short *)&IOCMP.TS128);
} /*end of CNSV_io_wait*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_event_judgement                           */
/*  CALLING SEQ.    : void CNSV_event_judgement ( void )                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : イベント判定処理                                      */
/****************************************************************************/
void CNSV_event_judgement(void)
{
short   s_Err;
short   s_name_len;
short   s_node_len;
buff_node_def *buff_node;

    if (IOCMP.fd == 0) {              /*$RECEIVE完了*/
        s_Err = FILE_GETRECEIVEINFO_((short *)&IOCMP.RINF);
        if (s_Err != 0) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","FILE_GETRECEIVEINFO_",s_Err,DEF_VAR_STOP);
            AbNormal_End();
        }
        s_name_len = s_node_len = 0;
        PROCESSHANDLE_DECOMPOSE_((short *)&IOCMP.RINF.z_sender
                                ,(short *)&IOCMP.cpu
                                ,,,IOCMP.node_name
                                ,ZSYS_VAL_LEN_SYSTEMNAME
                                ,&s_node_len
                                ,IOCMP.proc_name
                                ,ZSYS_VAL_LEN_PROCESSNAME
                                ,&s_name_len);
        IOCMP.node_name[s_node_len] = 0;
        IOCMP.proc_name[s_name_len] = 0;
        if (EXTRACEMODE) {
            rcv_trs = (lk_zac2001r_arg_1_def *)(IOCMP.addr - DEF_IOCMP_TRS_HD_POS);
            rcv_trs->func_flg = '1';
            memcpy(rcv_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(rcv_trs->trace_info.prog_id));
            memcpy(rcv_trs->trace_info.file_id,"$RECEIVE",8);
            memcpy(rcv_trs->trace_info.file_name,"$RECEIVE",8);
            memcpy(rcv_trs->trace_info.file_io_type,"READ    ",8);
            sprintf(trace_work,"%04d",IOCMP.fs_err);
            memcpy(rcv_trs->trace_info.guardian_errcode,trace_work,sizeof(rcv_trs->trace_info.guardian_errcode));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy(rcv_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(rcv_trs->trace_info.shori_end_time));
            sprintf(trace_work,"%05d",IOCMP.len);
            memcpy(rcv_trs->data_info.rec_len,trace_work,sizeof(rcv_trs->data_info.rec_len));
            TRACEOUT((char *)rcv_trs);
        }
        if (IOCMP.fs_err == 6) {        /*SYSTEMメッセージ*/
            CNSV_sysmsg_manage();
            /*◆バッファー開放◆*/
            buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
            myinfo.buffs.tail->next = buff_node;
            myinfo.buffs.tail = buff_node;
            myinfo.buffs.list_count++;
            buff_node->next = 0L;
            if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
        } else if (IOCMP.fs_err == 0) { /*ノーマルメッセージ*/
            CNSV_msg_manage();
        } else {                        /*その他、Guardianエラー*/
            AbNormal_End();
        }
    } else {
        CNSV_io_comp();
    }
    return;
} /*end of CNSV_event_judgement*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_sysmsg_manage                             */
/*  CALLING SEQ.    : void CNSV_sysmsg_manage ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : システムメッセージ処理                                */
/****************************************************************************/
void CNSV_sysmsg_manage(void)
{
short   s_Reply_Info[4];
short   s_Err;
short   s_result;
_cc_status i_CC;

    memset(s_Reply_Info,'\0',sizeof(s_Reply_Info));

    open_msg_p     = (zsys_ddl_smsg_open_def *)IOCMP.addr;
    close_msg_p    = (zsys_ddl_smsg_close_def *) IOCMP.addr;
    timer_expire_p = (zsys_ddl_smsg_timesignal_def *)IOCMP.addr;
    cancel_p       = (zsys_ddl_smsg_qmsgcancelled_def *)IOCMP.addr;
    cpudown_p      = (zsys_ddl_smsg_cpudown_def *)IOCMP.addr;
    cpuup_p        = (zsys_ddl_smsg_cpuup_def *)IOCMP.addr;
    r_cpudown_p    = (zsys_ddl_smsg_remotecpudown_def *)IOCMP.addr;
    r_cpuup_p      = (zsys_ddl_smsg_remotecpuup_def *)IOCMP.addr;
    node_down_p    = (zsys_ddl_smsg_nodedown_def *)IOCMP.addr;
    node_up_p      = (zsys_ddl_smsg_nodeup_def *)IOCMP.addr;

    switch (open_msg_p->u_z_msgnumber.z_msgnumber) {
        case ZSYS_VAL_SMSG_OPEN:            /*-103*/
            CNSV_open_msg((short *)s_Reply_Info);
            s_result = COM_STP_JUDGE(&COM_STP_arg,(char *)IOCMP.addr);
            if (s_result == 1) {
                myinfo.end_flag = -1;
            } else if (s_result == -1) {
                AbNormal_End();
            }
            break;
        case ZSYS_VAL_SMSG_CLOSE:           /*-104*/
            CNSV_close_msg();
            s_result = COM_STP_JUDGE(&COM_STP_arg,(char *)IOCMP.addr);
            if (s_result == 1) {
                myinfo.end_flag = -1;
            } else if (s_result == -1) {
                AbNormal_End();
            }
            break;
        case ZSYS_VAL_SMSG_CPUDOWN:         /*-2*/
            CNSV_cpu_down_msg();
            s_result = COM_STP_JUDGE(&COM_STP_arg,(char *)IOCMP.addr);
            if (s_result == 1) {
                myinfo.end_flag = -1;
            } else if (s_result == -1) {
                AbNormal_End();
            }
            break;
        case ZSYS_VAL_SMSG_CPUUP:           /*-3*/
            CNSV_cpu_up_msg();
            break;
        case ZSYS_VAL_SMSG_NODEDOWN:        /*-110*/
            CNSV_node_down_msg();
            s_result = COM_STP_JUDGE(&COM_STP_arg,(char *)IOCMP.addr);
            if (s_result == 1) {
                myinfo.end_flag = -1;
            } else if (s_result == -1) {
                AbNormal_End();
            }
            break;
        case ZSYS_VAL_SMSG_NODEUP:          /*-111*/
            CNSV_node_up_msg();
            break;
        case ZSYS_VAL_SMSG_QMSGCANCELLED:   /*-38*/
            CNSV_cancel_msg();
            break;
        case ZSYS_VAL_SMSG_TIMESIGNAL:      /*-22*/
            CNSV_signal_msg();
            break;
        case ZSYS_VAL_SMSG_REMOTECPUDOWN:   /*-100*/
            CNSV_r_cpu_down_msg();
            s_result = COM_STP_JUDGE(&COM_STP_arg,(char *)IOCMP.addr);
            if (s_result == 1) {
                myinfo.end_flag = -1;
            } else if (s_result == -1) {
                AbNormal_End();
            }
            break;
        case ZSYS_VAL_SMSG_REMOTECPUUP:     /*-113*/
            CNSV_r_cpu_up_msg();
            break;
        default:
            /*■不明システムメッセージ受信、メッセージ出力後正常応答*/
            CNSV_unknown_system_msg();      /**/
            break;
    }
    i_CC = REPLYX((char *)&s_Reply_Info[2],s_Reply_Info[1]
                 ,,IOCMP.RINF.z_messagetag,s_Reply_Info[0]);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
} /*end of CNSV_sysmsg_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_open_msg                                  */
/*  CALLING SEQ.    : void CNSV_open_msg ( short * )                        */
/*  ARGUMENT        : リプライ情報                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : オープンメッセージ処理                                */
/****************************************************************************/
void CNSV_open_msg(short * Reply_Info)
{
//-short   s_idx;

    if (PROCESSHANDLE_COMPARE_(IOCMP.RINF.z_sender.u_z_data.z_word
                              ,myinfo.creator_phandle) != 0) {
        /*クリエータの場合*/
        Reply_Info[0] = 0;
        Reply_Info[1] = 4;
        Reply_Info[2] = ZSYS_VAL_SMSG_OPEN;
        Reply_Info[3] = DEF_LABEL_CREATOR;
    } else if (memcmp(IOCMP.proc_name, "$ZL", 3) == 0) {
        /*LINKMONの場合*/
        Reply_Info[0] = 0;
        Reply_Info[1] = 4;
        Reply_Info[2] = ZSYS_VAL_SMSG_OPEN;
        Reply_Info[3] = DEF_LABEL_LINKMON;
    } else {    /*その他プロセス*/
        Reply_Info[0] = 48;
        Reply_Info[1] = 0;
        return;
    }
//-    for ( s_idx = 0; s_idx < DEF_MAX_NODE; s_idx++ ) {
//-        if (memcmp(IOCMP.node_name,openers[s_idx].node_name,strlen(IOCMP.node_name)) == 0) {    /*登録有り*/
//-            openers[s_idx].cpus[IOCMP.cpu].opener_count++;
//-            s_idx = DEF_MAX_NODE;
//-        } else if (openers[s_idx].node_name[0] == '\0') {                           /*登録無し*/
//-            memcpy(openers[s_idx].node_name,IOCMP.node_name,strlen(IOCMP.node_name));
//-            openers[s_idx].cpus[IOCMP.cpu].opener_count++;
//-            s_idx = DEF_MAX_NODE;
//-        }
//-    }

} /*end of CNSV_open_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_close_msg                                 */
/*  CALLING SEQ.    : void CNSV_close_msg ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : クローズメッセージ処理                                */
/****************************************************************************/
void CNSV_close_msg(void)
{
//-short idx;
//-    if (IOCMP.RINF.z_openlabel == DEF_LABEL_CREATOR) {      /*クリエータの場合終了条件を設定*/
//-       myinfo.end_flag = -1;
//-       return;
//-    }
//-    for ( idx = 0; idx < DEF_MAX_NODE; idx++ ) {
//-        if (memcmp(IOCMP.node_name,openers[idx].node_name,strlen(IOCMP.node_name)) == 0) {    /*登録有り*/
//-            openers[idx].cpus[IOCMP.cpu].opener_count--;
//-            idx = DEF_MAX_NODE;
//-        }
//-    }
//-    CNSV_stop_check(1);     /*CLOSEを受け取っているので固定で1を指定*/
} /*end of CNSV_close_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_signal_msg                                */
/*  CALLING SEQ.    : void CNSV_signal_msg ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : タイムシグナルメッセージ処理                          */
/****************************************************************************/
void CNSV_signal_msg(void)
{
    COM_TGA(timer_expire_p->z_parm2,(unsigned short *)&IOCMP.component,(unsigned short *)&IOCMP.thread,(unsigned short *)&IOCMP.event);
    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,IOCMP.event,0,0,IOCMP.len,(char *)IOCMP.addr);
} /*end of CNSV_signal_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_end_processing                            */
/*  CALLING SEQ.    : void CNSV_end_processin ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクション制御(サーバー)の終了処理を行う            */
/****************************************************************************/
void CNSV_end_processing(void)
{
lk_zac2001t_arg_1_def Trace_off;

    Trace_off.func_flg = '2';    /*トレース終了処理*/
    TRACEOUT((char *)&Trace_off);
    /*終了メッセージ出力*/
    message_output(DEF_EVT_PROC_NORMAL_END,DEF_MSGTTKB_NORMAL,DEF_NERR_NOMAL,"@X",myinfo.my_name,DEF_VAR_STOP);
    /*プログラム終了*/
    PROCESS_STOP_();
} /*end of CNSV_end_processing*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_cpu_down_msg                              */
/*  CALLING SEQ.    : void CNSV_cpu_down_msg ( void )                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : CPUダウンメッセージ処理                               */
/****************************************************************************/
void CNSV_cpu_down_msg(void)
{
//-short   s_idx;
//-short   s_before_count;
//-    for ( s_before_count = 0, s_idx = 0; s_idx < DEF_MAX_NODE; s_idx++ ) {
//-        if (memcmp(IOCMP.node_name,openers[s_idx].node_name,strlen(IOCMP.node_name)) == 0) {    /*登録有り*/
//-            s_before_count += openers[s_idx].cpus[cpudown_p->z_cpunumber].opener_count;
//-            openers[s_idx].cpus[cpudown_p->z_cpunumber].opener_count = 0;
//-            s_idx = DEF_MAX_NODE;
//-        }
//-    }
//-    CNSV_stop_check(s_before_count);
} /*end of CNSV_cpu_down_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_r_cpu_down_msg                            */
/*  CALLING SEQ.    : void CNSV_r_cpu_down_msg ( void )                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リモートCPUダウンメッセージ処理                       */
/****************************************************************************/
void CNSV_r_cpu_down_msg(void)
{
//-short   s_idx;
//-short   s_before_count;
//-    /*OPNNERが存在しないノードからのメッセージも受取る可能性あり*/
//-    for ( s_before_count = 0, s_idx = 0; s_idx < DEF_MAX_NODE; s_idx++ ) {
//-        if (memcmp(r_cpudown_p->z_nodename,openers[s_idx].node_name,r_cpudown_p->z_nodename_len) == 0) {    /*登録有り*/
//-            s_before_count += openers[s_idx].cpus[r_cpudown_p->z_cpunumber].opener_count;
//-            openers[s_idx].cpus[r_cpudown_p->z_cpunumber].opener_count = 0;
//-            s_idx = DEF_MAX_NODE;
//-        }
//-    }
//-    CNSV_stop_check(s_before_count);
} /*end of CNSV_r_cpu_down_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_node_down_msg                             */
/*  CALLING SEQ.    : void CNSV_node_down_msg ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ノードダウンメッセージ処理                            */
/****************************************************************************/
void CNSV_node_down_msg(void)
{
//-short   s_idx,s_idx1;
//-short   s_before_count;
//-    /*OPNNERが存在しないノードからのメッセージも受取る可能性あり*/
//-    for ( s_before_count = 0, s_idx = 0; s_idx < DEF_MAX_NODE; s_idx++ ) {
//-        if (memcmp(node_down_p->z_nodename,openers[s_idx].node_name,node_down_p->z_nodename_len) == 0) {    /*登録有り*/
//-            for (s_idx1 = 0; s_idx1 < 16; s_idx1++ ) {
//-                s_before_count += openers[s_idx].cpus[s_idx1].opener_count;
//-                openers[s_idx].cpus[s_idx1].opener_count = 0;
//-            }
//-            s_idx = DEF_MAX_NODE;
//-        }
//-    }
//-    CNSV_stop_check(s_before_count);
} /*end of CNSV_node_down_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_cpu_up_msg                                */
/*  CALLING SEQ.    : void CNSV_cpu_up_msg ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : CPUアップメッセージ処理                               */
/****************************************************************************/
void CNSV_cpu_up_msg(void)
{
    /*必要があればメッセージ出力*/
} /*end of CNSV_cpu_up_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_r_cpu_up_msg                              */
/*  CALLING SEQ.    : void CNSV_r_cpu_up_msg ( void )                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リモートCPUアップメッセージ処理                       */
/****************************************************************************/
void CNSV_r_cpu_up_msg(void)
{
    /*必要があればメッセージ出力*/
} /*end of CNSV_r_cpu_up_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_node_up_msg                               */
/*  CALLING SEQ.    : void CNSV_node_up_msg ( void )                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リモートノードアップ処理                              */
/****************************************************************************/
void CNSV_node_up_msg(void)
{
    /*必要があればメッセージ出力*/
} /*end of CNSV_node_up_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_cancel_msg                                */
/*  CALLING SEQ.    : void CNSV_cancel_msg ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : キャンセルメッセージ処理                              */
/****************************************************************************/
/*
■要求を受取るのはOutbound電文振分、コマンドサーバーの2箇所
■それぞれの管理テーブルを検索する必要がある
*/
void CNSV_cancel_msg(void)
{
short   s_idx;
short   s_find_flag;

    s_find_flag = false;                                              /*Outbound電文振分チェック*/
    for (s_idx = 0;s_idx < DEF_MAX_OUTBOUND; s_idx++ ) {
        if (ob_info[s_idx].ps_manage.reply_tag == cancel_p->z_tag) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_idx,DEF_EV_cancel_msg,0,0,IOCMP.len,(char *)IOCMP.addr);
            s_find_flag = true;
            s_idx = DEF_MAX_OUTBOUND;
        }
    }
    if (ci_info->cmd_rcv_manage.reply_tag == cancel_p->z_tag) {     /*コマンドサーバーチェック*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,IOCMP.thread,DEF_EV_cancel_msg,0,0,IOCMP.len,(char *)IOCMP.addr);
        s_find_flag = true;
    }
    if (s_find_flag == false) {
        /*■必要があればタグ不明メッセージ出力*/
    }
} /*end of CNSV_cancel_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_unknown_system_msg                        */
/*  CALLING SEQ.    : void CNSV_unknown_system_msg ( void )                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 不明システムメッセージ処理                            */
/****************************************************************************/
void CNSV_unknown_system_msg(void)
{
    /*不明システムメッセージ受信、メッセージ出力*/
    /*該当するメッセージ定義なし*/
} /*end of CNSV_unknown_system_msg*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_msg_manage                                */
/*  CALLING SEQ.    : void CNSV_msg_manage ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : ノーマルメッセージ処理                                */
/****************************************************************************/
/*
    READUPDATE完了時にリクエスターから受け取ったIPCの内容からコンポーネントとイベントを判定する
    現在対象は「C202 電文送信要求」、「C502 コマンド処理要求」
*/
void CNSV_msg_manage(void)
{
    c202 = (c202_def *)IOCMP.addr;
    if        (memcmp(c202->common_header.interface_code,DEF_IPC_IFCD_DEN_SND_REQ,sizeof(DEF_IPC_IFCD_DEN_SND_REQ)-1) == 0) {  /*C202電文送信要求*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,0,DEF_EV_text_send_req,0,0,IOCMP.len,(char *)c202);
    } else if (memcmp(c202->common_header.interface_code,DEF_IPC_IFCD_CMD_PRC_REQ,sizeof(DEF_IPC_IFCD_CMD_REQ)-1) == 0) {  /*C502コマンド処理要求*/
        CNSV_cmd_req();
    } else {            /*不明リクエスト受信*/
        CNSV_unknown_req();
    }
} /*end of CNSV_msg_manage*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_cmd_req                                   */
/*  CALLING SEQ.    : void CNSV_cmd_req ( void )                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : C502:コマンド要求                                     */
/****************************************************************************/
/*
    IPCの内容からコマンドを判定する
*/
void CNSV_cmd_req(void)
{
    c502 = (c502_def *)IOCMP.addr;
    if        (memcmp(c502->command_info.command_name,DEF_IPC_CMD_OPN,sizeof(DEF_IPC_CMD_OPN)-1) == 0) {  /*1010 オープン*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,0,DEF_EV_connect_req,0,0,IOCMP.len,(char *)c502);
    } else if (memcmp(c502->command_info.command_name,DEF_IPC_CMD_CLS,sizeof(DEF_IPC_CMD_CLS)-1) == 0) {  /*1020 クローズ*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,0,DEF_EV_disconnect_req,0,0,IOCMP.len,(char *)c502);
    } else if (memcmp(c502->command_info.command_name,DEF_IPC_CMD_FL_RE_READ,sizeof(DEF_IPC_CMD_FL_RE_READ)-1) == 0) {  /*4010 ファイル再読み込み*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,0,DEF_EV_file_reload,0,0,IOCMP.len,(char *)c502);
    } else {            /*不明リクエスト受信*/
        CNSV_unknown_req();
    }
} /*end of CNSV_cmd_req*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_unknown_req                               */
/*  CALLING SEQ.    : void CNSV_unknown_req ( void )                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 不明リクエスト処理                                    */
/****************************************************************************/
void CNSV_unknown_req(void)
{
short s_Err;
a001_def *Request;
a001_def Error_reply;
buff_node_def *buff_node;
_cc_status i_CC;
    Request = (a001_def *)IOCMP.addr;
/*◆エラーメッセージ出力*/
    message_output(DEF_EVT_REQ_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_IPC_SEISA_ERR,"@X@X@I","","",Request->common_header.interface_code,DEF_VAR_STOP);
/*応答編集*/
    memcpy(Error_reply.common_header.interface_code,&Request->common_header.interface_code,sizeof(Error_reply.common_header.interface_code));
    Error_reply.common_header.error_code = 9;
    memcpy(Error_reply.common_header.internal_error_code,DEF_NERR_IPC_SEISA_ERR,sizeof(Error_reply.common_header.internal_error_code));
    memset(Error_reply.common_header.filler_1,0x20,sizeof(Error_reply.common_header.filler_1));
    Error_reply.common_header.control_data_length = 0;
    i_CC = REPLYX((char *)&Error_reply,sizeof(Error_reply),,IOCMP.RINF.z_messagetag);
    if (_status_ne(i_CC)) {
        FILE_GETINFO_(myinfo.rcv_fd,&s_Err);
        message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","REPLYX",s_Err,DEF_VAR_STOP);
        AbNormal_End();
    }
    /*◆バッファー開放◆*/
    buff_node = (buff_node_def *)(IOCMP.addr - (DEF_BUF_CTL_HD_SIZE + DEF_BUF_ADJUST));
    myinfo.buffs.tail->next = buff_node;
    myinfo.buffs.tail = buff_node;
    myinfo.buffs.list_count++;
    buff_node->next = 0L;
    if (!myinfo.buffs.head) myinfo.buffs.head = buff_node;
} /*end of CNSV_unknown_req*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_io_comp                                   */
/*  CALLING SEQ.    : void CNSV_io_comp ( void )                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : I/O完了処理                                           */
/****************************************************************************/
void CNSV_io_comp(void)
{
short s_idx;
    if (IOCMP.tag == -30) {   /*nowait open complete search Outbound*/
        IOCMP.component = DEF_Component_Outbound;
        IOCMP.event = DEF_EV_open_comp;
        for (s_idx = 0;s_idx < cf[myinfo.cf_idx].ob_use;s_idx++) {
            if (IOCMP.fd == ob_info[s_idx].ipc_mng.ob_fd) {
                IOCMP.thread = s_idx;
                s_idx = cf[myinfo.cf_idx].ob_use;
            }
        }
        if (EXTRACEMODE) {
//          開始時間はOPEN発行時の時間を設そのまま使用
//          memset((char *)&ob_info[IOCMP.thread].ipc_mng.trs,' ',sizeof(lk_trace_def));
//          COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
//          memcpy((char *)ob_info[IOCMP.thread].ipc_mng.trs.trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            ob_info[IOCMP.thread].ipc_mng.trs.func_flg = '1';
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.prog_id,DEF_GFPCVX10,sizeof(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.prog_id));
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.file_id,"PROCESS ",8);
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.file_name,ob_info[IOCMP.thread].ipc_mng.outbound_name,strlen(ob_info[IOCMP.thread].ipc_mng.outbound_name));
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.file_io_type,"OPENcomp",8);
            sprintf(trace_work,"%04d",IOCMP.fs_err);
            memcpy(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.guardian_errcode,trace_work,sizeof(ob_info[IOCMP.thread].ipc_mng.trs.trace_info.guardian_errcode));
            memset(ob_info[IOCMP.thread].ipc_mng.trs.data_info.rec_len,'0',sizeof(ob_info[IOCMP.thread].ipc_mng.trs.data_info.rec_len));
            TRACEOUT((char *)&ob_info[IOCMP.thread].ipc_mng.trs);
        }
    } else {
        /*タグのデコード*/
        COM_TGA((unsigned long)IOCMP.tag,(unsigned short *)&IOCMP.component,(unsigned short *)&IOCMP.thread,(unsigned short *)&IOCMP.event);
    }
    /*PATHSEND完了判定*/
    if (IOCMP.fd == myinfo.pathsend_fd) {   /*PATHSEND完了はInbound、Outbound、コマンドサーバー*/
        CNSV_pathsend_comp();
    } else {
        switch (IOCMP.component) {          /*※Inbound、コマンドサーバーはPATHSENDのためコンポーネントで振り分けせず*/
            case DEF_Component_socket:      /*コネクション管理コンポーネント(so)*/
                CNSV_sock_comp();
                break;

            case DEF_Component_Listener:    /*リスナー管理コンポーネント(lsn)*/
                CNSV_lsn_comp();
                break;

            case DEF_Component_Outbound:    /*OUTBOUND電文振分管理コンポーネント(ob)*/
                CNSV_ob_comp();
                break;

            default:                        /*コンポーネントID不明*/
                AbNormal_End();
                break;
        }
    }
    /*■バッファ開放はそれぞれのコンポーネントで行う*/

} /*end of CNSV_io_comp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_pathsend_comp                             */
/*  CALLING SEQ.    : void CNSV_pathsend_comp ( void )                      */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : PATHSEND完了処理                                      */
/****************************************************************************/
void CNSV_pathsend_comp(void)
{
short   s_Err;
short   s_Pathsend_Err = 0;
short   s_FS_Err;
    if (IOCMP.fs_err) {
        s_Err = SERVERCLASS_SEND_INFO_(&s_Pathsend_Err,&s_FS_Err);
        if (s_Err) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","SERVERCLASS_SEND_INFO_",s_Err,DEF_VAR_STOP);
            AbNormal_End();
        }
        if (s_FS_Err == 40) {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_timeout,s_Pathsend_Err,s_FS_Err,IOCMP.len,(char *)IOCMP.addr);
        } else {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_PS_err,s_Pathsend_Err,s_FS_Err,IOCMP.len,(char *)IOCMP.addr);
        }
        if (EXTRACEMODE) {
            ps_trs = (lk_zac2001p_arg_1_def *)(IOCMP.addr - DEF_IOCMP_TRS_HD_POS);
            ps_trs->func_flg = '1';
            memcpy(ps_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ps_trs->trace_info.prog_id));
            memcpy(ps_trs->trace_info.file_id,"PATHSEND",8);
            if (IOCMP.component == DEF_Component_Inbound) {     /*Inbound電文振分*/
                sprintf(trace_work,"%s/%s",ib_info->pathmon_name,ib_info->serverclass_name);
                memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
            } else {                                            /*コマンドサーバー*/
                sprintf(trace_work,"%s/%s",ci_info->ps_manage.pathmon_name,ci_info->ps_manage.server_class);
                memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
            }
            memcpy(ps_trs->trace_info.file_io_type,"READ    ",8);
            if (s_Pathsend_Err) {
                sprintf(trace_work,"%04d",s_Pathsend_Err);
            } else {
                sprintf(trace_work,"%04d",s_FS_Err);
            }
            memcpy(ps_trs->trace_info.guardian_errcode,trace_work,sizeof(ps_trs->trace_info.guardian_errcode));
            /*開始時間はI/O発生ににセットされているので下記はコメントアウト*/
//            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
//            memcpy((char *)ps_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_start_time));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy(ps_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
            sprintf(trace_work,"%05d",IOCMP.len);
            memcpy(ps_trs->data_info.rec_len,trace_work,sizeof(ps_trs->data_info.rec_len));
            TRACEOUT((char *)ps_trs);
        }
    } else {
        r201 = (r201_def *)IOCMP.addr;
        r501 = (r501_def *)IOCMP.addr;
        if (EXTRACEMODE) {
            ps_trs = (lk_zac2001p_arg_1_def *)(IOCMP.addr - DEF_IOCMP_TRS_HD_POS);
            ps_trs->func_flg = '1';
            memcpy(ps_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ps_trs->trace_info.prog_id));
            memcpy(ps_trs->trace_info.file_id,"PATHSEND",8);
            if (IOCMP.component == DEF_Component_Inbound) {     /*Inbound電文振分*/
                sprintf(trace_work,"%s/%s",ib_info->pathmon_name,ib_info->serverclass_name);
                memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
            } else {                                            /*コマンドサーバー*/
                sprintf(trace_work,"%s/%s",ci_info->ps_manage.pathmon_name,ci_info->ps_manage.server_class);
                memcpy(ps_trs->trace_info.file_name,trace_work,strlen(trace_work));
            }
            memcpy(ps_trs->trace_info.file_io_type,"READ    ",8);
            sprintf(trace_work,"%04d",IOCMP.fs_err);
            memcpy(ps_trs->trace_info.guardian_errcode,trace_work,sizeof(ps_trs->trace_info.guardian_errcode));
            /*開始時間はI/O発生ににセットされているので下記はコメントアウト*/
//            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
//            memcpy((char *)ps_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_start_time));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy(ps_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ps_trs->trace_info.shori_end_time));
            sprintf(trace_work,"%05d",IOCMP.len);
            memcpy(ps_trs->data_info.rec_len,trace_work,sizeof(ps_trs->data_info.rec_len));
            TRACEOUT((char *)ps_trs);
        }
        if        (memcmp(r201->common_header.interface_code,DEF_IPC_IFCD_DEN_RCV_NT_RSP,sizeof(DEF_IPC_IFCD_DEN_RCV_NT_RSP)-1) == 0) {  /*R201電文受信応答*/
            if (r201->common_header.error_code != 0) {  /*PATHSENDエラーと同一*/
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Inbound,IOCMP.thread,DEF_EV_PS_err,0,0,IOCMP.len,(char *)IOCMP.addr);
            } else {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Inbound,IOCMP.thread,DEF_EV_text_recv_rsp,0,0,IOCMP.len,(char *)r201);
            }
        } else if (memcmp(r201->common_header.interface_code,DEF_IPC_IFCD_CMD_RSP,sizeof(DEF_IPC_IFCD_CMD_RSP)-1) == 0) {  /*R501コマンド応答*/
            if (r501->common_header.error_code != 0) {  /*PATHSENDエラーと同一*/
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,IOCMP.thread,DEF_EV_PS_err,0,0,IOCMP.len,(char *)r201);
            } else {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Command,IOCMP.thread,DEF_EV_signon_rsp,0,0,IOCMP.len,(char *)r201);
            }
        } else {            /*不明応答受信*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_unknown_resp,0,0,IOCMP.len,(char *)r201);
        }
    }
    /*バツファー開放は各コンポーネントで行う*/
} /*end of CNSV_pathsend_comp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_sock_comp                                 */
/*  CALLING SEQ.    : void CNSV_sock_comp ( void )                          */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コネクションI/O完了処理                               */
/****************************************************************************/
void CNSV_sock_comp(void)
{
_cc_status i_CC;
    if (IOCMP.fs_err) {
        IOCMP.event++;      /*xx完了イベントをエラーイベントに振り直す*/
    } else if (IOCMP.event == DEF_EV_sock_recv_comp && IOCMP.len == 0) {
        IOCMP.event++;      /*recv完了イベントをエラーイベントに振り直す*/
    }
    CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,IOCMP.event,0,0,IOCMP.len,(char *)IOCMP.addr);
    if ((IOCMP.event == DEF_EV_sock_recv_comp || IOCMP.event == DEF_EV_sock_recv_err || IOCMP.event == DEF_EV_sock_send_err) &&
        (sc_info[IOCMP.thread].recv_timer_tag != DEF_TAG_NULL)) {
        i_CC = CANCELTIMEOUT(sc_info[IOCMP.thread].recv_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[IOCMP.thread].recv_timer_tag = DEF_TAG_NULL;
    }
    if ((IOCMP.event == DEF_EV_sock_send_comp || IOCMP.event == DEF_EV_sock_send_err || IOCMP.event == DEF_EV_sock_recv_err) &&
        (sc_info[IOCMP.thread].send_timer_tag != DEF_TAG_NULL)) {
        i_CC = CANCELTIMEOUT(sc_info[IOCMP.thread].send_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[IOCMP.thread].send_timer_tag = DEF_TAG_NULL;
    }
    /*ソケットのI/O完了時疎通状態とし、無通信監視タイマーを停止*/
    if (sc_info[IOCMP.thread].idle_timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(sc_info[IOCMP.thread].idle_timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        sc_info[IOCMP.thread].idle_timer_tag = DEF_TAG_NULL;
    }
} /*end of CNSV_sock_comp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_lsn_comp                                  */
/*  CALLING SEQ.    : void CNSV_lsn_comp ( void )                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスナーI/O完了処理                                   */
/****************************************************************************/
void CNSV_lsn_comp(void)
{
short s_thread;
_cc_status i_CC;
char    c_interface[5];
    if (IOCMP.event != DEF_EV_async_req) {      /*イベントが非同期要求以外*/
        if (lp_info[IOCMP.thread].timer_tag != DEF_TAG_NULL) {  /*タグが初期値以外*/
            i_CC = CANCELTIMEOUT(lp_info[IOCMP.thread].timer_tag);
            if (_status_ne(i_CC)) {
                message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
                AbNormal_End();
            }
            lp_info[IOCMP.thread].timer_tag = DEF_TAG_NULL;
        }
    }
    if (IOCMP.fs_err == 0) {
        n101 = (n101_def *)IOCMP.addr;
        n102 = (n102_def *)IOCMP.addr;
        r103 = (r103_def *)IOCMP.addr;
        r104 = (r104_def *)IOCMP.addr;
        r105 = (r105_def *)IOCMP.addr;
        r106 = (r106_def *)IOCMP.addr;
        if (EXTRACEMODE) {
            ipc_trs = (lk_zac2001i_arg_1_def *)(IOCMP.addr - DEF_IOCMP_TRS_HD_POS);
            ipc_trs->func_flg = '1';
            memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
            memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
            memcpy(ipc_trs->trace_info.file_name,cf[myinfo.cf_idx].lc_conf[IOCMP.thread].process_name,cf[myinfo.cf_idx].lc_conf[IOCMP.thread].process_name_len);
            memcpy(ipc_trs->trace_info.file_io_type,"READ    ",8);
            sprintf(trace_work,"%04d",IOCMP.fs_err);
            memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
            /*開始時間はI/O発生ににセットされているので下記はコメントアウト*/
//            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
//            memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
            sprintf(trace_work,"%05d",IOCMP.len);
            memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
            TRACEOUT((char *)ipc_trs);
        }
        /*リスナーからの要求のみ該当コネクションを判定*/
        if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_NT,sizeof(DEF_IPC_IFCD_CON_NT)-1) == 0) {    /*N101*/
            s_thread = CNSV_sc_table_search((char *)&n101->line_info.site_name,DEF_tbl_chk_len_connection);
            if (s_thread == -1) {     /*該当コネクション定義なし*/
                memset(c_interface,'\0',sizeof(c_interface));
                memcpy(c_interface,n101->common_header.interface_code,sizeof(n101->common_header.interface_code));
                message_output(DEF_EVT_RSP_SEISA_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_DST_SELECT_ERR,"@X@X@X@X","","",c_interface,"not Found",DEF_VAR_STOP);
                /*リスナーとの認識が合わない、同期を取り直す意味でI/Oエラー処理を行う*/
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_Listener_io_err,0,0,IOCMP.len,(char *)n101);
                return;
            }
        }
        if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_SW_NT,sizeof(DEF_IPC_IFCD_CON_SW_NT)-1) == 0) {   /*N102*/
            s_thread = CNSV_sc_table_search((char *)&n102->line_info.site_name,DEF_tbl_chk_len_connection);
            if (s_thread == -1 && n102->line_info.site_name != ' ') {   /*該当コネクション定義なし、切断要求以外*/
                memset(c_interface,'\0',sizeof(c_interface));
                memcpy(c_interface,n102->common_header.interface_code,sizeof(n102->common_header.interface_code));
                message_output(DEF_EVT_RSP_SEISA_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_DST_SELECT_ERR,"@X@X@X@X","","",c_interface,"not Found",DEF_VAR_STOP);
                /*対象外リスナーからのリクエスト*/
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_excluded_req,0,0,IOCMP.len,(char *)n101);
                return;
            }
        }
        if        (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_NT,sizeof(DEF_IPC_IFCD_CON_NT)-1) == 0) {   /*N101*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_connect_notice,s_thread,0,IOCMP.len,(char *)n101);
        } else if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_SW_NT,sizeof(DEF_IPC_IFCD_CON_SW_NT)-1) == 0) {   /*N102*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_dis_reconnect_notice,s_thread,0,IOCMP.len,(char *)n101);
        } else if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_START_RSP,sizeof(DEF_IPC_IFCD_CON_START_RSP)-1) == 0) {   /*R103*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_connect_start_resp,s_thread,0,IOCMP.len,(char *)n101);
        } else if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_NT_RSP,sizeof(DEF_IPC_IFCD_CON_NT_RSP)-1) == 0) {   /*R104*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_connect_complete_resp,s_thread,0,IOCMP.len,(char *)n101);
        } else if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_DISCON_NT_RSP,sizeof(DEF_IPC_IFCD_DISCON_NT_RSP)-1) == 0) {   /*R105*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_discinnect_complete_resp,s_thread,0,IOCMP.len,(char *)n101);
        } else if (memcmp(n101->common_header.interface_code,DEF_IPC_IFCD_CON_SW_NT_RSP,sizeof(DEF_IPC_IFCD_CON_SW_NT_RSP)-1) == 0) {   /*R106*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_chg_disconnect_notice,s_thread,0,IOCMP.len,(char *)n101);
        } else {            /*不明リクエスト受信、I/Oバッファの開放が必要*/
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_unknown_resp,0,0,IOCMP.len,(char *)n101);
        }
    } else {                /*リスナーI/Oエラー*/
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,IOCMP.component,IOCMP.thread,DEF_EV_Listener_io_err,0,0,IOCMP.len,(char *)n101);
    }
} /*end of CNSV_lsn_comp*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_ob_comp                                   */
/*  CALLING SEQ.    : void CNSV_ob_comp ( void )                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : OUTBOUND電文振分I/O完了処理                           */
/****************************************************************************/
void CNSV_ob_comp(void)
{
short   s_thread;
char    c_interface[5];
_cc_status i_CC;

    if (ob_info[IOCMP.thread].ipc_mng.timer_tag != DEF_TAG_NULL) {
        i_CC = CANCELTIMEOUT(ob_info[IOCMP.thread].ipc_mng.timer_tag);
        if (_status_ne(i_CC)) {
            message_output(DEF_EVT_PROCEDURE_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_SYSIF_LGC_ERR,"@X@5","CANCELTIMEOUT",0,DEF_VAR_STOP);
            AbNormal_End();
        }
        ob_info[IOCMP.thread].ipc_mng.timer_tag = DEF_TAG_NULL;
    }
    if (IOCMP.event == DEF_EV_open_comp) {  /*オープン完了*/
        if (IOCMP.fs_err) {
            IOCMP.event = DEF_EV_open_err;
        }
        CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,IOCMP.thread,IOCMP.event,0,0,IOCMP.len,(char *)IOCMP.addr);
    } else {                                /*I/O完了*/
        r107 = (r107_def *)IOCMP.addr;
        if (EXTRACEMODE) {
            ipc_trs = (lk_zac2001i_arg_1_def *)(IOCMP.addr - DEF_IOCMP_TRS_HD_POS);
            ipc_trs->func_flg = '1';
            memcpy(ipc_trs->trace_info.prog_id,DEF_GFPCVX10,sizeof(ipc_trs->trace_info.prog_id));
            memcpy(ipc_trs->trace_info.file_id,"PROCESS ",8);
            memcpy(ipc_trs->trace_info.file_name,cf[myinfo.cf_idx].ob_conf[IOCMP.thread].process_name,cf[myinfo.cf_idx].ob_conf[IOCMP.thread].process_name_len);
            memcpy(ipc_trs->trace_info.file_io_type,"READ    ",8);
            sprintf(trace_work,"%04d",IOCMP.fs_err);
            memcpy(ipc_trs->trace_info.guardian_errcode,trace_work,sizeof(ipc_trs->trace_info.guardian_errcode));
            /*開始時間はI/O発生ににセットされているので下記はコメントアウト、I/O開始から完了までの時間■*/
//            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
//            memcpy((char *)ipc_trs->trace_info.shori_start_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_start_time));
            COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
            memcpy(ipc_trs->trace_info.shori_end_time,(char *)&myinfo.ts_char.hh,sizeof(ipc_trs->trace_info.shori_end_time));
            sprintf(trace_work,"%05d",IOCMP.len);
            memcpy(ipc_trs->data_info.rec_len,trace_work,sizeof(ipc_trs->data_info.rec_len));
            TRACEOUT((char *)ipc_trs);
        }
        if (IOCMP.fs_err) {
            IOCMP.event = DEF_EV_outbound_io_err;
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,IOCMP.thread,IOCMP.event,0,0,IOCMP.len,(char *)IOCMP.addr);
            return;
        }
        if        (memcmp(r107->common_header.interface_code,DEF_IPC_IFCD_CON_STS_NT_RSP,sizeof(DEF_IPC_IFCD_CON_STS_NT_RSP)-1) == 0) {  /*R107 コネクション状態通知応答*/
            s_thread = CNSV_sc_table_search((char *)&r107->line_info.site_name,DEF_tbl_chk_len_connection);
            if (s_thread == -1) {
                memset(c_interface,'\0',sizeof(c_interface));
                memcpy(c_interface,r107->common_header.interface_code,sizeof(r107->common_header.interface_code));
                message_output(DEF_EVT_RSP_SEISA_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_DST_SELECT_ERR,"@X@X@X@X","","",c_interface,"not Found",DEF_VAR_STOP);
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,IOCMP.thread,DEF_EV_outbound_io_err,0,0,IOCMP.len,(char *)IOCMP.addr);
                return;
            }
            if (r107->common_header.error_code != 0) {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_thread,DEF_EV_outbound_io_err,0,0,IOCMP.len,(char *)IOCMP.addr);
            } else {
                CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,s_thread,DEF_EV_con_state_notice,0,0,IOCMP.len,(char *)IOCMP.addr);
            }
        } else {
            CNSV_add_list(&myinfo.free_list,&myinfo.send_wait,DEF_Component_Outbound,IOCMP.thread,DEF_EV_unknown_resp,0,0,IOCMP.len,(char *)IOCMP.addr);
        }
    }
} /*end of CNSV_ob_comp*/
