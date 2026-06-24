/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/02/05＞         *
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
/*        WRITTEN-DATE      ････ 2025/02/05                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/02/05 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <errno.h>      nolist
#include <string.h>     nolist
#include <tal.h>        nolist
#ifdef _TANDEM_SOURCE
#include <cextdecs.h(READUPDATEX,FILE_OPEN_,REPLYX,CANCELTIMEOUT,SETMODE)> nolist
#include <cextdecs.h(FILE_GETINFO_,PROCESSHANDLE_COMPARE_,PROCESSHANDLE_DECOMPOSE_)> nolist
#else
#include <cextdecs.h> nolist
#endif
/* USER HEADER     */
#include <GFPCGXB0.h> nolist
#include <common.h> nolist
#include <errcd.h> nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_cmp_ipc_interface.h" nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_event.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_sys.h" nolist
#include "GFPCVX20_thread_factory.h" nolist
#include "GFPCVX20_thread_header.h" nolist
#include "GFPCVX20_util.h" nolist
#include "GFPCVX20_ipc_wrapper.h" nolist // IWYU pragma: keep

/****************************************************************************/
/*   内部データ定義                                                         */
/****************************************************************************/
typedef struct __opener_info_t
{
    COM_STP_arg_1_def opener_info;
    bool              initialized;
} opener_info_t;

static opener_info_t opener_info; /* オープナー情報テーブル       */
/*****************************************************************************/
/*  FUNCTION        :cncl_create_ipc                                         */
/*  CALLING SEQ.    :thread_object_t *cncl_create_ipc(void)                  */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :スレッドオブジェクトのポインタ,                         */
/*                  :※失敗時は内部で異常終了する可能性                      */
/*  DESCRIPTION     :IPC通信用スレッドを生成し初期化する                     */
/*****************************************************************************/
thread_object_t *cncl_create_ipc()
{
    thread_object_t     *thread_ipc;
    thread_header_t     *hd;
    cmp_ipc_interface_t *cmp_ipc;
    thread_ipc             = cncl_create_thread(ipc_interface);
    hd                     = cncl_get_threadInfo(thread_ipc);
    hd->io_complete_buffer = alloc_io_mem(&io_mem);
    if (!hd->io_complete_buffer) {
        cncl_ems_procedure_error("cncl_create_ipc", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    memset(hd->io_complete_buffer, 0, sizeof(io_trace_buf_t));
    cmp_ipc              = (cmp_ipc_interface_t *)cncl_get_component(thread_ipc);
    cmp_ipc->i_openers   = (COM_STP_arg_1_def *)&(opener_info.opener_info);
    cmp_ipc->receive_fno = -1;
    if (!opener_info.initialized) {
        COM_STP_INIT(&(opener_info.opener_info));
        opener_info.initialized = true;
    }
    return thread_ipc;
}
/*****************************************************************************/
/*  FUNCTION        :receive_open                                            */
/*  CALLING SEQ.    :void receive_open(short *fno)                           */
/*  ARGUMENT        :fno:オープンしたファイル番号を格納するポインタ          */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :$RECEIVEファイルをオープンする処理                      */
/*****************************************************************************/
#define DEF_RECEIVE_FILENAME "$RECEIVE"
void receive_open(short *fno)
{
    short      s_err;
    _cc_status cc;
    s_err = FILE_OPEN_(DEF_RECEIVE_FILENAME, (short)strlen(DEF_RECEIVE_FILENAME), fno, ZSYS_VAL_OPENACC_READWRITE,
                       ZSYS_VAL_OPENEXCL_SHARED, 1, DEF_RECVDEPTH, no_param, no_param, no_param, no_param, no_param);
    if (s_err) {
        cncl_ems_fileio_error(no_param, no_param, DEF_RECEIVE_FILENAME, DEF_COM_IOM_FUNC_OPEN, no_param, no_param,
                              (short)errno, DEF_NERR_FILE_OPN_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_OPN_ERR);
    }
    cc = SETMODE(*fno, 30, 3, no_param, no_param);
    if (_status_ne(cc)) {
        FILE_GETINFO_(*fno, &s_err, no_param, no_param, no_param, no_param, no_param);
        cncl_ems_procedure_error("SETMODE", s_err, DEF_NERR_FILE_OPN_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_OPN_ERR);
        ;
    }
}
/*****************************************************************************/
/*  FUNCTION        :receive_readupdate                                      */
/*  CALLING SEQ.    :void receive_readupdate(                                */
/*                   short fno,                                              */
/*                   char *read_buff,                                        */
/*                   unsigned short buff_length,                             */
/*                   Event_tag_t *event)                                     */
/*  ARGUMENT        :fno        :受信用ファイル番号                          */
/*                  :read_buff  :受信データを格納するバッファ                */
/*                  :buff_length:読み込みバッファのサイズ                    */
/*                  :event      :完了時に通知されるイベント                  */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :READUPDATEXを実行しデータを読み込む                     */
/*****************************************************************************/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
void receive_readupdate(short fno, char *read_buff, unsigned short buff_length, Event_tag_t *event)
{
    _cc_status cc;
    short      s_err;
    cc = READUPDATEX(fno, read_buff, buff_length, no_param, (long)event);
    if (_status_ne(cc)) {
        FILE_GETINFO_(fno, &s_err, no_param, no_param, no_param, no_param, no_param);
        cncl_ems_fileio_error(no_param, no_param, DEF_RECEIVE_FILENAME, "READUPDATEX", no_param, no_param, s_err,
                              DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
    }
}
#pragma clang diagnostic pop
/*****************************************************************************/
/*  FUNCTION        :cncl_ipc_start                                          */
/*  CALLING SEQ.    :short cncl_ipc_start(thread_object_t *thread_ipc)       */
/*  ARGUMENT        :thread_ipc:IPC用スレッドオブジェクト                    */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :IPCスレッドを開始し受信処理を行う                       */
/*****************************************************************************/
short cncl_ipc_start(thread_object_t *thread_ipc)
{
    thread_header_t     *hd;
    cmp_ipc_interface_t *cmp_ipc;
    Event_tag_t         *receive_event;
    cmp_ipc = (cmp_ipc_interface_t *)cncl_get_component(thread_ipc);
    hd      = cncl_get_threadInfo(thread_ipc);
    if (cmp_ipc->receive_fno == -1) {
        receive_open(&(cmp_ipc->receive_fno));
    }
    receive_event = cncl_get_io_tag(thread_ipc, cmp_ipc->receive_fno, cncl_ipc_received);
    if(!receive_event)
    {
        cncl_ems_procedure_error("cncl_get_io_tag", 0, DEF_NERR_CNCL_RES_XHAUST);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
    }
    receive_readupdate(cmp_ipc->receive_fno, hd->io_complete_buffer->data_info.rec_area,
                       sizeof(hd->io_complete_buffer->data_info.rec_area), receive_event);
    return 0;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ipc_open_msg                                       */
/*  CALLING SEQ.    :void cncl_ipc_open_msg(Event_tag_t *event)              */
/*  ARGUMENT        :event:オープン要求のイベント情報                        */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :OPENメッセージ受信時の処理を行う                        */
/*****************************************************************************/
void cncl_ipc_open_msg(Event_tag_t *event)
{
    zsys_ddl_smsg_open_reply_def open_reply;
    zsys_ddl_smsg_open_def      *smsg_open = (zsys_ddl_smsg_open_def *)event->io_info.data->data_info.rec_area;
    Application                 *app       = cncl_get_App();
    short                        reply_err = 0;
    short                        s_err;
    _cc_status                   cc;

    memset(&open_reply, 0, sizeof(zsys_ddl_smsg_open_reply_def));
    open_reply.z_msgnumber = ZSYS_VAL_SMSG_OPEN;

    if (PROCESSHANDLE_COMPARE_(event->io_info.RINF.z_sender.u_z_data.z_word, app->my_info.creator_phandle)) {
        open_reply.z_openid = DEF_LABEL_CREATOR;
    } else if (!memcmp(event->io_info.proc_name, "$ZL", 3)) {
        open_reply.z_openid = DEF_LABEL_LINKMON;
    } else if (!memcmp(smsg_open->u_z_data.z_qualifier, DEF_CMDS_QUALIFIER, strlen(DEF_CMDS_QUALIFIER))) {
        open_reply.z_openid = DEF_LABEL_COMMAND;
    } else {
        reply_err = 48;
    }
    cc = REPLYX((const char *)&open_reply, (unsigned short)sizeof(zsys_ddl_smsg_open_reply_def), /* written */ no_param,
                event->io_info.RINF.z_messagetag, reply_err);

    if (_status_ne(cc)) {
        FILE_GETINFO_(event->io_info.fd, &s_err, no_param, no_param, no_param, no_param, no_param);
        // REPLYXはprocedure_error
        cncl_ems_procedure_error("REPLYX", s_err, DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
    }
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ipc_signal_msg                                     */
/*  CALLING SEQ.    :short cncl_ipc_signal_msg(Event_tag_t *event)           */
/*  ARGUMENT        :event:シグナル通知イベント                              */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :シグナルメッセージ受信時の処理を行う                    */
/*****************************************************************************/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
short cncl_ipc_signal_msg(Event_tag_t *event)
{
    short                         ret;
    zsys_ddl_smsg_timesignal_def *tms       = (zsys_ddl_smsg_timesignal_def *)(event->io_info.data->data_info.rec_area);
    Event_tag_t                  *event_tms = (Event_tag_t *)tms->z_parm2;
    thread_header_t *hd = cncl_get_threadInfo( event_tms->thread);
    hd->sig_ev_tag = NULL;
    event_tms->signal_timeout_info.sparam   = tms->z_parm1;  // 使ってない。
    event_tms->io_info = event->io_info;
    ret                                     = event_tms->func(event_tms);
    cncl_remove_event(event_tms);
    return ret;
}
#pragma clang diagnostic pop
/*****************************************************************************/
/*  FUNCTION        :cncl_ipc_sys_message                                    */
/*  CALLING SEQ.    :short cncl_ipc_sys_message(Event_tag_t *event)          */
/*  ARGUMENT        :event:システムメッセージイベント                        */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :システムメッセージを受信し応答する                      */
/*****************************************************************************/
short cncl_ipc_sys_message(Event_tag_t *event)
{
    zsys_ddl_smsg_open_def *open_msg_p = (zsys_ddl_smsg_open_def *)event->io_info.data->data_info.rec_area;
    cmp_ipc_interface_t    *cmp_ipc    = cncl_get_component(event->thread);
    short                   ret = 0, ret_signal = 0;
    short                   s_err = 0;
    _cc_status              cc;

    switch (open_msg_p->u_z_msgnumber.z_msgnumber) {
        case ZSYS_VAL_SMSG_OPEN:       /*-103*/
            cncl_ipc_open_msg(event);
            ret = COM_STP_JUDGE(cmp_ipc->i_openers, event->io_info.data->data_info.rec_area);
            break;
        case ZSYS_VAL_SMSG_TIMESIGNAL: /*-22*/
            ret_signal = cncl_ipc_signal_msg(event);
            /* no break */
        case ZSYS_VAL_SMSG_NODEDOWN:      /*-110*/
        case ZSYS_VAL_SMSG_REMOTECPUDOWN: /*-100*/
        case ZSYS_VAL_SMSG_CLOSE:         /*-104*/
        case ZSYS_VAL_SMSG_CPUDOWN:       /*-2*/
            ret = COM_STP_JUDGE(cmp_ipc->i_openers, event->io_info.data->data_info.rec_area);
            /* no break */
        case ZSYS_VAL_SMSG_CPUUP:         /*-3*/
        case ZSYS_VAL_SMSG_NODEUP:        /*-111*/
        case ZSYS_VAL_SMSG_QMSGCANCELLED: /*-38*/
        case ZSYS_VAL_SMSG_REMOTECPUUP:   /*-113*/
        default:
            cc = REPLYX(no_param, no_param, no_param, event->io_info.RINF.z_messagetag, no_param);
            if (_status_ne(cc)) {
                FILE_GETINFO_(event->io_info.fd, &s_err, no_param, no_param, no_param, no_param, no_param);
                // REPLYXはprocedure_error
                cncl_ems_procedure_error("REPLYX", s_err, DEF_NERR_FILE_IO_ERR);
                cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
            }
            break;
    }
    /* COM_STP_JUDGEのプロセス停止判定 または signalhandlerの終了要求 */
    if (ret == 1 || ret_signal != 0) cncl_get_App()->stopwait = true;
    return ret;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ipc_message                                        */
/*  CALLING SEQ.    :short cncl_ipc_message(Event_tag_t *event)              */
/*  ARGUMENT        :event:IPCメッセージイベント情報                         */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :IPC電文の受信内容を判定し応答を返す                     */
/*****************************************************************************/
short cncl_ipc_message(Event_tag_t *event)
{
    // TODO:ここは関数化したいなあ…。
    thread_header_t   *hd_ipc      = cncl_get_threadInfo(event->thread);
    common_header_def *ipc_comm_hd = (common_header_def *)event->io_info.data->data_info.rec_area;
    unsigned short     reply_length;
    char               reply_msg[sizeof(r502_def)];
    short              s_err  = 0;
    _cc_status         cc;

    memset(reply_msg, ' ', sizeof(reply_msg));
    // TODO:受信データのトレースをとる？
    if (!memcmp(ipc_comm_hd->interface_code, DEF_IPC_IFCD_DEN_SND_REQ, sizeof(ipc_comm_hd->interface_code))) {
        thread_object_t *thread_trans;
        cmp_trans_t     *cmp_trans;
        thread_header_t *hd_snd;
        c202_def        *send_req   = (c202_def *)ipc_comm_hd;
        r202_def        *r202_reply = (r202_def *)reply_msg;
        reply_length                = sizeof(r202_def);

        memcpy(r202_reply->common_header.interface_code, DEF_IPC_IFCD_DEN_SND_RSP,
               sizeof(DEF_IPC_IFCD_DEN_SND_RSP) - 1);
        r202_reply->common_header.control_data_length = 0;

        // 電文送信要求
        thread_trans = cncl_trans_req_send_select_transport(event->io_info.data);
        if (thread_trans) {
            // 送信先スレッドが見つかった場合は、IPC受信スレッドに当たらしい受信バッファを割当て、電文振分に正常応答を返す。
            cmp_trans = cncl_get_component(thread_trans);
            hd_snd    = cncl_get_threadInfo(cmp_trans->snd);
            if (hd_snd && hd_snd->stopwait == false && cmp_trans->line_status == e_line_connected) {
                // 切断待ちでなく、回線状態が接続状態であるとき送信可能。
                hd_ipc->io_complete_buffer = alloc_io_mem(&io_mem);
                if (!hd_ipc->io_complete_buffer) {
                    cncl_ems_procedure_error("cncl_ipc_message", 0, DEF_NERR_CNCL_RES_XHAUST);
                    cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_CNCL_RES_XHAUST);
                }
                event->io_info.data                  = NULL;
                r202_reply->common_header.error_code = DEF_IPC_ERRCD_OK;
                memcpy(r202_reply->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL) - 1);
            } else {
                // 切断待ちまたは、回線状態が未接続であるとき送信不可能。
                r202_reply->common_header.error_code = DEF_IPC_ERRCD_NG;
                memcpy(r202_reply->common_header.internal_error_code, DEF_NERR_CNCL_NOT_CON,
                       sizeof(DEF_NERR_CNCL_NOT_CON) - 1);
            }
        } else {
            // 送信先スレッドが見つからないor切断中or送信キュー超過の場合は送信エラーを返す。
            r202_reply->common_header.error_code = DEF_IPC_ERRCD_NG;
            memcpy(r202_reply->common_header.internal_error_code, DEF_NERR_CNCL_NO_SUCH_DST,
                   sizeof(DEF_NERR_CNCL_NO_SUCH_DST) - 1);
            cncl_ems_ipc_invalid_request("No link/line or que", (char *)send_req, DEF_NERR_CNCL_NO_SUCH_DST);
        }
    } else if (!memcmp(ipc_comm_hd->interface_code, DEF_IPC_IFCD_CMD_PRC_REQ, sizeof(ipc_comm_hd->interface_code))) {
        /* コマンド処理(R502 コマンド処理 コマンドサーバ⇔コネクション制御(クライアント) */
        c502_def *c502_req   = (c502_def *)ipc_comm_hd;
        r502_def *r502_reply = (r502_def *)reply_msg;
        reply_length         = sizeof(r502_def);
        char *command_name   = c502_req->command_info.command_name;

        cncl_ems_cmd_rcv(no_param, &(c502_req->command_info.connection_logical_name),
                         c502_req->command_info.command_name, (char *)&(c502_req->command_info.connection_logical_name),
                         DEF_NERR_NOMAL);

        // 正常応答(R502)で編集しておく。
        memcpy(r502_reply, c502_req, sizeof(r502_def));
        memcpy(r502_reply->common_header.interface_code, DEF_IPC_IFCD_CMD_PRC_RSP, sizeof(DEF_IPC_IFCD_CMD_PRC_RSP));
        r502_reply->common_header.error_code = DEF_IPC_ERRCD_OK;
        memcpy(r502_reply->common_header.internal_error_code, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL) - 1);
        r502_reply->common_header.control_data_length = 0; // workaround for r502_def issue.

        if (!memcmp(command_name, DEF_IPC_CMD_OPN, sizeof(c502_req->command_info.command_name))) {
            /* オープンコマンド (1010)*/
            thread_object_t *thread_trans = cncl_search_trans_thread(&(c502_req->command_info.connection_logical_name));
            if (thread_trans) {
                Event_tag_t *event;
                event = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_connect_request, 0, NULL);
                cncl_post_event(event);
            } else {
                r502_reply->common_header.error_code = DEF_IPC_ERRCD_NG;
                // TODO:エラーコードが未定義なのかも？
                memcpy(r502_reply->common_header.internal_error_code, DEF_NERR_CNCL_NO_SUCH_DST,
                       sizeof(DEF_NERR_CNCL_NO_SUCH_DST) - 1);
            }
        } else if (!memcmp(command_name, DEF_IPC_CMD_CLS, sizeof(c502_req->command_info.command_name))) {
            /* クローズコマンド (1020)*/
            thread_object_t *thread_trans = cncl_search_trans_thread(&(c502_req->command_info.connection_logical_name));
            if (thread_trans) {
                Event_tag_t     *event;
                cmp_trans_t     *cmp_trans = cncl_get_component(thread_trans);
                thread_header_t *snd_hd    = cncl_get_threadInfo(cmp_trans->snd);
                thread_header_t *rcv_hd    = cncl_get_threadInfo(cmp_trans->rcv);
                if (snd_hd) snd_hd->stopwait = true;
                if (rcv_hd) rcv_hd->stopwait = true;
                cmp_trans->line_discon_reason = e_discon_by_cls_instruct;
                event = cncl_get_scheduled_tag(thread_trans, -1, cncl_trans_close_request, 0, NULL);
                cncl_post_event(event);
            } else {
                r502_reply->common_header.error_code = DEF_IPC_ERRCD_NG;
                // TODO:エラーコードが未定義なのかも？
                memcpy(r502_reply->common_header.internal_error_code, DEF_NERR_CNCL_NO_SUCH_DST,
                       sizeof(DEF_NERR_CNCL_NO_SUCH_DST) - 1);
            }
        } else if (!memcmp(command_name, DEF_IPC_CMD_FL_RE_READ, sizeof(c502_req->command_info.command_name))) {
            /* 構成変更(4010) */
            /* DEF_NERR_RE_READ_CMD_HAKKOU_ERR      ファイル再読込みコマンド発行エラー         */
            /* DEF_NERR_RE_READ_CMD_SEISA_ERR       ファイル再読込みコマンド精査エラー         */
            /* DEF_NERR_RE_READ_ERR                 ファイル再読込み失敗                       */
            /* DEF_NERR_DST_SELECT_ERR              送信先選択不可                             */
            short            result;
            result = cncl_load_config_collection_request(&(c502_req->command_info.connection_logical_name));
            if(result){
                char  *internal_error_code = DEF_NERR_RE_READ_ERR;
                if(result == -2) internal_error_code = DEF_NERR_DST_SELECT_ERR;
                r502_reply->common_header.error_code = DEF_IPC_ERRCD_NG;
                memcpy(r502_reply->common_header.internal_error_code, internal_error_code, strlen(internal_error_code));
            }
        } else {
            /* コマンド識別不正 */
            r502_reply->common_header.error_code = DEF_IPC_ERRCD_NG;
            memcpy(r502_reply->common_header.internal_error_code, DEF_NERR_IPC_SEISA_ERR,
                   sizeof(DEF_NERR_IPC_SEISA_ERR) - 1);
            cncl_ems_ipc_invalid_header(no_param, no_param, ipc_comm_hd->interface_code, "interface_code",
                                        DEF_NERR_IPC_SEISA_ERR);
        }
        // コマンド実行結果 EMS
        char cmd[10+1];
        char gflin_pkey[sizeof(gflin_pkey_def)+1];
        memset(cmd,0,sizeof(cmd));
        memset(gflin_pkey,0,sizeof(gflin_pkey));
        memcpy(cmd,c502_req->command_info.command_name,sizeof(c502_req->command_info.command_name));
        cncl_ems_cmd_result(
            no_param, &(c502_req->command_info.connection_logical_name), c502_req->command_info.command_name,
            r502_reply->common_header.error_code == DEF_IPC_ERRCD_OK ? "OK   " : "ERROR",
            r502_reply->common_header.error_code == DEF_IPC_ERRCD_OK ? DEF_NERR_NOMAL : DEF_NERR_CNCL_CMD_ERR);
    } else {
        /* 対象外電文(電文種別不明) ヘッダのみ返す。 */
        /*応答編集*/
        common_header_def *common_reply = (common_header_def *)reply_msg;
        reply_length                    = sizeof(common_header_def);
        memcpy(common_reply->interface_code, &ipc_comm_hd->interface_code, sizeof(common_reply->interface_code));
        common_reply->interface_code[0] = 'R';
        common_reply->error_code        = DEF_IPC_ERRCD_NG;
        memcpy(common_reply->internal_error_code, DEF_NERR_IPC_SEISA_ERR, sizeof(common_reply->internal_error_code));
        memset(common_reply->filler_1, 0x20, sizeof(common_reply->filler_1));
        common_reply->control_data_length = 0;
        cncl_ems_ipc_invalid_header(no_param, no_param, ipc_comm_hd->interface_code, "interface_code",
                                    DEF_NERR_IPC_SEISA_ERR);
    }
    // 応答送信
    cc = REPLYX((char *)&reply_msg, reply_length, no_param, event->io_info.RINF.z_messagetag, no_param);
    if (_status_ne(cc)) {
        FILE_GETINFO_(event->io_info.fd, &s_err, no_param, no_param, no_param, no_param, no_param);
        // REPLYXはprocedure_error
        cncl_ems_procedure_error("REPLYX", s_err, DEF_NERR_FILE_IO_ERR);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_FILE_IO_ERR);
    }
    return s_err;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ipc_received                                       */
/*  CALLING SEQ.    :short cncl_ipc_received(Event_tag_t *event)             */
/*  ARGUMENT        :event:受信完了イベント情報                              */
/*  RETURN CODE     :0,エラーコード                                          */
/*  DESCRIPTION     :受信完了後のメッセージ処理を行い再度受信を開始          */
/*****************************************************************************/
short cncl_ipc_received(Event_tag_t *event)
{
    io_info_t *io_info = &(event->io_info);
    //    cmp_ipc_thread_t *cmp_ipc = cncl_get_component(event->thread);
    //    thread_header_t *hd =cncl_get_threadInfo(event->thread);
    short      s_node_len;
    short      s_name_len;
    short      s_ret = 0;
    // 何に使っているのかわからないが倣う
    s_ret = PROCESSHANDLE_DECOMPOSE_((short *)&io_info->RINF.z_sender, (short *)&io_info->cpu, no_param, no_param,
                             io_info->node_name, ZSYS_VAL_LEN_SYSTEMNAME, &s_node_len, io_info->proc_name,
                             ZSYS_VAL_LEN_PROCESSNAME, &s_name_len, no_param);
    if(s_ret != ZFIL_ERR_OK){
       s_node_len = s_name_len = 0;
    }
    io_info->node_name[s_node_len] = 0;
    io_info->proc_name[s_name_len] = 0;
    switch (io_info->fs_err) {
        case ZFIL_ERR_OK:
            cncl_ipc_message(event);
            break;
        case ZFIL_ERR_SYSMESS:
            cncl_ipc_sys_message(event);
            break;
        default:
            cncl_ems_response_error(no_param, no_param, "SYS_MSG", DEF_NERR_IPC_SEISA_ERR);
            cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_IPC_SEISA_ERR);
            break;
    }
    return cncl_ipc_start(event->thread);
}
