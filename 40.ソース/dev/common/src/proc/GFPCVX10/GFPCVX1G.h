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
#ifndef CNSV_GLOBAL
#define CNSV_GLOBAL
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <errno.h> nolist
#include <memory.h> nolist
#include <stdbool.h> nolist
#include <stdarg.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <ctype.h> nolist
#include <math.h> nolist
#include <tal.h> nolist
#include <netinet/in.h> nolist
#include <arpa/inet.h> nolist
#include <netdb.h> nolist
#include <sys/socket.h> nolist
#include <zsysc> nolist
#include <cextdecs.h> nolist

/* USER HEADER     */
#include "file.h"
#include "ipc.h" nolist
//-#include <ipc_isys.h> nolist
#include "common.h" nolist
#include "ems.h" nolist
#include "errcd.h" nolist
#define DEF_MSGTTKB_NORMAL 0        /*メッセージ通知区分(正常)*/
#define DEF_MSGTTKB_SYSTEM_ERR 1    /*メッセージ通知区分(システムエラー)※GFP通信は未使用*/
#define DEF_MSGTTKB_WARNING 2       /*メッセージ通知区分(警告)※GFP通信は未使用*/
#define DEF_MSGTTKB_GYOM_ERR 3      /*メッセージ通知区分(業務エラー)*/
//#include <errcd.h> noist
/*■errcd.hが作成途中であるため、重複しないものを下記に展開、errcd.h完成後下記は削除する■*/
/* 内部エラーコード */
//#define DEF_NERR_NOMAL                      "0000000"            /* 正常                                       */
//#define DEF_NERR_FILE_IO_ERR                "SCAA001"            /* ファイルIOエラー                           */
//#define DEF_NERR_FILE_OPN_ERR               "SCAA002"            /* ファイルオープンエラー                     */
//#define DEF_NERR_PROC_OPN_ERR               "SCAA003"            /* プロセスオープンエラー                     */
//#define DEF_NERR_PSEND_TIMEOUT              "SCAA004"            /* Pathsendエラー(タイムアウト)               */
//#define DEF_NERR_PSEND_ERR_RE_OK            "SCAA005"            /* Pathsendエラー(リトライ可)                 */
//#define DEF_NERR_PSEND_ERR_RE_NG            "SCAA006"            /* Pathsendエラー(リトライ不可)               */
//#define DEF_NERR_PSEND_ERR_RE_OUT           "SCAA007"            /* Pathsendエラー(リトライアウト)             */
//#define DEF_NERR_IPC_HAKKOU_ERR             "SCAA008"            /* IPC発行エラー                              */
//#define DEF_NERR_IPC_SEISA_ERR              "SCAA009"            /* IPC精査エラー                              */
//#define DEF_NERR_SOCKET_GEN_ERR             "SCAA010"            /* SOCKET生成エラー                           */
//#define DEF_NERR_BIND_ERR                   "SCAA011"            /* BINDエラー                                 */
//#define DEF_NERR_LISTEN_ERR                 "SCAA012"            /* LISTENエラー                               */
//#define DEF_NERR_ACCEPT_ERR                 "SCAA013"            /* ACCEPTエラー                               */
//#define DEF_NERR_ACCEPT2_ERR                "SCAA014"            /* ACCEPT2エラー                              */
//#define DEF_NERR_SETSOCKOPT_ERR             "SCAA015"            /* SETSOCKOPTエラー                           */
//#define DEF_NERR_PRM_RD_ERR                 "SCAA016"            /* パラメータ読込エラー(必須パラメータ無し)   */
//#define DEF_NERR_PRM_RD_ERR_INV             "SCAA017"            /* パラメータ読込エラー(設定不正)             */
//#define DEF_NERR_MEM_GET_ERR                "SCAA018"            /* メモリ取得エラー                           */
//#define DEF_NERR_TMF_ERR                    "SCAA019"            /* TMFエラー                                  */
//#define DEF_NERR_SYSIF_LGC_ERR              "SCAA020"            /* プログラム/システム間論理矛盾              */
//#define DEF_NERR_CON_STS_CHK_ERR            "SCBD001"            /* コネクションステータスチェックエラー       */
//#define DEF_NERR_CON_ERR                    "SCBD002"            /* CONNECTエラー                              */
//#define DEF_NERR_CON_TIMEOUT                "SCBD003"            /* CONNECTエラー(タイムアウト)                */
//#define DEF_NERR_SHT_RETRY_OVER             "SCBD004"            /* ショートリトライ回数オーバー               */
//#define DEF_NERR_LNG_RETRY_OVER             "SCBD005"            /* ロングリトライ回数オーバー                 */
//#define DEF_NERR_DST_ADRS_CHK_ERR           "SCBC001"            /* 接続先アドレスチェックエラー               */
//#define DEF_NERR_CON_NUM_OVER               "SCBB001"            /* コネクション数オーバー検知                 */
//#define DEF_NERR_RCV_ERR                    "SCBA001"            /* RCVエラー                                  */
//#define DEF_NERR_RCV_TIMEOUT                "SCBA002"            /* RCVエラー(タイムアウト)                    */
//#define DEF_NERR_PACKET_RCV_TIMEOUT         "SCBA003"            /* RCVエラー(分割パケット受信タイムアウト)    */
//#define DEF_NERR_RCV_DENBUN_LEN_ERR         "SCBA004"            /* 受信電文データ長チェックエラー             */
//#define DEF_NERR_RCV_SIZE_ERR               "SCBA005"            /* 受信完了サイズチェックエラー               */
//#define DEF_NERR_SEND_ERR                   "SCBA006"            /* SENDエラー                                 */
//#define DEF_NERR_SEND_TIMEOUT               "SCBA007"            /* SENDエラー(タイムアウト)                   */
//#define DEF_NERR_SEND_DENBUN_LEN_ERR        "SCBA008"            /* 送信電文データ長チェックエラー             */
//#define DEF_NERR_RE_READ_CMD_HAKKOU_ERR     "SCBQ001"            /* ファイル再読込みコマンド発行エラー         */
//#define DEF_NERR_RE_READ_CMD_SEISA_ERR      "SCBA009"            /* ファイル再読込みコマンド精査エラー         */
//#define DEF_NERR_RE_READ_ERR                "SCBA010"            /* ファイル再読込み失敗                       */
//#define DEF_NERR_DST_SELECT_ERR             "SCCF001"            /* 送信先選択不可                             */
//#define DEF_NERR_SMK_RSP_SEISA_ERR          "SCCE001"            /* 仕向応答精査エラー                         */
//#define DEF_NERR_KC_ENC_ERR                 "SCCE002"            /* 電文暗号化エラー(KC)                       */
//#define DEF_NERR_KMAC_CALC_ERR              "SCCF002"            /* 電文認証値算出エラー(KMAC)                 */
//#define DEF_NERR_SMK_REQ_DUAL_SEND          "SCCF003"            /* 仕向要求二重送信チェックエラー             */
//#define DEF_NERR_SMK_RSP_DUAL_RCV           "SCCE003"            /* 仕向応答二重受信チェックエラー             */
//#define DEF_NERR_HSMK_REQ_SEISA_ERR         "SCCE004"            /* 被仕向要求精査エラー                       */
//#define DEF_NERR_HSMK_REQ_MTI_HANTE_ERR     "SCCE005"            /* 被仕向要求MTI判定不可                      */
//#define DEF_NERR_KC_DEC_ERR                 "SCCE006"            /* 電文復号エラー(KC)                         */
//#define DEF_NERR_KMAC_HANTE_ERR             "SCCE007"            /* 電文認証値判定エラー(KMAC)                 */
//#define DEF_NERR_TO_FURIWAKE_FC2002_ERR     "SCCE008"            /* 東阪振分エラー(FC2002)                     */
//#define DEF_NERR_HSMK_REQ_DUAL_RCV_ERR      "SCCE009"            /* 被仕向要求二重受信チェックエラー           */
//#define DEF_NERR_HSMK_RSP_DUAL_SEND_ERR     "SCCF004"            /* 被仕向応答二重送信チェックエラー           */
//#define DEF_NERR_LCN_GET_ERR                "SCCE010"            /* LCN取得エラー                              */
//#define DEF_NERR_RCV_DENBUN_HEADR_ERR       "SCCE011"            /* 受信電文ヘッダ精査エラー                   */
//#define DEF_NERR_DENBUN_DEC_ERR             "SCCE012"            /* 電文復号処理エラー                         */
//#define DEF_NERR_MTI_GET_ERR                "SCCE013"            /* MTI取得エラー                              */
//#define DEF_NERR_FURIWAKE_DST_HANTE_ERR     "SCCE014"            /* 振分先判定エラー                           */
//#define DEF_NERR_TO_FURIWAKE_FC2004_ERR     "SCCE015"            /* 東阪振分エラー(FC2004)                     */
//#define DEF_NERR_CON_SELECT_ERR             "SCCF005"            /* コネクション選択エラー                     */
//#define DEF_NERR_FREE_CON_NON               "SCCF006"            /* 空きコネクション無し                       */
//#define DEF_NERR_ATALLA_FURIWAKE_ERR        "SCCA016"            /* Atalla振分エラーリプライ                   */
//#define DEF_NERR_BOXCAR_ERR_CODE311         "SCCA017"            /* BOXCARエラーリプライコード311              */
//#define DEF_NERR_BOXCAR_ERR                 "SCCA018"            /* BOXCARエラーリプライコード311以外          */
//#define DEF_NERR_HSMK_REQ_BITMAP_ERR        "SCDI002"            /* 被仕向要求ビットマップ展開エラー           */
//#define DEF_NERR_SMK_REQ_SEISA_ERR          "SCDI003"            /* 仕向要求精査エラー                         */
//#define DEF_NERR_SMK_RSP_TIMEOUT            "SCDI004"            /* 仕向応答待ちタイムアウト                   */
//#define DEF_NERR_SMK_RSP_MCH_ERR            "SCDI006"            /* 仕向応答マッチングエラー(レートレスポンス) */
//#define DEF_NERR_BITMAP_DEC_ERR             "SCDI007"            /* ビットマップ展開エラー                     */
//#define DEF_NERR_BITMAP_ENC_ERR             "SCDI008"            /* ビットマップ結合エラー                     */
//#define DEF_NERR_TIMER_ERR                  "SCDI009"            /* タイマー発行エラー                         */
//#define DEF_NERR_TIMER_CNSL_ERR             "SCDI012"            /* タイマー解除エラー                         */
//#define DEF_NERR_TIMEOUT_DETECT             "SCDI013"            /* タイムアウト検知                           */
//#define DEF_NERR_MCH_ERR                    "SCDI014"            /* マッチング失敗                             */
//#define DEF_NERR_LG_SWC_CMD_ERR             "SCEO001"            /* ログファイル切替コマンド受付エラー         */
//#define DEF_NERR_LG_SWC_ERR                 "SCEO002"            /* ログファイル切替処理エラー                 */
//#define DEF_NERR_CMD_SEISA_ERR              "SCFQ001"            /* コマンド精査エラー                         */
//#define DEF_NERR_CMD_HAKKO_ERR              "SCFQ002"            /* コマンド発行エラー                         */
//#define DEF_NERR_CMD_ERR_DONE               "SCFQ003"            /* コマンドエラー完了                         */

#include "GFPOGGZ3_encode.h" nolist
#include "GFPOGGZ4_traceout.h" nolist
#include "GFPCGX50.h" nolist    /*システム日時処理*/
#include "GFPCGX60.h" nolist    /*IOタグ生成&解析*/
#include "GFPCGX90.h" nolist    /*ユニーク日時取得*/
#include "GFPCGXA0.h" nolist    /*トランザクション管理*/
#include "GFPCGXB0.h" nolist    /*IOモジュール*/
#include "GFPCGXC0.h" nolist    /*オープナープロセス管理*/
#include "GFPCGXD0.h" nolist    /*ASSIGN取得*/
#include "GFPCGXG0.h" nolist    /*プロセス情報取得*/

/* define宣言 */
#define DEF_NORMAL_TERMINATION 0                        /*正常終了*/
#define DEF_ABNORMAL_TERMINATION 1                      /*異常終了*/

#define DEF_LABEL_COMMAND 2000                          /*コマンドインターフェース用OPENラベル*/
#define DEF_LABEL_CREATOR 2001                          /*クリエータ用OPENラベル*/
#define DEF_LABEL_LINKMON 2002                          /*LINKMONOOPENラベル*/

#define DEF_FILE_CLOSED -1                              /*ファイルCLOSE*/
#define DEF_TAG_NULL -1                                 /*タグNULL*/
#define DEF_TAG_SOCKET 9999                             /*socket用タグ値*/
#define DEF_TAG_RECV  10000                             /*$RECEIVE用I/Oタグ*/
#define DEF_TABLE_FREE 0                                /*テーブル状態空き*/
#define DEF_TABLE_USE 1                                 /*テーブル状態使用中*/
#define DEF_SOCKET_TIMER 1000                           /*WAIT用ソケットI/Oタイマー*/
#define DEF_MAX_textlen_size 6                          /*電文長最大バイト数*/

#define DEF_GFPHI_PKEY_LEN 39                           /*物理名情報ファイルキー長*/
#define DEF_GFLIN_PKEY_LEN 24                           /*回線管理サーチ長(1+1+5+5+6+6)*/

#define DEF_MAX_NODE 10                                 /*最大ノード数*/
#define DEF_MAX_CONNECTION CCS_MAX_connection           /*コネクション管理テーブルのエレメント数*/
#define DEF_MAX_interface CCS_MAX_interface             /*インターフェースorステーション管理テーブルのエレメント数*/
#define DEF_MAX_LISTNER CCS_MAX_listners                /*リスナープロセス管理テーブルのエレメント数*/
#define DEF_MAX_OUTBOUND CCS_MAX_outbound               /*電文振分(outbound)プロセス管理テーブルのエレメント数*/
#define DEF_MAX_INBOUND_PS CCS_MAX_inbound              /*電文振分(inbound)同時PATHSEND数MAX*/
#define DEF_MAX_RECV_QUEUE CCS_MAX_recvqueue            /*受信キュー数MAX*/
#define DEF_MAX_SEND_QUEUE CCS_MAX_sendqueue            /*送信キュー数MAX*/
#define DEF_RECEIVE_FILENAME "$RECEIVE"                 /*$RECEIVE file name*/
#define DEF_RCV_NOWAITDEPTH 1                           /* enable nowait operations */
#define DEF_LISTENER_NOWAITDEPTH 2                      /*スイッチャーに対するNOWAIT I/O数*/
#define DEF_OUTBOUND_NOWAITDEPTH 1                      /*Outbound電文振分に対するNOWAIT I/O数*/
#define DEF_ALLOW_NOWAIT_OPERATIONS 30
#define DEF_COMPLETE_OPERATIONS_ANY_ORDER 1

#define DEF_C201_hd_len 24
#define DEF_C201_msg_info_len 150
#define DEF_C201_txt_len 2
/*■DEF_RECVDEPTH(現在値1142、定義上の最大16300)*/
/*コネクションrecv+send+recv_timer+send_timer*/
/*リスナーWR+timer*/
/*Inbound電文振分同時PATHSEND数*/
/*Outbound電文振分数*/
/*$RECEIVEのREADUPDATE、コマンドサーバー*/
#define DEF_RECVDEPTH ((DEF_MAX_CONNECTION * 4) + \
                       (DEF_MAX_LISTNER * 3) + \
                       DEF_MAX_INBOUND_PS + \
                       (DEF_MAX_OUTBOUND * 2) + \
                       1 + 1)
/*■バッファ数*/
/*コネクションrecv+send*/
/*リスナーASYNC+WR*/
/*Inbound電文振分同時PATHSEND数*/
/*Outbound電文振分数*/
/*$RECEIVEのREADUPDATE、処理中、コマンドサーバー*/
/*想定される送信、受信キュー数*/
#define DEF_MAX_BUFFERS ((DEF_MAX_CONNECTION * 2) + \
                        (DEF_MAX_LISTNER * 2) + \
                        DEF_MAX_INBOUND_PS + \
                        DEF_MAX_OUTBOUND + \
                        1 + 1 + 1 + \
                        DEF_MAX_RECV_QUEUE + DEF_MAX_SEND_QUEUE)
/*■待ちリスト数*/
/*コネクションopenまたはcloseコマンド*/
/*リスナーopenまたはcloseコマンド+状態通知*/
/*Inbound電文振  電文受信通知*/
/*Outbound電文振分 openまたはcloseコマンド＋状態通知*/
/*コマンドサーバー自動閉局*/
/*想定される送信、受信キュー数*/
#define DEF_MAX_Event_list ((DEF_MAX_CONNECTION * 2) + \
                (DEF_MAX_LISTNER * DEF_MAX_CONNECTION) + \
                DEF_MAX_INBOUND_PS + \
                DEF_MAX_OUTBOUND + (DEF_MAX_OUTBOUND * DEF_MAX_CONNECTION) + \
                DEF_MAX_CONNECTION + DEF_MAX_RECV_QUEUE + DEF_MAX_SEND_QUEUE)
#define DEF_MSG_PROC_NOWAITDEPTH 1          /* enable nowait operations */
#define DEF_PROCESS_NOWAITDEPTH 2           /* enable nowait operations */
#define DEF_COLLECTOR_FILENAME "$0"         /*メッセージコレクタープロセス名*/
#define DEF_ALLOW_NOWAIT_OPERATIONS 30
#define DEF_COMPLETE_OPERATIONS_ANY_ORDER 1

#define DEF_NORMAL_TERMINATION 0            /*正常終了*/
#define DEF_ABDEF_NORMAL_TERMINATION 1      /*異常終了*/

#define DEF_FILE_CLOSED -1                  /*ファイルCLOSE*/
#define DEF_TAG_NULL -1                     /*タグNULL*/
#define DEF_Not_Found -1                    /*該当なし*/
#define DEF_POSITIONING_MODE_Approximate 0
#define DEF_POSITIONING_MODE_Generic     1
#define DEF_POSITIONING_MODE_Exact       2

#define DEF_TRACE_OPEN                  "OPEN    "
#define DEF_TRACE_START                 "START   "
#define DEF_TRACE_READ                  "READ    "
#define DEF_TRACE_WRITE                 "WRITE   "
#define DEF_TRACE_REWRITE               "REWRITE "
#define DEF_TRACE_DELETE                "DELETE  "
#define DEF_TRACE_UNLOCK                "UNLOCK  "
#define DEF_TRACE_CLOSE                 "CLOSE   "

#define DEF_textlen_type_BIN            1
#define DEF_textlen_type_BCD            2
#define DEF_textlen_type_ASCII          3
#define DEF_textlen_type_EBCDIC         4
#define DEF_datalen_outside             0   /*データレングスINCLUDE識別 "O"：含めない*/
#define DEF_datalen_inside              1   /*データレングスINCLUDE識別 "I"：含める*/

#define DEF_GROUP_len_interface         12  /*コネクション数管理、インターフェース単位の比較長*/
#define DEF_GROUP_len_station           18  /*コネクション数管理、ステーション単位の比較長*/

#define DEF_VAR_STOP 0              /*message_output関数パラメータの終端を示すためのダミー定義*/
#define DEF_DUMMY_INDEX 0           /*1時開発用ダミーインデックス*/
/*コネクション制御(サーバ)コンポーネント*/
#define DEF_Component_system        1       /*プロセス管理コンポーネント(sys)*/
#define DEF_Component_socket        2       /*コネクション管理コンポーネント(so)*/
#define DEF_Component_Listener      3       /*リスナー管理コンポーネント(lsn)*/
#define DEF_Component_Inbound       4       /*INBOUND電文振分管理コンポーネント(ib)*/
#define DEF_Component_Outbound      5       /*OUTBOUND電文振分管理コンポーネント(ob)*/
#define DEF_Component_Command       6       /*コマンドサーバー管理コンポーネント(ci)*/

/*イベント*/
/*注意：xxI/Oエラーの採番はI/O完了時にtagがらイベント番号を設定、その後にエラーコードを判定し振り直すを簡単にするため正常完了+1で定義すること*/
#define DEF_EV_none                       0 /*イベントなし(イベント処理済)*/
#define DEF_EV_open_msg                 101 /*OPEN*/
#define DEF_EV_close_msg                102 /*CLOSE*/
#define DEF_EV_cpu_down                 103 /*CPUダウン*/
#define DEF_EV_cpu_up                   104 /*CPUアップ*/
#define DEF_EV_rcpu_down                105 /*リモートCPUダウン*/
#define DEF_EV_rcpu_up                  106 /*リモートCPUアップ*/
#define DEF_EV_node_down                107 /*ノードダウン*/
#define DEF_EV_node_up                  108 /*ノードアップ*/
#define DEF_EV_text_send_req            501 /*C202 電文送信要求*/
#define DEF_EV_connect_req              602 /*C502-1010 接続要求*/
#define DEF_EV_disconnect_req           603 /*C502-1020 切断要求*/
#define DEF_EV_file_reload              604 /*C502-4010 ファイル再読み込み*/
#define DEF_EV_unknown_req              605 /*不明リクエスト受信*/
#define DEF_EV_timeout                  110 /*タイムアウトメッセージ*/
#define DEF_EV_io_timeout               112 /*I/O完了待ちタイマー*/
#define DEF_EV_idle_timeout             224 /*無通信監視タイマー*/
#define DEF_EV_recv_timeout             223 /*残データ受信完了待ちタイマー*/
#define DEF_EV_send_timeout             227 /*残データ送信完了待ちタイマー*/
#define DEF_EV_S_retry_timeout          113 /*ショートリトライタイマー*/
#define DEF_EV_L_retry_timeout          114 /*ロングリトライタイマー*/
#define DEF_EV_cancel_msg               130 /*キャンセル受信*/
#define DEF_EV_sock_recv_comp           221 /*recv完了*/
#define DEF_EV_sock_recv_err            222 /*recvエラー*/
#define DEF_EV_sock_send_comp           225 /*send完了*/
#define DEF_EV_sock_send_err            226 /*sendエラー*/
#define DEF_EV_PS_comp                  131 /*PATHSEND完了*/
#define DEF_EV_text_recv_rsp            401 /*R201 電文受信通知応答*/
#define DEF_EV_recv_text_send_req       402 /*C201 電文受信通知送信要求*/
#define DEF_EV_signon_rsp               606 /*R501 コマンド応答(自動signon)*/
#define DEF_EV_unknown_resp             132 /*不明応答受信*/
#define DEF_EV_PS_err                   133 /*PATHSENDエラー*/
#define DEF_EV_async_req                300 /*A001:非同期要求*/
#define DEF_EV_connect_notice           301 /*N101:コネクション接続通知*/
#define DEF_EV_dis_reconnect_notice     302 /*N102:コネクション入替・切断指示通知*/
#define DEF_EV_connect_start_resp       303 /*R103:コネクション接続開始応答*/
#define DEF_EV_connect_complete_resp    304 /*R104:コネクション接続完了通知応答*/
#define DEF_EV_discinnect_complete_resp 305 /*R105:コネクション切断完了通知応答*/
#define DEF_EV_chg_disconnect_notice    306 /*R106:コネクション入替・切断完了通知応答*/
#define DEF_EV_chg_sock_close           307 /*コネクション入換ソケットクローズ*/
#define DEF_EV_chg_sock_open            308 /*コネクション入換ソケットオープン*/
#define DEF_EV_Listener_io_err          309 /*リスナーI/Oエラー*/
#define DEF_EV_tell_port_status         310 /*ポートステータス送信*/
#define DEF_EV_excluded_req             311 /*対象外要求*/
#define DEF_EV_open_comp                502 /*Nowaitオープン完了*/
#define DEF_EV_open_err                 503 /*Nowaitオープンエラー*/
#define DEF_EV_nw_open_timeout          504 /*Nowaitオープンタイムアウト*/
#define DEF_EV_con_state_notice         505 /*R107:コネクション状態通知応答*/
#define DEF_EV_outbound_io_err          506 /*Outbound電文振分I/Oエラー*/
#define DEF_EV_port_open                211 /*データポート設定要求*/
#define DEF_EV_port_close               212 /*データポート解放要求*/
#define DEF_EV_signon_req               507 /*自動サインオン要求*/

#define DEF_GCLST_con_state_nochange      0 /*コネクション状態無変更*/
#define DEF_GCLST_con_state_close         1 /*切断*/
#define DEF_GCLST_con_state_listen        2 /*接続待ち*/
#define DEF_GCLST_con_state_open          3 /*接続*/
#define DEF_GCLST_con_state_reconnect     4 /*再接続待ち*/
#define DEF_GCLST_proc_state_nocgange     0 /*プロセス状態無変更*/
#define DEF_GCLST_proc_state_close        1 /*回線クローズコマンド受信済み*/
#define DEF_GCLST_proc_state_open         2 /*回線オープンコマンド受信済み*/

#define DEF_tbl_chk_len_connection       24 /*コネクション単位指定*/
#define DEF_tbl_chk_len_station          18 /*ステーション単位指定*/
#define DEF_tbl_chk_len_interface        12 /*インターフェース単位指定*/

/* 構造体宣言 */
/*タイムスタンプ取得*/
#pragma fieldalign shared2 __NSK_UniqueTimeStamp128
typedef struct __NSK_UniqueTimeStamp128 {
    long long NS[2];
} NSK_UniqueTimeStamp128;

/*Eventノード*/
#pragma fieldalign shared2 __Event_Node_def
typedef struct __Event_Node_def {
    struct __Event_Node_def *next;  /*次ノード*/
    short   compo;                  /*コンポーネントID*/
    short   thread;                 /*スレッド*/
    short   event;                  /*イベント*/
    short   option1;                /*オプション情報1*/
    short   option2;                /*オプション情報2*/
    short   len;                    /*テキスト長*/
    char    *text;                  /*テキストポインター*/
    long long event_time;           /*イベントを登録した時間*/
} Event_Node_def;

/*Eventリスト*/
#pragma fieldalign shared2 __Event_List_def
typedef struct __Event_List_def {
    int                 list_count;    /*管理ノード数*/
    Event_Node_def      *head;          /*リスト先頭*/
    Event_Node_def      *tail;          /*リスト末尾*/
} Event_List_def;

/*トレース定義*/
#pragma fieldalign shared2 __lk_trace
typedef struct __lk_trace
{
   char                            func_flg;
   struct
   {
      char                            prog_id[8];
      char                            file_id[8];
      char                            file_name[47];
      char                            file_io_type[8];
      char                            guardian_errcode[4];
      char                            shori_start_time[12];
      char                            shori_end_time[12];
   } trace_info;
   struct
   {
      char                            rec_len[5];
   } data_info;
} lk_trace_def;

#pragma fieldalign shared2 __trace_call_def
typedef struct __trace_call_def
{
    char                            Dummy_Area;             /*lk_zac201i_arg_1_def.data_info.rec_area位置調整*/
    lk_zac2001i_arg_1_def           trs;
} trace_call_def;
#define DEF_BUF_ADJUST (1+1+8+8+47+8+4+12+12+5)             /*IO位置*/
#define DEF_IOCMP_TRS_HD_POS (1+8+8+47+8+4+12+12+5)         /*I/O完了時のトレースヘッダー位置*/
#define DEF_TRS_HD_SIZE  (1+8+8+47+8+4+12+12)               /*トレース固定情報サイズ*/

/*バッファーヘッダー*/
#define DEF_BUF_CTL_HD_SIZE 4                               /*バッファーヘッダー長(次リストのポインター部長)*/
#define DEF_BUF_SIZE (106+176+9999+1)                       /*バッファー取得サイズ(トレース＋IPCヘッダー+電文長+バウンダリー調整)*/
#pragma fieldalign shared2 __buff_node_def
typedef struct __buff_node_def
{
    struct  __buff_node_def *next;
    char    dt[DEF_BUF_SIZE];                               /*トレース情報を含めたサイズを指定*/
} buff_node_def;

/*バッファー管理*/
#pragma fieldalign shared2 __buff_list_def
typedef struct __buff_list_def
{
    int                 list_count;
    buff_node_def       *head;
    buff_node_def       *tail;
} buff_list_def;

#define DEF_c202_ADJUST 176                                 /*c202電文送信要求調整値*/

/*自プロセス情報*/
#pragma fieldalign shared2 __myinfo_def
typedef struct __myinfo_def
{
    short   rcv_fd;                                         /*$RECEIVEファイル番号*/
    char    *rcv_p;                                         /*$RECEIVEバッファポインター*/
    NSK_UniqueTimeStamp128 TS128;                           /*READUPDATE開始時間(トレース取得用)*/
    short   pathsend_fd;                                    /*PATHSENDオペレーション番号*/
    short   cf_idx;                                         /*コンフィグインデックス*/
    short   end_flag;                                       /*終了条件フラグ*/
    /**/
    short   lp_info_use_cnt;                                /*リスナープロセス管理使用数*/
    short   sc_info_use_cnt;                                /*コネクション管理使用数*/
    /*RUN PARAM*/
    long    pathsend_io_timer;                              /*PATHSEND完了待ちタイマー*/
    long    process_io_timer;                               /*プロセスI/O完了待ちタイマー*/
    long    file_io_timer;                                  /*ファイルI/O完了待ちタイマー*/
    long    nowait_open_timer;                              /*Nowaitオープン完了待ちタイマー*/
    long    socket_io_timer;                                /*ソケットI/O完了待ちタイマー*/
    short   pathsend_retry_count;                           /*PATHSENDリトライ回数*/

    short   my_handle[ZSYS_VAL_PHANDLE_WLEN];               /*自プロセスハンドル*/
    short   creator_phandle[ZSYS_VAL_PHANDLE_WLEN];         /*クリエータープロセスハンドル*/
    /*PARAM(SRV-LOGICAL-ID)より取得*/
    char    site;                                           /*サイト識別*/
    char    network;                                        /*N/W識別*/
    char    group[5];                                       /*グループ識別*/
    char    server_class_name[8];                           /*サーバークラス論理名*/
    char    server_class_num[4];                            /*サーバークラス論理番号*/
    /*物理名情報ファイルより取得*/
    char    pathmon_name[16];                               /*物理名情報ファイルで設定されたPATHMON名*/
    char    my_server_class[16];                            /*物理名情報ファイルで設定されたサーバークラス名*/
    /*ファイル名情報*/
    short   GFPHI_name_len;
    char    GFPHI_name[ZSYS_VAL_LEN_FILENAME+1];            /*物理名情報ファイル*/
    short   GFPHI_fd;
    short   GFNWI_name_len;
    char    GFNWI_name[ZSYS_VAL_LEN_FILENAME+1];            /*NW情報ファイル*/
    short   GFLIN_name_len;
    char    GFLIN_name[ZSYS_VAL_LEN_FILENAME+1];            /*回線管理ファイル*/
    short   GCLST_name_len;
    char    GCLST_name[ZSYS_VAL_LEN_FILENAME+1];            /*回線ステータスファイル*/
    short   GCLST_fd;                                       /*回線ステータスファイルファイル番号*/
    /*プロセス制御、イベントリスト管理、バッファー管理*/
    Event_List_def free_list;                               /*空きリスト*/
    Event_List_def send_wait;                               /*内部イベントリスト*/
    buff_list_def  buffs;
    /*運用監視端末情報*/
    char    uytrmmon[13];                                   /*運用監視端末出力PATHMON名*/
    char    uytrmmonlen[2];
    char    uytrmsrv[12];                                   /*運用監視端末出力サーバ名*/
    char    uytrmsrvlen[2];
    char    uytrmtimer[4];                                  /*運用監視端末出力I/Oタイマー*/
    /*自ノード、自プロセス名*/
    char            my_name[ZSYS_VAL_LEN_PROCESSNAME+1];    /*自プロセス名*/
    short           my_name_len;                            /*自プロセス名長*/
    char            my_node[ZSYS_VAL_LEN_SYSTEMNAME];
    short           my_node_name_len;
    /*障害調査用*/
    long long       start_time;                             /*プロセス起動タイムスタンプ*/
    COM_SDT_arg_2_def   ts_char;                            /*システム日時取得*/
    COM_SDT_arg_3_def   ts_short;                           /*システム日時取得*/
    long long           ts_64;                              /*システム日時取得*/
    /*調査用情報*/
    struct  {
        short   alloc_list_count;                           /*イベントリスト取得数*/
        short   alloc_buffs_count;                          /*バッファー取得数*/
        char    *event_node_top;                            /*イベントリスト先頭*/
        char    *event_node_tail;                           /*イベントリスト最終*/
        char    *buff_list_top;                             /*バッファー先頭*/
        char    *buff_list_tail;                            /*バッファー最終*/
    } failed_analyze_info;
} myinfo_def;

/* I/O完了情報 */
#pragma fieldalign shared2 __IOC_def
typedef struct __IOC_def                                    /*I/O完了情報*/
{
    short   fd;                                             /*ファイル番号*/
    short   fs_err;                                         /*エラーコード*/
    short   len;                                            /*完了長*/
    short   component;                                      /*コンポーネント番号*/
    short   thread;                                         /*スレッド番号(event_judgement等で設定)*/
    short   event;                                          /*イベント番号*/
    short   index;                                          /*インデックスpathsend完了時に設定*/
    long    tag;                                            /*完了TAG*/
    long    addr;                                           /*バッファーアドレス*/
    NSK_UniqueTimeStamp128 TS128;                           /*READUPDATE開始時間(トレース取得用(障害発生時の調査用)*/
    zsys_ddl_receiveinformation_def RINF;                   /*$RECEIVE完了情報*/
    short   cpu;
    char    node_name[ZSYS_VAL_LEN_SYSTEMNAME+1];
    char    proc_name[ZSYS_VAL_LEN_PROCESSNAME+1];
} IOC_def;

//-/*オープナー情報テーブル*/
//-#pragma fieldalign shared2 __openers_def
//-typedef struct __openers_def
//-{
//-    char    node_name[ZSYS_VAL_LEN_SYSTEMNAME];         /*ノード名*/
//-    struct
//-    {
//-        short   opener_count;                           /* オープナー数 */
//-    } cpus[16];
//-} openers_def;

/*コネクション定義情報/回線管理情報*/
#pragma fieldalign shared2 __sc_conf_def
typedef struct __sc_conf_def
{
    char    site_name;                  /*サイト識別*/
    char    nw_name;                    /*N/W識別*/
    char    group_name[5];              /*グループ識別*/
    char    interface_name[5];          /*インタフェース識別*/
    char    station_name[6];            /*ステーション識別*/
    char    src_connection_name[6];     /*コネクション識別*/
    char    lc_sc_sign[6];              /*リスナーコネクション識別*/
    char    tcpip_name[6+1];            /*TCP/IPプロセス名(socket_set_inet_nameで使用)*/
    char    local_ipaddr[15+1];         /*ローカルIPアドレス*/
    char    local_port_no[5+1];         /*ローカルポート番号*/
    char    remote_ipaddr[15+1];        /*リモートIPアドレス*/
    char    remote_port_no[5+1];        /*リモートポート番号*/
    char    use_on_off;                 /*有効、無効フラグ*/
} sc_conf_def;
#define DEF_len_sc_conf 82              /*コネクション定義情報/回線管理情報エレメント長*/
/*コネクション管理情報*/
#pragma fieldalign shared2 __sc_info_def
typedef struct __sc_info_def
{
    char                site_name;                  /*サイト識別*/
    char                nw_name;                    /*N/W識別*/
    char                group_name[5];              /*グループ識別*/
    char                interface_name[5];          /*インタフェース識別*/
    char                station_name[6];            /*ステーション識別*/
    char                src_connection_name[6];     /*コネクション識別*/

    short               status;             /*スレッドステータス*/
    short               sock_fd;            /*ソケットファイル番号*/
    struct sockaddr_in  sock;               /*ソケット構造体 リスナーから受け取ったソケット構造体*/
    char                remote_ipaddr[15+1];    /*リモートIPアドレス*/
    char                remote_port_no[5+1];    /*リモートポート番号*/
    short               connection_status;  /*コネクション状態 コネクション状態、回線ステータスファイルに変換して格納用*/
    char                disconnect_info[2]; /*切断情報 回線ステータスファイル格納用*/
    short               error_code;         /*エラーコード 回線ステータスファイル格納用/最後に発生したエラー番号*/
    long long           update_time;        /*更新時間 ステータス更新時間*/
    short               hdlen;              /*ヘッダー長(初期recv時に全体電文長を含んだ長さを設定する)*/
    short               textlen_pos;        /*データレングス開始位置(1を引いた値を設定)*/
    short               textlen_size;       /*データレングス項目長*/
    short               textlen_type;       /*データレングス項目属性 BCD=1、ASCII=2、EBCDIC=3*/
    short               textlen_include;    /*データレングスINCLUDE識別 0=含めない、1=含める*/
    char                *recv_p;            /*受信バッフーァポインター(Inbound引き渡し後NULL)*/
    short               recv_timer_tag;     /*受信完了待ちタイマータグ*/
    short               recv_len;           /*受信データ長*/
    short               recv_comp_len;      /*受信完了長*/
    long                recv_timer_value;   /*後続データ受信待ちタイマー*/
    char                *send_p;            /*送信バッファーポインター*/
    short               send_timer_tag;     /*送信完了待ちタイマータグ*/
    short               send_total_len;     /*送信全体長*/
    short               send_comp_len;      /*送信完了長*/
    long                send_timer_value;   /*SEND完了待ちタイマー*/
    Event_List_def      send_wait;          /*送信待ちイベントリスト情報*/
    short               listner_index;      /*リスナーテーブルインデックス*/
    short               station_index;      /*ステーションテーブルインデックス*/
    short               outbound_index;     /*Outbound電文振分テーブルインデックス*/
    short               group_no;           /*グループ番号(インターフェース単位/ステーション単位のグループ番号)*/
    long                idle_timer_value;   /*無通信状態監視タイマー*/
    short               idle_timer_tag;     /*無通信状態監視タイマータグ*/
    COM_UNQ_arg_1_def   ach_timestamp;      /*受信完了時間タイムタンプ*/
    char                ach_uniq_ts[16];    /*受信完了時間ユニークタイムスタンプ*/
    struct                                  /*コネクション状態情報*/
    {
        char    connection_status[2];       /*コネクション状態*/
        char    connection_status_time[20]; /*コネクション状態変更日時*/
    } status_info;
    struct                                  /*プロセス状態情報*/
    {
        char    process_status[2];          /*プロセス状態*/
        char    process_status_time[14];    /*プロセス状態変更日時*/
    } process_info;
    struct                                  /*コネクション情報*/
    {
        char    error_code[4];              /*エラーコード*/
        char    disconnect_reason[2];       /*切断理由*/
    } connection_info;
} sc_info_def;

/*インターフェース・ステーション定義情報*/
#pragma fieldalign shared2 __st_conf_def
typedef struct __st_conf_def
{
    char    site_name;              /*プライマリーキー情報.サイト識別*/
    char    nw_name;                /*プライマリーキー情報.N/W識別*/
    char    group_name[5];          /*プライマリーキー情報.グループ識別*/
    char    interface_name[5];      /*プライマリーキー情報.インタフェース識別*/
    char    station_name[6];        /*プライマリーキー情報.ステーション識別*/

    char    nw_if[20];              /*N/W識別情報.インタフェース名*/
    char    nw_station[11];         /*N/W識別情報.ステーション名*/

    short   manage_unit;            /*管理単位情報.開局/閉局管理単位*/
    short   length_pos;             /*電文項目位置情報.データレングス開始位置*/
    short   length_size;            /*電文項目位置情報.データレングス項目長*/
    short   length_attr;            /*電文項目位置情報.データレングス項目属性*/
    short   length_id;              /*電文項目位置情報.データレングスINCLUDE識別*/
    short   text_pos;               /*電文項目位置情報.電文データ開始位置*/
    char    connect_after[2];       /*コネクション後処理情報.コネクション後処理種類*/
    short   connect_data_len;       /*特定データ長*/
    char    connect_data[50];       /*コネクション後処理情報.特定デー(変換して格納するため100/2=50)*/
    long    send_wait_timer;        /*send完了待ちタイマー*/
    long    recv_wait_timer;        /*残データ受信待ちタイマー*/
    long    idle_timer;             /*無通信監視タイマー*/
    long    short_retry_count;      /*ショートリトライカウント*/
    long    long_retry_count;       /*ロングリトライカウント*/
    long    short_retry_timer;      /*ショートリトライタイマー*/
    long    long_retry_timer;       /*ロングリトライタイマー*/
    short   validity_flag;          /*有効、無効フラグ*/
} st_conf_def;

/*リスナー定義情報*/
#pragma fieldalign shared2 __lc_conf_def
typedef struct __lc_conf_def
{
    char    site_name;                              /*サイト識別*/
    char    nw_name;                                /*N/W識別*/
    char    group_name[5];                          /*グループ識別*/
    char    lc_sc_sign[6];                          /*回線管理のコネクション識別*/
    char    interface_name[5];      /*プライマリーキー情報.インタフェース識別*/
    char    serverclass_logical_name[8];            /*サーバクラス論理KEY.サーバクラス論理ID.サーバクラス種類*/
    char    serverclass_logical_num[4];             /*サーバクラス論理KEY.サーバクラス論理ID.サーバクラス論理番号*/
    char    pathmon_name[16];                       /*サーバクラス情報.PATHMON名*/
    char    serverclass_name[15+1];                 /*サーバクラス情報.サーバクラス名*/
    short   process_name_len;                       /*リスナープロセス名長*/
    char    process_name[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1];   /*リスナープロセス名*/
    char    tcpip_name[6+1];                        /*TCP/IPプロセス名(socket_set_inet_nameで使用)*/
    char    local_ipaddr[15+1];                     /*ローカルIPアドレス*/
    char    local_port_no[5+1];                     /*ローカルポート番号*/
    char    validity_flag;                          /*有効、無効フラグ*/
} lc_conf_def;

/*リスナー管理情報*/
#pragma fieldalign shared2 __lc_info_def
typedef struct __lc_info_def
{
    char            site_name;          /*サイト識別*/
    char            nw_name;            /*N/W識別*/
    char            group_name[5];      /*グループ識別*/
    char            connect_id[6];      /*コネクション識別*/
    short           mng_no;             /*管理単位番号*/
    short           manage_count;       /*従属コネクション数*/
    short           station_index;      /*ステーションテーブルインデックス*/
    short           process_name_len;   /*リスナープロセス名長*/
    char            process_name[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1]; /*リスナープロセス名*/
} lc_info_def;

/*リスナープロセス管理情報*/
#pragma fieldalign shared2 __lp_info_def
typedef struct __lp_info_def
{
    char            site_name;                              /*サイト識別*/
    char            nw_name;                                /*N/W識別*/
    char            group_name[5];                          /*グループ識別*/
    char            pathmon_name[16];   /*サーバクラス情報.PATHMON名*/
    char            server_class[15+1]; /*サーバクラス情報.サーバクラス名*/

    short           listner_fd;         /*リスナープロセスファイル番号*/
    short           status;             /*処理状態  0：初期、1：オープン処理中、2：オープン済、3：クローズ処理中*/
    short           retry_type;         /*実施中のリトライタイプ、初期=0、ショートリトライ=1、ロングリトライ=2*/
    long            retry_timer_value;  /*リトライタイマー値*/
    long            retry_count;        /*リトライカウンター*/
    long            retry_max;          /*リトライ最大数*/
    char            *async_p;           /*非同期バッファーアドレス*/
    char            *buf_p;             /*バッファーアドレス*/
    short           timer_tag;          /*タイマータグ*/
    Event_List_def  send_wait;          /*送信待ちイベントリスト情報*/
    short           process_name_len;   /*リスナープロセス名長*/
    char            process_name[ZSYS_VAL_LEN_UNIQUEPROCESSNAME+1]; /*リスナープロセス名*/
    char            listner_name[ZSYS_VAL_LEN_PROCESSDESCR+1];  /*トレース用*/
} lp_info_def;

/*Inbound電文振分情報*/
#pragma fieldalign shared2 __ib_info_def
typedef struct __ib_info_def
{
    char    site_name;                  /*サイト識別*/
    char    nw_name;                    /*N/W識別*/
    char    group_name[5];              /*グループ識別*/
//    char    interface_name[5];          /*インタフェース識別*/
//    char    station_name[6];            /*ステーション識別*/
    short   validity_flag;              /*有効、無効フラグ*/
    char    pathmon_name[16];           /*サーバクラス情報.PATHMON名*/
    short   pathmon_name_len;           /*PATHMON名長*/
    char    serverclass_name[15+1];     /*サーバクラス情報.サーバクラス名*/
    short   serverclass_name_len;       /*サーバークラス名長*/
    short   search_index;               /*空きテーブルインデックス*/
    short   req_cnt;                    /*発行中のPATHSEND数*/
    struct                              /*PATHSENDテーブル*/
    {
        short   use_flag;               /*使用有無*/
        short   ps_len;                 /*PATHEND長*/
        char    *ps_buf_p;              /*PATHENDバッファポインター*/
        short   retry_cnt;              /*リトライ回数*/
        short   save_len;               /*退避バッファ使用長*/
        char    *save_p;                /*退避バッファポインター*/
        short   sc_thread;              /*電文を受け取ったコネクション管理*/
    } ps_req[DEF_MAX_INBOUND_PS];
    Event_List_def      send_wait;      /*送信待ちイベントリスト情報*/
} ib_info_def;

/*Outbound電文振分定義情報*/
#pragma fieldalign shared2 __ob_conf_def
typedef struct __ob_conf_def
{
    char    site_name;                          /*サイト識別*/
    char    nw_name;                            /*N/W識別*/
    char    group_name[5];                      /*グループ識別*/
//    char    interface_name[5];                  /*インタフェース識別*/
//    char    station_name[6];                    /*ステーション識別*/
    struct
    {
        struct
        {
            char    srv_cls_kind[8];            /*サーバクラス論理ID.サーバクラス種類*/
            char    srv_cls_num[4];             /*サーバクラス論理ID.サーバクラス論理番号*/
            char    srv_cls_redundan_num[4];    /*サーバクラス冗長化番号*/
        } srv_cls_id;                           /*サーバクラス論理ID*/
    } srv_cls_key;                              /*サーバクラス論理KEY*/
    char    pathmon_name[16];                   /*PATHMON名*/
    char    server_class[16];                   /*サーバクラス名*/
    char    process_name[ZSYS_VAL_LEN_FILENAME+1];  /*プロセス名*/
    short   process_name_len;                   /*プロセス名長(trace用)*/
    char    validity_flag;                      /*有効、無効フラグ*/
} ob_conf_def;

/*Outbound電文振分情報*/
#pragma fieldalign shared2 __ob_info_def
typedef struct __ob_info_def
{
    struct                                  /*PATHEND要求管理*/
    {
        short           reply_tag;          /*REPLYタグ*/
    } ps_manage;
    struct                                  /*IPC管理*/
    {
        short           ob_fd;              /*ファイル番号*/
        short           ob_status;          /*ステータス*/
        short           timer_tag;          /*タイマータグ*/
        char            *buf_p;             /*バッファーアドレス*/
        short           retry_type;         /*実施中のリトライタイプ、初期=0、ショートリトライ=1、ロングリトライ=2*/
        long            retry_timer_value;  /*リトライタイマー値*/
        long            retry_count;        /*リトライカウンター*/
        long            retry_max;          /*リトライ最大数*/
        Event_List_def  send_wait;          /*送信待ちイベントリスト情報*/
        char            outbound_name[ZSYS_VAL_LEN_PROCESSDESCR+1]; /*トレース用*/
        lk_trace_def    trs;                /*OPEN、CLOSEトレース取得用*/
    } ipc_mng;
    short               station_index;      /*ステーションテーブルインデックス*/
} ob_info_def;

/*コマンド管理テーブル*/
#pragma fieldalign shared2 __ci_info_def
typedef struct __ci_info_def
{
    struct
    {
        short   reply_tag;
    } cmd_rcv_manage;
    struct
    {
        char            pathmon_name[16];   /*PATHMON名*/
        short           pathmon_name_len;   /*PATHMON名*/
        char            server_class[16];   /*サーバクラス名*/
        short           server_class_len;   /*サーバクラス名*/
        Event_List_def  send_wait;          /*送信待ちイベントリスト情報*/
        char            *buf_p;             /*バッファーアドレス*/
    } ps_manage;
} ci_info_def;

/*コンフィグ情報テーブル*/
#pragma fieldalign shared2 __cf_def
typedef struct __cf_def
{
    short           sc_use;         /*コネクション定義使用数 テーブル使用数(1~200)*/
    sc_conf_def     *sc_conf;       /*コネクション定義情報 sc_confのポインター*/
    short           if_use;         /*インターフェース・ステーション管理数 テーブル使用数(1~80)*/
    st_conf_def     *st_conf;       /*インターフェース・ステーション管理情報 st_confのポインター*/
    char            nw_kubun[2];    /*NW識別情報.NW区分*/
    char            connect_num_mng_lyr;    /*コネクション数管理単位*/
    char            filler_1;       /*予備*/
    short           lc_use;         /*リスナー定義使用数 テーブル使用数(1~30)*/
    lc_conf_def     *lc_conf;       /*リスナー定義情報 lc_confのポインター*/
    short           ob_use;         /*Outbound電文振分定義使用数 テーブル使用数(1~100)*/
    ob_conf_def     *ob_conf;       /*Outbound電文振分定義情報 ob_confのポインター*/
} cf_def;

/*I/Oモジュール用インターフェース*/
#pragma fieldalign shared2 __fileio_def
typedef struct __fileio_def
{
    char                func_type[4];
    char                sub_prog_sts[2];
    COM_IOM_arg_3_def   arg3;
    COM_IOM_arg_4_def   arg4;
    COM_IOM_arg_5_def   arg5;
    COM_IOM_arg_6_def   arg6;
} fileio_def;

#endif /* CNSV_GLOBAL */
