/**
 * @brief GFPCSJ70.h 
 *
 * @date 2025/03/06 新規作成 by HAS
 *
*/

#ifndef _NWM_STE_H_
#define _NWM_STE_H_

/* ------------------------------------------------------------------------------------------ */
/* Define 定義                                                                                */
/* ------------------------------------------------------------------------------------------ */

/* 開局・閉局・エコー要求電文精査(NWM_STE_check_reqmsg) */
/* 開局・閉局・エコー応答電文精査(NWM_STE_check_rspmsg) */
#define NWM_STE_SEISA_NORMAL                0    /* 精査OK     (許可応答)                   */
#define NWM_STE_SEISA_KYOHI                 1    /* 精査エラー（拒否応答）                  */
#define NWM_STE_SEISA_SHOGAI                2    /* 精査エラー（障害電文通知）              */
#define NWM_STE_SEISA_HAKI                  3    /* 精査エラー（破棄）                      */

/* 開局・閉局・エコー要求電文編集(NWM_STE_edit_reqmsg) */
/* 開局・閉局・エコー応答電文編集(NWM_STE_edit_rspmsg) */
#define NWM_STE_EDIT_NORMAL                 0    /* 編集正常                                */
#define NWM_STE_EDIT_ERROR                  -1   /* 編集異常                                */

/* 開局・閉局・エコー局状態チェック（要求受信）(NWM_STE_cst_check_req_rcv) */
#define NWM_STE_SST_KYOKA                   0    /* 許可応答                                */
#define NWM_STE_SST_KYOHI                   1    /* 拒否応答                                */
#define NWM_STE_SST_HAKI                    2    /* 電文破棄                                */

/* 開局・閉局局状態チェック（応答送信不可）(NWM_STE_cst_check_rsp_err) */
/* 開局・閉局局状態チェック（仕向応答）(NWM_STE_cst_check_rsp_rcv) */
#define NWM_STE_SST_UPDATE                  0    /* 管理ファイル更新あり                    */
#define NWM_STE_SST_NOUPDATE                1    /* 管理ファイル更新なし                    */
#define NWM_STE_SST_UPDATE_RETRY            2    /* 管理ファイル更新あり（開局リトライ有り）*/
#define NWM_STE_SST_NOUPDATE_RETRY          3    /* 管理ファイル更新なし（開局リトライ有り）*/

/* 開局・閉局・エコー局状態チェック（コマンド）(NWM_STE_cst_check_command) */
#define NWM_STE_CMD_OK_SEND                 0    /* コマンド受付可（電文送信あり）          */
#define NWM_STE_CMD_OK_NOSEND               1    /* コマンド受付可（電文送信なし）          */
#define NWM_STE_CMD_NG                      2    /* コマンド受付不可                        */

/* ------------------------------------------------------------------------------------------ */
/* 関数のプロトタイプ宣言                                                                     */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局_エコー要求電文精査        */
short  NWM_STE_check_reqmsg      ( char*                  /* 受信応答                         */
                                 , char*                  /* 受信応答レングス                 */
                                 , char*                  /* NW情報rec(GP)                    */
                                 , char*                  /* NW情報rec(IF)                    */
                                 , char*                  /* 接続先固有情報rec(NW)            */
                                 , char*                  /* 接続先固有情報rec(IF)            */
                                 , char*                  /* 接続先固有情報rec(ST)            */
                                 , char*                  /* 接続先固有情報rec(CN)            */
                                 , gflin_pkey_def*        /* コネクション論理ID               */
                                 , t_rcv_info_def*        /* 処理結果情報                     */
                                 , char* );               /* 局状態管理レコード               */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局_エコー応答電文精査        */
short  NWM_STE_check_rspmsg      ( char*                  /* 受信応答                         */
                                 , char*                  /* 受信応答レングス                 */
                                 , char*                  /* 仕向要求                         */
                                 , char*                  /* NW情報rec(GP)                    */
                                 , char*                  /* NW情報rec(IF)                    */
                                 , char*                  /* 接続先固有情報rec(NW)            */
                                 , char*                  /* 接続先固有情報rec(IF)            */
                                 , char*                  /* 接続先固有情報rec(ST)            */
                                 , char*                  /* 接続先固有情報rec(CN)            */
                                 , gflin_pkey_def*        /* コネクション論理ID               */
                                 , t_rcv_info_def* );     /* 処理結果情報                     */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局_エコー要求電文編集        */
short  NWM_STE_edit_reqmsg       ( char*                  /* 送信電文                         */
                                 , char*                  /* NW情報rec(GP)                    */
                                 , char*                  /* NW情報rec(IF)                    */
                                 , char*                  /* 接続先固有情報rec(NW)            */
                                 , char*                  /* 接続先固有情報rec(IF)            */
                                 , char*                  /* 接続先固有情報rec(ST)            */
                                 , char*                  /* 接続先固有情報rec(CN)            */
                                 , gflin_pkey_def*        /* コネクション論理ID               */
                                 , NWM_CTU_INI_arg_2_def* /* カット対象日付管理File情報       */
                                 , t_rcv_info_def*        /* 処理結果情報                     */
                                 , oggz1in_def*           /* EMS出力共通情報                  */
                                 , ems_info_add* );       /* EMS出力付加情報                  */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局_エコー応答電文編集        */
short  NWM_STE_edit_rspmsg       ( char*                  /* 送信応答                         */
                                 , char*                  /* 被仕向要求                       */
                                 , char*                  /* NW情報rec(Group)                 */
                                 , char*                  /* NW情報rec(InterFace)             */
                                 , char*                  /* 接続先固有情報rec(NetWork)       */
                                 , char*                  /* 接続先固有情報rec(InterFace)     */
                                 , char*                  /* 接続先固有情報rec(Station)       */
                                 , char*                  /* 接続先固有情報rec(connection)    */
                                 , gflin_pkey_def*        /* コネクション論理ID               */
                                 , NWM_CTU_INI_arg_2_def* /* カット対象日付管理File情報       */
                                 , t_rcv_info_def*        /* 処理結果情報                     */
                                 , oggz1in_def*           /* EMS出力共通情報                  */
                                 , ems_info_add* );       /* EMS出力付加情報                  */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局エコー局状態チェック(要求) */
short  NWM_STE_cst_check_req_rcv ( char*                  /* 局状態                           */
                                 , t_rcv_info_def* );     /* 処理結果情報                     */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局局状態チェック(仕向応答)   */
short  NWM_STE_cst_check_rsp_rcv ( char*                  /* 局状態                           */
                                 , char*                  /* 要求種別                         */
                                 , char*                  /* 受信電文                         */
                                 , t_rcv_info_def* );     /* 処理結果情報                     */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局局状態チェック(応答不可)   */
short  NWM_STE_cst_check_rsp_err ( char*                  /* 局状態                           */
                                 , t_rcv_info_def* );     /* 処理結果情報                     */
/* ------------------------------------------------------------------------------------------ */
                                                          /* 開閉局_エコー局状態チェック      */
short  NWM_STE_cst_check_command ( char*                  /* 局状態                           */
                                 , char*                  /* NW情報レコード(インタフェース単位) */
                                 , t_rcv_info_def* );     /* 処理結果情報                     */
/* ------------------------------------------------------------------------------------------ */

#endif
