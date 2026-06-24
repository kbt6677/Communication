/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXB0                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               IOモジュール                                */
/*                                                                           */
/*                               指定ファイルに対するファイル操作            */
/*                               (OPEN/CLOSE、読込、書込、更新、削除等)を    */
/*                               実行する。                                  */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････ EMS出力モジュール                           */
/*                               トレースモジュール                          */
/*        WRITTEN-DATE      ････ 2024-10-01                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  T.Fukunaga 2024/10/01 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/****************************************************************************/
/*   ｺﾝﾊﾟｲﾙｵﾌﾟｼｮﾝ                                                           */
/****************************************************************************/
#pragma ENV COMMON

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdio.h> nolist
#include <stdlib.h> nolist
#include <string.h> nolist
#include <ctype.h> nolist
#include <cextdecs.h> nolist
#include <tal.h> nolist

/* USER HEADER     */
#include "GFPOGGZ4_traceout.h"
#include "GFPCGXB0.h"
#include "GFPCGXD0.h"
#include "GFPCGX50.h"

/* vproc関数の宣言 */
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define DEF_KINOU_LEN                     4
#define DEF_KINOU_OPEN                    "OPEN"
#define DEF_KINOU_SRED                    "SRED"
#define DEF_KINOU_NRED                    "NRED"
#define DEF_KINOU_ADD                     "ADD "
#define DEF_KINOU_UPDT                    "UPDT"
#define DEF_KINOU_DELT                    "DELT"
#define DEF_KINOU_ULOC                    "ULOC"
#define DEF_KINOU_CLOS                    "CLOS"
#define DEF_EOF                           1
#define DEF_TIMEOUT                       40
#define DEF_DISCFULL                      43
#define DEF_DUPLICATE                     10
#define DEF_STATUS_LEN                    2
#define DEF_NO_ERR                        "00"
#define DEF_FILEOPEN_ERR                  "Z1"
#define DEF_EOF_ERR                       "Z2"
#define DEF_READ_ERR                      "Z3"
#define DEF_WRITE_ERR                     "Z4"
#define DEF_TIMEOUT_ERR                   "Z5"
#define DEF_DISCFULL_ERR                  "Z6"
#define DEF_DUPLICATE_ERR                 "Z7"
#define DEF_OTHER_ERR                     "Z9"
#define DEF_KEYSIKI_LEN                   2
#define DEF_KEYSIKI_PRI                   "10"
#define DEF_KEYSIKI_A1                    "01"
#define DEF_KEYSIKI_A2                    "02"
#define DEF_KEYSIKI_A3                    "03"
#define DEF_KEYSIKI_A4                    "04"
#define DEF_KEYSIKI_A5                    "05"
#define DEF_KEYSIKI_A6                    "06"
#define DEF_KEYSIKI_A7                    "07"
#define DEF_KEYSIKI_A8                    "08"
#define DEF_KEYSIKI_A9                    "09"
#define DEF_KEYSPC_PK                     "PK"
#define DEF_KEYSPC_A1                     "A1"
#define DEF_KEYSPC_A2                     "A2"
#define DEF_KEYSPC_A3                     "A3"
#define DEF_KEYSPC_A4                     "A4"
#define DEF_KEYSPC_A5                     "A5"
#define DEF_KEYSPC_A6                     "A6"
#define DEF_KEYSPC_A7                     "A7"
#define DEF_KEYSPC_A8                     "A8"
#define DEF_KEYSPC_A9                     "A9"
#define DEF_NOLOCK                        0
#define DEF_LOCK                          1
#define DEF_LOCKFREE                      2
#define DEF_SYOUJUN                       0
#define DEF_KOUJUN                        1
#define DEF_PROCEDURE_FILE_OPEN_          "FILE_OPEN_"
#define DEF_PROCEDURE_FILE_SETKEY_        "FILE_SETKEY_"
#define DEF_PROCEDURE_READX               "READX"
#define DEF_PROCEDURE_READLOCKX           "READLOCKX"
#define DEF_PROCEDURE_WRITEX              "WRITEX"
#define DEF_PROCEDURE_WRITEUPDATEUNLOCKX  "WRITEUPDATEUNLOCKX"
#define DEF_PROCEDURE_WRITEUPDATEX        "WRITEUPDATEX"
#define DEF_PROCEDURE_UNLOCKREC           "UNLOCKREC"
#define DEF_PROCEDURE_FILE_CLOSE_         "FILE_CLOSE_"
#define DEF_PARTITION_KEY_NOT             0
#define DEF_PARTITION_KEY_KOTEI           1
#define DEF_PARTITION_KEY_MODULUS         2
#define DEF_RETURN_OK                     0
#define DEF_RETURN_NG                     1
#define DEF_READ_TYPE_NEXT                "NEXT"
#define DEF_READ_TYPE_REVS                "REVS"
#define DEF_BIT0_ON                       32768
#define DEF_BIT1_ON                       16384
#define DEF_JPN_TIME                      2

#pragma fieldalign shared2 trace_time_format
typedef struct trace_time_format
{
    char    c_start_time[20];
    short   b_start_time[8];
    char    c_end_time[20];
    short   b_end_time[8];
} trace_time_def;


/****************************************************************************/
/*   プロトタイプ関数宣言                                                   */
/****************************************************************************/
// プロトタイプ関数宣言
short COM_IOM_file_open_proc(char *, COM_IOM_arg_3_def*,
                     COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                     COM_IOM_arg_6_def*);
short COM_IOM_file_start_read_proc(char *, COM_IOM_arg_3_def*,
                           COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                           COM_IOM_arg_6_def*);
short COM_IOM_file_next_read_proc(char *, COM_IOM_arg_3_def*,
                          COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                          COM_IOM_arg_6_def*);
short COM_IOM_record_add_proc(char *, COM_IOM_arg_3_def*,
                      COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                      COM_IOM_arg_6_def*);
short COM_IOM_record_update_proc(char *, COM_IOM_arg_3_def*,
                         COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                         COM_IOM_arg_6_def*);
short COM_IOM_record_delete_proc(char *, COM_IOM_arg_3_def*,
                         COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                         COM_IOM_arg_6_def*);
short COM_IOM_record_unlock_proc(char *, COM_IOM_arg_3_def*,
                         COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                         COM_IOM_arg_6_def*);
short COM_IOM_file_close_proc(char *, COM_IOM_arg_3_def*,
                      COM_IOM_arg_4_def*, COM_IOM_arg_5_def*,
                      COM_IOM_arg_6_def*);
short COM_IOM_awaitio_proc(COM_IOM_arg_4_def*, long, COM_IOM_arg_6_def*);
short COM_IOM_convert_key_type_proc(char*);
void  COM_IOM_create_par_key_proc(char*, COM_IOM_arg_5_def*, char*);
//void  TRACEOUT(char *);
/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_IOM                                        */
/*  CALLING SEQ.    : short COM_IOM(char   *                                */
/*                                , char   *                                */
/*                                , struct *                                */
/*                                , struct *                                */
/*                                , struct *                                */
/*                                , struct *)                               */
/*  ARGUMENT        : 1.func_type     (I)   機能名識別                      */
/*                  : 2.sub_status    (O)   サブプログラムステータス        */
/*                  : 3.trace_inf     (I)   トレース情報                    */
/*                  : 4.file_inf      (I/O) ファイル情報                    */
/*                  : 5.in_inf        (I)   入力情報                        */
/*                  : 6.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 -1:異常  1:障害発生                            */
/*  DESCRIPTION     : IOモジュールのコントロールを行う                      */
/****************************************************************************/
short  COM_IOM( char *func_type,
                char *sub_status,
                COM_IOM_arg_3_def* trace_inf,
                COM_IOM_arg_4_def* file_inf,
                COM_IOM_arg_5_def* in_inf,
                COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short   s_rtn_cd;         //戻り値

    s_rtn_cd       = DEF_RETURN_OK;

    memset((char*)sub_status, 0x00, DEF_STATUS_LEN);
    memset((char*)out_inf, 0x00, sizeof(COM_IOM_arg_6_def));

    //パーティション識別を判定
    if(( memcmp( func_type, DEF_KINOU_SRED, DEF_KINOU_LEN ) == 0 ) ||
       ( memcmp( func_type, DEF_KINOU_ADD,  DEF_KINOU_LEN ) == 0 ) ||
       ( memcmp( func_type, DEF_KINOU_UPDT, DEF_KINOU_LEN ) == 0 ) ||
       ( memcmp( func_type, DEF_KINOU_DELT, DEF_KINOU_LEN ) == 0 )) {
        if(in_inf->part_key_type != DEF_PARTITION_KEY_NOT   &&
           in_inf->part_key_type != DEF_PARTITION_KEY_KOTEI &&
           in_inf->part_key_type != DEF_PARTITION_KEY_MODULUS){
            memcpy(sub_status, DEF_OTHER_ERR, DEF_STATUS_LEN);
            return -1;    /* パラメータ不正 */
        }
    }

    //機能名識別判定
    if( memcmp( func_type, DEF_KINOU_OPEN, DEF_KINOU_LEN ) == 0 ) {
        //ﾌｧｲﾙｵｰﾌﾟﾝ
        s_rtn_cd = COM_IOM_file_open_proc(sub_status, trace_inf, file_inf,
                                  in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_SRED, DEF_KINOU_LEN ) == 0 ) {
        //読込開始
        s_rtn_cd = COM_IOM_file_start_read_proc(sub_status, trace_inf, file_inf,
                                        in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_NRED, DEF_KINOU_LEN ) == 0 ) {
        //読込継続
        s_rtn_cd = COM_IOM_file_next_read_proc(sub_status, trace_inf, file_inf,
                                       in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_ADD,  DEF_KINOU_LEN ) == 0 ) {
        //ﾚｺｰﾄﾞ追加
        s_rtn_cd = COM_IOM_record_add_proc(sub_status, trace_inf, file_inf,
                                   in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_UPDT, DEF_KINOU_LEN ) == 0 ) {
        //ﾚｺｰﾄﾞ更新
        s_rtn_cd = COM_IOM_record_update_proc(sub_status, trace_inf, file_inf,
                                      in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_DELT, DEF_KINOU_LEN ) == 0 ) {
        //ﾚｺｰﾄﾞ削除
        s_rtn_cd = COM_IOM_record_delete_proc(sub_status, trace_inf, file_inf,
                                      in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_ULOC, DEF_KINOU_LEN ) == 0 ) {
        //ﾚｺｰﾄﾞﾛｯｸ解除
        s_rtn_cd = COM_IOM_record_unlock_proc(sub_status, trace_inf, file_inf,
                                      in_inf, out_inf );
    }
    else if( memcmp( func_type, DEF_KINOU_CLOS, DEF_KINOU_LEN ) == 0 ) {
        //ﾌｧｲﾙｸﾛｰｽﾞ
        s_rtn_cd = COM_IOM_file_close_proc(sub_status, trace_inf, file_inf,
                                   in_inf, out_inf );
    }
    else {
        //上記以外
        memcpy(sub_status, DEF_OTHER_ERR, DEF_STATUS_LEN);
        return -1;  /* パラメータ不正 */
    }

    //IO物理ﾌｧｲﾙ名設定
    memcpy(out_inf->file_name, file_inf->file_name, sizeof(out_inf->file_name));

    if(s_rtn_cd != 0){
        out_inf->rec_len = 0;
        memset((char*)out_inf->rec_area, ' ', sizeof(out_inf->rec_area));
        return 1;   /* IOエラー発生 */
    }
    return 0;   /* 正常 */
} /* end of COM_IOM */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  COM_IOM_file_open_proc                         */
/*  CALLING SEQ.    : short COM_IOM_file_open_proc(char   *                 */
/*                                               , struct *                 */
/*                                               , struct *                 */
/*                                               , struct *                 */
/*                                               , struct *)                */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : ファイルオープン処理を行う                            */
/****************************************************************************/
short COM_IOM_file_open_proc( char *sub_status,
                              COM_IOM_arg_3_def* trace_inf,
                              COM_IOM_arg_4_def* file_inf,
                              COM_IOM_arg_5_def* in_inf,
                              COM_IOM_arg_6_def* out_inf )
{
    /* 変数定義 */
    short                   s_err_code;         //戻り値
    short                   s_rtn_cd;           //リターンコード
    char                    ch_filename[48];    //物理ファイル名
    short                   s_name_len;         //物理ファイル名長
    trace_time_def          wk_trace_time;      //トレース用処理時刻
    long long               wk_julian_time;     //ユリウス暦
    char*                   pch_ptr;            //物理ﾌｧｲﾙ名長取得用
    lk_zac2001f_arg_1_def   wk_trace_info;      //トレース出力情報
    short                   wk_item[10];        //FILE_GETINFOLISTBYNAME_取得アイテム
    short                   wk_result[10];      //FILE_GETINFOLISTBYNAME_取得情報
    short                   wk_syncdepth;       //FILE_OPEN_で使用するsyncdepth

    s_err_code = 0;
    s_rtn_cd = 0;
    memset(ch_filename, ' ', sizeof(ch_filename));
    ch_filename[47] = 0;
    s_name_len = 0;
    memset(&wk_trace_time, ' ', sizeof(trace_time_def) );
    wk_julian_time = 0;
    pch_ptr = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);

    memset((char*)sub_status, 0x00, DEF_STATUS_LEN);

    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //物理ファイル名の取得
    memmove( ch_filename, file_inf->file_name,
             sizeof(ch_filename)-1 );

    pch_ptr = strchr( ch_filename, ' ' );

    //文字列中のスペース位置を取得し物理ファイル名長を設定
    if( pch_ptr == 0 ) {
        s_name_len = sizeof(file_inf->file_name);
    } else {
        s_name_len = (short)(pch_ptr - ch_filename);
    }

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);

    s_err_code = 0;

    //ﾌｧｲﾙ情報取得
    memset(wk_item, 0x00, sizeof(wk_item));
    memset(wk_result, 0x00, sizeof(wk_result));
    wk_item[0] = 66;    //Audited属性
    s_err_code = FILE_GETINFOLISTBYNAME_ ( ch_filename
                                         , s_name_len
                                         , wk_item
                                         , 1
                                         , (short *)&wk_result
                                         , 10
                                         );
    if (s_err_code == 0) {
         if (wk_result[0] == 0) {
             wk_syncdepth = 1;          //対象ﾌｧｲﾙはNON-AUDIT
         } else {
             wk_syncdepth = 0;          //対象ﾌｧｲﾙAUDIT
         }
    } else {
         wk_syncdepth = 0;              //ﾌｧｲﾙ情報取得ｴﾗｰ
    }
    //ﾌｧｲﾙｵｰﾌﾟﾝ
    s_err_code = FILE_OPEN_ ( ch_filename
                            , s_name_len
                            , &file_inf->file_no
                            , // access
                            , // exclusion
                            , 1
                            , wk_syncdepth
                            , // options
                            , // seq-block-buffer-id
                            , // seq-block-buffer-len
                            , // primary-processhandle
                            , 1
                            );

    //ﾌｧｲﾙｵｰﾌﾟﾝ結果判定
    if( file_inf->file_no == -1 ) {  //ｵｰﾌﾟﾝｴﾗｰ
        memmove( sub_status, DEF_FILEOPEN_ERR, DEF_STATUS_LEN );
        memmove( out_inf->err_proc,
                 DEF_PROCEDURE_FILE_OPEN_,
                 strlen(DEF_PROCEDURE_FILE_OPEN_) );
        out_inf->guardian_errcode = s_err_code;
        s_rtn_cd = 1;
    } else {  //ｵｰﾌﾟﾝ正常
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
        out_inf->guardian_errcode = 0;
    }

    s_err_code = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);

    //ﾄﾚｰｽﾌｧｲﾙ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
  
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);

    
    return s_rtn_cd;
} /* end of COM_IOM_file_open_proc */

/****************************************************************************/
/*  FUNCTION        : 1.2.0  COM_IOM_file_start_read_proc                   */
/*  CALLING SEQ.    : short COM_IOM_file_start_read_proc(char   *           */
/*                                                     , struct *           */
/*                                                     , struct *           */
/*                                                     , struct *           */
/*                                                     , struct *)          */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : レコードの初回読込を行う                              */
/****************************************************************************/
short COM_IOM_file_start_read_proc( char  *sub_status,
                            COM_IOM_arg_3_def* trace_inf,
                            COM_IOM_arg_4_def* file_inf,
                            COM_IOM_arg_5_def* in_inf,
                            COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short                    s_err_code;         //ｴﾗｰｺｰﾄﾞ
    short                    s_rtn_cd;           //ﾘﾀｰﾝｺｰﾄﾞ
    short                    s_key_size;         //KEY長
    char                     c_key_area[152];    //KEYｴﾘｱ
    short                    s_compare_size;     //ｺﾝﾍﾟｱ長
    short                    s_key_spc;          //KEY変換ｴﾘｱ
    lk_zac2001f_arg_1_def    wk_trace_info;      //ﾄﾚｰｽ出力情報
    trace_time_def           wk_trace_time;      //ﾄﾚｰｽ用処理時刻
    long long                wk_julian_time;     //ﾕﾘｳｽ暦
    short                    s_asc_desc_type;    //昇順降順識別

    s_err_code = 0;
    s_rtn_cd = 0;
    s_key_size = 0;
    memset(c_key_area, ' ', sizeof(c_key_area));
    s_compare_size = 0;
    s_key_spc = 0;
    s_asc_desc_type = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);
    memset(&wk_trace_time, ' ', sizeof(trace_time_def));
    wk_julian_time = 0;

    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //キー種別の変換を呼出
    s_key_spc = COM_IOM_convert_key_type_proc(in_inf->key_type);

     //パーティション識別の判定
    if(in_inf->part_key_type == DEF_PARTITION_KEY_KOTEI ||
       in_inf->part_key_type == DEF_PARTITION_KEY_MODULUS){
        if (s_key_spc == 0){        // 主ｷｰﾌｧｲﾙの場合
            //ﾊﾟｰﾃｨｼｮﾝｷｰ生成処理を呼出
            COM_IOM_create_par_key_proc(DEF_KINOU_SRED, in_inf, c_key_area);

            //ﾊﾟｰﾃｨｼｮﾝｷｰ以降のｷｰ情報を設定
            memcpy(&c_key_area[2], in_inf->key_value, in_inf->key_len);

            //ｷｰ長にﾊﾟｰﾃｨｼｮﾝｷｰ長を加算
            s_key_size     = in_inf->key_len + 2;
            s_compare_size = in_inf->compare_len + 2;
        }else{                      // ｵﾙﾀﾈｰﾄｷｰﾌｧｲﾙの場合
            memcpy(c_key_area, in_inf->key_value, in_inf->key_len);
            s_key_size     = in_inf->key_len;
            s_compare_size = in_inf->compare_len;
        }

    }else{
        memcpy(c_key_area, in_inf->key_value, in_inf->key_len);
        s_key_size     = in_inf->key_len;
        s_compare_size = in_inf->compare_len;

    }
    
    if(in_inf->asc_desc_type == DEF_KOUJUN){
        s_asc_desc_type = (short)DEF_BIT1_ON;
    }

    //ｷｰﾎﾟｼﾞｼｮﾝの設定
    FILE_SETKEY_ ( file_inf->file_no
                 , c_key_area
                 , s_key_size
                 , s_key_spc
                 , in_inf->positioning_mode
                 , s_asc_desc_type
                 , s_compare_size
                 );

    //ｴﾗｰ取得
    s_err_code = FILE_GETINFO_ ( file_inf->file_no
                               , &out_inf->guardian_errcode
                             );

    //Guardianｴﾗｰｺｰﾄﾞの判定
    if( out_inf->guardian_errcode != 0 ) {
        memmove( out_inf->err_proc,
                 DEF_PROCEDURE_FILE_SETKEY_,
                 strlen(DEF_PROCEDURE_FILE_SETKEY_));
        memmove( sub_status, DEF_OTHER_ERR, DEF_STATUS_LEN );
        return 1;
    }

    s_err_code = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);
    


    //初回読込

    //LOCKフラグの判定
    if( in_inf->lock_flg == DEF_NOLOCK ) {

        READX ( file_inf->file_no
              , out_inf->rec_area
              , (short)in_inf->rec_len
              , (unsigned short *)&out_inf->rec_len
              );
    } else {

        READLOCKX ( file_inf->file_no
                  , out_inf->rec_area
                  , (short)in_inf->rec_len
                  , (unsigned short *)&out_inf->rec_len
                  );
    }

    s_err_code = 0;

    //読込結果取得
    s_err_code = COM_IOM_awaitio_proc( file_inf,
                               in_inf->io_timer,
                               out_inf );

    //読込結果取得のエラー判定
    if( s_err_code != 0 ) {
        switch( s_err_code ) {
          case DEF_EOF:
            memmove( sub_status, DEF_EOF_ERR, DEF_STATUS_LEN );
            break;
          case DEF_TIMEOUT:
            memmove( sub_status, DEF_TIMEOUT_ERR, DEF_STATUS_LEN );
            break;
          default :
            memmove( sub_status, DEF_READ_ERR, DEF_STATUS_LEN );
            break;
        }
        out_inf->guardian_errcode = s_err_code;
        if(s_err_code != DEF_EOF){
            if( in_inf->lock_flg == DEF_NOLOCK ) {
                memmove( out_inf->err_proc,
                         DEF_PROCEDURE_READX,
                         strlen(DEF_PROCEDURE_READX) );
            } else {
                memmove( out_inf->err_proc,
                         DEF_PROCEDURE_READLOCKX,
                         strlen(DEF_PROCEDURE_READLOCKX) );
            }
            s_rtn_cd = 1;
        }
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
    }

    s_err_code = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);
    

    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    if (s_key_spc == 0){
        memcpy((char *)&s_key_spc,DEF_KEYSPC_PK,sizeof(s_key_spc));
    }
    memcpy(&wk_trace_info.data_info.key_type, &s_key_spc,
           sizeof(wk_trace_info.data_info.key_type));
    memset(&wk_trace_info.data_info.key_value, 0x20, sizeof(wk_trace_info.data_info.key_value));
    memcpy(&wk_trace_info.data_info.key_value, in_inf->key_value, in_inf->key_len);
    if(in_inf->asc_desc_type == DEF_SYOUJUN){
        memcpy(wk_trace_info.data_info.next_revs_type, DEF_READ_TYPE_NEXT,
               sizeof(wk_trace_info.data_info.next_revs_type));
    }else{
        memcpy(wk_trace_info.data_info.next_revs_type, DEF_READ_TYPE_REVS,
               sizeof(wk_trace_info.data_info.next_revs_type));
    }
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    sprintf(wk_trace_info.data_info.rec_len, "%05d", in_inf->rec_len);
    
    memcpy(&wk_trace_info.data_info.rec_area, out_inf->rec_area,
           sizeof(wk_trace_info.data_info.rec_area));
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);

    return s_rtn_cd;
    
} /* end of COM_IOM_file_start_read_proc */

/****************************************************************************/
/*  FUNCTION        : 1.3.0  COM_IOM_file_next_read_proc                    */
/*  CALLING SEQ.    : short COM_IOM_file_next_read_proc(char   *            */
/*                                                     , struct *           */
/*                                                     , struct *           */
/*                                                     , struct *           */
/*                                                     , struct *)          */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : レコードの継続読込を行う                              */
/****************************************************************************/
short COM_IOM_file_next_read_proc( char *sub_status,
                           COM_IOM_arg_3_def* trace_inf,
                           COM_IOM_arg_4_def* file_inf,
                           COM_IOM_arg_5_def* in_inf,
                           COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short                   s_err_code;        //ｴﾗｰｺｰﾄﾞ
    short                   s_rtn_cd;          //ﾘﾀｰﾝｺｰﾄﾞ
    trace_time_def          wk_trace_time;     //ﾄﾚｰｽ用処理時刻
    long long               wk_julian_time;    //ﾕﾘｳｽ暦
    lk_zac2001f_arg_1_def   wk_trace_info;     //ﾄﾚｰｽ出力情報

    s_err_code = 0;
    s_rtn_cd = 0;
    memset(&wk_trace_time, ' ', sizeof(trace_time_def) );
    wk_julian_time = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);

    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);

    //継続読込
    
    //LOCKﾌﾗｸﾞの判定
    if( in_inf->lock_flg == DEF_NOLOCK ) {

        READX ( file_inf->file_no
              , out_inf->rec_area
              , (short)in_inf->rec_len
              , (unsigned short *)&out_inf->rec_len
              );
    } else {

        READLOCKX ( file_inf->file_no
                  , out_inf->rec_area
                  , (short)in_inf->rec_len
                  , (unsigned short *)&out_inf->rec_len
                  );
    }

    s_err_code = 0;

    //読込結果取得
    s_err_code = COM_IOM_awaitio_proc( file_inf,
                               in_inf->io_timer,
                               out_inf );
    
    //読み込み結果取得のｴﾗｰ判定
    if( s_err_code != 0 ) {
        switch( s_err_code ) {
          case DEF_EOF:
            memmove( sub_status, DEF_EOF_ERR, DEF_STATUS_LEN );
            break;
          case DEF_TIMEOUT:
            memmove( sub_status, DEF_TIMEOUT_ERR, DEF_STATUS_LEN );
            break;
          default :
            memmove( sub_status, DEF_READ_ERR, DEF_STATUS_LEN );
            break;
        }
        out_inf->guardian_errcode = s_err_code;
        if(s_err_code != DEF_EOF){
            if( in_inf->lock_flg == DEF_NOLOCK ) {
                memmove( out_inf->err_proc,
                         DEF_PROCEDURE_READX,
                         strlen(DEF_PROCEDURE_READX) );
            } else {
                memmove( out_inf->err_proc,
                         DEF_PROCEDURE_READLOCKX,
                         strlen(DEF_PROCEDURE_READLOCKX) );
            }
            s_rtn_cd = 1;
        }
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
    }
    
    s_err_code = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);

    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    
    memcpy(&wk_trace_info.data_info.key_type, in_inf->key_type,
           sizeof(wk_trace_info.data_info.key_type));
    memset(&wk_trace_info.data_info.key_value, 0x20, sizeof(wk_trace_info.data_info.key_value));
    memcpy(&wk_trace_info.data_info.key_value, in_inf->key_value, in_inf->key_len);
    if(in_inf->asc_desc_type == DEF_SYOUJUN){
        memcpy(wk_trace_info.data_info.next_revs_type, DEF_READ_TYPE_NEXT,
               sizeof(wk_trace_info.data_info.next_revs_type));
    }else{
        memcpy(wk_trace_info.data_info.next_revs_type, DEF_READ_TYPE_REVS,
               sizeof(wk_trace_info.data_info.next_revs_type));
    }
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    sprintf(wk_trace_info.data_info.rec_len, "%05d", in_inf->rec_len);
    memcpy(&wk_trace_info.data_info.rec_area, out_inf->rec_area,
           sizeof(wk_trace_info.data_info.rec_area));
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);

    return s_rtn_cd;
} /* end of COM_IOM_file_next_read_proc */

/****************************************************************************/
/*  FUNCTION        : 1.4.0  COM_IOM_record_add_read_proc                   */
/*  CALLING SEQ.    : short COM_IOM_record_add_proc(char   *                */
/*                                                , struct *                */
/*                                                , struct *                */
/*                                                , struct *                */
/*                                                , struct *)               */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : レコード追加を行う                                    */
/****************************************************************************/
short COM_IOM_record_add_proc( char *sub_status,
                       COM_IOM_arg_3_def* trace_inf,
                       COM_IOM_arg_4_def* file_inf,
                       COM_IOM_arg_5_def* in_inf,
                       COM_IOM_arg_6_def* out_inf )
{
  
    //変数定義
    short                   s_err_code;         //ｴﾗｰｺｰﾄﾞ
    short                   s_rtn_cd;           //ﾘﾀｰﾝｺｰﾄﾞ
    trace_time_def          wk_trace_time;      //ﾄﾚｰｽ用処理時刻
    long long               wk_julian_time;     //ﾕﾘｳｽ暦
    char                    c_par_key_area[2];  //ﾊﾟｰﾃｨｼｮﾝｷｰｴﾘｱ
    lk_zac2001f_arg_1_def   wk_trace_info;      //ﾄﾚｰｽ出力情報

    s_err_code = 0;
    s_rtn_cd = 0;
    memset(&wk_trace_time, ' ', sizeof(trace_time_def) );
    wk_julian_time = 0;
    memset( c_par_key_area, ' ', sizeof(c_par_key_area) );
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);


    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);

    //パーティション識別の判定
    if(in_inf->part_key_type == DEF_PARTITION_KEY_KOTEI ||
       in_inf->part_key_type == DEF_PARTITION_KEY_MODULUS){

        //ﾊﾟｰﾃｨｼｮﾝｷｰ生成処理の呼出
        COM_IOM_create_par_key_proc(DEF_KINOU_ADD, in_inf, c_par_key_area);

        //ﾊﾟｰﾃｨｼｮﾝｷｰの設定
        memcpy(in_inf->rec_area, c_par_key_area, sizeof(c_par_key_area));
    }

    //ﾚｺｰﾄﾞ出力
    WRITEX ( file_inf->file_no
           , in_inf->rec_area
           , (short)in_inf->rec_len
           , (unsigned short *)&out_inf->rec_len
           );

    s_err_code = 0;

    //出力結果取得
    s_err_code = COM_IOM_awaitio_proc( file_inf,
                               in_inf->io_timer,
                               out_inf );

    //出力結果のｴﾗｰ判定
    if( s_err_code != 0 ) {
        memmove( out_inf->err_proc,
                 DEF_PROCEDURE_WRITEX,
                 strlen(DEF_PROCEDURE_WRITEX) );
        switch( s_err_code ) {
          case DEF_TIMEOUT:
            memmove( sub_status, DEF_TIMEOUT_ERR, DEF_STATUS_LEN );
            break;
          case DEF_DISCFULL:
            memmove( sub_status, DEF_DISCFULL_ERR, DEF_STATUS_LEN );
            break;
          case DEF_DUPLICATE:
            memmove( sub_status, DEF_DUPLICATE_ERR, DEF_STATUS_LEN );
            break;
          default :
            memmove( sub_status, DEF_WRITE_ERR, DEF_STATUS_LEN );
            break;
        }
        out_inf->guardian_errcode = s_err_code;
        s_rtn_cd = 1;
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
        memcpy(out_inf->rec_area, in_inf->rec_area, in_inf->rec_len);
    }
    
    s_err_code = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);

    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    sprintf(wk_trace_info.data_info.rec_len, "%05d", in_inf->rec_len);
    memcpy(&wk_trace_info.data_info.rec_area, in_inf->rec_area,
           sizeof(wk_trace_info.data_info.rec_area));
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);
    
    return s_rtn_cd;
} /* end of COM_IOM_record_add_proc */

/****************************************************************************/
/*  FUNCTION        : 1.5.0  COM_IOM_record_update_proc                     */
/*  CALLING SEQ.    : short COM_IOM_record_update_proc(char   *             */
/*                                                   , struct *             */
/*                                                   , struct *             */
/*                                                   , struct *             */
/*                                                   , struct *)            */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : レコード更新を行う                                    */
/****************************************************************************/
short COM_IOM_record_update_proc( char  *sub_status,
                          COM_IOM_arg_3_def* trace_inf,
                          COM_IOM_arg_4_def* file_inf,
                          COM_IOM_arg_5_def* in_inf,
                          COM_IOM_arg_6_def* out_inf )
{

    //変数定義
    short                   s_err_code;       //ｴﾗｰｺｰﾄﾞ
    short                   s_rtn_cd;         //ﾘﾀｰﾝｺｰﾄﾞ
    trace_time_def          wk_trace_time;    //ﾄﾚｰｽ用処理時刻
    long long               wk_julian_time;   //ﾕﾘｳｽ暦
    lk_zac2001f_arg_1_def   wk_trace_info;    //ﾄﾚｰｽ出力情報

    s_err_code = 0;
    s_rtn_cd = 0;
    memset(&wk_trace_time, ' ', sizeof(trace_time_def) );
    wk_julian_time = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);
    
    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);

    //ﾚｺｰﾄﾞ更新

    //LOCKﾌﾗｸﾞの判定
    if( in_inf->lock_flg == DEF_LOCKFREE ) {

        WRITEUPDATEUNLOCKX ( file_inf->file_no
                           , in_inf->rec_area
                           , (short)in_inf->rec_len
                           , (unsigned short *)&out_inf->rec_len
                           );
    } else {

        WRITEUPDATEX ( file_inf->file_no
                     , in_inf->rec_area
                     , (short)in_inf->rec_len
                     , (unsigned short *)&out_inf->rec_len
                     );
    }

    s_err_code = 0;

    //更新結果取得
    s_err_code = COM_IOM_awaitio_proc( file_inf,
                               in_inf->io_timer,
                               out_inf );

    //出力結果のエラー判定
    if( s_err_code != 0 ) {
        if( in_inf->lock_flg == DEF_LOCKFREE ) {
            memmove( out_inf->err_proc,
                     DEF_PROCEDURE_WRITEUPDATEUNLOCKX,
                     strlen(DEF_PROCEDURE_WRITEUPDATEUNLOCKX) );
        }
        else {
            memmove( out_inf->err_proc,
                     DEF_PROCEDURE_WRITEUPDATEX,
                     strlen(DEF_PROCEDURE_WRITEUPDATEX) );
        }
        switch( s_err_code ) {
          case DEF_TIMEOUT:
            memmove( sub_status, DEF_TIMEOUT_ERR, DEF_STATUS_LEN );
            break;
          case DEF_DISCFULL:
            memmove( sub_status, DEF_DISCFULL_ERR, DEF_STATUS_LEN );
            break;
          case DEF_DUPLICATE:
            memmove( sub_status, DEF_DUPLICATE_ERR, DEF_STATUS_LEN );
            break;
          default :
            memmove( sub_status, DEF_WRITE_ERR, DEF_STATUS_LEN );
            break;
        }
        out_inf->guardian_errcode = s_err_code;
        s_rtn_cd = 1;
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
    }

    s_err_code = 0;
    memcpy(out_inf->rec_area, in_inf->rec_area, in_inf->rec_len);

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);

    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    sprintf(wk_trace_info.data_info.rec_len, "%05d", in_inf->rec_len);
    memcpy(&wk_trace_info.data_info.rec_area, in_inf->rec_area,
           sizeof(wk_trace_info.data_info.rec_area));
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);
    
    return s_rtn_cd;
} /* end of COM_IOM_record_update_proc */

/****************************************************************************/
/*  FUNCTION        : 1.6.0  COM_IOM_record_delete_proc                     */
/*  CALLING SEQ.    : short COM_IOM_record_delete_proc(char   *             */
/*                                                   , struct *             */
/*                                                   , struct *             */
/*                                                   , struct *             */
/*                                                   , struct *)            */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : レコード削除を行う                                    */
/****************************************************************************/
short COM_IOM_record_delete_proc( char *sub_status,
                          COM_IOM_arg_3_def* trace_inf,
                          COM_IOM_arg_4_def* file_inf,
                          COM_IOM_arg_5_def* in_inf,
                          COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short                   s_err_code;       //ｴﾗｰｺｰﾄﾞ
    short                   s_rtn_cd;         //ﾘﾀｰﾝｺｰﾄﾞ
    trace_time_def          wk_trace_time;    //ﾄﾚｰｽ用処理時刻
    long long               wk_julian_time;   //ﾕﾘｳｽ暦
    lk_zac2001f_arg_1_def   wk_trace_info;    //ﾄﾚｰｽ出力情報

    s_err_code = 0;
    s_rtn_cd = 0;
    memset(&wk_trace_time, ' ', sizeof(trace_time_def) );
    wk_julian_time = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);
    
    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);

    //ﾚｺｰﾄﾞ削除
    WRITEUPDATEUNLOCKX ( file_inf->file_no
                       , in_inf->rec_area
                       , 0
                       , (unsigned short *)&out_inf->rec_len
                       );

    s_err_code = 0;

    //削除結果取得
    s_err_code = COM_IOM_awaitio_proc( file_inf,
                               in_inf->io_timer,
                               out_inf );
    
    //出力結果のエラー判定
    if( s_err_code != 0 ) {
        memmove( out_inf->err_proc,
                 DEF_PROCEDURE_WRITEUPDATEUNLOCKX,
                 strlen(DEF_PROCEDURE_WRITEUPDATEUNLOCKX) );
        switch( s_err_code ) {
          case DEF_TIMEOUT:
            memmove( sub_status, DEF_TIMEOUT_ERR, DEF_STATUS_LEN );
            break;
          case DEF_DISCFULL:
            memmove( sub_status, DEF_DISCFULL_ERR, DEF_STATUS_LEN );
            break;
          case DEF_DUPLICATE:
            memmove( sub_status, DEF_DUPLICATE_ERR, DEF_STATUS_LEN );
            break;
          default :
            memmove( sub_status, DEF_OTHER_ERR, DEF_STATUS_LEN );
            break;
        }
        out_inf->guardian_errcode = s_err_code;
        s_rtn_cd = 1;
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
    }

    s_rtn_cd = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);

    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    sprintf(wk_trace_info.data_info.rec_len, "%05d", in_inf->rec_len);
    memcpy(&wk_trace_info.data_info.rec_area, in_inf->rec_area,
           sizeof(wk_trace_info.data_info.rec_area));
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);
    
    return s_rtn_cd;
} /* end of COM_IOM_record_delete_proc */

/****************************************************************************/
/*  FUNCTION        : 1.7.0  COM_IOM_record_unlock_proc                     */
/*  CALLING SEQ.    : short COM_IOM_record_unlock_proc(char   *             */
/*                                                   , struct *             */
/*                                                   , struct *             */
/*                                                   , struct *             */
/*                                                   , struct *)            */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : レコードロックの解除を行う                            */
/****************************************************************************/
short COM_IOM_record_unlock_proc( char  *sub_status,
                          COM_IOM_arg_3_def* trace_inf,
                          COM_IOM_arg_4_def* file_inf,
                          COM_IOM_arg_5_def* in_inf,
                          COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short                   s_rtn_cd;         //ﾘﾀｰﾝｺｰﾄﾞ
    short                   s_err_code;       //ｴﾗｰｺｰﾄﾞ
    trace_time_def          wk_trace_time;    //ﾄﾚｰｽ用処理時刻
    long long               wk_julian_time;   //ﾕﾘｳｽ暦
    lk_zac2001f_arg_1_def   wk_trace_info;    //ﾄﾚｰｽ出力情報

    s_rtn_cd = 0;
    s_err_code = 0;
    memset(&wk_trace_time, ' ', sizeof(trace_time_def));
    wk_julian_time = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);

    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);

    //ﾚｺｰﾄﾞﾛｯｸ解除
    UNLOCKREC ( file_inf->file_no );

    s_err_code = 0;

    //解除結果取得
    s_err_code = COM_IOM_awaitio_proc( file_inf,
                               in_inf->io_timer,
                               out_inf );
    
    //出力結果のｴﾗｰ判定
    if( s_err_code != 0 ) {
        memmove( out_inf->err_proc,
                 DEF_PROCEDURE_UNLOCKREC,
                 strlen(DEF_PROCEDURE_UNLOCKREC) );
        memmove( sub_status, DEF_OTHER_ERR, DEF_STATUS_LEN );
        out_inf->guardian_errcode = s_err_code;
        s_rtn_cd = 1;
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
    }
    
    s_err_code = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);
    
    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);
    
    return s_rtn_cd;
} /* end of COM_IOM_record_unlock_proc */

/****************************************************************************/
/*  FUNCTION        : 1.8.0  COM_IOM_file_close_proc                        */
/*  CALLING SEQ.    : short COM_IOM_file_close_proc(char   *                */
/*                                                , struct *                */
/*                                                , struct *                */
/*                                                , struct *                */
/*                                                , struct *)               */
/*  ARGUMENT        : 1.sub_status    (O)   サブプログラムステータス        */
/*                  : 2.trace_inf     (I)   トレース情報                    */
/*                  : 3.file_inf      (I/O) ファイル情報                    */
/*                  : 4.in_inf        (I)   入力情報                        */
/*                  : 5.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : ファイルクローズ処理を行う                            */
/****************************************************************************/
short COM_IOM_file_close_proc( char  *sub_status,
                       COM_IOM_arg_3_def* trace_inf,
                       COM_IOM_arg_4_def* file_inf,
                       COM_IOM_arg_5_def* in_inf,
                       COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short                   s_rtn_cd;         //ﾘﾀｰﾝｺｰﾄﾞ
    short                   s_err_code;       //ｴﾗｰｺｰﾄﾞ
    trace_time_def          wk_trace_time;    //ﾄﾚｰｽ用処理時刻
    long long               wk_julian_time;   //ﾕﾘｳｽ暦
    lk_zac2001f_arg_1_def   wk_trace_info;    //ﾄﾚｰｽ出力情報

    s_rtn_cd = 0;
    s_err_code = 0;
    memset(&wk_trace_time, 0x00, sizeof(trace_time_def) );
    wk_julian_time = 0;
    memset(&wk_trace_info, ' ', lk_zac2001f_arg_1_def_Size);
    
    memset((char*)sub_status, ' ', DEF_STATUS_LEN);
    memset((char*)out_inf, ' ', sizeof(COM_IOM_arg_6_def));
    out_inf->guardian_errcode = 0;
    out_inf->rec_len = 0;

    //開始時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_start_time,(COM_SDT_arg_3_def *)wk_trace_time.b_start_time,&wk_julian_time);
    
    //ﾌｧｲﾙｸﾛｰｽﾞ
    s_err_code = FILE_CLOSE_ ( file_inf->file_no );

    //ﾌｧｲﾙｸﾛｰｽﾞ結果判定
    if( s_err_code != 0 ) {
        memmove( sub_status, DEF_OTHER_ERR, DEF_STATUS_LEN );
        memmove( out_inf->err_proc,
                 DEF_PROCEDURE_FILE_CLOSE_,
                 strlen(DEF_PROCEDURE_FILE_CLOSE_) );
        out_inf->guardian_errcode = s_err_code;
        s_rtn_cd = 1;
    } else {
        memmove( sub_status, DEF_NO_ERR, DEF_STATUS_LEN );
    }
    
    s_err_code = 0;

    //終了時間を取得
    COM_SDT(DEF_JPN_TIME,(COM_SDT_arg_2_def *)wk_trace_time.c_end_time,(COM_SDT_arg_3_def *)wk_trace_time.b_end_time,&wk_julian_time);

    //ﾄﾚｰｽ出力
    wk_trace_info.func_flg = '1';
    memcpy(&wk_trace_info.trace_info.prog_id, trace_inf->prog_id,
           sizeof(wk_trace_info.trace_info.prog_id));
    memcpy(&wk_trace_info.trace_info.file_id, file_inf->file_id,
           sizeof(wk_trace_info.trace_info.file_id));
    memcpy(&wk_trace_info.trace_info.file_name, file_inf->file_name,
           sizeof(wk_trace_info.trace_info.file_name));
    memcpy(&wk_trace_info.trace_info.file_io_type, trace_inf->file_io_type,
           sizeof(wk_trace_info.trace_info.file_io_type));
    
    sprintf(wk_trace_info.data_info.guardian_errcode, "%04d", out_inf->guardian_errcode);
    memcpy(&wk_trace_info.data_info.shori_start_time, &wk_trace_time.c_start_time[8],
           sizeof(wk_trace_info.data_info.shori_start_time));
    memcpy(&wk_trace_info.data_info.shori_end_time, &wk_trace_time.c_end_time[8],
           sizeof(wk_trace_info.data_info.shori_end_time));
    
    TRACEOUT((char *)&wk_trace_info);
    
    return s_rtn_cd;

} /* end of COM_IOM_file_close_proc */

/****************************************************************************/
/*  FUNCTION        : 1.9.0  COM_IOM_awaitio_proc                           */
/*  CALLING SEQ.    : short COM_IOM_awaitio_proc(struct *                   */
/*                                             , long                       */
/*                                             , struct *)                  */
/*  ARGUMENT        : 1.sub_status    (O)   ファイル情報                    */
/*                  : 2.io_timer      (I)   IOタイマー                      */
/*                  : 3.out_inf       (I/O) 出力情報                        */
/*  RETURN CODE     : 0:正常 1:障害発生                                     */
/*  DESCRIPTION     : AWAIT I/O処理を行う                                   */
/****************************************************************************/
short COM_IOM_awaitio_proc( COM_IOM_arg_4_def* file_inf,
                    long               io_timer,
                    COM_IOM_arg_6_def* out_inf )
{
    //変数定義
    short       s_err_code;   //ｴﾗｰｺｰﾄﾞ
    long        l_adder;
    short       s_rec_len;

    s_err_code = 0;
    l_adder    = 0;
    
    //AWAIT I/O
    AWAITIOX ( &file_inf->file_no
             , &l_adder
             , (unsigned short *)&s_rec_len
             ,
             , io_timer
             );

    //Guardianｴﾗｰｺｰﾄﾞを取得
    s_err_code = FILE_GETINFO_ ( file_inf->file_no
                               , &out_inf->guardian_errcode
                               );
    
    //Guardianエラーコードの判定
    if( out_inf->guardian_errcode != 0 ) {
      s_err_code = out_inf->guardian_errcode;
    }

    out_inf->rec_len = s_rec_len;
    return s_err_code;
} /* end of COM_IOM_awaitio_proc */

/****************************************************************************/
/*  FUNCTION        : 1.10.0  COM_IOM_convert_key_type_proc                 */
/*  CALLING SEQ.    : short COM_IOM_convert_key_type_proc(char *)           */
/*  ARGUMENT        : 1.key_siki      (O)   キー識別                        */
/*  RETURN CODE     : キー種別                                              */
/*  DESCRIPTION     : 入力情報のキー識別をキー種別へ変換を行う              */
/****************************************************************************/
short COM_IOM_convert_key_type_proc( char*  key_siki )
{

    //変数定義
    short       s_key_spc;  //変換後ｷｰｴﾘｱ

    s_key_spc = 0;
    
    //入力情報のｷｰ識別の判定

    if( memcmp( key_siki, DEF_KEYSIKI_PRI, DEF_KEYSIKI_LEN ) == 0 ) {
        s_key_spc = 0;
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A1, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A1, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A2, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A2, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A3, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A3, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A4, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A4, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A5, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A5, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A6, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A6, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A7, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A7, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A8, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A8, sizeof(s_key_spc));
    }
    else if( memcmp( key_siki, DEF_KEYSIKI_A9, DEF_KEYSIKI_LEN ) == 0 ) {
        memmove( (char*)&s_key_spc, DEF_KEYSPC_A9, sizeof(s_key_spc));
    }
  
    return s_key_spc;
} /* end of COM_IOM_convert_key_type_proc */

/****************************************************************************/
/*  FUNCTION        : 1.11.0  COM_IOM_create_par_key_proc                   */
/*  CALLING SEQ.    : void  COM_IOM_create_par_key_proc(char   *            */
/*                  :                                  ,struct *            */
/*                  :                                  ,char   *)           */
/*  ARGUMENT        : 1.key_siki      (O)   キー識別                        */
/*                  : 2.in_inf        (I)   入力情報                        */
/*                  ; 3.create_key    (O)   生成キー                        */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : キー情報をもとにパーションキーを生成する              */
/****************************************************************************/
void COM_IOM_create_par_key_proc( char*                  func_type,
                          COM_IOM_arg_5_def* in_inf,
                          char*                 create_key)
{
    //変数定義
    char  s_key_spc[3];      // 生成キー
    char  s_change_str[2];   // 数字変換エリア
    short s_get_number;      // 取得値
    short s_keisu;           // 計数
    short s_key_size;        // キー長
    short s_mos_total_batch; // モジュラス合計値(一括)
    short s_mos_total_split; // モジュラス合計値(分割)

    memset(s_key_spc, ' ', sizeof(s_key_spc));
    s_get_number = 0;
    s_keisu           = 2;
    s_key_size        = 0;
    s_mos_total_batch = 0;
    s_mos_total_split = 0;

    //キー識別の判定
    if(in_inf->part_key_type == DEF_PARTITION_KEY_KOTEI){
        s_key_size = 2;
        //固定キーのキー生成
        if( memcmp( func_type, DEF_KINOU_SRED, DEF_KINOU_LEN ) == 0 ) {
            memcpy(create_key,
                &in_inf->key_value[in_inf->part_key_position - s_key_size - 1], s_key_size);
        }else{
            memcpy(create_key, &in_inf->rec_area[in_inf->part_key_position - 1], s_key_size);
        }
        return;
    }else{
        s_key_size = in_inf->part_key_len;
    }
    
    //ﾓｼﾞｭﾗｽｷｰのｷｰ生成処理
    
    while(s_key_size > 0){

        memset(s_change_str, 0x00, sizeof(s_change_str));

        //数値変換対象の値を取得
        if( memcmp( func_type, DEF_KINOU_SRED, DEF_KINOU_LEN ) == 0 ) {
            memcpy(s_change_str,
                &in_inf->key_value[in_inf->part_key_position - 2 - 1 + s_key_size - 1], 1);
        }else{
            memcpy(s_change_str,
                &in_inf->rec_area[in_inf->part_key_position -1 + s_key_size - 1], 1);
        }

        //文字を数値に変換
        if (isdigit(s_change_str[0])) {
            s_get_number = (short)atoi(s_change_str);
        }else{ /* 数字以外はゼロとして扱う */
            s_get_number = 0;
        }

        //モジュラス合計値(一括)を加算
        s_mos_total_batch += (s_get_number * s_keisu);

        //係数の切り替え
        if(s_keisu == 1){
            s_keisu = 2;
        }else{
            s_get_number = s_get_number * s_keisu;
            s_keisu = 1;
        }

        //取得値のが10を超えるかの判定
        if(s_get_number >=10 ){
            s_mos_total_split += s_get_number % 10;
            s_mos_total_split += s_get_number / 10;
        }else{
            s_mos_total_split += s_get_number;
        }

        //ｷｰ長の減算
        s_key_size--;
    }
    
    s_mos_total_batch = (short)(s_mos_total_batch % 10);
    s_mos_total_batch = (short)((10 - s_mos_total_batch) % 10);
    
    s_mos_total_split = (short)(s_mos_total_split % 10);
    s_mos_total_split = (short)((10 - s_mos_total_split) % 10);
    
    //ｷｰの生成
    sprintf(s_key_spc, "%d%d", s_mos_total_batch, s_mos_total_split);

    //数値変換対象の値を取得
    memcpy(create_key, s_key_spc, 2);

  return;
} /* end of COM_IOM_create_par_key_proc */
