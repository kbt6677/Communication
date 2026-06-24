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
#ifndef _GFPCVX1D_H_
#define _GFPCVX1D_H_

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/
/*----------------------------------------------------*/
/* 自ﾌﾟﾛｾｽ情報                                        */
/*----------------------------------------------------*/
myinfo_def myinfo;              /*自プロセス情報*/
IOC_def IOCMP;                  /*I/O完了情報*/
Event_Node_def *InEvent;            /*内部イベント情報*/
//-openers_def openers[DEF_MAX_NODE];  /*オープナー数管理テーブル*/
COM_STP_arg_1_def COM_STP_arg;  /*オープナープロセス管理*/
cf_def cf[2];                   /*コンフィグテーブル*/
sc_info_def     *sc_info;       /*コネクション管理情報 sc_infoのポインター*/
sc_info_def     *sc_info2;      /*コネクション管理情報 sc_infoのポインター(更新用)*/
lc_info_def     *lc_info;       /*リスナー管理情報 lc_infoのポインター*/
lp_info_def     *lp_info;       /*リスナープロセス管理情報*/
ib_info_def     *ib_info;       /*Inbound電文振分情報 ib_infoのポインター*/
ob_info_def     *ob_info;       /*Outbound電文振分管理情報 ob_confのポインター*/
ci_info_def     *ci_info;       /*コマンドサーバー情報*/
zsys_ddl_smsg_open_def           *open_msg_p;
zsys_ddl_smsg_close_def          *close_msg_p;
zsys_ddl_smsg_timesignal_def     *timer_expire_p;
zsys_ddl_smsg_qmsgcancelled_def  *cancel_p;
zsys_ddl_smsg_cpudown_def        *cpudown_p;
zsys_ddl_smsg_cpuup_def          *cpuup_p;
zsys_ddl_smsg_remotecpudown_def  *r_cpudown_p;
zsys_ddl_smsg_remotecpuup_def    *r_cpuup_p;
zsys_ddl_smsg_nodedown_def       *node_down_p;
zsys_ddl_smsg_nodeup_def         *node_up_p;

n101_def        *n101;          /*コネクション接続通知*/
n102_def        *n102;          /*コネクション入替・切断指示通知*/
r103_def        *r103;          /*コネクション接続開始応答*/
r104_def        *r104;          /*コネクション接続完了通知応答*/
r105_def        *r105;          /*コネクション切断完了通知応答*/
r106_def        *r106;          /*コネクション入替・切断完了通知応答*/
r201_def        *r201;          /*電文受信通知応答*/
c202_def        *c202;          /*電文送信要求*/
r107_def        *r107;          /*コネクション状態通知応答*/
r501_def        *r501;          /*コマンド応答*/
c502_def        *c502;          /*コマンド処理要求(コマンドサーバーからのリクエスト)*/

fileio_def      gflin_io;           /* 回線管理ファイルI/O                      */
fileio_def      gclst_io;           /* 回線ステータスファイルI/O                */

db_gflin_def    *gflin_rec;       /* 回線管理レコードバッファ                 */
db_gclst_def    *gclst_recin;     /* 回線ステータスレコードバッファ(入力)     */
db_gclst_def    *gclst_recout;    /* 回線ステータスレコードバッファ(出力)     */

/* トレースGLOBAL定義 */
char EXMYSRVCLSNAME[15];    /*サーバークラス名*/
char EXMYPROCNAME[6];       /*自プロセス名*/
short EXTRACEMODE;          /*トレースモード*/
char EXTRACEFILENAME[47];   /*トレースファイル名*/
short EXTRACEFILENO;        /*トレースファイル番号*/

lk_zac2001i_arg_1_def *ipc_trs; /*IPCトレース*/
lk_zac2001p_arg_1_def *ps_trs;  /*PATHSENDトレース*/
lk_zac2001r_arg_1_def *rcv_trs; /*$RECEIVE完了トレース*/
lk_zac2001t_arg_1_def *tmf_trs; /*TMFトレース*/
trace_call_def        *trs_tmp;
char trace_work[64];            /*トレース編集用ワーク*/

oggz1in_def z1_in;           /*EMS出力インターフェース用*/

#endif
