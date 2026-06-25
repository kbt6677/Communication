/******************************************************************************
 *                                                                             *
 *                               ＜GFP通信制御＞                               *
 *                                                                             *
 *                               ＜コネクション制御(クライアント)＞            *
 *                                                                             *
 *        VERSION                               :＜1.0.0＞                     *
 *                                                                             *
 *        CREATE DATE                           :＜作成日 2025/02/01＞         *
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
/*        WRITTEN-DATE      ････ 2025/02/01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  ISYS 工藤   2025/02/01 新規作成                                     */
/****************************************************************************/
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stddef.h> nolist
#include <stdio.h> nolist
#include <string.h> nolist
#ifdef _TANDEM_SOURCE
#include <cextdecs.h(PROCESS_STOP_)> nolist
#else
#include <cextdecs.h> nolist
#endif
/* USER HEADER     */
#include <common.h> nolist
#include <errcd.h> nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_fileIO.h" nolist
#include "GFPCVX20_app.h" nolist
#include "GFPCVX20_ems_trace.h" nolist
#include "GFPCVX20_io_mem.h" nolist
#include "GFPCVX20_myinfo.h" nolist
#include "GFPCVX20_util.h" nolist

/****************************************************************************/
/*   グローバル変数定義                                                     */
/****************************************************************************/
// トレース用グローバル変数
char        EXMYSRVCLSNAME[15];
char        EXMYPROCNAME[6];
short       EXTRACEMODE;  // 多分ユーザ側で外部参照が必要なのはこれだけ
char        EXTRACEFILENAME[47];
short       EXTRACEFILENO;

// EMS用work
oggz1in_def oggz1in;

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
//EMSリターンコード
#define DEF_EMS_DEF_RET_CODE     "0"
//#define CNCL_SIZE_LEN(type, str) (strlen(str) > sizeof(type) - 1 ? sizeof(type) - 1 : strlen(str))

#define CNCL_SIZE_LEN(type, str) (strnlen_isys((const char *)str,sizeof(type)))

/*****************************************************************************/
/*  FUNCTION        :cncl_set_notify_type                                    */
/*  CALLING SEQ.    :void cncl_set_notify_type(ems_notify_type_t notify_type)*/
/*  ARGUMENT        :notify_type :  e_cncl_ems_notify_normal                 */
/*                                  e_cncl_ems_notify_system                 */
/*                                  e_cncl_ems_notify_error                  */
/*                                  e_cncl_ems_notify_warning                */
/*  RETURN CODE     : なし                                                   */
/*  DESCRIPTION     : EMSメッセージ通知区分を上書きする                      */
/*****************************************************************************/
void cncl_ems_set_notify_type(ems_notify_type_t notify_type)
{
    const static char cncl_ems_notify_type[] = {'S','*','S','E','W'};
    if(notify_type < e_cncl_ems_notify_default || notify_type >= e_cncl_ems_notify_id_max)
    {
        cncl_ems_procedure_error("cncl_set_notify_type", (short)notify_type, DEF_NERR_PRM_RD_ERR_INV);
        cncl_ems_abnormal_end(DEF_PROC_ABNORMAL_END, DEF_NERR_PRM_RD_ERR_INV);
    }
    oggz1in.emsinf.emsgkinf.msgttkb = cncl_ems_notify_type[notify_type];
}
/*****************************************************************************/
/*  FUNCTION        :get_ems_com                                             */
/*  CALLING SEQ.    :const oggz1in_def* get_ems_com(void)                    */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :oggz1in_defへの定数ポインタ                             */
/*  DESCRIPTION     :EMSリンク情報を取得する                                 */
/*****************************************************************************/
const oggz1in_def *get_ems_com() { return &oggz1in; }
/*****************************************************************************/
/*  FUNCTION        :initial_ems                                             */
/*  CALLING SEQ.    :void initial_ems(                                       */
/*                   const char *my_program_id,                              */
/*                   char network,                                           */
/*                   char uytrmmon[13],                                      */
/*                   char uytrmmonlen[2],                                    */
/*                   char uytrmsrv[12],                                      */
/*                   char uytrmsrvlen[2],                                    */
/*                   char uytrmtimer[4])                                     */
/*  ARGUMENT        :my_program_id:プログラムID                              */
/*                  :network      :NW ID                                     */
/*                  :uytrmmon     :モニタ名                                  */
/*                  :uytrmmonlen  :モニタ名長                                */
/*                  :uytrmsrv     :サーバ名                                  */
/*                  :uytrmsrvlen  :サーバ名長                                */
/*                  :uytrmtimer   :タイマー                                  */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :EMS出力用の各種情報を初期化する                         */
/*****************************************************************************/
void initial_ems(const char *my_program_id, char network, char uytrmmon[13], char uytrmmonlen[2], char uytrmsrv[12],
                 char uytrmsrvlen[2], char uytrmtimer[4])
{
    // EMSリンケージ：NW識別変換
    struct __cnv_nw
    {
        char  nw_id;
        char *nw_div;
    } cnv_nw[] = {
        {DEF_NW_ID_JCN,      DEF_NW_KUBUN_CARDNET }, // JCN(CUP)
        {DEF_NW_ID_VISA,     DEF_NW_KUBUN_VISANET }, // Visanet
        {DEF_NW_ID_MASTER,   DEF_NW_KUBUN_BANKNET }, // Banknet
        {DEF_NW_ID_AMEX,     DEF_NW_KUBUN_AEGN    }, // AEGEN
        {DEF_NW_ID_DISCOVER, DEF_NW_KUBUN_DISCOVER}, // Discover
        {DEF_NW_ID_NYCE,     DEF_NW_KUBUN_NYCE    }, // NYCE
        {DEF_NW_ID_JLink,    DEF_NW_KUBUN_JLINK   }, // J-Link(通信受信時)
        {DEF_NW_ID_UnionPay, DEF_NW_KUBUN_UNIONPAY}  // UnionPay
    };
    myinfo_def *myInfo = &(cncl_get_App()->my_info);

    // EMS出力モジュールI/F用変数初期化
    memset(&oggz1in, ' ', sizeof(oggz1in));
    // リターンコード
    memcpy(&oggz1in.emsinf.rcd, DEF_EMS_DEF_RET_CODE, sizeof(oggz1in.emsinf.rcd));
    // // メッセージ通知区分
    // oggz1in.emsinf.emsgkinf.msgttkb = 'S';
    // システム名(GFP)
    memcpy(oggz1in.emsinf.emsgkinf.sysnm, DEF_EMS_SYSNM_GFP, strlen(DEF_EMS_SYSNM_GFP));
    //
    memcpy(oggz1in.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM, strlen(DEF_EMS_SRV_KBN_COM));
    // メッセージ出力元プログラム名
    memcpy(oggz1in.emsinf.emsgkinf.prgid, my_program_id, sizeof(oggz1in.emsinf.emsgkinf.prgid));
    // メッセージ出力元プログラム名
    memcpy(oggz1in.emsinf.emsgkinf.trmnm, myInfo->my_name, myInfo->my_name_len);


    memcpy(oggz1in.uytrminf.uytrmmon, uytrmmon, sizeof(oggz1in.uytrminf.uytrmmon));
    memcpy(oggz1in.uytrminf.uytrmmonlen, uytrmmonlen, sizeof(oggz1in.uytrminf.uytrmmonlen));
    memcpy(oggz1in.uytrminf.uytrmsrv, uytrmsrv, sizeof(oggz1in.uytrminf.uytrmsrv));
    memcpy(oggz1in.uytrminf.uytrmsrvlen, uytrmsrvlen, sizeof(oggz1in.uytrminf.uytrmsrvlen));

    memcpy(oggz1in.uytrminf.proctimer, uytrmtimer, sizeof(oggz1in.uytrminf.proctimer));

    // NW識別を変換してEMSリンケージへ設定
    for (size_t i = 0; i < sizeof(cnv_nw) / sizeof(struct __cnv_nw); ++i) {
        if (cnv_nw[i].nw_id == network) {
            memcpy(oggz1in.emsinf.emsgkinf.h_nw_kbn, cnv_nw[i].nw_div, strlen(cnv_nw[i].nw_div));
            break;
        }
    }
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_setmsgid                                       */
/*  CALLING SEQ.    :void cncl_ems_setmsgid(short msgid)                     */
/*  ARGUMENT        :msgid:EMSに設定するメッセージID                         */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :EMSメッセージ出力時のIDや区分を設定                     */
/*****************************************************************************/
void cncl_ems_setmsgid(short msgid)
{
    int  i;
    int  item_size;
    int  item_cnt;
    char buffer[64];

    // GFP通信制御メッセージ番号
    snprintf(buffer, sizeof(buffer), "%05d", msgid);
    memcpy(&oggz1in.emsinf.msgid, buffer, strlen(buffer));
    // メッセージ通知区分
    // プロセス起動、プロセス正常終了
    if (msgid == DEF_EVT_PROC_START || msgid == DEF_EVT_PROC_NORMAL_END) {
        oggz1in.emsinf.emsgkinf.msgttkb = '*';
    } else {
        oggz1in.emsinf.emsgkinf.msgttkb = 'S';
    }
    // ②サーバークラス論理ID (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[0].msgtbl_vl, DEF_SC_CON_CLT, sizeof(DEF_SC_CON_CLT) - 1);

    item_size = sizeof(oggz1in.emsinf.emsnninf.msgtbl[0]);
    item_cnt  = sizeof(oggz1in.emsinf.emsnninf.msgtbl) / item_size;
    for (i = 1; i < item_cnt; i++) {
        memset(&oggz1in.emsinf.emsnninf.msgtbl[i], ' ', item_size);
    }
    memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, DEF_NERR_NOMAL, sizeof(DEF_NERR_NOMAL) - 1);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_abnormal_end                                   */
/*  CALLING SEQ.    :void cncl_ems_abnormal_end(short status,                */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :status       :停止コード                                */
/*                  :inter_errcd  :内部エラーコード                          */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :異常終了時にEMS出力し、PROCESS_STOP_を実行              */
/*****************************************************************************/
void cncl_ems_abnormal_end(short status, ems_inter_errcd_t inter_errcd)
{
    Application *app    = cncl_get_App();
    myinfo_def  *myInfo = &(app->my_info);
    /*------------------------------------------------*/
    /*    メッセージ出力：プロセス異常終了            */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_PROC_ABNORMAL_END);
    // ③プロセス名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, myInfo->my_name, myInfo->my_name_len);
    memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, sizeof(ems_inter_errcd_t) - 1);
    GFPOGGZ1(&oggz1in);

    // プロセス終了
    PROCESS_STOP_(no_param, no_param, status, no_param, no_param, no_param, no_param, no_param);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_param_err                                      */
/*  CALLING SEQ.    :void cncl_ems_param_err(msgtbl_vl_t param_name,         */
/*                   short ercd, ems_inter_errcd_t inter_errcd)              */
/*  ARGUMENT        :param_name:パラメータ名                                 */
/*                  :ercd      :エラー番号                                   */
/*                  :inter_errcd:内部エラーコード                            */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :パラメータ取得エラーをEMS出力する                       */
/*****************************************************************************/
void cncl_ems_param_err(msgtbl_vl_t param_name, short ercd, ems_inter_errcd_t inter_errcd)
{
    msgtbl_vl_t work_buff;

    cncl_ems_setmsgid(DEF_EVT_PARAM_GET_ERR);
    // ③パラメータ名(20バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, param_name, strlen(param_name));
    snprintf(work_buff, sizeof(msgtbl_vl_t), "%05d", ercd);
    // ④エラー番号
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, work_buff, strlen(work_buff));
    memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, sizeof(ems_inter_errcd_t) - 1);
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_config_error                                   */
/*  CALLING SEQ.    :void cncl_ems_config_error(void *pri_key,               */
/*                   size_t key_len, ems_inter_errcd_t inter_errcd)          */
/*  ARGUMENT        :pri_key:対象キー                                        */
/*                  :key_len:キーの長さ                                      */
/*                  :inter_errcd:内部エラーコード                            */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ファイルに対象レコードが存在しない等の設定エラー通知    */
/*****************************************************************************/
void cncl_ems_config_error(void *pri_key, size_t key_len, filename_l_t l_f_name,ems_inter_errcd_t inter_errcd)
{
    /*------------------------------------------------*/
    /*    メッセージ編集：設定情報エラー              */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_CONFIG_ERR);
    // ③ファイル論理名 (8バイト)
    //memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO) - 1);
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, l_f_name, strnlen_isys(l_f_name,sizeof(filename_l_t)-1));
    // ④キー (40バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, pri_key, key_len);
    // ⑤エラー項目/エラー理由 (40バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, "target record not found",
           sizeof("target record not found") - 1);
    memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, sizeof(ems_inter_errcd_t) - 1);
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_common_module_error                            */
/*  CALLING SEQ.    :void cncl_ems_common_module_error(                      */
/*                   const sub_prog_sts_t sub_prog_sts,                      */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :sub_prog_sts:モジュール内ステータス                     */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :共通モジュールエラーをEMS出力する                       */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_common_module_error(const ems_mod_id mod_id, const short s_err, const sub_prog_sts_t sub_prog_sts,
                                 const ems_inter_errcd_t inter_errcd)
{
    /*------------------------------------------------*/
    /*    メッセージ編集：共通モジュールエラー        */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_COMMON_MOD_ERR);
    // ③モジュールID(8バイト)
    if (_arg_present(mod_id)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, mod_id, CNCL_SIZE_LEN(ems_mod_id, mod_id));
    } else {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, DEF_GFPCVX20, sizeof(DEF_GFPCVX20) - 1);
    }
    // ④エラーコード(4バイト)
    if (_arg_present(sub_prog_sts)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, sub_prog_sts, sizeof(sub_prog_sts_t) - 1);
    } else if (_arg_present(s_err)) {
        char buffer[6];
        snprintf(buffer, sizeof(buffer), "%05d", (int)s_err);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(inter_errcd)) {
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, sizeof(ems_inter_errcd_t) - 1);
    }
    GFPOGGZ1(&oggz1in);
    return;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_procedure_error                                */
/*  CALLING SEQ.    :void cncl_ems_procedure_error(const char *procedure_name*/
/*                   short errcd,                                            */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :procedure_name:エラーが起きた手続名                     */
/*                  :errcd         :エラーコード                             */
/*                  :inter_errcd   :内部エラーコード                         */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :特定プロシージャで起きたエラーをEMS出力                 */
/*****************************************************************************/
void cncl_ems_procedure_error(const char *procedure_name, short errcd, ems_inter_errcd_t inter_errcd)
{
    char buffer[5];
    /*------------------------------------------------*/
    /*    メッセージ編集：共通モジュールエラー        */
    /*------------------------------------------------*/
    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_PROCEDURE_ERR);
    // ③プロシージャ名(40バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, procedure_name, strlen(procedure_name));
    // ④エラーコード(4バイト)
    snprintf(buffer, sizeof(buffer), "%04d", errcd);
    cobolization(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, buffer,
                 sizeof(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl));
    memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, sizeof(ems_inter_errcd_t) - 1);
    GFPOGGZ1(&oggz1in);
    return;
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_process_start                                  */
/*  CALLING SEQ.    :void cncl_ems_process_start(void)                       */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :プロセス起動時にEMSへ起動メッセージを出力               */
/*****************************************************************************/
void cncl_ems_process_start()
{
    Application *app     = cncl_get_App();
    myinfo_def  *my_Info = &(app->my_info);

    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_PROC_START);
    // ③プロセス名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, my_Info->my_name, my_Info->my_name_len);
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_normal_end                                     */
/*  CALLING SEQ.    :void cncl_ems_normal_end(void)                          */
/*  ARGUMENT        :なし                                                    */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :正常終了時にEMSへ終了メッセージを出力                   */
/*****************************************************************************/
void cncl_ems_normal_end()
{
    Application *app     = cncl_get_App();
    myinfo_def  *my_Info = &(app->my_info);
    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_PROC_NORMAL_END);
    // ③プロセス名 (8バイト)
    memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, my_Info->my_name, my_Info->my_name_len);
    GFPOGGZ1(&oggz1in);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_ems_response_error                                 */
/*  CALLING SEQ.    :void cncl_ems_response_error(                           */
/*                   ems_lcn_t lcn,                                          */
/*                   ems_srvcls_name_t srvcls_from,                          */
/*                   ems_ipc_id_t ipc_id,                                    */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :srvcls_from :送信元サーバクラス名                       */
/*                  :ipc_id      :IPCインタフェースID                        */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :応答エラー時の情報をEMSへ通知する                       */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_response_error(ems_lcn_t lcn, ems_srvcls_name_t srvcls_from, ems_ipc_id_t ipc_id,
                            ems_inter_errcd_t inter_errcd)
{
    // 任意メッセージ部初期化
    cncl_ems_setmsgid(DEF_EVT_RSP_ERR);

    if (_arg_present(lcn)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    }
    if (_arg_present(srvcls_from)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, srvcls_from, CNCL_SIZE_LEN(ems_srvcls_name_t, srvcls_from));
    }
    if (_arg_present(ipc_id)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, ipc_id, sizeof(ems_ipc_id_t));
    }
    if (_arg_present(inter_errcd)) {
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    }
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_ipc_invalid_header                             */
/*  CALLING SEQ.    :void cncl_ems_ipc_invalid_header(                       */
/*                   ems_lcn_t lcn, gflin_pkey_def *recv_con_jd,             */
/*                   ems_ipc_id_t ipc_id, ems_ipc_err_col_t ipc_err_col,     */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :recv_con_jd :回線キー情報                               */
/*                  :ipc_id      :IPCインタフェースID                        */
/*                  :ipc_err_col :不正項目                                   */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :IPC電文のヘッダ不正をEMSへ通知                          */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_ipc_invalid_header(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_ipc_id_t ipc_id,
                                ems_ipc_err_col_t ipc_err_col, ems_inter_errcd_t inter_errcd)
{
    cncl_ems_setmsgid(DEF_EVT_HEADR_SEISA_ERR);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(recv_con_jd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, recv_con_jd, sizeof(gflin_pkey_def));
    if (_arg_present(ipc_id)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, ipc_id, sizeof(ems_ipc_id_t));
    if (_arg_present(ipc_err_col))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, ipc_err_col, CNCL_SIZE_LEN(ems_ipc_err_col_t, ipc_err_col));
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_ipc_invalid_response(廃止:使用していない)      */
/*  CALLING SEQ.    :void cncl_ems_ipc_invalid_response(                     */
/*                   ems_lcn_t lcn, gflin_pkey_def *recv_con_jd,             */
/*                   ems_ipc_id_t ipc_id, ems_err_reason_t err_reason,       */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :recv_con_jd :回線キー情報                               */
/*                  :ipc_id      :IPCインタフェースID                        */
/*                  :err_reason  :エラー要因                                 */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :IPC電文応答の不正をEMSへ通知                            */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_ipc_invalid_response(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_ipc_id_t ipc_id,
                                  ems_err_reason_t err_reason, ems_inter_errcd_t inter_errcd)
{
    cncl_ems_setmsgid(DEF_EVT_RSP_SEISA_ERR);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(recv_con_jd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, recv_con_jd, sizeof(gflin_pkey_def));
    if (_arg_present(ipc_id)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, ipc_id, sizeof(ems_ipc_id_t));
    if (_arg_present(err_reason))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, err_reason, CNCL_SIZE_LEN(ems_err_reason_t, err_reason));
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_pathsend_outstanding_exceeded                  */
/*  CALLING SEQ.    :void cncl_ems_pathsend_outstanding_exceeded(            */
/*                   gflin_pkey_def *gflin_pkey,                             */
/*                   tcpip_prc_name_t tcpip_prc_name,                        */
/*                   struct in_addr *remote_address,                         */
/*                   unsigned short remote_port,                             */
/*                   struct in_addr *local_address,                          */
/*                   unsigned short local_port,                              */
/*                   short max_outstanding,                                  */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :gflin_pkey     : 回線ID                                 */
/*                   tcpip_prc_name : TCP/IPプロセス名                       */
/*                   remote_address : 相手先IPアドレス                       */
/*                   remote_port    : 相手先ポート番号                       */
/*                   local_address  : 自IPアドレス                           */
/*                   local_port     : 自ポート番号                           */
/*                   max_outstanding: 許容Outstanding上限                    */
/*                   inter_errcd    : 内部エラーコード                       */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :PATHSEND Outstanding超過をEMSへ通知する。               */
/*                   ・メッセージID(DEF_EVT_PSEND_QUE_NON)を設定             */
/*                   ・指定がある項目のみメッセージ領域に格納                */
/*                   ・IPアドレスは文字列化(inet_ntoa)して格納               */
/*                   ・数値項目(ポート/上限)は5桁ゼロ詰めで文字列化          */
/*                   ・GFPOGGZ1を呼び出し出力                                */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_pathsend_outstanding_exceeded(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port, short max_outstanding,
                      ems_inter_errcd_t inter_errcd)
{
    char           buffer[6];
    c_ip_address_t remote, local;
    memset(remote, 0, sizeof(c_ip_address_t));
    memset(local, 0, sizeof(c_ip_address_t));

    cncl_ems_setmsgid(DEF_EVT_PSEND_QUE_NON);
    if (_arg_present(gflin_pkey)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, gflin_pkey, sizeof(gflin_pkey_def));
    }
    if (_arg_present(tcpip_prc_name)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, tcpip_prc_name,
               CNCL_SIZE_LEN(tcpip_prc_name_t, tcpip_prc_name));
    }
    if (_arg_present(remote_address)) {
        strncpy(remote, inet_ntoa(*remote_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, remote, CNCL_SIZE_LEN(c_ip_address_t, remote));
    }
    if (_arg_present(remote_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", remote_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(local_address)) {
        strncpy(local, inet_ntoa(*local_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[5].msgtbl_vl, local, CNCL_SIZE_LEN(c_ip_address_t, local));
    }
    if (_arg_present(local_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", local_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(max_outstanding)) {
        snprintf(buffer, sizeof(buffer), "%05d", max_outstanding);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[7].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(inter_errcd)) {
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    }
    GFPOGGZ1(&oggz1in);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_ems_ipc_invalid_request                            */
/*  CALLING SEQ.    :void cncl_ems_ipc_invalid_request(                      */
/*                   ems_err_reason_detail_t err_reason,                     */
/*                   ems_err_ipc_h24 err_ipc,                                */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :err_reason: エラー理由詳細コード                        */
/*                   err_ipc   : IPCエラー内容(H24形式)                      */
/*                   inter_errcd: 内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :IPC無効要求エラーをEMSに出力する。                      */
/*                   ・エラーメッセージID(DEF_EVT_REQ_ERR)を設定             */
/*                   ・err_reasonをメッセージ領域に格納                      */
/*                   ・err_ipcは16進文字列へ変換して格納                     */
/*                   ・inter_errcdが指定されていればEMSメッセージへ格納      */
/*                   ・GFPOGGZ1を呼び出しEMSへ出力                           */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_ipc_invalid_request(ems_err_reason_detail_t err_reason, ems_err_ipc_h24 err_ipc,
                                 ems_inter_errcd_t inter_errcd)
{
    char work_buffer[sizeof(ems_err_ipc_h24) * 2 + 1];
    cncl_ems_setmsgid(DEF_EVT_REQ_ERR);
    if (_arg_present(err_reason))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, err_reason,
               CNCL_SIZE_LEN(ems_err_reason_detail_t, err_reason));
    if (_arg_present(err_ipc)) {
        binhex_encoder(err_ipc, sizeof(ems_err_ipc_h24), work_buffer, sizeof(work_buffer));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, work_buffer, strlen(work_buffer));
    }
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}

/*****************************************************************************/
/*  FUNCTION        :cncl_ems_fileio_error                                   */
/*  CALLING SEQ.    :void cncl_ems_fileio_error(ems_lcn_t lcn,               */
/*                   gflin_pkey_def *recv_con_jd, filename_l_t filename,     */
/*                   ems_fio_ope_t ope, ems_fio_key_t key_val,               */
/*                   size_t key_val_len, short errcd,                        */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :recv_con_jd :回線キー情報                               */
/*                  :filename    :ファイル名                                 */
/*                  :ope         :操作内容                                   */
/*                  :key_val     :キー情報                                   */
/*                  :key_val_len :キーのバイト数                             */
/*                  :errcd       :ガーディアンエラーコード                   */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ファイル操作時のエラーをEMS出力                         */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_fileio_error(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, filename_l_t filename, ems_fio_ope_t ope,
                          ems_fio_key_t key_val, size_t key_val_len, short errcd, ems_inter_errcd_t inter_errcd)
{
    char str_errcd[6];
    cncl_ems_setmsgid(DEF_EVT_FILE_IO_ERR);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(recv_con_jd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, recv_con_jd, sizeof(gflin_pkey_def));
    if (_arg_present(filename))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, filename, CNCL_SIZE_LEN(filename_l_t, filename));
    if (_arg_present(ope))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, filename, CNCL_SIZE_LEN(ems_fio_ope_t, ope));
    if (_arg_present(key_val) && _arg_present(key_val_len))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[5].msgtbl_vl, key_val, key_val_len);
    if (_arg_present(errcd)) {
        snprintf(str_errcd, sizeof(str_errcd), "%05d", errcd);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, str_errcd, sizeof(str_errcd) - 1);
    }
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_nw_error                                       */
/*  CALLING SEQ.    :void cncl_ems_nw_error(gflin_pkey_def *gflin_pkey,      */
/*                   tcpip_prc_name_t tcpip_prc_name,                        */
/*                   struct in_addr *remote_address,                         */
/*                   unsigned short remote_port,                             */
/*                   struct in_addr *local_address,                          */
/*                   unsigned short local_port,                              */
/*                   short s_err,                                            */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :gflin_pkey     : 回線ID                                 */
/*                  :tcpip_prc_name:TCP/IPプロセス名                         */
/*                  :remote_address:相手先アドレス                           */
/*                  :remote_port   :相手先ポート番号                         */
/*                  :local_address :自プロセスアドレス                       */
/*                  :local_port    :自プロセスポート番号                     */
/*                  :s_err         :ガーディアンエラーコード                 */
/*                  :inter_errcd   :内部エラーコード                         */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ソケット通信のエラーをEMSへ通知                         */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_nw_error(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port, short s_err,
                      ems_inter_errcd_t inter_errcd)
{
    char           buffer[6];
    c_ip_address_t remote, local;
    memset(remote, 0, sizeof(c_ip_address_t));
    memset(local, 0, sizeof(c_ip_address_t));

    cncl_ems_setmsgid(DEF_EVT_CONN_SYOGAI);
    if (_arg_present(gflin_pkey)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, gflin_pkey, sizeof(gflin_pkey_def));
    }
    if (_arg_present(tcpip_prc_name)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, tcpip_prc_name,
               CNCL_SIZE_LEN(tcpip_prc_name_t, tcpip_prc_name));
    }
    if (_arg_present(remote_address)) {
        strncpy(remote, inet_ntoa(*remote_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, remote, CNCL_SIZE_LEN(c_ip_address_t, remote));
    }
    if (_arg_present(remote_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", remote_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(local_address)) {
        strncpy(local, inet_ntoa(*local_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[5].msgtbl_vl, local, CNCL_SIZE_LEN(c_ip_address_t, local));
    }
    if (_arg_present(local_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", local_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(s_err)) {
        snprintf(buffer, sizeof(buffer), "%05d", s_err);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[7].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(inter_errcd)) {
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    }
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_procio_error                                   */
/*  CALLING SEQ.    :void cncl_ems_procio_error(ems_lcn_t lcn,               */
/*                   gflin_pkey_def *recv_con_jd, filename_l_t filename,     */
/*                   ems_fio_ope_t ope, short errcd,                         */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :recv_con_jd :回線キー情報                               */
/*                  :filename    :ファイル名                                 */
/*                  :ope         :操作内容                                   */
/*                  :errcd       :ガーディアンエラーコード                   */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :プロセス通信時のエラーをEMSへ通知                       */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_procio_error(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, filename_l_t filename, ems_fio_ope_t ope,
                          short errcd, ems_inter_errcd_t inter_errcd)
{
    char str_errcd[6];
    cncl_ems_setmsgid(DEF_EVT_PROC_IO_ERR);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(recv_con_jd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, recv_con_jd, sizeof(gflin_pkey_def));
    if (_arg_present(filename))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, filename, CNCL_SIZE_LEN(filename_l_t, filename));
    if (_arg_present(ope))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, filename, CNCL_SIZE_LEN(ems_fio_ope_t, ope));
    if (_arg_present(errcd)) {
        snprintf(str_errcd, sizeof(str_errcd), "%05d", errcd);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[5].msgtbl_vl, str_errcd, sizeof(str_errcd) - 1);
    }
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_cmd_rcv                                        */
/*  CALLING SEQ.    :void cncl_ems_cmd_rcv   (ems_lcn_t lcn,                 */
/*                        gflin_pkey_def *recv_con_jd,                       */
/*                        ems_cmd cmd, ems_cmd param,                        */
/*                        ems_inter_errcd_t inter_errcd);                    */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :recv_con_jd :回線キー情報                               */
/*                  :filename    :ファイル名                                 */
/*                  :cmd         :コマンドコード                             */
/*                  :param       :パラメータ                                 */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :コマンドIPCの処理結果                                   */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_cmd_rcv(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_cmd cmd, ems_cmd param,
                        ems_inter_errcd_t inter_errcd)
{
    cncl_ems_setmsgid(DEF_EVT_CMD_RCV);
    cncl_ems_set_notify_type(e_cncl_ems_notify_normal);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(recv_con_jd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, recv_con_jd, sizeof(gflin_pkey_def));
    if (_arg_present(cmd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, cmd, sizeof(ems_cmd));
    if (_arg_present(param))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, param, sizeof(ems_cmd));
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_cmd_result                                     */
/*  CALLING SEQ.    :void cncl_ems_cmd_result(ems_lcn_t lcn,                 */
/*                        gflin_pkey_def *recv_con_jd,                       */
/*                        ems_cmd cmd, ems_rslt rslt,                        */
/*                        ems_inter_errcd_t inter_errcd);                    */
/*  ARGUMENT        :lcn         :LCN情報                                    */
/*                  :recv_con_jd :回線キー情報                               */
/*                  :filename    :ファイル名                                 */
/*                  :cmd         :コマンドコード                             */
/*                  :rslt        :処理結果                                   */
/*                  :inter_errcd :内部エラーコード                           */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :コマンドIPCの処理結果                                   */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_cmd_result(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_cmd cmd, ems_rslt rslt,
                     ems_inter_errcd_t inter_errcd)
{
    cncl_ems_setmsgid(DEF_EVT_CMD);
    cncl_ems_set_notify_type(e_cncl_ems_notify_normal);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(recv_con_jd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, recv_con_jd, CNCL_SIZE_LEN(gflin_pkey_def,recv_con_jd));
    if (_arg_present(cmd))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, cmd, CNCL_SIZE_LEN(ems_cmd,cmd));
    if (_arg_present(rslt))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, rslt, CNCL_SIZE_LEN(ems_rslt,rslt));
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_ipc_length_error                               */
/*  CALLING SEQ.    :void cncl_ems_ipc_length_error(gflin_pkey_def *gflin_pkey,*/
/*                   tcpip_prc_name_t tcpip_prc_name,                        */
/*                   struct in_addr *remote_address,                         */
/*                   unsigned short remote_port,                             */
/*                   struct in_addr *local_address,                          */
/*                   unsigned short local_port,                              */
/*                   size_t ipc_total_length,                                */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :gflin_pkey     : 回線ID                                 */
/*                  :tcpip_prc_name    :TCP/IPプロセス名                     */
/*                  :remote_address    :相手先アドレス                       */
/*                  :remote_port       :相手先ポート番号                     */
/*                  :local_address     :自プロセスアドレス                   */
/*                  :local_port        :自プロセスポート番号                 */
/*                  :ipc_total_length  :認識したメッセージ長                 */
/*                  :inter_errcd       :内部エラーコード                     */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ブランドNWから受信した電文長がC201のテキストエリア長を  */
/*                   超過しているまたは電文長がヘッダ長より短い(電文長不正)  */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_ipc_length_error(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port, size_t ipc_total_length,
                      ems_inter_errcd_t inter_errcd)
{
    char           buffer[6];
    c_ip_address_t remote, local;
    memset(remote, 0, sizeof(c_ip_address_t));
    memset(local, 0, sizeof(c_ip_address_t));

    cncl_ems_setmsgid(DEF_EVT_MSG_LEN_ERR);

    if (_arg_present(gflin_pkey)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, gflin_pkey, sizeof(gflin_pkey_def));
    }
    if (_arg_present(tcpip_prc_name)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, tcpip_prc_name,
               CNCL_SIZE_LEN(tcpip_prc_name_t, tcpip_prc_name));
    }
    if (_arg_present(remote_address)) {
        strncpy(remote, inet_ntoa(*remote_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, remote, CNCL_SIZE_LEN(c_ip_address_t, remote));
    }
    if (_arg_present(remote_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", remote_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(local_address)) {
        strncpy(local, inet_ntoa(*local_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[5].msgtbl_vl, local, CNCL_SIZE_LEN(c_ip_address_t, local));
    }
    if (_arg_present(local_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", local_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(ipc_total_length)) {
        snprintf(buffer, sizeof(buffer), "%05u", (unsigned int)ipc_total_length);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[7].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(inter_errcd)) {
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    }
    GFPOGGZ1(&oggz1in);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_p6d_error                                      */
/*  CALLING SEQ.    :void cncl_ems_p6d_error(                                */
/*                   ems_lcn_t lcn,                                          */
/*                   ems_pathmon_name_t pathmon_name,                        */
/*                   ems_srvcls_name_t s9s_name,                             */
/*                   short errcd,                                            */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :lcn          : 論理回線識別子                           */
/*                   pathmon_name : Pathmon名                                */
/*                   s9s_name     : サーバクラス名                           */
/*                   errcd        : 外部/FSエラーコード（数値）              */
/*                   inter_errcd  : 内部エラーコード（文字列）               */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :PATHSEND系エラーのEMS出力を行う。                       */
/*                   ・メッセージID(DEF_EVT_PSEND_ERR_DETECT)を設定          */
/*                   ・引数が_presentの項目のみEMSメッセージ領域へ格納       */
/*                   ・errcdは5桁ゼロ詰め文字列に整形して格納                */
/*                   ・GFPOGGZ1を呼び出しEMSへ出力                           */
/*****************************************************************************/

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_p6d_error(ems_lcn_t lcn, ems_pathmon_name_t pathmon_name, ems_srvcls_name_t s9s_name, short errcd,
                       ems_inter_errcd_t inter_errcd)
{
    char work[6];
    cncl_ems_setmsgid(DEF_EVT_PSEND_ERR_DETECT);
    if (_arg_present(lcn)) memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, lcn, sizeof(ems_lcn_t));
    if (_arg_present(pathmon_name))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, pathmon_name,
               CNCL_SIZE_LEN(ems_pathmon_name_t, pathmon_name));
    if (_arg_present(s9s_name))
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, s9s_name, CNCL_SIZE_LEN(ems_srvcls_name_t, s9s_name));
    if (_arg_present(errcd)) {
        memset(work, 0, sizeof(work));
        snprintf(work, sizeof(work), "%05d", errcd);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, work, CNCL_SIZE_LEN(work, work));
    }
    if (_arg_present(inter_errcd))
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    GFPOGGZ1(&oggz1in);
}

/*****************************************************************************/
/*  FUNCTION        :trace_start                                             */
/*  CALLING SEQ.    :void trace_start(                                       */
/*                   io_trace_buf_t *trace,                                  */
/*                   const char *fileId,                                     */
/*                   const char *fileName,                                   */
/*                   const char *fileIOtype,                                 */
/*                   const size_t recLen)                                    */
/*  ARGUMENT        :trace    : トレースバッファ                             */
/*                   fileId   : ファイル識別子                               */
/*                   fileName : ファイル名                                   */
/*                   fileIOtype:入出力種別(READ/WRITE/OPEN等)                */
/*                   recLen   : 対象レコード長                               */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :トレース開始時にトレースバッファを初期化し、処理情報を  */
/*                   記録する。                                              */
/*                   ・バッファ先頭をスペースで初期化                        */
/*                   ・日時を取得し処理開始時刻を設定                        */
/*                   ・プログラムIDやファイル情報(fileId/fileName)を格納     */
/*                   ・I/O種別(fileIOtype)を設定                             */
/*                   ・Guardianエラーコード領域を初期化                      */
/*                   ・レコード長を文字列化して格納                          */
/*****************************************************************************/
void trace_start(io_trace_buf_t *trace, const char *fileId, const char *fileName, const char *fileIOtype,
                 const size_t recLen)
{
    COM_SDT_arg_2_def datetime_c;
    COM_SDT_arg_3_def datetime_b;
    long long         datetime_l;
    char              trace_work[6];
    COM_SDT(2 /* JPN */, &datetime_c, &datetime_b, &datetime_l);
    memset(&trace->trace_info, ' ', sizeof(trace->trace_info));
    memcpy((char *)trace->trace_info.shori_start_time, datetime_c.hh, sizeof(trace->trace_info.shori_start_time));
    trace->func_flg = DEF_TRACE_FUNC_OUT;
    memcpy(trace->trace_info.prog_id, DEF_GFPCVX20, sizeof(trace->trace_info.prog_id));
    // memcpy(send_buff.trace_info.shori_end_time,datetime_c.hh,sizeof(send_buff.trace_info.shori_end_time));
    memcpy(trace->trace_info.file_id, fileId, strnlen_isys(fileId, sizeof(trace->trace_info.file_id)));
    memcpy(trace->trace_info.file_name, fileName, strnlen_isys(fileName, sizeof(trace->trace_info.file_name)));
    memcpy(trace->trace_info.file_io_type, fileIOtype,
           strnlen_isys(fileIOtype, sizeof(trace->trace_info.file_io_type)));
    snprintf(trace_work, sizeof(trace_work), "%04d", 0);
    memcpy(trace->trace_info.guardian_errcode, trace_work, sizeof(trace->trace_info.guardian_errcode));
    snprintf(trace_work, sizeof(trace_work), "%05u", (unsigned int)recLen);
    memcpy(trace->data_info.rec_len, trace_work, sizeof(trace->data_info.rec_len));
}
/*****************************************************************************/
/*  FUNCTION        :trace_end                                               */
/*  CALLING SEQ.    :void trace_end(                                         */
/*                   io_trace_buf_t *trace,                                  */
/*                   const char *fileIOtype,                                 */
/*                   const size_t readlen,                                   */
/*                   const int i_err)                                        */
/*  ARGUMENT        :trace     : トレースバッファ                            */
/*                   fileIOtype: 入出力種別(READ/WRITE/OPEN等)               */
/*                   readlen   : 読込／書込バイト数                          */
/*                   i_err     : エラーコード                                */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :I/O処理完了時にトレースバッファへ処理終了情報を記録する。*/
/*                   ・日時を取得し処理終了時刻を設定                        */
/*                   ・プログラムIDを設定                                    */
/*                   ・I/O種別を記録                                         */
/*                   ・Guardianエラーコードを文字列化し格納                  */
/*                   ・実際の入出力長を文字列化し格納                        */
/*                   ・TRACEOUTマクロを呼び出しトレースを出力                */
/*****************************************************************************/
void trace_end(io_trace_buf_t *trace, const char *fileIOtype, const size_t readlen, const int i_err)
{
    COM_SDT_arg_2_def datetime_c;
    COM_SDT_arg_3_def datetime_b;
    long long         datetime_l;
    char              trace_work[6];
    // 書込後データのトレース
    COM_SDT(2 /* JPN */, &datetime_c, &datetime_b, &datetime_l);
    trace->func_flg = DEF_TRACE_FUNC_OUT;
    memcpy(trace->trace_info.prog_id, DEF_GFPCVX20, sizeof(trace->trace_info.prog_id));
    memcpy(trace->trace_info.shori_end_time, datetime_c.hh, sizeof(trace->trace_info.shori_end_time));
    memcpy(trace->trace_info.file_io_type, fileIOtype, sizeof(DEF_TRACE_IO_TYPE_RD) - 1);
    snprintf(trace_work, sizeof(trace_work), "%04d", i_err);
    memcpy(trace->trace_info.guardian_errcode, trace_work, sizeof(trace->trace_info.guardian_errcode));
    snprintf(trace_work, sizeof(trace_work), "%05u", (unsigned int)readlen);
    memcpy(trace->data_info.rec_len, trace_work, sizeof(trace->data_info.rec_len));
    TRACEOUT((char *)trace);
}
/*****************************************************************************/
/*  FUNCTION        :cncl_ems_nw_error                                       */
/*  CALLING SEQ.    :void cncl_ems_nw_connect_normal(gflin_pkey_def *gflin_pkey,*/
/*                   tcpip_prc_name_t tcpip_prc_name,                        */
/*                   struct in_addr *remote_address,                         */
/*                   unsigned short remote_port,                             */
/*                   struct in_addr *local_address,                          */
/*                   unsigned short local_port,                              */
/*                   ems_inter_errcd_t inter_errcd)                          */
/*  ARGUMENT        :gflin_pkey     : 回線ID                                 */
/*                  :tcpip_prc_name:TCP/IPプロセス名                         */
/*                  :remote_address:相手先アドレス                           */
/*                  :remote_port   :相手先ポート番号                         */
/*                  :local_address :自プロセスアドレス                       */
/*                  :local_port    :自プロセスポート番号                     */
/*                  :s_err         :ガーディアンエラーコード                 */
/*                  :inter_errcd   :内部エラーコード                         */
/*  RETURN CODE     :なし                                                    */
/*  DESCRIPTION     :ソケット通信のエラーをEMSへ通知                         */
/*****************************************************************************/
#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_nw_connect_normal(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port,
                      ems_inter_errcd_t inter_errcd)
{
    char           buffer[6];
    c_ip_address_t remote, local;
    memset(remote, 0, sizeof(c_ip_address_t));
    memset(local, 0, sizeof(c_ip_address_t));

    cncl_ems_setmsgid(DEF_EVT_CONN_CONNECTED);
    cncl_ems_set_notify_type(e_cncl_ems_notify_normal);
    if (_arg_present(gflin_pkey)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[1].msgtbl_vl, gflin_pkey, sizeof(gflin_pkey_def));
    }
    if (_arg_present(tcpip_prc_name)) {
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[2].msgtbl_vl, tcpip_prc_name,
               CNCL_SIZE_LEN(tcpip_prc_name_t, tcpip_prc_name));
    }
    if (_arg_present(remote_address)) {
        strncpy(remote, inet_ntoa(*remote_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[3].msgtbl_vl, remote, CNCL_SIZE_LEN(c_ip_address_t, remote));
    }
    if (_arg_present(remote_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", remote_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[4].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(local_address)) {
        strncpy(local, inet_ntoa(*local_address), sizeof(c_ip_address_t));
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[5].msgtbl_vl, local, CNCL_SIZE_LEN(c_ip_address_t, local));
    }
    if (_arg_present(local_port)) {
        snprintf(buffer, sizeof(buffer), "%05d", local_port);
        memcpy(oggz1in.emsinf.emsnninf.msgtbl[6].msgtbl_vl, buffer, sizeof(buffer) - 1);
    }
    if (_arg_present(inter_errcd)) {
        memcpy(oggz1in.emsinf.emsgkinf.inter_errcd, inter_errcd, CNCL_SIZE_LEN(ems_inter_errcd_t, inter_errcd));
    }
    GFPOGGZ1(&oggz1in);
}
