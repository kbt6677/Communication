/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
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
#ifndef _GFPCVX1E_H_
#define _GFPCVX1E_H_
/* USER HEADER     */
#include "GFPCVX1G.h" nolist
extern myinfo_def myinfo;              /*自プロセス情報*/
extern IOC_def IOCMP;                  /*I/O完了情報*/
extern Event_Node_def *InEvent;            /*内部イベント情報*/
//-extern openers_def openers[DEF_MAX_NODE];  /*オープナー数管理テーブル*/
extern COM_STP_arg_1_def COM_STP_arg;  /*オープナープロセス管理*/
extern cf_def cf[2];                   /*コンフィグテーブル*/
extern sc_info_def     *sc_info;       /*コネクション管理情報 sc_infoのポインター*/
extern sc_info_def     *sc_info2;      /*コネクション管理情報 sc_infoのポインター(更新用)*/
extern lc_info_def     *lc_info;       /*リスナー管理情報 lc_infoのポインター*/
extern lp_info_def     *lp_info;       /*リスナープロセス管理情報*/
extern ib_info_def     *ib_info;       /*Inbound電文振分情報 ib_infoのポインター*/
extern ob_info_def     *ob_info;       /*Outbound電文振分管理情報 ob_confのポインター*/
extern ci_info_def     *ci_info;       /*コマンドサーバー情報*/
extern zsys_ddl_smsg_open_def           *open_msg_p;
extern zsys_ddl_smsg_close_def          *close_msg_p;
extern zsys_ddl_smsg_timesignal_def     *timer_expire_p;
extern zsys_ddl_smsg_qmsgcancelled_def  *cancel_p;
extern zsys_ddl_smsg_cpudown_def        *cpudown_p;
extern zsys_ddl_smsg_cpuup_def          *cpuup_p;
extern zsys_ddl_smsg_remotecpudown_def  *r_cpudown_p;
extern zsys_ddl_smsg_remotecpuup_def    *r_cpuup_p;
extern zsys_ddl_smsg_nodedown_def       *node_down_p;
extern zsys_ddl_smsg_nodeup_def         *node_up_p;

extern n101_def *n101;          /*コネクション接続通知*/
extern n102_def *n102;          /*コネクション入替・切断指示通知*/
extern r103_def *r103;          /*コネクション接続開始応答*/
extern r104_def *r104;          /*コネクション接続完了通知応答*/
extern r105_def *r105;          /*コネクション切断完了通知応答*/
extern r106_def *r106;          /*コネクション入替・切断完了通知応答*/
extern r201_def *r201;          /*電文受信通知応答*/
extern c202_def *c202;          /*電文送信要求*/
extern r107_def *r107;          /*コネクション状態通知応答*/
extern r501_def *r501;          /*コマンド応答*/
extern c502_def *c502;          /*コマンド処理要求(コマンドサーバーからのリクエスト)*/

extern fileio_def   gflin_io;       /*回線管理ファイルI/O*/
extern fileio_def   gclst_io;       /*回線ステータスファイルI/O*/

extern db_gflin_def *gflin_rec;     /*回線管理レコードバッファー*/
extern db_gclst_def *gclst_recin;   /*回線ステータスレコードバッファー*/
extern db_gclst_def *gclst_recout;  /*回線ステータスレコードバッファー*/

/* トレースGLOBAL定義 */
extern char EXMYSRVCLSNAME[15];
extern char EXMYPROCNAME[6];
extern short EXTRACEMODE;
extern char EXTRACEFILENAME[47];
extern short EXTRACEFILENO;

extern lk_zac2001i_arg_1_def *ipc_trs;
extern lk_zac2001p_arg_1_def *ps_trs;
extern lk_zac2001r_arg_1_def *rcv_trs;
extern lk_zac2001t_arg_1_def *tmf_trs;
extern trace_call_def        *trs_tmp;
extern char trace_work[64];

extern oggz1in_def z1_in;

#endif
