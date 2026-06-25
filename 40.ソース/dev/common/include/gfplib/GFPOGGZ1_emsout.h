/* GFPOGGZ1 EMS出力モジュール */
#ifndef GFPOGGZ1_H
#define GFPOGGZ1_H

/* -------------------------------------------------------*/
/* 構造体のtypedef定義                                    */
/* -------------------------------------------------------*/

// EMS出力 引数
//   業務の lk_cg010_in を通信制御用に定義。
//   全ての項目につけられていたprefix (lk_cg010_out, lk_cg010_in) を外した。
typedef struct __oggz1in
{
    char  subrcd;
    struct {
        char  proctimer[4];
        char  uytrmmon[13];
        char  uytrmmonlen[2];
        char  uytrmsrv[12];
        char  uytrmsrvlen[2];
    } uytrminf;
    struct {
        char  rcd;
        char  msgid[5];
        struct {
            char  msgttkb;
            char  emsgkinf_yobi1;
            char  sysnm[3];
            char  emsgkinf_yobi2;
            char  srv_kbn[3];
            char  emsgkinf_yobi3;
            char  h_nw_kbn[2];
            char  emsgkinf_yobi4;
            char  s_nw_kbn[2];
            char  emsgkinf_yobi5;
            char  prgid[8];
            char  emsgkinf_yobi6;
            char  trmnm[8];
            char  emsgkinf_yobi7;
            char  inter_errcd[7];
            char  emsgkinf_yobi8[6];
        } emsgkinf;
        struct {
            char  ktmg[40];
            struct {
                char  msgtbl_prm[40];
                char  msgtbl_vl[80];
            } msgtbl[10];
        } emsnninf;
    } emsinf;
} oggz1in_def;

/* -------------------------------------------------------*/
/* 関数プロトタイプ宣言                                   */
/* -------------------------------------------------------*/

// EMS出力
_cobol void GFPOGGZ1(oggz1in_def *);


/* -------------------------------------------------------*/
/* 定数定義                                               */
/* -------------------------------------------------------*/

#define DEF_EMS_MSGTTKB_NORMAL          '*'    // メッセージ通知区分 正常
#define DEF_EMS_MSGTTKB_WARN            'W'    // メッセージ通知区分 警告
#define DEF_EMS_MSGTTKB_GYM_ERR         'E'    // メッセージ通知区分 業務エラー
#define DEF_EMS_MSGTTKB_SYS_ERR         'S'    // メッセージ通知区分 システムエラー

#endif
