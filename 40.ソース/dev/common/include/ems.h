/**
 * @brief ems.h 共通ヘッダファイル
 *
 * @date 2024/10/03 新規作成 by makino.shinnosuke
*/
#ifndef _ems_H
#define _ems_H

#include "GFPOGGZ1_emsout.h" /* EMS出力モジュール */

/* EMSイベント番号 */
#define DEF_EVT_MSG_LEN_ERR             29000               /* メッセージ長エラー                  */
#define DEF_EVT_PSEND_QUE_NON           29001               /* PATHSENDキューなし                  */
#define DEF_EVT_ACTL_IO_ERR             29002               /* 非同期I/Oエラー                     */
#define DEF_EVT_HEADR_SEISA_ERR         29003               /* ヘッダー精査エラー                  */
#define DEF_EVT_DATA_FLD_SEISA_ERR      29004               /* データフィールド精査エラー          */
#define DEF_EVT_CONF_RE_READ            29005               /* コンフィグ再読み込み                */
#define DEF_EVT_RSP_CONN_NON            29006               /* 応答コネクションなし                */
#define DEF_EVT_PSEND_ERR_DETECT        29007               /* PATHSENDエラー検知                  */
#define DEF_EVT_REQ_ERR                 29008               /* リクエストエラー                    */
#define DEF_EVT_RSP_ERR                 29009               /* 応答エラー                          */
#define DEF_EVT_CHECK_DIGIT             29010               /* チェックデジットエラー              */
#define DEF_EVT_RSP_TIMEOUT_DETECT      29011               /* 応答待ちタイムアウト検知            */
#define DEF_EVT_FILE_IO_ERR_WARN        29012               /* ファイルI/Oエラー（WARNING）        */
#define DEF_EVT_FILE_IO_ERR             29013               /* ファイルI/Oエラー                   */
#define DEF_EVT_PROC_IO_ERR             29014               /* プロセスI/Oエラー                   */
#define DEF_EVT_ADDR_CHECK_ERR          29015               /* アドレスチェックエラー              */
#define DEF_EVT_ASN_FILE_GET_ERR        29016               /* アサインファイル取得エラー          */
#define DEF_EVT_CONFIG_ERR              29017               /* 設定情報エラー                      */
#define DEF_EVT_CONN_SYOGAI             29100               /* コネクション障害                    */
#define DEF_EVT_ALL_COMM_SYOGAI         29101               /* 全コネクション障害                  */
#define DEF_EVT_CONN_NUM_OVER           29102               /* コネクションオーバー                */
#define DEF_EVT_DENBUN_HAKI_CN_CTRL     29103               /* 電文破棄（コネクション制御）        */
#define DEF_EVT_FURIWAKE_HANTE_ERR      29104               /* 振分先判定不可                      */
#define DEF_EVT_MTI_HANTE_ERR           29105               /* MTI判定エラー                       */
#define DEF_EVT_FURIWAKE_RATE_UPDT      29106               /* 東阪比率変更                        */
#define DEF_EVT_QFILE_IO_ERR            29107               /* キューファイルI/Oエラー             */
#define DEF_EVT_DECRPT_ERR              29108               /* 復号化エラー                        */
#define DEF_EVT_ENCRPT_ERR              29109               /* 暗号化エラー                        */
#define DEF_EVT_UKAI_ERR                29110               /* 迂回不能                            */
#define DEF_EVT_SITE_UKAI_ERR           29111               /* サイト障害迂回不能                  */
#define DEF_EVT_RSP_SEISA_ERR           29112               /* 応答精査エラー                      */
#define DEF_EVT_CHUKEI_ERR              29113               /* 中継不能エラー                      */
#define DEF_EVT_DST_SHITEI_ERR          29114               /* 送信先指定エラー                    */
#define DEF_EVT_ERR_REP_RCV             29115               /* エラー応答受信                      */
#define DEF_EVT_ACCEPT_REJECT           29116               /* 接続拒否                            */
#define DEF_EVT_CONN_CONNECTED          29117               /* コネクション接続完了                */
#define DEF_EVT_BITMAP_EXPND_ERR        29200               /* BITMAP展開エラー                    */
#define DEF_EVT_BITMAP_ASMBL_ERR        29201               /* BITMAP結合エラー                    */
#define DEF_EVT_BITMAP_SEISA_ERR        29202               /* BITMAP精査エラー                    */
#define DEF_EVT_CTRL_DENBUN_NON         29203               /* 制御電文なし                        */
#define DEF_EVT_SIGN_ON_RCV             29204               /* サインオン受信                      */
#define DEF_EVT_SIGN_OFF_RCV            29205               /* サインオフ受信                      */
#define DEF_EVT_KYOKU_STS_SCTL_ERR      29206               /* 局状態同期エラー                    */
#define DEF_EVT_KEY_EXCH_START          29207               /* キー交換開始                        */
#define DEF_EVT_KEY_EXCH_END            29208               /* キー交換終了                        */
#define DEF_EVT_KEY_SCTL_ERR            29209               /* キー同期エラー                      */
#define DEF_EVT_SAF_ON_REQ_STRT         29210               /* SAFオン依頼開始                     */
#define DEF_EVT_SAF_ON_REQ_FIN          29211               /* SAFオン依頼終了                     */
#define DEF_EVT_SAF_OFF_REQ_START       29212               /* SAFオフ依頼開始                     */
#define DEF_EVT_SAF_OFF_REQ_END         29213               /* SAFオフ依頼終了                     */
#define DEF_EVT_KYOKU_STS_UPDATE        29214               /* 局状態変更                          */
#define DEF_EVT_ATALLA_RSP_ERR          29215               /* ATALLAレスポンスエラー              */
#define DEF_EVT_SEND_ERR_NTF_RCV        29216               /* 送信不可通知受信                    */
#define DEF_EVT_DENBUN_MATCH_ERR        29217               /* 電文マッチングエラー                */
#define DEF_EVT_SHOGAI_NTF_CRE_REQ      29218               /* 障害通知作成依頼                    */
#define DEF_EVT_CUT_OVER_RCV            29219               /* カットオーバー受信                  */
#define DEF_EVT_CUT_OVER_DATE_UPDT      29220               /* カットオーバー対象日付更新          */
#define DEF_EVT_NTF_DENBUN_RCV          29221               /* 通知電文受信                        */
#define DEF_EVT_KYOKU_STS_ERR           29222               /* 局状態エラー                        */
#define DEF_EVT_SAF_STS_ERR             29223               /* SAF状態エラー                       */
#define DEF_EVT_KEY_EXCH_REQ            29224               /* 鍵交換依頼                          */
#define DEF_EVT_PROC_START              29225               /* プロセス起動                        */
#define DEF_EVT_PROC_NORMAL_END         29226               /* プロセス正常終了                    */
#define DEF_EVT_PROC_ABNORMAL_END       29227               /* プロセス異常終了                    */
#define DEF_EVT_PROCEDURE_ERR           29228               /* プロシージャコールエラー            */
#define DEF_EVT_PARAM_GET_ERR           29229               /* パラメータ取得エラー                */
#define DEF_EVT_COMMON_MOD_ERR          29230               /* 共通モジュールエラー                */
#define DEF_EVT_SIGN_ON_RETRY_OVER      29231               /* 自動開局要求送信リトライオーバー    */
#define DEF_EVT_SIGN_ON_RETRY_ERR       29232               /* 自動開局要求送信リトライエラー      */
#define DEF_EVT_DENBUN_HAKI             29300               /* 電文破棄（コネクション制御以外）    */
#define DEF_EVT_CMD_RCV                 29301               /* コマンド受信                        */
#define DEF_EVT_CMD                     29302               /* コマンド（処理結果）                */
#define DEF_EVT_LCN_GET_ERR             29303               /* LCN取得エラー                       */
#define DEF_EVT_LOG_FILE_CHANGE         29304               /* ログファイル切替                    */

#endif

