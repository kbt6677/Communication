/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････ 共通ヘッダ・内部エラーコード定義            */
/*                                                                           */
/*        AUTHER            ････                                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ 2025-10-31                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  xxxxxxxx   2025/10/31 (J0680)新規作成                               */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef ERRCD_H
#define ERRCD_H

#define DEF_NERR_NOMAL                    "0000000"  /* 正常                                     */


/* A  : 共通 */
#define DEF_NERR_FILE_IO_ERR              "SCAA001"  /* ファイルIOエラー                         */
#define DEF_NERR_FILE_OPN_ERR             "SCAA002"  /* ファイルオープンエラー                   */
#define DEF_NERR_PROC_OPN_ERR             "SCAA003"  /* プロセスオープンエラー                   */

#define DEF_NERR_PSEND_TIMEOUT            "SCAA004"  /* Pathsendエラー(タイムアウト)             */
#define DEF_NERR_PSEND_ERR_RE_OK          "SCAA005"  /* Pathsendエラー(リトライ可)               */
#define DEF_NERR_PSEND_ERR_RE_NG          "SCAA006"  /* Pathsendエラー(リトライ不可)             */
#define DEF_NERR_PSEND_ERR_RE_OUT         "SCAA007"  /* Pathsendエラー(リトライアウト)           */

#define DEF_NERR_IPC_SEISA_ERR            "SCAA009"  /* IPC精査エラー                            */

#define DEF_NERR_PRM_RD_ERR               "SCAA016"  /* パラメータ読込エラー(必須パラメータ無し) */
#define DEF_NERR_PRM_RD_ERR_INV           "SCAA017"  /* パラメータ読込エラー(設定不正)           */

#define DEF_NERR_TMF_ERR                  "SCAA019"  /* TMFエラー                                */
#define DEF_NERR_SYSIF_LGC_ERR            "SCAA020"  /* プログラム/システム間論理矛盾            */

#define DEF_NERR_RE_READ_ERR              "SCAA021"  /* ファイル再読込み失敗                     */

#define DEF_NERR_LCN_GET_ERR              "SCAA022"  /* LCN取得エラー                            */
#define DEF_NERR_CMD_HAKKO_ERR            "SCAA023"  /* コマンド発行エラー                       */
#define DEF_NERR_ERRLOG_OUTPUT_ERR        "SCAA024"  /* エラーログ出力エラー                     */


/* B  : 伝送制御 */
#define DEF_NERR_SOCKET_GEN_ERR           "SCBA001"  /* SOCKET生成エラー                         */
#define DEF_NERR_SETSOCKOPT_ERR           "SCBA002"  /* SETSOCKOPTエラー                         */
#define DEF_NERR_BIND_ERR                 "SCBA003"  /* BINDエラー                               */
#define DEF_NERR_LISTEN_ERR               "SCBA004"  /* LISTENエラー                             */
#define DEF_NERR_ACCEPT_ERR               "SCBA005"  /* ACCEPTエラー                             */
#define DEF_NERR_ACCEPT2_ERR              "SCBA006"  /* ACCEPT2エラー                            */
#define DEF_NERR_CONNECT_ERR              "SCBA007"  /* CONNECTエラー                            */
#define DEF_NERR_SEND_ERR                 "SCBA008"  /* SENDエラー                               */
#define DEF_NERR_RCV_ERR                  "SCBA009"  /* RCVエラー                                */
#define DEF_NERR_SHT_RETRY_OVER           "SCBA010"  /* ショートリトライ回数オーバー             */
#define DEF_NERR_LNG_RETRY_OVER           "SCBA011"  /* ロングリトライ回数オーバー               */

#define DEF_NERR_CON_STS_CHK_ERR          "SCBD001"  /* コネクションステータスチェックエラー     */

/* BB : リスナー */
#define DEF_NERR_LSTN_DST_ADRS_CHK_ERR    "SCBB001"  /* 接続先アドレスチェックエラー             */
#define DEF_NERR_LSTN_CON_NUM_OVER        "SCBB002"  /* コネクション数オーバー検知               */
#define DEF_NERR_LSTN_ACC_RETRY_OVER      "SCBB003"  /* ACCEPTリトライオーバー                   */
#define DEF_NERR_LSTN_FREE_CON_NON        "SCBB004"  /* 空きコネクション無し                     */
#define DEF_NERR_LSTN_ALL_TBL_CON         "SCBB005"  /* 全コネクション接続済                     */


/* BC : コネクション制御クライアント */
#define DEF_NERR_CNCL_RES_XHAUST          "SCBC001" /* リソース割当てエラー                      */
#define DEF_NERR_CNCL_NO_SUCH_DST         "SCBC002" /* 送信先定義なし                            */
#define DEF_NERR_CNCL_NOT_CON             "SCBC003" /* コネクション選択エラー(未接続)            */
#define DEF_NERR_CNCL_TIMER_ERR           "SCBC004" /* タイマー発行エラー                        */
#define DEF_NERR_CNCL_CMD_ERR             "SCBC005" /* コマンドエラー完了                        */
#define DEF_NERR_CNCL_EVLST_BROKEN        "SCBC006" /* イベントリスト破損                        */

/* C  : 電文振分中継 */
/* CE : 電文振分(inbound) */
#define DEF_NERR_KC_DEC_ERR               "ECCE006"  /* 電文復号エラー(KC)                       */
#define DEF_NERR_KMAC_HANTE_ERR           "ECCE007"  /* 電文認証値判定エラー(KMAC)               */

#define DEF_NERR_RCV_DENBUN_HEADR_ERR     "ECCE011"  /* 受信電文ヘッダ精査エラー                 */
#define DEF_NERR_DENBUN_DEC_ERR           "ECCE012"  /* 電文復号処理エラー                       */
#define DEF_NERR_MTI_GET_ERR              "ECCE013"  /* MTI取得エラー                            */
#define DEF_NERR_FURIWAKE_DST_HANTE_ERR   "ECCE014"  /* 振分先判定エラー                         */
#define DEF_NERR_TO_FURIWAKE_FC2004_ERR   "ECCE015"  /* 東阪振分エラー(FC2004)                   */

/* CF : 電文振分(outbound) */
#define DEF_NERR_DST_SELECT_ERR           "SCCF001"  /* 送信先選択不可                           */
#define DEF_NERR_KC_ENC_ERR               "SCCE002"  /* 電文暗号化エラー(KC)                     */
#define DEF_NERR_KMAC_CALC_ERR            "SCCF002"  /* 電文認証値算出エラー(KMAC)               */
#define DEF_NERR_CON_SELECT_ERR           "SCCF005"  /* コネクション選択エラー                   */
#define DEF_NERR_DENBUN_ENC_ERR           "SCCF008"  /* 電文暗号化処理エラー                     */
#define DEF_NERR_CON_SEND_ERR             "SCCF009"  /* コネクション制御送信エラー               */

/* CH : 電文中継(GET) */
#define DEF_NERR_QGET_EXPIRE              "SCCH001"  /* 送信期限切れエラー                       */

/* D  : 制御電文機能 */
#define DEF_NERR_HSMK_REQ_SEISA_ERR       "SCDA001"  /* 被仕向要求精査エラー                     */
#define DEF_NERR_SMK_RSP_SEISA_ERR        "SCDA002"  /* 仕向応答精査エラー                       */

/* DA : 制御電文共通 */
#define DEF_NERR_SYS_NO_MAKE_ERR          "SCDA016"  /* システム採番生成失敗                     */
#define DEF_NERR_CTL_LOG_OUTPUT_ERR       "SCDA018"  /* 制御電文ログ出力エラー                   */

/* DI : 制御電文振分 */
#define DEF_NERR_CMSD_SMK_RSP_TIMEOUT     "ECDI001"  /* 仕向応答待ちタイムアウト                 */
#define DEF_NERR_CMSD_SMK_RSP_MCH_ERR     "ECDI002"  /* 仕向応答マッチングエラー                 */
#define DEF_NERR_CMSD_CTL_DISCARD         "ECDI003"  /* 制御電文破棄                             */
#define DEF_NERR_CMSD_CTL_SEND_ERR        "SCDI004"  /* 制御電文送信エラー                       */
#define DEF_NERR_CMSD_BITMAP_DEC_ERR      "ECDI005"  /* ビットマップ展開エラー                   */
#define DEF_NERR_CMSD_BITMAP_ENC_ERR      "SCDI006"  /* ビットマップ結合エラー                   */
#define DEF_NERR_CMSD_TIMER_ERR           "SCDI007"  /* タイマー発行エラー                       */
#define DEF_NERR_CMSD_TIMER_CNSL_ERR      "SCDI008"  /* タイマー解除エラー                       */

/* DJ : 局制御 */
#define DEF_NERR_HSMK_SST_OPNING          "ECDJ002"  /* 局状態エラー(被仕向開局処理中)           */
#define DEF_NERR_HSMK_SST_OPN             "ECDJ004"  /* 局状態エラー(被仕向開局)                 */
#define DEF_NERR_HSMK_SST_CLSING          "ECDJ006"  /* 局状態エラー(被仕向閉局処理中)           */
#define DEF_NERR_HSMK_SST_CLS             "ECDJ008"  /* 局状態エラー(被仕向閉局)                 */
#define DEF_NERR_HSMK_SST_ECHO            "ECDJ022"  /* 局状態エラー(被仕向エコー)               */
#define DEF_NERR_SMK_SST_OPNING           "ECDJ010"  /* 局状態エラー(仕向開局処理中)             */
#define DEF_NERR_SMK_SST_CLSING           "ECDJ016"  /* 局状態エラー(仕向閉局処理中)             */
#define DEF_NERR_SMK_SST_CLS              "ECDJ028"  /* 局状態エラー(仕向閉局)                   */
#define DEF_NERR_SMK_SST_ECHO             "ECDJ024"  /* 局状態エラー(仕向エコー)                 */

#define DEF_NERR_HSMK_REQ_SEISA           "ECDI001"  /* 被仕向要求精査エラー                     */
#define DEF_NERR_SMK_REQ_SEISA            "ECDI003"  /* 仕向要求精査エラー                       */
#define DEF_NERR_HSMK_CLS_REQ_SEISA       "ECDJ003"  /* 被仕向閉局要求精査エラー                 */
#define DEF_NERR_SMK_OPN_REQ_SEISA        "ECDJ009"  /* 仕向開局要求精査エラー                   */
#define DEF_NERR_SMK_OPN_STAT_CHK         "ECDJ010"  /* 仕向開局処理中の局状態チェックエラー     */
#define DEF_NERR_SMK_CLS_REQ_SEISA        "ECDJ015"  /* 仕向閉局要求精査エラー                   */
#define DEF_NERR_SMK_CLS_STAT_CHK         "ECDJ016"  /* 仕向閉局処理中の局状態チェックエラー     */
#define DEF_NERR_SMK_ECHO_STAT_CHK        "ECDJ024"  /* 仕向エコーテスト局状態チェックエラー     */

#define DEF_NERR_SIGN_ON_RETRY_OVER       "SCDJ032"  /* 自動開局要求送信リトライオーバー         */
#define DEF_NERR_SIGN_ON_RETRY_ERR        "SCDJ033"  /* 自動開局要求送信リトライエラー           */

/* DK : 鍵制御 */
#define DEF_NERR_KEY_SEND_ERR             "SCBA006"  /* SENDエラー                               */
#define DEF_NERR_HSMK_SEISA_ERR           "SCDK001"  /* 被仕向鍵交換要求精査エラー               */
#define DEF_NERR_HSMK_STATION_ST_ERR      "SCDK002"  /* 被仕向鍵交換局状態不正                   */
#define DEF_NERR_HSMK_DECODE_ERR          "SCDK003"  /* 被仕向鍵交換鍵復号エラー                 */
#define DEF_NERR_SMK_SEISA_ERR            "SCDK006"  /* 仕向鍵交換要求精査エラー                 */
#define DEF_NERR_SMK_STATION_ST_ERR       "SCDK007"  /* 仕向鍵交換局状態チェックエラー           */
#define DEF_NERR_SMK_KEY_ENC_ERR          "SCDK008"  /* 仕向鍵交換鍵生成エラー                   */
#define DEF_NERR_CHK_DIGIT_ERR            "SCDK013"  /* チェックデジットエラー                   */
#define DEF_NERR_MSG_MAKE_ERR             "SCDK014"  /* 電文作成失敗                             */
#define DEF_NERR_ATALLA_RSP_ERR           "SCDK017"  /* ATALLAレスポンスエラー                   */
#define DEF_NERR_OPPSITE_KEY_UPD_ERR      "SCDK018"  /* 他ノード鍵管理ファイル更新エラー         */

/* DL : カットオーバー */
#define DEF_NWM_CTO_CEN_STS_NG            "ECCK201"
#define DEF_NWM_CTO_MSG_FMT_NG            "ECCK200"


/* DN : 通知電文制御 */
#define DEF_NERR_NTC_RECV_SEISA           "SCDN001"  /* 障害電文通知精査エラー                   */
#define DEF_NERR_NTC_RESP_EDIT            "SCDN002"  /* 障害電文通知応答編集エラー               */
#define DEF_NERR_NTC_RECV                 "SCDN003"  /* 障害電文通知受信                         */
#define DEF_NERR_REQ_RECV                 "SCDN004"  /* 障害電文要求受信                         */

/* E  : 共通インタフェース機能 */
/* EO : ログ出力 */
#define DEF_NERR_LOGS_DUPLICATE_PKEY      "SCDN001"  /* プライマリーキー重複                     */
#define DEF_NERR_LOGS_DUPLICATE_A1KEY     "SCDN002"  /* オルタネートキー重複                     */
#define DEF_NERR_LOGS_LOG_FULL            "SCDN003"  /* ログファイルFULL                         */
#define DEF_NERR_LOGS_NO_UPDATES          "SCDN004"  /* 更新対象未存在                           */
#define DEF_NERR_LOGS_OTHER_ERR           "SCDN005"  /* ログ出力エラー                           */


/* F  : 運用サポート機能 */
/* FQ : コマンドサーバ */
#define DEF_NERR_CMD_ERR_DONE             "SCFQ003"  /* コマンドエラー完了                       */
#define DEF_NERR_CMD_TARGET_ERR           "SCFQ004"  /* コマンド対象指定エラー                   */
#define DEF_NERR_EXCEEDING_UPPER_LIMIT    "WCFQ001"  /* 照会最大数超過                           */


#endif /* ERRCD_H */
