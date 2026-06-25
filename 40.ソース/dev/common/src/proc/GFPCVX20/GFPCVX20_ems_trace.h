#pragma once
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
/*****                    <<     head PROGRAM      >>                    *****/
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
/* USER HEADER     */
#include <GFPOGGZ1_emsout.h> nolist
#include <GFPOGGZ4_traceout.h> nolist
#include <ems.h> nolist
#include "GFPCVX20_cmp_trans.h" nolist
#include "GFPCVX20_fileIO.h" nolist
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
// 機能フラグ
#define DEF_TRACE_FILE_ID_PROC  "PROCESS"
#define DEF_TRACE_FILE_ID_PATHSEND  "PATHSEND"
#define DEF_TRACE_IO_TYPE_WR    "WRITE"
#define DEF_TRACE_IO_TYPE_RD    "READ"
#define DEF_TRACE_IO_TYPE_OP    "OPEN"
#define DEF_TRACE_IO_TYPE_OPCMP "OPENcomp"
#define DEF_TRACE_IO_TYPE_CL    "CLOSE"

#define DEF_PROC_ABNORMAL_END   1    // プロセス異常終了
#define DEF_PROC_NORMAL_END     0    // プロセス正常終了

/****************************************************************************/
/*   外部データ定義                                                         */
/****************************************************************************/
typedef char msgtbl_vl_t[80 + 1];
typedef char ems_inter_errcd_t[7];
typedef char ems_lcn_t[15];
typedef char ems_srvcls_name_t[15 + 1];
typedef char ems_pathmon_name_t[16 + 1];
typedef char ems_ipc_id_t[4];
typedef char ems_ipc_err_col_t[10 + 1];
typedef char ems_err_reason_t[10 + 1];
typedef char ems_err_reason_detail_t[20];
typedef char ems_err_ipc_h24[24];
typedef char ems_fio_ope_t[15 + 1];
typedef char ems_fio_key_t[40];
typedef char ems_mod_id[8 + 1];
typedef char ems_cmd[10];
typedef char ems_rslt[5];

//GFPOGGZ4が内部で使用するグローバル変数
extern char  EXMYSRVCLSNAME[15];
extern char  EXMYPROCNAME[6];
extern short EXTRACEMODE;               //トレースのON/OFFを設定する
extern char  EXTRACEFILENAME[47];
extern short EXTRACEFILENO;

// EMSメッセージ通知区分
typedef enum __ems_notify_type_t
{
    e_cncl_ems_notify_default,          // デフォルト(最小値チェック) ’S’
    e_cncl_ems_notify_normal,           // 正常       ’*’
    e_cncl_ems_notify_system,           // システム   ’S’
    e_cncl_ems_notify_error,            // 業務エラー ’E’
    e_cncl_ems_notify_warning,          // 警告       ’W’
    e_cncl_ems_notify_id_max            // 最大値(引数エラーチェック)
} ems_notify_type_t;

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/
void initial_ems(const char *my_program_id, char network, char uytrmmon[13], char uytrmmonlen[2], char uytrmsrv[12],
                 char uytrmsrvlen[2], char uytrmtimer[4]);

const oggz1in_def *get_ems_com();

void cncl_ems_set_notify_type(ems_notify_type_t notify_type);

void cncl_ems_abnormal_end(short status, ems_inter_errcd_t inter_errcd);
void cncl_ems_param_err(msgtbl_vl_t param_name, short ercd, ems_inter_errcd_t inter_errcd);
void cncl_ems_config_error(void *pri_key, size_t key_len, filename_l_t l_f_name, ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_common_module_error(const ems_mod_id mod_id, const short s_err, const sub_prog_sts_t sub_prog_sts,
                                 const ems_inter_errcd_t inter_errcd);
void cncl_ems_procedure_error(const char *procedure_name, short errcd, ems_inter_errcd_t inter_errcd);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_response_error(ems_lcn_t lcn, ems_srvcls_name_t srvcls_from, ems_ipc_id_t ipc_id,
                            ems_inter_errcd_t inter_errcd);
void cncl_ems_normal_end();
void cncl_ems_process_start();
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_ipc_invalid_header(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_ipc_id_t ipc_id,
                                ems_ipc_err_col_t ipc_err_col, ems_inter_errcd_t inter_errcd);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_ipc_invalid_response(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_ipc_id_t ipc_id,
                                  ems_err_reason_t err_reason, ems_inter_errcd_t inter_errcd);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_fileio_error(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, filename_l_t filename, ems_fio_ope_t ope,
                          ems_fio_key_t key_val, size_t key_val_len, short errcd, ems_inter_errcd_t inter_errcd);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_nw_error(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port, short s_err,
                      ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_procio_error(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, filename_l_t filename, ems_fio_ope_t ope,
                          short errcd, ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_cmd_result(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_cmd cmd, ems_rslt rslt,
                        ems_inter_errcd_t inter_errcd);
#ifdef _TANDEM_SOURCE
_extensible
#endif
    void
    cncl_ems_cmd_rcv(ems_lcn_t lcn, gflin_pkey_def *recv_con_jd, ems_cmd cmd, ems_cmd param,
                     ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_ipc_length_error(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port, size_t ipc_total_length,
                      ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_p6d_error(ems_lcn_t lcn, ems_pathmon_name_t pathmon_name, ems_srvcls_name_t s9s_name, short errcd,
                       ems_inter_errcd_t inter_errcd);

void trace_end(io_trace_buf_t *trace, const char *fileIOtype, const size_t readlen, const int i_err);
void trace_start(io_trace_buf_t *trace, const char *fileId, const char *fileName, const char *fileIOtype,
                 const size_t recLen);

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_ipc_invalid_request(ems_err_reason_detail_t err_reason, ems_err_ipc_h24 err_ipc,ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_pathsend_outstanding_exceeded(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name, struct in_addr *remote_address,
                      unsigned short remote_port, struct in_addr *local_address, unsigned short local_port, short max_outstanding,
                      ems_inter_errcd_t inter_errcd);

#ifdef _TANDEM_SOURCE
_extensible
#else
#define _arg_present
#endif
    void
    cncl_ems_nw_connect_normal(gflin_pkey_def *gflin_pkey, tcpip_prc_name_t tcpip_prc_name,
                               struct in_addr *remote_address, unsigned short remote_port,
                               struct in_addr *local_address, unsigned short local_port, ems_inter_errcd_t inter_errcd);

// ptr = getenv("PM-TRACE-FLG");
// if (ptr) {
//     memcpy(PM_TRACE_FLG,ptr,2);
//     EXTRACEMODE = 1;
// } else {
//     memcpy(PM_TRACE_FLG,"00",2);
//     EXTRACEMODE = 0;
// }

// memset(EXMYSRVCLSNAME,' ',sizeof(EXMYSRVCLSNAME));
// ptr = getenv("PM-MY-SERVERCLASS");
// if (ptr) {
//     len = (short)_min(sizeof(EXMYSRVCLSNAME),strlen(ptr));
//     memcpy(EXMYSRVCLSNAME,ptr,len);
// }

// strcpy(work_buff,"ALTRC");
// err = get_assign_msg_by_name((char *)work_buff
//                             ,(assign_msg_type *)&assign_msg);
// if (err != 0) {
