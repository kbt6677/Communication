/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCSD70                                    */
/*        FUNCTION          ････ 開局・閉局・エコー電文精査[Discover]        */
/*                                                                           */
/*                               ネットワーク経由の電文を受け取って          */
/*                               IPCを精査する                               */
/*                               処理結果を返す                              */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS N.Miki                                 */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2025-04-30                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  N.Miki     2025/04/30 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*   DEFINE定義                                                              */
/*****************************************************************************/
#define DEF_ERR_BIT7                        "BIT7"      // エラー発生エリア：7
#define DEF_ERR_BIT11                       "BIT11"     // エラー発生エリア：11
#define DEF_ERR_BIT32                       "BIT32"     // エラー発生エリア：32
#define DEF_ERR_BIT37                       "BIT37"     // エラー発生エリア：37
#define DEF_ERR_BIT100                      "BIT100"    // エラー発生エリア：100
#define DEF_ERR_BITMAP                      "BITMAP"    // エラー発生エリア：BITMAP

#define DEF_NW_CORD_061                     "061"       // NW管理情報コード:061
#define DEF_NW_CORD_062                     "062"       // NW管理情報コード:062
#define DEF_RES_CORD_OK                     "00"        // RESPONCEコード:正常
#define DEF_RES_CORD_N1                     "N1"        // RESPONCEコード:N1
#define DEF_RES_CORD_N3                     "N3"        // RESPONCEコード:N3

#define DEF_CTLTEXT_TY_OFFSET               2           // 制御機能種別(制御電文区分)オフセット
#define DEF_CTLINT_TY_OFFSET                3           // 制御機能種別(内部処理区分)オフセット

#define DEF_STTE_STS_SIZE                   2           // 局状態の桁数

/*===========================================================================*/
/*   リターンコード                                                          */
/*===========================================================================*/
#define DEF_RET_OK                          0           // 返却値:正常
#define DEF_RET_NG                         -1           // 返却値:異常
// リターンコード(NWM_STE_check_reqmsg)
// リターンコード(NWM_STE_check_rspmsg)
#define DEF_RET_DATA_BREAK                  3           // 返却値:精査結果(電文破棄)
// リターンコード(NWM_STE_cst_check_req_rcv)
#define DEF_CHK_RES_OK                      0           // 局状態チェック：許可応答
#define DEF_CHK_RES_BREAK                   2           // 局状態チェック：電文破棄
// リターンコード(NWM_STE_cst_check_rsp_err)
// リターンコード(NWM_STE_cst_check_rsp_err)
#define DEF_RET_FILE_UPDATE                 0           // 管理ファイル:更新あり
#define DEF_RET_FILE_NO_UPDATE              1           // 管理ファイル:更新なし
#define DEF_RET_FILE_UPDATE_SIGNON_RETRY    2           // 管理ファイル:更新あり（開局リトライ有り）
#define DEF_RET_FILE_NO_UPDATE_SIGNON_RETRY 3           // 管理ファイル:更新なし（開局リトライ有り）
// リターンコード(NWM_STE_cst_check_rsp_err)
#define DEF_STATUS_RESULT_OK                0           // コマンド受:付可（電文送信あり）
#define DEF_STATUS_RESULT_OK_NO_DATA        1           // コマンド受:付可（電文送信なし）
#define DEF_STATUS_RESULT_NG                2           // コマンド受:付不可


/*===========================================================================*/
/*   内部エラーコード                                                        */
/*===========================================================================*/
#define DEF_NERR_HSMK_REQ_SEISA             "SCDI001"   // 内部エラーコード:被仕向要求精査エラー
#define DEF_NERR_HSMK_CLS_REQ_SEISA         "SCDJ003"   // 内部エラーコード:被仕向閉局要求精査エラー

#define DEF_NERR_SMK_REQ_SEISA              "SCDI003"   // 内部エラーコード:仕向要求精査エラー
#define DEF_NERR_SMK_OPN_REQ_SEISA          "SCDJ009"   // 内部エラーコード:仕向開局要求精査エラー
#define DEF_NERR_SMK_OPN_STAT_CHK           "SCDJ010"   // 内部エラーコード:仕向開局処理中の局状態チェックエラー
#define DEF_NERR_SMK_CLS_STAT_CHK           "SCDJ016"   // 内部エラーコード:仕向閉局処理中の局状態チェックエラー
#define DEF_NERR_SMK_ECHO_STAT_CHK          "SCDJ024"   // 内部エラーコード:仕向エコーテスト局状態チェックエラー


/*===========================================================================*/
/*   プロトタイプ宣言                                                        */
/*===========================================================================*/
short   CMIN_check_datetime(char*);                 // 日付形式チェック
short   CMIN_get_day_of_year(char*, char*);         // 通算日算出処理  
bool    NWM_STE_isdigit(char*,unsigned long);
