/**
 * @brief common.h 共通ヘッダーファイル
 *
 * @date 2024/10/02 新規作成 by makino.shinnosuke
 *
*/

#ifndef _Common_H_
#define _Common_H_

/* 外部パラメータ */
#define DEF_SRV_LOGICAL_ID                  "SRV-LOGICAL-ID"            /* サーバクラス論理ID                       */
#define DEF_ENVIRONMENT_GROUP_ID            "ENVIRONMENT-GROUP-ID"      /* 自環境情報                               */
#define DEF_MSG_MON_NAME                    "MSG-MON-NAME"              /* 運用監視端末出力サーバ.PATHMON名         */
#define DEF_MSG_SRV_NAME                    "MSG-SRV-NAME"              /* 運用監視端末出力サーバ.サーバクラス名    */
#define DEF_FILE_IO_TIMER_10MSECOND         "FILE-IO-TIMER-10MSECOND"   /* ファイルI/Oタイマー                      */
#define DEF_PSEND_TIMER_10MSECOND           "PSEND-TIMER-10MSECOND"     /* PATHSENDタイマー                         */
#define DEF_PSEND_RETRY_CNT                 "PSEND-RETRY-CNT"           /* PATHSENDリトライ回数                     */
#define DEF_PM_TRACE_FLG                    "PM-TRACE-FLG"              /* トレースフラグ                           */
#define DEF_PM_MY_SERVERCLASS               "PM-MY-SERVERCLASS"         /* 自サーバクラス名                         */
#define DEF_DENBUN_EXPIRY_SECOND            "DENBUN-EXPIRY-SECOND"      /* 送信期限切れタイマー                     */
#define DEF_QUE_READ_TIMER_10MSECOND        "QUE-READ-TIMER-10MSECOND"  /* キューファイル読み込み完了待ちタイマー   */
#define DEF_SOCKET_IO_TIMER_10MSECOND       "SOCKET-IO-TIMER-10MSECOND" /* ソケットI/Oタイマ                        */
#define DEF_PROC_IO_TIMER_10MSECOND         "PROC-IO-TIMER-10MSECOND"   /* プロセスW/Rタイマー                      */

/* ASSIGN名 */
#define DEF_ASN_GFPHI                       "GFPHI"                     /* 物理名情報ファイル                       */
#define DEF_ASN_TRACE                       "ALTRC"                     /* トレースファイル                         */

/* 直前エコーテスト結果 */
#define DEF_LAST_ECHO_OK                    "OK"        /* 正常                                     */
#define DEF_LAST_ECHO_KYOHI_RCV             "E1"        /* 異常（拒否応答受信）                     */
#define DEF_LAST_ECHO_TIMEOUT               "E2"        /* 異常（タイムアウト）                     */
#define DEF_LAST_ECHO_KYOHI_SEND            "E3"        /* 異常（拒否応答送信）                     */
#define DEF_LAST_ECHO_SEND_ERR              "E4"        /* 異常（送信不可）                         */

/* 鍵種類 */
#define DEF_KEY_TYPE_KPE                    "KPE "      /* KPE                                      */
#define DEF_KEY_TYPE_KC                     "KC  "      /* KC                                       */
#define DEF_KEY_TYPE_KMAC                   "KMAC"      /* KMAC                                     */

/* 鍵使用アルゴリズム */
#define DEF_KEY_USE_ALG_DES1                "DS1"       /* Single-DES                               */
#define DEF_KEY_USE_ALG_DES2                "DS2"       /* Triple-DES(2KEY)                         */
#define DEF_KEY_USE_ALG_DES3                "DS3"       /* Triple-DES(3KEY)                         */

/* 最新キーインデックス */
#define DEF_NEW_KEY_INDEX01                 "01"        /* キー情報(1)が最新                        */
#define DEF_NEW_KEY_INDEX02                 "02"        /* キー情報(2)が最新                        */

/* 回線運用識別 */
#define DEF_OPERATION_ID_ACTIVE             " "         /* 通常回線(ACTIV)）                        */
#define DEF_OPERATION_ID_BACKUP             "B"         /* DR回線(BACKUP)）                         */
#define DEF_OPERATION_ID_PRE                "P"         /* サービスイン前(PRE)                      */

/* コネクション状態 */
#define DEF_CONNECT_STS_DISCONN             "DC"        /* 切断                                     */
#define DEF_CONNECT_STS_LISTEN              "LS"        /* 接続待ち                                 */
#define DEF_CONNECT_STS_CONNECT             "CN"        /* 接続                                     */
#define DEF_CONNECT_STS_RECONNECT           "RE"        /* 再接続処理中                             */

/* プロセス状態 */
#define DEF_PROC_STS_OPN                    "OP"        /* 回線オープンコマンド受信済み             */
#define DEF_PROC_STS_CLS                    "CL"        /* 回線クローズコマンド受信済み             */

/* 切断理由 */
#define DEF_DISCON_BY_CLS_CMD               "00"        /* クローズコマンドによる切断               */
#define DEF_DISCON_BY_CLS_INSTRUCT          "10"        /* 切断指示/入替指示インタフェースによる切断*/
#define DEF_DISCON_BY_DETECT                "91"        /* コネクション切断検出                     */
#define DEF_DISCON_BY_RE_CON_OVER           "92"        /* コネクション再接続リトライオーバ         */

/* 再接続ステータス */
#define DEF_RE_CONNECT_STS_ON               "ON"        /* 再接続処理中                             */
#define DEF_RE_CONNECT_STS_OFF              "OF"        /* 通常状態（再接続処理をしていない状態）   */

/* 局状態 */
#define DEF_STTE_STS_OPN                    "10"        /* 開局                                     */
#define DEF_STTE_STS_CLS                    "90"        /* 閉局                                     */
#define DEF_STTE_STS_OPNING                 "11"        /* 開局処理中                               */
#define DEF_STTE_STS_CLOSING                "91"        /* 閉局処理中                               */

/* データ長形式 */
#define DEF_DATA_LEN_TYPE_CNS               '0'         /* 固定長(データ長フィールドなし)           */
#define DEF_DATA_LEN_TYPE_L1                '1'         /* 可変長(L)                                */
#define DEF_DATA_LEN_TYPE_L2                '2'         /* 可変長(LL)                               */
#define DEF_DATA_LEN_TYPE_L3                '3'         /* 可変長(LLL)                              */
#define DEF_DATA_LEN_TYPE_L4                '4'         /* 可変長(LLLL)                             */

/* データ長属性 */
#define DEF_DATA_LEN_ATTR_BIN1              'N'         /* バイナリ                                 */
#define DEF_DATA_LEN_ATTR_BCD1              'B'         /* BCD                                      */
#define DEF_DATA_LEN_ATTR_ASC1              'A'         /* ASCII                                    */
#define DEF_DATA_LEN_ATTR_EBC1              'E'         /* EBCDIC                                   */

/* データ部属性 */
#define DEF_DATA_AREA_ATTR_BIN              'N'         /* バイナリ                                 */
#define DEF_DATA_AREA_ATTR_BCD              'B'         /* BCD                                      */
#define DEF_DATA_AREA_ATTR_ASC              'A'         /* ASCII                                    */
#define DEF_DATA_AREA_ATTR_EBC              'E'         /* EBCDIC                                   */
#define DEF_DATA_AREA_ATTR_JIS              'J'         /* JIS8                                     */

/* コード変換要否 */
#define DEF_CODE_CHANGE_NEED_OFF            0           /* 不要                                     */
#define DEF_CODE_CHANGE_NEED_ON             1           /* 要                                       */

/* サイト識別 */
#define DEF_SITE_ID_TKY                     'E'         /* 東京                                     */
#define DEF_SITE_ID_OSK                     'W'         /* 大阪                                     */

/* N/W識別 */
#define DEF_NW_ID_GFP                       'G'         /* GFP                                      */
#define DEF_NW_ID_VISA                      'V'         /* VISA                                     */
#define DEF_NW_ID_MASTER                    'M'         /* MasterCard                               */
#define DEF_NW_ID_AMEX                      'A'         /* AMEX                                     */
#define DEF_NW_ID_DISCOVER                  'D'         /* DISCOVER                                 */
#define DEF_NW_ID_JCN                       'C'         /* JCN(CUP)                                 */
#define DEF_NW_ID_NYCE                      'N'         /* NYCE                                     */
#define DEF_NW_ID_JLink                     'L'         /* J-Link                                   */
#define DEF_NW_ID_UnionPay                  'U'         /* UnionPay                                 */

/* 有効無効フラグ */
#define DEF_INVALID_FLG_OFF                 ' '         /* 有効なレコード                           */
#define DEF_INVALID_FLG_ON                  '1'         /* 無効なレコード                           */

/* 仕向・被仕向区分 */
#define DEF_S_H_KUBUN_SIMUKE                'S'         /* 仕向                                     */
#define DEF_S_H_KUBUN_HISIMUKE              'H'         /* 被仕向                                   */

/* 取引ステータス */
#define DEF_TRHK_STS_REQ_SEND               "QS"        /* 要求電文送信済み（応答電文待ち）         */
#define DEF_TRHK_STS_RSP_RCV                "PR"        /* 応答電文受信済み                         */
#define DEF_TRHK_STS_RSP_TIMEOUT            "TO"        /* 応答電文待ちタイムアウト                 */
#define DEF_TRHK_STS_REQ_SENDERR            "QE"        /* 要求電文送信不可                         */
#define DEF_TRHK_STS_NTF_SEND               "NS"        /* 通知電文送信済み                         */
#define DEF_TRHK_STS_RSP_SEND               "PS"        /* 要求電文受信→応答電文送信済み           */
#define DEF_TRHK_STS_RSP_SENDERR            "PE"        /* 要求電文受信→応答電文送信不可           */
#define DEF_TRHK_STS_NTF_RCV                "NR"        /* 通知電文受信済み                         */

/* 開局/閉局管理単位 */
#define DEF_OPN_CLS_MNG_LYR_IF              'I'         /* インタフェース単位                       */
#define DEF_OPN_CLS_MNG_LYR_ST              'S'         /* ステーション単位                         */
#define DEF_OPN_CLS_MNG_LYR_CO              'C'         /* コネクション単位                         */

/* エコーテスト管理単位 */
#define DEF_ECHO_TEST_MNG_LYR_IF            'I'         /* インタフェース単位                       */
#define DEF_ECHO_TEST_MNG_LYR_ST            'S'         /* ステーション単位                         */
#define DEF_ECHO_TEST_MNG_LYR_CO            'C'         /* コネクション単位                         */

/* 鍵交換管理単位 */
#define DEF_KEY_CHANGE_MNG_LYR_IF           'I'         /* インタフェース単位                       */
#define DEF_KEY_CHANGE_MNG_LYR_ST           'S'         /* ステーション単位                         */
#define DEF_KEY_CHANGE_MNG_LYR_NO           ' '         /* 鍵交換無し                               */

/* カットオーバー管理単位 */
#define DEF_CUT_OVER_MNG_LYR_IF             'I'         /* インタフェース単位                       */
#define DEF_CUT_OVER_MNG_LYR_ST             'S'         /* ステーション単位                         */
#define DEF_CUT_OVER_MNG_LYR_NO             ' '         /* カットオーバー無し                       */

/* SAF送信管理単位 */
#define DEF_SAF_SEND_MNG_LYR_IF             'I'         /* インタフェース単位                       */
#define DEF_SAF_SEND_MNG_LYR_ST             'S'         /* ステーション単位                         */
#define DEF_SAF_SEND_MNG_LYR_NO             ' '         /* SAF送信無し                              */

/* 障害電文通知単位 */
#define DEF_SHOGAI_NTF_MNG_LYR_IF           'I'         /* インタフェース単位                       */
#define DEF_SHOGAI_NTF_MNG_LYR_ST           'S'         /* ステーション単位                         */
#define DEF_SHOGAI_NTF_MNG_LYR_NO           ' '         /* 障害電文通知無し                         */

/* コネクション数管理単位 */
#define DEF_CONNECT_NUM_MNG_LYR_IF          'I'         /* インタフェース単位                       */
#define DEF_CONNECT_NUM_MNG_LYR_ST          'S'         /* ステーション単位                         */
#define DEF_CONNECT_NUM_MNG_LYR_NO          ' '         /* 管理対象外                               */

/* データレングス項目属性 */
#define DEF_DATA_LEN_ATTR_BIN3              "BIN"       /* Binary                                   */
#define DEF_DATA_LEN_ATTR_BCD3              "BCD"       /* BCD                                      */
#define DEF_DATA_LEN_ATTR_ASC3              "ASC"       /* ASCII                                    */
#define DEF_DATA_LEN_ATTR_EBC3              "EBC"       /* EBCDIC                                   */

/* データレングスINCLUDE識別 */
#define DEF_DATA_LEN_INCLUDE_IN             'I'         /* 含める                                   */
#define DEF_DATA_LEN_INCLUDE_OUT            'O'         /* 含めない                                 */

/* MTI項目属性 */
#define DEF_MTI_ITEM_ATTR_BCD               "BCD"       /* BCD                                      */
#define DEF_MTI_ITEM_ATTR_ASC               "ASC"       /* ASCII                                    */
#define DEF_MTI_ITEM_ATTR_EBC               "EBC"       /* EBCDIC                                   */

/* コネクション後処理種類 */
#define DEF_CONNECT_NEXT_OPN_SEND           "SO"        /* 開局電文を送信                           */
#define DEF_CONNECT_NEXT_DATA_SEND          "DT"        /* 特定データを送信                         */
#define DEF_CONNECT_NEXT_NO                 "  "        /* コネクション確立後の処理が無い           */

/* コネクション数管理種類 */
#define DEF_CONNECT_NUM_MNG_KIND_A          'A'         /* コネクション数オーバー・接続要求を拒否   */
#define DEF_CONNECT_NUM_MNG_KIND_B          'B'         /* コネクション数オーバー・他すべてを切断   */

/* 送信先再選択要否 */
#define DEF_DST_RE_SELECT_NEED_OFF          '0'         /* 再選択不要                               */
#define DEF_DST_RE_SELECT_NEED_ON           '1'         /* 再選択要                                 */

/* Active/Standby区分 */
#define DEF_ACT_STB_ID_ACT                  'A'         /* Active                                   */
#define DEF_ACT_STB_ID_STB                  'S'         /* Standby                                  */

/* モードフラグ */
#define DEF_MODE_FLG_OFF                    '0'         /* 本番モード                               */
#define DEF_MODE_FLG_ON                     '1'         /* 試験モード                               */

/* 動作モード */
#define DEF_MOVE_MODE_ACQ                   'A'         /* GFP as Acquirer                          */
#define DEF_MOVE_MODE_ISS                   'I'         /* GFP as Issuer                            */

/* Administrative電文応答有無 */
#define DEF_ADMN_DENBUN_RSP_OFF             '0'         /* 応答無し                                 */
#define DEF_ADMN_DENBUN_RSP_ON              '1'         /* 応答有り                                 */

/* Redsys識別 */
#define DEF_REDSYS_ID_OFF                   '0'         /* Redsys以外                               */
#define DEF_REDSYS_ID_ON                    '1'         /* Redsys                                   */

/* 東阪振分区分 */
#define DEF_TKY_OSK_ALL                     '1'         /* 常に東阪振分の対象                       */
#define DEF_TKY_OSK_ERR                     '2'         /* エラー時のみ東阪振分の対象               */
#define DEF_TKY_OSK_OTH                     '3'         /* 東阪振分の対象外                         */

/* 受信電文振分・電文種別 */
#define DEF_DENBUN_GYOM_REQ                 '1'         /* 業務要求                                 */
#define DEF_DENBUN_GYOM_RSP                 '2'         /* 業務応答                                 */
#define DEF_DENBUN_CTRL_REQ                 '3'         /* 制御要求                                 */
#define DEF_DENBUN_CTRL_RSP                 '4'         /* 制御応答                                 */

/* SAF送信状態 */
#define DEF_SAF_SEND_STS_ON_REQ             "10"        /* SAF送信開始依頼中(ON-REQ)                */
#define DEF_SAF_SEND_STS_ON                 "11"        /* SAF送信開始状態(ON)                      */
#define DEF_SAF_SEND_STS_OFF_REQ            "20"        /* SAF送信停止依頼中(OFF-REQ)               */
#define DEF_SAF_SEND_STS_OFF                "21"        /* SAF送信停止状態(OFF)                     */

/* 外部NW用キューファイル・処理コード区分 */
#define DEF_SHORI_KUBUN_RECV                "00"        /* 通常電文受信                             */
#define DEF_SHORI_KUBUN_FAULTTEXT           "01"        /* 障害電文作成依頼                         */
#define DEF_SHORI_KUBUN_SENDERROR           "09"        /* 電文送信不可応答                         */
#define DEF_SHORI_KUBUN_SEND                "10"        /* 電文送信依頼                             */
#define DEF_SHORI_KUBUN_RETURN              "11"        /* 業務折返し                               */
#define DEF_SHORI_KUBUN_TIMEOUT             "12"        /* T/O通知                                  */

/* 受信時局状態 */
#define DEF_RCV_KYOKU_STS_OPN               '0'         /* 開局                                     */
#define DEF_RCV_KYOKU_STS_OPNING            '1'         /* 開局処理中                               */
#define DEF_RCV_KYOKU_STS_CLSING            '8'         /* 閉局処理中                               */
#define DEF_RCV_KYOKU_STS_CLS               '9'         /* 閉局                                     */

/* NW区分 */
#define DEF_NW_KUBUN_JLINK8                 "J8"        /* J-LINK8                                  */
#define DEF_NW_KUBUN_JLINK9                 "J9"        /* J-LINK9                                  */
#define DEF_NW_KUBUN_CARDNET                "CA"        /* CARDNET                                  */
#define DEF_NW_KUBUN_DISCOVER               "DI"        /* Discover                                 */
#define DEF_NW_KUBUN_NYCE                   "NY"        /* NYCE                                     */
#define DEF_NW_KUBUN_AEGN                   "AX"        /* AEGEN                                    */
#define DEF_NW_KUBUN_BANKNET                "BK"        /* Banknet                                  */
#define DEF_NW_KUBUN_VISANET                "VI"        /* Visanet                                  */
#define DEF_NW_KUBUN_UNIONPAY               "UP"        /* UnionPay                                 */
#define DEF_NW_KUBUN_JLINK                  "JL"        /* J-Link(通信受信時)                       */

/* 送信電文種別 */
#define DEF_SEND_DENBUN_REQ                 '1'         /* 業務要求                                 */
#define DEF_SEND_DENBUN_RSP                 '2'         /* 業務応答                                 */
#define DEF_SEND_DENBUN_CTR_REQ             '3'         /* 制御要求                                 */
#define DEF_SEND_DENBUN_CTR_RSP             '4'         /* 制御応答                                 */
#define DEF_SEND_DENBUN_HB                  '5'         /* ハートビート                             */
#define DEF_SEND_DENBUN_OTH                 '9'         /* 破棄通知                                 */

/* 電文フォーマット区分 */
#define DEF_DENBUN_FMT_8583                 '0'         /* 8583フォーマット                         */

/* 送受信識別 */
#define DEF_REQ_SEND                        '1'         /* 要求電文送信                             */
#define DEF_RSP_RCV                         '2'         /* 応答電文受信/タイムアウト                */
#define DEF_REQ_RCV                         '1'         /* 要求電文受信                             */
#define DEF_RSP_SEND                        '2'         /* 応答電文送信/応答電文送信不可            */
#define DEF_DST_RCV                         '1'         /* 接続先からの受信電文                     */
#define DEF_DST_SEND                        '2'         /* 接続先への送信電文                       */

/* 電文情報有無 */
#define DEF_DENBUN_IFO_OFF                  '0'         /* 電文情報無し                             */
#define DEF_DENBUN_IFO_ON                   '1'         /* 電文情報有り                             */

/* メッセージ種別 */
#define DEF_MSG_KYOHI                       "RJ"        /* リジェクト                               */
#define DEF_MSG_HEARTBEAT                   "HB"        /* ハートビート                             */
#define DEF_MSG_IDLE                        "00"        /* アイドル                                 */
#define DEF_MSG_OTHER                       "ZZ"        /* その他                                   */

/* 東阪振分情報・振分先 */
#define DEF_FURI_DST_TKY                    'E'         /* 東京                                     */
#define DEF_FURI_DST_OSK                    'W'         /* 大阪                                     */

/* 振分先選択処理区分 */
#define DEF_FURI_ALL_OK                     "10"        /* 常に東阪振分の対象電文(正常)             */
#define DEF_FURI_ALL_UKI                    "11"        /* 常に東阪振分の対象電文(迂回)             */
#define DEF_FURI_ONLY_ERR_OK                "20"        /* エラー時のみ東阪振分の対象電文(正常)     */
#define DEF_FURI_ONLY_ERR_UKI               "21"        /* エラー時のみ東阪振分の対象電文(迂回)     */
#define DEF_FURI_NA                         "30"        /* 東阪振分の対象外電文                     */

/* 受信電文振分先設定ファイル MTI(特殊) */
#define DEF_GFQSW_MTI_CONTROL               "YYY1"      /* 制御電文機能で処理                       */
#define DEF_GFQSW_MTI_RETURN                "YYY2"      /* 電文振分処理で折返し応答                 */
#define DEF_GFQSW_MTI_ILLEGAL               "ZZZZ"      /* MTI不正                                  */

/* エラー電文識別 */
#define DEF_ELG_NOTICE                      '1'         /* 接続先から受信したエラー電文通知         */
#define DEF_ELG_INTERNAL                    '2'         /* 内部エラー検知                           */

/* EMSリンケージ情報 */
#define DEF_EMS_SYSNM_GFP                   "GFP"       /* EMSリンケージ情報.EMS出力情報.業務共通メッセージ.システム名 */
#define DEF_EMS_SRV_KBN_COM                 "COM"       /* EMSリンケージ情報.EMS出力情報.業務共通メッセージ.SERVER分類 */

/* ファイルID */
#define DEF_GFLIN                           "GFLIN"     /* 回線管理ファイル                         */
#define DEF_GCLST                           "GCLST"     /* 回線ステータスファイル                   */
#define DEF_GCSST                           "GCSST"     /* 局状態管理ファイル                       */
#define DEF_GCEST                           "GCEST"     /* エコー状態管理ファイル                   */
#define DEF_GCCUT                           "GCCUT"     /* カット対象日付管理ファイル               */
#define DEF_GLNLG                           "GLNLG"     /* N/W通信ログ                              */
#define DEF_GLELG                           "GLELG"     /* エラー出力ログ                           */
#define DEF_GLMLG                           "GLMLG"     /* 制御電文ログ                             */
#define DEF_GFMTL                           "GFMTL"     /* 制御電文管理ファイル                     */
#define DEF_GCKEY                           "GCKEY"     /* 鍵管理ファイル                           */
#define DEF_GFNSW                           "GFNSW"     /* 東阪振分比率設定ファイル                 */
#define DEF_GFQSW                           "GFQSW"     /* 受信電文振分先設定ファイル               */
#define DEF_GFNWI                           "GFNWI"     /* N/W情報ファイル                          */
#define DEF_GFPHI                           "GFPHI"     /* 物理名情報ファイル                       */
#define DEF_GCSAF                           "GCSAF"     /* SAF送信状態管理ファイル                  */
#define DEF_GCSCN                           "GCSCN"     /* 受信コネクション数管理ファイル           */
#define DEF_GFELI                           "GFELI"     /* 制御電文エレメント情報ファイル           */
#define DEF_GFQBK                           "GFQBK"     /* 送信電文折返し振分先設定ファイル         */
#define DEF_GFNWS                           "GFNWS"     /* 接続先固有情報ファイル                   */

/* プログラムID */
#define DEF_GFPCVX00                        "GFPCVX00"  /* リスナー                                 */
#define DEF_GFPCVX10                        "GFPCVX10"  /* コネクション制御(サーバ)                 */
#define DEF_GFPCVX20                        "GFPCVX20"  /* コネクション制御(クライアント)           */
#define DEF_GFPCVX30                        "GFPCVX30"  /* 電文振分(inbound)                        */
#define DEF_GFPCVX40                        "GFPCVX40"  /* 電文振分(outbound)                       */
#define DEF_GFPCVX50                        "GFPCVX50"  /* 電文中継(PUT)                            */
#define DEF_GFPCVX60                        "GFPCVX60"  /* 電文中継(GET)                            */
#define DEF_GFPCVX70                        "GFPCVX70"  /* 制御電文振分                             */
#define DEF_GFPCVX80                        "GFPCVX80"  /* 局状態・エコー制御                       */
#define DEF_GFPCVX90                        "GFPCVX90"  /* 鍵交換制御                               */
#define DEF_GFPCVXA0                        "GFPCVXA0"  /* カットオーバー制御                       */
#define DEF_GFPCVXB0                        "GFPCVXB0"  /* SAF送信制御                              */
#define DEF_GFPCVXC0                        "GFPCVXC0"  /* 通知電文制御                             */
#define DEF_GFPCVXD0                        "GFPCVXD0"  /* コマンドサーバ                           */
#define DEF_GFPCRXE0                        "GFPCRXE0"  /* コマンドI/F                              */
#define DEF_GFPCVXF0                        "GFPCVXF0"  /* ログ出力                                 */
#define DEF_GFPCVXG0                        "GFPCVXG0"  /* LCN採番                                  */
#define DEF_GFPCVXH0                        "GFPCVXH0"  /* 鍵管理ファイルIO                         */
#define DEF_GFPOVX04                        "GFPOVX04"  /* システム採番・生成                       */
#define DEF_GFPOVX01                        "GFPOVX01"  /* タイマー制御                             */
#define DEF_BCRFS010                        "BCRFS010"  /* ATALLA振分                               */
#define DEF_GFPVO0M0                        "GFPVO0M0"  /* 運用監視端末出力                         */
#define DEF_GFPOGGZ1                        "GFPOGGZ1"  /* メッセージ出力                           */
#define DEF_GFPOGGZ4                        "GFPOGGZ4"  /* トレース出力                             */
#define DEF_GFPCGX20                        "GFPCGX20"  /* BITMAP展開                               */
#define DEF_GFPCGX30                        "GFPCGX30"  /* BITMAP組立                               */
#define DEF_GFPCGX40                        "GFPCGX40"  /* PATHSEND処理                             */
#define DEF_GFPCGX50                        "GFPCGX50"  /* システム日時取得                         */
#define DEF_GFPCGX60                        "GFPCGX60"  /* I/Oタグ生成                              */
#define DEF_GFPCGX70                        "GFPCGX70"  /* I/Oタグ解析                              */
#define DEF_GFPCGX80                        "GFPCGX80"  /* エラー出力ログ編集出力                   */
#define DEF_GFPCGX90                        "GFPCGX90"  /* ATALLAコマンド生成・解析                 */
#define DEF_GFPCGXA0                        "GFPCGXA0"  /* トランザクション管理                     */
#define DEF_GFPCGXB0                        "GFPCGXB0"  /* I/Oモジュール                            */
#define DEF_GFPCGXC0                        "GFPCGXC0"  /* オープナープロセス管理                   */
#define DEF_GFPCGXD0                        "GFPCGXD0"  /* ASSIGN情報取得                           */
#define DEF_GFPCGXE0                        "GFPCGXE0"  /* ユニーク日時取得                         */
#define DEF_GFPCGXF0                        "GFPCGXF0"  /* 日時変換                                 */
#define DEF_GFPOGGZ3                        "GFPOGGZ3"  /* コード変換                               */
#define DEF_GFPCSX00                        "GFPCSX00"  /* MTI取得対象電文判定[ダミー]              */
#define DEF_GFPCSV00                        "GFPCSV00"  /* MTI取得対象電文判定[VisaNet]             */
#define DEF_GFPCSU00                        "GFPCSU00"  /* MTI取得対象電文判定[UnionPay]            */
#define DEF_GFPCSX10                        "GFPCSX10"  /* 暗号・復号処理[ダミー]                   */
#define DEF_GFPCSJ10                        "GFPCSJ10"  /* 暗号・復号処理[CARDNET]                  */
#define DEF_GFPCSX20                        "GFPCSX20"  /* 暗号・復号処理用初期処理[ダミー]         */
#define DEF_GFPCSJ20                        "GFPCSJ20"  /* 暗号・復号処理用初期処理[CARDNET]        */
#define DEF_GFPCSX30                        "GFPCSX30"  /* 電文ヘッダ編集[ダミー]                   */
#define DEF_GFPCSJ30                        "GFPCSJ30"  /* 電文ヘッダ編集[CARDNET]                  */
#define DEF_GFPCSV30                        "GFPCSV30"  /* 電文ヘッダ編集[VisaNet]                  */
#define DEF_GFPCSU30                        "GFPCSU30"  /* 電文ヘッダ編集[UnionPay]                 */
#define DEF_GFPCS930                        "GFPCS930"  /* 電文ヘッダ編集[J-Link]                   */
#define DEF_GFPCSJ40                        "GFPCSJ40"  /* 電文種別判定[CARDNET]                    */
#define DEF_GFPCSV40                        "GFPCSV40"  /* 電文種別判定[VisaNet]                    */
#define DEF_GFPCSB40                        "GFPCSB40"  /* 電文種別判定[Banknet]                    */
#define DEF_GFPCSA40                        "GFPCSA40"  /* 電文種別判定[AEGN]                       */
#define DEF_GFPCSN40                        "GFPCSN40"  /* 電文種別判定[NYCE]                       */
#define DEF_GFPCSU40                        "GFPCSU40"  /* 電文種別判定[UnionPay]                   */
#define DEF_GFPCS940                        "GFPCS940"  /* 電文種別判定[J-Link]                     */
#define DEF_GFPCSD40                        "GFPCSD40"  /* 電文種別判定[Discover]                   */
#define DEF_GFPCSJ50                        "GFPCSJ50"  /* 要求応答マッチングキー生成[CARDNET]      */
#define DEF_GFPCSV50                        "GFPCSV50"  /* 要求応答マッチングキー生成[VisaNet]      */
#define DEF_GFPCSB50                        "GFPCSB50"  /* 要求応答マッチングキー生成[Banknet]      */
#define DEF_GFPCSA50                        "GFPCSA50"  /* 要求応答マッチングキー生成[AEGN]         */
#define DEF_GFPCSN50                        "GFPCSN50"  /* 要求応答マッチングキー生成[NYCE]         */
#define DEF_GFPCSU50                        "GFPCSU50"  /* 要求応答マッチングキー生成[UnionPay]     */
#define DEF_GFPCS950                        "GFPCS950"  /* 要求応答マッチングキー生成[J-Link]       */
#define DEF_GFPCSD50                        "GFPCSD50"  /* 要求応答マッチングキー生成[Discover]     */
#define DEF_GFPCSJ60                        "GFPCSJ60"  /* 開局・閉局電文精査[CARDNET]              */
#define DEF_GFPCSV60                        "GFPCSV60"  /* 開局・閉局電文精査[VisaNet]              */
#define DEF_GFPCSB60                        "GFPCSB60"  /* 開局・閉局電文精査[Banknet]              */
#define DEF_GFPCSA60                        "GFPCSA60"  /* 開局・閉局電文精査[AEGN]                 */
#define DEF_GFPCSN60                        "GFPCSN60"  /* 開局・閉局電文精査[NYCE]                 */
#define DEF_GFPCSU60                        "GFPCSU60"  /* 開局・閉局電文精査[UnionPay]             */
#define DEF_GFPCS960                        "GFPCS960"  /* 開局・閉局電文精査[J-Link]               */
#define DEF_GFPCSD60                        "GFPCSD60"  /* 開局・閉局電文精査[Discover]             */
#define DEF_GFPCSJ70                        "GFPCSJ70"  /* 開局・閉局電文編集[CARDNET]              */
#define DEF_GFPCSV70                        "GFPCSV70"  /* 開局・閉局電文編集[VisaNet]              */
#define DEF_GFPCSB70                        "GFPCSB70"  /* 開局・閉局電文編集[Banknet]              */
#define DEF_GFPCSA70                        "GFPCSA70"  /* 開局・閉局電文編集[AEGN]                 */
#define DEF_GFPCSN70                        "GFPCSN70"  /* 開局・閉局電文編集[NYCE]                 */
#define DEF_GFPCSU70                        "GFPCSU70"  /* 開局・閉局電文編集[UnionPay]             */
#define DEF_GFPCS970                        "GFPCS970"  /* 開局・閉局電文編集[J-Link]               */
#define DEF_GFPCSD70                        "GFPCSD70"  /* 開局・閉局電文編集[Discover]             */
#define DEF_GFPCSJ80                        "GFPCSJ80"  /* エコーテスト電文精査[CARDNET]            */
#define DEF_GFPCSV80                        "GFPCSV80"  /* エコーテスト電文精査[VisaNet]            */
#define DEF_GFPCSB80                        "GFPCSB80"  /* エコーテスト電文精査[Banknet]            */
#define DEF_GFPCSA80                        "GFPCSA80"  /* エコーテスト電文精査[AEGN]               */
#define DEF_GFPCSN80                        "GFPCSN80"  /* エコーテスト電文精査[NYCE]               */
#define DEF_GFPCSU80                        "GFPCSU80"  /* エコーテスト電文精査[UnionPay]           */
#define DEF_GFPCS980                        "GFPCS980"  /* エコーテスト電文精査[J-Link]             */
#define DEF_GFPCSD80                        "GFPCSD80"  /* エコーテスト電文精査[Discover]           */
#define DEF_GFPCSJ90                        "GFPCSJ90"  /* エコーテスト電文編集[CARDNET]            */
#define DEF_GFPCSV90                        "GFPCSV90"  /* エコーテスト電文編集[VisaNet]            */
#define DEF_GFPCSB90                        "GFPCSB90"  /* エコーテスト電文編集[Banknet]            */
#define DEF_GFPCSA90                        "GFPCSA90"  /* エコーテスト電文編集[AEGN]               */
#define DEF_GFPCSN90                        "GFPCSN90"  /* エコーテスト電文編集[NYCE]               */
#define DEF_GFPCSU90                        "GFPCSU90"  /* エコーテスト電文編集[UnionPay]           */
#define DEF_GFPCS990                        "GFPCS990"  /* エコーテスト電文編集[J-Link]             */
#define DEF_GFPCSD90                        "GFPCSD90"  /* エコーテスト電文編集[Discover]           */
#define DEF_GFPCSJA0                        "GFPCSJA0"  /* 鍵交換電文精査[CARDNET]                  */
#define DEF_GFPCSAA0                        "GFPCSAA0"  /* 鍵交換電文精査[AEGN]                     */
#define DEF_GFPCSNA0                        "GFPCSNA0"  /* 鍵交換電文精査[NYCE]                     */
#define DEF_GFPCSUA0                        "GFPCSUA0"  /* 鍵交換電文精査[UnionPay]                 */
#define DEF_GFPCS9A0                        "GFPCS9A0"  /* 鍵交換電文精査[J-Link]                   */
#define DEF_GFPCSDA0                        "GFPCSDA0"  /* 鍵交換電文精査[Discover]                 */
#define DEF_GFPCSJB0                        "GFPCSJB0"  /* 鍵交換電文編集[CARDNET]                  */
#define DEF_GFPCSAB0                        "GFPCSAB0"  /* 鍵交換電文編集[AEGN]                     */
#define DEF_GFPCSNB0                        "GFPCSNB0"  /* 鍵交換電文編集[NYCE]                     */
#define DEF_GFPCSUB0                        "GFPCSUB0"  /* 鍵交換電文編集[UnionPay]                 */
#define DEF_GFPCS9B0                        "GFPCS9B0"  /* 鍵交換電文編集[J-Link]                   */
#define DEF_GFPCSDB0                        "GFPCSDB0"  /* 鍵交換電文編集[Discover]                 */
#define DEF_GFPCSJC0                        "GFPCSJC0"  /* ATALLAコマンド作成[CARDNET]              */
#define DEF_GFPCSAC0                        "GFPCSAC0"  /* ATALLAコマンド作成[AEGN]                 */
#define DEF_GFPCSNC0                        "GFPCSNC0"  /* ATALLAコマンド作成[NYCE]                 */
#define DEF_GFPCSUC0                        "GFPCSUC0"  /* ATALLAコマンド作成[UnionPay]             */
#define DEF_GFPCS9C0                        "GFPCS9C0"  /* ATALLAコマンド作成[J-Link]               */
#define DEF_GFPCSDC0                        "GFPCSDC0"  /* ATALLAコマンド作成[Discover]             */
#define DEF_GFPCSJD0                        "GFPCSJD0"  /* ATALLAレスポンス解析[CARDNET]            */
#define DEF_GFPCSAD0                        "GFPCSAD0"  /* ATALLAレスポンス解析[AEGN]               */
#define DEF_GFPCSND0                        "GFPCSND0"  /* ATALLAレスポンス解析[NYCE]               */
#define DEF_GFPCSUD0                        "GFPCSUD0"  /* ATALLAレスポンス解析[UnionPay]           */
#define DEF_GFPCS9D0                        "GFPCS9D0"  /* ATALLAレスポンス解析[J-Link]             */
#define DEF_GFPCSDD0                        "GFPCSDD0"  /* ATALLAレスポンス解析[Discover]           */
#define DEF_GFPCSJE0                        "GFPCSJE0"  /* カットオーバー電文精査[CARDNET]          */
#define DEF_GFPCSNE0                        "GFPCSNE0"  /* カットオーバー電文精査[NYCE]             */
#define DEF_GFPCSUE0                        "GFPCSUE0"  /* カットオーバー電文精査[UnionPay]         */
#define DEF_GFPCSJF0                        "GFPCSJF0"  /* カットオーバー電文編集[CARDNET]          */
#define DEF_GFPCSNF0                        "GFPCSNF0"  /* カットオーバー電文編集[NYCE]             */
#define DEF_GFPCSUF0                        "GFPCSUF0"  /* カットオーバー電文編集[UnionPay]         */
#define DEF_GFPCSXG0                        "GFPCSXG0"  /* カット対象日付更新[ダミー]               */
#define DEF_GFPCSJG0                        "GFPCSJG0"  /* カット対象日付更新[CARDNET]              */
#define DEF_GFPCSXH0                        "GFPCSXH0"  /* カット対象日付更新用初期処理[ダミー]     */
#define DEF_GFPCSJH0                        "GFPCSJH0"  /* カット対象日付更新用初期処理[CARDNET]    */
#define DEF_GFPCSVI0                        "GFPCSVI0"  /* SAF送信電文精査[VisaNet]                 */
#define DEF_GFPCSVI0                        "GFPCSVI0"  /* SAF送信電文編集[VisaNet]                 */
#define DEF_GFPCSJJ0                        "GFPCSJJ0"  /* 通知電文精査[CARDNET]                    */
#define DEF_GFPCSVJ0                        "GFPCSVJ0"  /* 通知電文精査[VisaNet]                    */
#define DEF_GFPCS9J0                        "GFPCS9J0"  /* 通知電文精査[J-Link]                     */
#define DEF_GFPCSXK0                        "GFPCSXK0"  /* 通知電文編集[ダミー]                     */
#define DEF_GFPCS9K0                        "GFPCS9K0"  /* 通知電文編集[J-Link]                     */

/* サーバクラス種類 */
#define DEF_SC_LISTEN                       "SCNLISTN"  /* リスナー                                 */
#define DEF_SC_CON_SVR                      "SCNCONSV"  /* コネクション制御(サーバ)                 */
#define DEF_SC_CON_CLT                      "SCNCONCL"  /* コネクション制御(クライアント)           */
#define DEF_SC_FURI_I                       "SCNMSDSI"  /* 電文振分(inbound)                        */
#define DEF_SC_FURI_O                       "SCNMSDSO"  /* 電文振分(outbound)                       */
#define DEF_SC_CTRL_CHU_I                   "SCNRLCMI"  /* 制御電文中継(inbound)                    */
#define DEF_SC_CTRL_CHU_O                   "SCNRLCMO"  /* 制御電文中継(outbound)                   */
#define DEF_SC_GYOM_CHU_I                   "SCNRLBMI"  /* 業務電文中継(inbound)                    */
#define DEF_SC_GYOM_CHU_O                   "SCNRLBMO"  /* 業務電文中継(outbound)                   */
#define DEF_SC_CTRL_IF_I                    "SCNCMIFI"  /* 制御電文I/F(inbound)                     */
#define DEF_SC_CTRL_IF_O                    "SCNCMIFO"  /* 制御電文I/F(outbound)                    */
#define DEF_SC_CTRL_FURI                    "SCNCMDST"  /* 制御電文振分                             */
#define DEF_SC_CNT_STS_ECH                  "SCNCSSTE"  /* 局状態・エコー制御                       */
#define DEF_SC_KEY_EXCH                     "SCNCSKYX"  /* 鍵交換制御                               */
#define DEF_SC_CUT_OVER                     "SCNCSCUT"  /* カットオーバー制御                       */
#define DEF_SC_SAF                          "SCNCSSAF"  /* SAF送信制御                              */
#define DEF_SC_NTF_MSG                      "SCNCSNTF"  /* 通知電文制御                             */
#define DEF_SC_CMD_SRV                      "SCNCMDSV"  /* コマンドサーバ                           */
#define DEF_SC_LOG_OUT                      "SCNLOGSV"  /* ログ出力                                 */
#define DEF_SC_GFP_LCN                      "SCNLCNNM"  /* GFP内部LCN採番                           */
#define DEF_SC_GCKEY_UPD                    "SCNKEYFU"  /* 鍵管理ファイル更新                       */
#define DEF_SC_SYS_NUM                      "SCNSERNM"  /* システム採番・生成                       */
#define DEF_SC_TIMER                        "SCNTMRCT"  /* タイマー制御                             */
#define DEF_SC_ATALLA                       "SCNATLDS"  /* ATALLA振分                               */
#define DEF_SC_UNYO_KANSHI                  "SCNOPMTO"  /* 運用監視端末出力                         */
#define DEF_SC_NAME_DEFAULT                 "}}}}}}}}"  /* サーバクラス論理名指定なし               */
#define DEF_SC_NUM_DEFAULT                  "}}}}"      /* サーバクラス論理番号指定なし             */
#define DEF_SC_DUP_DEFAULT                  "}}}}"      /* サーバクラス冗長化番号指定なし           */

/* プロセス種類 */
#define DEF_PRC_LISTEN                      "PSNLISTN"  /* リスナー                                 */
#define DEF_PRC_CON_SRV                     "PSNCONSV"  /* コネクション制御(サーバ)                 */
#define DEF_PRC_CON_CLT                     "PSNCONCL"  /* コネクション制御(クライアント)           */
#define DEF_PRC_FURI_I                      "PSNMSDSI"  /* 電文振分(inbound)                        */
#define DEF_PRC_FURI_O                      "PSNMSDSO"  /* 電文振分(outbound)                       */
#define DEF_PRC_PUT                         "PSNRLPUT"  /* 電文中継(PUT)                            */
#define DEF_PRC_GET                         "PSNRLGET"  /* 電文中継(GET)                            */
#define DEF_PRC_FURI_CTR                    "PSNCMDST"  /* 制御電文振分                             */
#define DEF_PRC_CNT_STS_ECH                 "PSNCSSTE"  /* 局状態・エコー制御                       */
#define DEF_PRC_KEY_EXCH                    "PSNCSKYX"  /* 鍵交換制御                               */
#define DEF_PRC_CUT_OVER                    "PSNCSCUT"  /* カットオーバー制御                       */
#define DEF_PRC_SAF                         "PSNCSSAF"  /* SAF送信制御                              */
#define DEF_PRC_NTF_MSG                     "PSNCSNTF"  /* 通知電文制御                             */
#define DEF_PRC_CMD_SRV                     "PSNCMDSV"  /* コマンドサーバ                           */
#define DEF_PRC_CMD_IF                      "PSNCMDIF"  /* コマンドI/F                              */
#define DEF_PRC_LOG_OUT                     "PSNLOGSV"  /* ログ出力                                 */
#define DEF_PRC_GFP_LCN                     "PSNLCNNM"  /* GFP内部LCN採番                           */
#define DEF_PRC_GCKEY_UPD                   "PSNKEYFU"  /* 鍵管理ファイル更新                       */
#define DEF_PRC_SYS_NUM                     "PSNSERNM"  /* システム採番・生成                       */
#define DEF_PRC_TIMER                       "PSNTRINM"  /* タイマー制御                             */
#define DEF_PRC_ATALLA                      "PSNATLDS"  /* ATALLA振分                               */
#define DEF_PRC_UNYO_KANSHI                 "PSNOPMTO"  /* 運用監視端末出力                         */

/* ファイル種類 */
#define DEF_FL_LIN_MG                       "FLNGFLIN"  /* 回線管理ファイル                         */
#define DEF_FL_LIN_STS                      "FLNGCLST"  /* 回線ステータスファイル                   */
#define DEF_FL_RCV_CON_NUM                  "FLNGCSCN"  /* 受信コネクション数管理ファイル           */
#define DEF_FL_CEN_STS                      "FLNGCSST"  /* 局状態管理ファイル                       */
#define DEF_FL_ECH_STS                      "FLNGCEST"  /* エコー状態管理ファイル                   */
#define DEF_FL_KEY_MG                       "FLNGCKEY"  /* 鍵管理ファイル                           */
#define DEF_FL_CUT_DATE_MG                  "FLNGCCUT"  /* カット対象日付管理ファイル               */
#define DEF_FL_SAF_SND_STS                  "FLNGCSAF"  /* SAF送信状態管理ファイル                  */
#define DEF_FL_NW_LOG                       "FLNGLNLG"  /* N/W通信ログ                              */
#define DEF_FL_ERR_LOG                      "FLNGLELG"  /* エラー出力ログ                           */
#define DEF_FL_CTRL_DEN_LOG                 "FLNGLMLG"  /* 制御電文ログ                             */
#define DEF_FL_CTRL_DEN_MG                  "FLNGFMTL"  /* 制御電文管理ファイル                     */
#define DEF_FL_FURI_RATE                    "FLNGFNSW"  /* 東阪振分比率設定ファイル                 */
#define DEF_FL_RCV_DEN_FURI                 "FLNGFQSW"  /* 受信電文振分先設定ファイル               */
#define DEF_FL_RTN_DST                      "FLNGFQBK"  /* 送信電文折返し振分先設定ファイル         */
#define DEF_FL_CTRL_MSG_ELM                 "FLNGFELI"  /* 制御電文エレメント情報ファイル           */
#define DEF_FL_NW_INFO                      "FLNGFNWI"  /* N/W情報ファイル                          */
#define DEF_FL_PHSIC_INFO                   "FLNGFPHI"  /* 物理名情報ファイル                       */
#define DEF_FL_NW_QUE                       "FLNGQNWQ"  /* 外部NW用キューファイル                   */
#define DEF_FL_NW_KOYU_INFO                 "FLNGFNWS"  /* 接続先固有情報ファイル                   */

/* IPC設計・共通ヘッダ・インターフェースコード */
#define DEF_IPC_IFCD_CON_NT_ACTL_REQ        "A001"      /* 接続通知用非同期要求                     */
#define DEF_IPC_IFCD_CON_NT                 "N101"      /* コネクション接続通知                     */
#define DEF_IPC_IFCD_CON_SW_NT              "N102"      /* コネクション入替・切断指示通知           */
#define DEF_IPC_IFCD_CON_START_REQ          "C103"      /* コネクション接続開始要求                 */
#define DEF_IPC_IFCD_CON_START_RSP          "R103"      /* コネクション接続開始応答                 */
#define DEF_IPC_IFCD_CON_NT_REQ             "C104"      /* コネクション接続完了通知要求             */
#define DEF_IPC_IFCD_CON_NT_RSP             "R104"      /* コネクション接続完了通知応答             */
#define DEF_IPC_IFCD_DISCON_NT_REQ          "C105"      /* コネクション切断完了通知要求             */
#define DEF_IPC_IFCD_DISCON_NT_RSP          "R105"      /* コネクション切断完了通知応答             */
#define DEF_IPC_IFCD_CON_SW_NT_REQ          "C106"      /* コネクション入替・切断完了通知要求       */
#define DEF_IPC_IFCD_CON_SW_NT_RSP          "R106"      /* コネクション入替・切断完了通知応答       */
#define DEF_IPC_IFCD_CON_STS_NT_REQ         "C107"      /* コネクション状態通知要求                 */
#define DEF_IPC_IFCD_CON_STS_NT_RSP         "R107"      /* コネクション状態通知応答                 */
#define DEF_IPC_IFCD_DEN_RCV_NT_REQ         "C201"      /* 電文受信通知要求                         */
#define DEF_IPC_IFCD_DEN_RCV_NT_RSP         "R201"      /* 電文受信通知応答                         */
#define DEF_IPC_IFCD_DEN_SND_REQ            "C202"      /* 電文送信要求                             */
#define DEF_IPC_IFCD_DEN_SND_RSP            "R202"      /* 電文送信応答                             */
#define DEF_IPC_IFCD_Q_RGST_REQ             "C301"      /* キュー登録要求                           */
#define DEF_IPC_IFCD_Q_RGST_RSP             "R301"      /* キュー登録応答                           */
#define DEF_IPC_IFCD_Q_GET_NT_REQ           "C302"      /* キュー取出し通知要求                     */
#define DEF_IPC_IFCD_Q_GET_NT_RSP           "R302"      /* キュー取出し通知応答                     */
#define DEF_IPC_IFCD_NW_MSG_REQ             "C401"      /* NW電文受信要求                           */
#define DEF_IPC_IFCD_NW_MSG_RSP             "R401"      /* NW電文受信応答                           */
#define DEF_IPC_IFCD_CTRL_MSG_REQ           "C402"      /* 制御電文作成要求                         */
#define DEF_IPC_IFCD_CTRL_MSG_RSP           "R402"      /* 制御電文作成応答                         */
#define DEF_IPC_IFCD_CMD_REQ                "C501"      /* コマンド要求                             */
#define DEF_IPC_IFCD_CMD_RSP                "R501"      /* コマンド応答                             */
#define DEF_IPC_IFCD_CMD_PRC_REQ            "C502"      /* コマンド処理要求                         */
#define DEF_IPC_IFCD_CMD_PRC_RSP            "R502"      /* コマンド処理応答                         */
#define DEF_IPC_IFCD_LG_OUT_REQ_DEN_REQ     "C601"      /* ログ出力要求                             */
#define DEF_IPC_IFCD_LG_OUT_RSP_DEN_REQ     "R601"      /* ログ出力要求                             */
#define DEF_IPC_IFCD_LCN_NUM_REQ            "C701"      /* LCN採番要求                              */
#define DEF_IPC_IFCD_LCN_NUM_RSP            "R701"      /* LCN採番応答                              */
#define DEF_IPC_IFCD_GCKEY_UPD_REQ          "C801"      /* 鍵管理ファイル更新要求                   */
#define DEF_IPC_IFCD_GCKEY_UPD_RSP          "R801"      /* 鍵管理ファイル更新応答                   */
#define DEF_IPC_IFCD_SYSTEM_NUM_REQ         "SS01"      /* システム採番要求                         */
#define DEF_IPC_IFCD_SYSTEM_NUM_RSP         "SS02"      /* システム採番応答                         */

/* IPC設計・共通ヘッダ・エラーコード */
#define DEF_IPC_ERRCD_OK                    0           /* 正常応答                                 */
#define DEF_IPC_ERRCD_FAILMSG               8           /* 障害電文                                 */
#define DEF_IPC_ERRCD_NG                    9           /* 拒否(異常)応答                           */

/* IPC設計・回線情報・コネクション切断対象単位 */
#define DEF_IPC_DISCON_UNI_INF              'I'         /* インターフェース単位                     */
#define DEF_IPC_DISCON_UNI_STA              'S'         /* ステーション単位                         */

/* IPC設計・データ部・登録/更新区分 */
#define DEF_IPC_RGST_KUBUN_RGS              '1'         /* 登録                                     */
#define DEF_IPC_RGST_KUBUN_UPD              '2'         /* 更新                                     */

/* IPC設計・コマンド情報・コマンド識別 */
#define DEF_IPC_CMD_OPN                     "1010"      /* オープン                                 */
#define DEF_IPC_CMD_CLS                     "1020"      /* クローズ                                 */
#define DEF_IPC_CMD_LSN_START               "1031"      /* リスナー（開始）、                       */
#define DEF_IPC_CMD_LSN_END                 "1032"      /* リスナー（終了）                         */
#define DEF_IPC_CMD_STS_DSP                 "1000"      /* コネクションステータス照会               */
#define DEF_IPC_CMD_CNT_OPN                 "2010"      /* 開局、                                   */
#define DEF_IPC_CMD_CNT_OPN_ABS             "2011"      /* 開局強制実行、                           */
#define DEF_IPC_CMD_CNT_OPN_UPD             "2012"      /* 開局状態更新のみ                         */
#define DEF_IPC_CMD_CNT_OPN_AUT_REQ         "2013"      /* 自動開局(開局要求電文受信)               */
#define DEF_IPC_CMD_CNT_OPN_AUT_CON         "2014"      /* 自動開局(コネクション確立)               */
#define DEF_IPC_CMD_CNT_CLS                 "2020"      /* 閉局、                                   */
#define DEF_IPC_CMD_CNT_CLS_ABS             "2021"      /* 閉局強制実行、                           */
#define DEF_IPC_CMD_CNT_CLS_UPD             "2022"      /* 閉局状態更新のみ                         */
#define DEF_IPC_CMD_CNT_STS_DSP             "2000"      /* 局状態照会                               */
#define DEF_IPC_CMD_ECH_SND                 "2110"      /* エコー送信                               */
#define DEF_IPC_CMD_ECH_STS_DSP             "2100"      /* エコーステータス照会                     */
#define DEF_IPC_CMD_KEY_EXC_REQ             "2210"      /* 鍵交換依頼                               */
#define DEF_IPC_CMD_KEY_EXC                 "2220"      /* 鍵交換                                   */
#define DEF_IPC_CMD_SAF_SND_START           "2310"      /* SAF送信開始                              */
#define DEF_IPC_CMD_SAF_SND_END             "2320"      /* SAF送信停止                              */
#define DEF_IPC_CMD_SAF_SND_STS_DSP         "2300"      /* SAF送信ステータス照会コマンド            */
#define DEF_IPC_CMD_LOG_FL_EXC              "3010"      /* ログファイル切替                         */
#define DEF_IPC_CMD_FL_RE_READ              "4010"      /* ファイル再読み込み                       */
#define DEF_IPC_CMD_FL_RE_READ_GFLIN        "4011"      /* 接続構成変更(ファイル再読み込み)         */
#define DEF_IPC_CMD_FL_RE_READ_GFNSW        "4012"      /* 東阪振分比率変更(ファイル再読み込み)     */

/* IPC設計・制御電文情報・要求種別 */
#define DEF_CTLREQ_HISIMUKE                 "10"        /* 被仕向要求                               */
#define DEF_CTLREQ_SIMUKE                   "20"        /* 仕向応答                                 */
#define DEF_CTLREQ_TIMEOUT                  "30"        /* 仕向応答タイムアウト                     */
#define DEF_CTLREQ_SIMUKE_ERROR             "40"        /* 仕向要求送信不可                         */
#define DEF_CTLREQ_HISIMUKE_ERROR           "50"        /* 被仕向応答送信不可                       */

/* IPC設計・制御電文情報・応答種別 */
#define DEF_CTLRSP_SEND                     "10"        /* 送信電文有り                             */
#define DEF_CTLRSP_NOSEND                   "20"        /* 送信電文無し                             */

/* 制御機能種別・制御機能区分 */
#define DEF_CTLFNC_CNT_STS_ECH              '1'         /* 局状態・エコー制御                       */
#define DEF_CTLFNC_KEY_EXCH                 '2'         /* 鍵交換制御                               */
#define DEF_CTLFNC_CUT_OVER                 '3'         /* カットオーバー制御                       */
#define DEF_CTLFNC_SAF                      '4'         /* SAF送信制御                              */
#define DEF_CTLFNC_NTF_MSG                  '5'         /* 通知電文制御                             */

/* 制御機能種別・要求応答区分 */
#define DEF_CTLMSG_REQUEST                  '1'         /* 要求                                     */
#define DEF_CTLMSG_RESPONSE                 '2'         /* 応答                                     */
#define DEF_CTLMSG_NOTICE                   '3'         /* 通知                                     */

/* 制御機能種別・制御電文区分 */
#define DEF_CTLTXT_CNT_OPN                  '1'         /* 開局(サインオン)                         */
#define DEF_CTLTXT_CNT_CLS                  '2'         /* 閉局(サインオフ)                         */
#define DEF_CTLTXT_ECH_SND                  '3'         /* エコーテスト                             */
#define DEF_CTLTXT_KEY_EXC_REQ              '4'         /* 鍵交換依頼                               */
#define DEF_CTLTXT_KEY_EXC                  '5'         /* 鍵交換                                   */
#define DEF_CTLTXT_CUT_START                '6'         /* カットオーバー,EOD開始                   */
#define DEF_CTLTXT_CUT_END                  '7'         /* EOD終了                                  */
#define DEF_CTLTXT_SAF_SND_START            '8'         /* SAF送信開始                              */
#define DEF_CTLTXT_SAF_SND_END              '9'         /* SAF送信停止                              */
#define DEF_CTLTXT_FAL                      'A'         /* 障害電文,REJECT,Administrative電文       */

/* 制御機能種別・内部処理区分 */
#define DEF_CTLINT_NORMAL                   '1'         /* 通常                                     */
#define DEF_CTLINT_FORCE                    '2'         /* 強制実行                                 */
#define DEF_CTLINT_UPDATEONLY               '3'         /* 状態更新のみ                             */
#define DEF_CTLINT_NORMAL_ACQUIRER          '4'         /* Discover:Acquirer                        */
#define DEF_CTLINT_NORMAL_ISSUER            '5'         /* Discover:Issuer                          */
#define DEF_CTLINT_AUTO_REQUEST             '6'         /* 自動開局(開局要求電文受信)               */
#define DEF_CTLINT_AUTO_CONNECT             '7'         /* 自動開局(コネクション確立)               */
#define DEF_CTLINT_ALLOW                    'A'         /* 許可応答                                 */
#define DEF_CTLINT_DENY                     'B'         /* 拒否応答                                 */

#endif

