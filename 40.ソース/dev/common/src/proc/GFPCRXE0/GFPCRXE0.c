/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCRXE0                                    */
/*        FUNCTION          ････ コマンドI/F                                 */
/*                                                                           */
/*                               画面またはINファイルでコマンド入力を受け    */
/*                               付け、入力されたコマンドを精査し、コマン    */
/*                               ドサーバに送信する。                        */
/*                                                                           */
/*                                                                           */
/*        AUTHER            ････ ISYS K.Mishima                              */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-10-24                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  K.Mishima  2024/10/24 (J0680)新規作成                               */
/*  1.0  K.Mishima  2025/12/18 (J0680)fgetsのエラーハンドリングを追加        */
/*                                                                           */
/*****************************************************************************/
#include "GFPCRXE0.h"   nolist      // コマンドI/Fヘッダ
#include "vproc.h"      nolist      // vproc

// GLOBAL
static my_info_def t_my_info;
static infile_info_def t_infile_info;
static command_sep_def t_command_sep;
static timer_value_def t_timer_value;
static oggz1in_def t_psd_ems;
static COM_PSD_arg_4_def t_psd_add;
// トレース出力モジュール用
char  EXMYSRVCLSNAME[15];
char  EXMYPROCNAME[6];
short EXTRACEMODE;
char  EXTRACEFILENAME[47];
short EXTRACEFILENO;


/****************************************************************************/
/*  FUNCTION        : 0.0.0  main                                           */
/*  CALLING SEQ.    : int  main(void)                                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : 0                                                     */
/*  DESCRIPTION     : エントリポイント                                      */
/****************************************************************************/
int main(void)
{
    /*------------------------------------------------*/
    /*    初期処理                                    */
    /*------------------------------------------------*/
    CMDI_init();

    /*------------------------------------------------*/
    /*    主処理                                      */
    /*------------------------------------------------*/
    CMDI_main();

    /*------------------------------------------------*/
    /*    終了処理                                    */
    /*------------------------------------------------*/
    CMDI_finish();

    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 1.0.0  CMDI_init                                      */
/*  CALLING SEQ.    : void  CMDI_init(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 初期処理                                              */
/****************************************************************************/
void CMDI_init(void)
{
    /* 変数定義 */
    startup_msg_type t_startup_msg;
    short startup_len;
    char fname[ZSYS_VAL_LEN_FILENAME+1];
    short fnm_len = 0;
    short return_code;
    short s_error = 0;
    char iom_func_type[4];                      //機能名識別
    char iom_sub_prog_sts[2];                   //サブプログラムステータス
    COM_IOM_arg_3_def iom_trc;                  //トレース情報
    COM_IOM_arg_4_def iom_fil;                  //ファイル情報
    COM_IOM_arg_5_def iom_inp;                  //入力情報
    COM_IOM_arg_6_def iom_out;                  //出力情報
    db_gfphi_def       t_gfphi_pk;              //物理名情報ファイルPK
    db_gfphi_def*      pt_gfphi_rec;            //物理名情報ファイルレコード
    lk_zac2001r_arg_1_def trace_if;             //トレースI/F
    char wk_volume[8];
    char wk_subvolume[8];
    char wk_file[8];
    char filename[24];
    short file_num;
    long space_loc;
    char assign_file_id[32];
    char cnvstr[64];

    /* グローバルの初期化 */
    memset(&t_my_info,NULL, sizeof(t_my_info));
    memset(t_my_info.cmd_srv_domain_name,0x20,sizeof(t_my_info.cmd_srv_domain_name));
    memset(t_my_info.cmd_srv_serverclass_name,0x20,sizeof(t_my_info.cmd_srv_serverclass_name));
    memset(&t_infile_info,NULL, sizeof(t_infile_info));
    memset(&t_timer_value,NULL, sizeof(t_timer_value));
    t_my_info.completion_code = DEF_COMPLETION_NORMAL;

    /* ローカル変数の初期化 */
    memset(wk_volume,NULL,sizeof(wk_volume));
    memset(wk_subvolume,NULL,sizeof(wk_subvolume));
    memset(wk_file,NULL,sizeof(wk_file));
    memset(fname,NULL,sizeof(fname));
    memset(filename,NULL,sizeof(filename));
    memset(assign_file_id,0x20,sizeof(assign_file_id)-1);

    // EMSリンケージ：N/W識別変換
    struct __cnvnw {
        char nwid;
        char *nwdiv;
    } cnvnw[] = {
        {DEF_NW_ID_JCN,      DEF_NW_KUBUN_CARDNET},         // JCN(CUP)
        {DEF_NW_ID_VISA,     DEF_NW_KUBUN_VISANET},         // Visanet
        {DEF_NW_ID_MASTER,   DEF_NW_KUBUN_BANKNET},         // Banknet
        {DEF_NW_ID_AMEX,     DEF_NW_KUBUN_AEGN},            // AEGEN
        {DEF_NW_ID_DISCOVER, DEF_NW_KUBUN_DISCOVER},        // Discover
        {DEF_NW_ID_NYCE,     DEF_NW_KUBUN_NYCE},            // NYCE
        {DEF_NW_ID_JLink,    DEF_NW_KUBUN_JLINK},           // J-Link(通信受信時)
        {DEF_NW_ID_UnionPay, DEF_NW_KUBUN_UNIONPAY}         // UnionPay
    };


    /* 標準入力のOPEN */
    return_code = fopen_std_file(0,s_error);
    if(return_code){
        //標準出力OPEN前のため個別でabend
        //コンプリッションコードを異常でセット
        t_my_info.completion_code = DEF_COMPLETION_ERROR;
        // プロセス終了
        PROCESS_STOP_(, , DEF_PROC_ABNORMAL_END,t_my_info.completion_code);
    }
    /* 標準出力のOPEN */ 
    return_code = fopen_std_file(1,s_error);
    if(return_code){
        //標準出力OPENエラー時は標準出力ができないため個別でabend
        //コンプリッションコードを異常でセット
        t_my_info.completion_code = DEF_COMPLETION_ERROR;
        // プロセス終了
        PROCESS_STOP_(, , DEF_PROC_ABNORMAL_END,t_my_info.completion_code);
    }

    /* startup_msg取得 */
    return_code = get_startup_msg(&t_startup_msg,&startup_len);
    if(return_code){
        printf("startup msg error :No %d\n",return_code);
        // 異常終了処理
        CMDI_abend();
    }


    /* 起動モード設定 */
    if (memcmp(t_startup_msg.infile.parts.file," ",1) == 0)
    {
        /* プロンプトコマンドモードに設定 */
        t_my_info.startup_mode = DEF_INP_PROMPT_MODE;
    }else{
        /* インファイルコマンドモードに設定 */
        t_my_info.startup_mode = DEF_INP_INFILE_MODE;
    }

    /* 出力モード設定 */
    if (memcmp(t_startup_msg.outfile.parts.file," ",1) == 0)
    {
        /* プロンプト出力モードに設定 */
        t_my_info.output_mode = DEF_OUT_PROMPT_MODE;
    }else{
        /* OUTファイル出力モードに設定 */
        t_my_info.output_mode = DEF_OUT_OUTFILE_MODE;
    }

    if(t_my_info.startup_mode == DEF_INP_INFILE_MODE){

        /* EDITREADの初期設定 */
        return_code = EDITREADINIT(t_infile_info.edit_controlblk
                    ,DEF_INFILE_NUM
                    ,DEF_INFILE_BUFFER_LEN);
        if(return_code){
            printf("EDITREADINIT error:%d\n",return_code);
            // 異常終了処理
            CMDI_abend();
        }
        /* wkエリアの初期化 */
        memset(wk_volume,NULL,sizeof(wk_volume));
        memset(wk_subvolume,NULL,sizeof(wk_subvolume));
        memset(wk_file,NULL,sizeof(wk_file));
    }
    /* パラメータ取得 */
    CMDI_get_params();
    /* プロセス情報取得 */
    return_code = COM_PRC(&t_my_info.procinfo);
    if(return_code){
        printf("process info error :No %d\n",return_code);
        // 異常終了処理
        CMDI_abend();
    }
    // EMS出力モジュールI/F用変数初期化
    memset(&t_psd_ems, ' ', sizeof(t_psd_ems));
    //運用監視端末プロセスタイマー設定
    snprintf(cnvstr,sizeof(cnvstr),"%4d",t_timer_value.pathsend_timer);
    memcpy(t_psd_ems.uytrminf.proctimer, cnvstr, strlen(cnvstr));
    //運用監視端末出力PATHMON名長
    snprintf(cnvstr, sizeof(cnvstr), "%02d", strlen(t_my_info.ems_mon_name));
    memcpy(t_psd_ems.uytrminf.uytrmmonlen, cnvstr, strlen(cnvstr));
    //運用監視端末出力PATHMON名
    memcpy(t_psd_ems.uytrminf.uytrmmon,t_my_info.ems_mon_name, strlen(t_my_info.ems_mon_name));
    //運用監視端末出力サーバクラス名長
    snprintf(cnvstr, sizeof(cnvstr), "%02d", strlen(t_my_info.ems_srv_name));
    memcpy(t_psd_ems.uytrminf.uytrmsrvlen, cnvstr, strlen(cnvstr));
    //運用監視端末出力サーバクラス名
    memcpy(t_psd_ems.uytrminf.uytrmsrv,t_my_info.ems_srv_name,strlen(t_my_info.ems_srv_name));

    // システム名(GFP)
    memcpy(t_psd_ems.emsinf.emsgkinf.sysnm, DEF_EMS_SYSNM_GFP, sizeof(DEF_EMS_SYSNM_GFP)-1);
    // サーバ分類(COM)
    memcpy(t_psd_ems.emsinf.emsgkinf.srv_kbn, DEF_EMS_SRV_KBN_COM, sizeof(DEF_EMS_SRV_KBN_COM)-1);
    // メッセージ出力元プログラム名
    memcpy(t_psd_ems.emsinf.emsgkinf.prgid, DEF_CMDIF_PROGRAM_ID, sizeof(DEF_CMDIF_PROGRAM_ID)-1);
    // メッセージ出力元プロセス名
    memcpy(t_psd_ems.emsinf.emsgkinf.trmnm, t_my_info.procinfo.my_pname, strlen(t_my_info.procinfo.my_pname));

    // N/W識別を変換してEMSリンケージへ設定
    for (int i = 0; i < sizeof(cnvnw)/sizeof(struct __cnvnw); ++i) {
        if (cnvnw[i].nwid == t_my_info.my_env.nw_id) {
            memcpy(t_psd_ems.emsinf.emsgkinf.h_nw_kbn, cnvnw[i].nwdiv, strlen(cnvnw[i].nwdiv));
            break;
        }
    }

    // Pathsendモジュールarg4変数初期化
    memset(&t_psd_add, ' ', sizeof(t_psd_add));
    // メッセージ出力元プログラム名(arg4)
    memcpy(t_psd_add.srv_logical_id, DEF_CMDIF_PROGRAM_ID, sizeof(DEF_CMDIF_PROGRAM_ID)-1);

    
    /* トレースモジュール初期化*/
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    trace_if.func_flg = DEF_TRACE_FUNC_INI;
    memcpy(trace_if.trace_info.prog_id,DEF_CMDIF_PROGRAM_ID, sizeof(DEF_CMDIF_PROGRAM_ID)-1);
    return_code = TRACEOUT((char *)&trace_if);
    if (return_code != 1){
        printf("TRACEINIT error\n");
        CMDI_abend();
    }

    /* 物理名情報ファイルのASSIGN情報取得 */
    memcpy(assign_file_id,DEF_ASN_GFPHI,strlen(DEF_ASN_GFPHI));
    COM_ASN( assign_file_id,fname,&fnm_len);
    if(fnm_len == 0){
        printf("ASSIGN GFPHI error\n");
        CMDI_abend();
    }

    /* IOモジュール(物理名情報ファイル)の共通部分を設定 */
    memset(&iom_trc, 0x20, sizeof(iom_trc));
    memcpy(iom_trc.prog_id, DEF_CMDIF_PROGRAM_ID, sizeof(DEF_CMDIF_PROGRAM_ID)-1);
    memcpy(iom_trc.file_id, DEF_LOGICALNAME_GFPHI,
           sizeof(DEF_LOGICALNAME_GFPHI)-1);
    memcpy(iom_trc.file_name, fname, fnm_len);
    memset(&iom_fil, 0x20, sizeof(iom_fil));
    memcpy(iom_fil.file_id,DEF_LOGICALNAME_GFPHI ,
           strlen(DEF_LOGICALNAME_GFPHI));
    memcpy(iom_fil.file_name, fname, fnm_len);
    /* 物理名情報ファイルOPEN時のarg初期化 */
    memset(iom_func_type,0x20,sizeof(iom_func_type));
    memset(&iom_sub_prog_sts, 0x20, sizeof(iom_sub_prog_sts));
    memset(&iom_out,0x20, sizeof(iom_out));

    /* 物理名情報ファイルOPEN時の呼び出しarg設定 */
    memcpy(iom_func_type,DEF_COM_IOM_FUNC_OPEN,sizeof(iom_func_type));
    memcpy(iom_trc.file_io_type, DEF_TRC_TYPE_OPEN,strlen(DEF_TRC_TYPE_OPEN));
    memset(&iom_inp,0x20,sizeof(iom_inp));
    memcpy(&iom_inp.io_timer,&t_timer_value.io_timer,sizeof(iom_inp.io_timer));

    /* 物理名情報ファイルOPEN */
    return_code = COM_IOM(iom_func_type,iom_sub_prog_sts,&iom_trc
                         ,&iom_fil,&iom_inp,&iom_out);
    if(return_code){
        printf("FILE OPEN error GFPHI:%d\n",return_code);
        printf("GUARDIAN ERROR CODE:%d\n",iom_out.guardian_errcode);
        // 異常終了処理
        CMDI_abend();
    }

    /* 物理名情報ファイルREAD時のkeyvalue編集 */
    memcpy(&t_gfphi_pk.pri_key,&t_my_info.my_env,sizeof(t_my_info.my_env));
    memcpy(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,DEF_LOGICALNAME_CMDSV
          ,sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memset(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num,'0'
          ,sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memset(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num,'0'
          ,sizeof(t_gfphi_pk.pri_key.srv_cls_key.srv_cls_mlt_num));
    memset(&t_gfphi_pk.pri_key.prc_file_key,'}'
          ,sizeof(t_gfphi_pk.pri_key.prc_file_key));

    /* 物理名情報ファイルREAD時のarg初期化 */
    memset(&iom_sub_prog_sts, 0x20, sizeof(iom_sub_prog_sts));
    memset(&iom_out,0x20, sizeof(iom_out));

    /* 物理名情報ファイルREAD時の呼び出しarg設定 */
    iom_inp.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    iom_inp.part_key_position = 0;
    iom_inp.part_key_len = 0;
    memcpy(&iom_inp.key_value,&t_gfphi_pk.pri_key,sizeof(t_gfphi_pk.pri_key));
    memcpy(&iom_inp.key_type,DEF_COM_IOM_KEYTYPE_PRI,sizeof(&iom_inp.key_type));
    iom_inp.key_len = sizeof(t_gfphi_pk.pri_key);
    iom_inp.compare_len = sizeof(t_gfphi_pk.pri_key);
    iom_inp.positioning_mode = DEF_MODE_EXACT;
    iom_inp.lock_flg = DEF_COM_IOM_NOLOCK;
    iom_inp.asc_desc_type = DEF_COM_IOM_ASCEND;
    iom_inp.io_timer = t_timer_value.io_timer;
    iom_inp.rec_len = sizeof(t_gfphi_pk);

    /* ファイルIOモジュール呼び出し */
    return_code = COM_IOM(DEF_COM_IOM_FUNC_STARTREAD,iom_sub_prog_sts,&iom_trc
                         ,&iom_fil,&iom_inp,&iom_out);
    /* リターンコードチェック */
    if(return_code){
        printf("FILE READ error GFPHI\n");
        printf("GUARDIAN ERROR CODE:%d",iom_out.guardian_errcode);
        // 異常終了処理
        CMDI_abend();
    }
    /* レコードが見つからない場合 */
    if(memcmp(iom_sub_prog_sts,DEF_COM_IOM_EOF_ERR,sizeof(iom_sub_prog_sts)) == 0){
        printf("RECORD NOT FOUND CMDSRV");
        CMDI_abend();
    }
    /* レコード領域を構造体の形で読む */
    pt_gfphi_rec = (db_gfphi_def*)iom_out.rec_area;

    /* レコード領域からコマンドサーバのPATHMON名、サーバクラス名を取得する。 */
    memcpy(t_my_info.cmd_srv_domain_name,pt_gfphi_rec->srv_cls_info.domain_name
           ,sizeof(t_my_info.cmd_srv_domain_name));
    memcpy(t_my_info.cmd_srv_serverclass_name,pt_gfphi_rec->srv_cls_info.srv_cls_name
           ,sizeof(t_my_info.cmd_srv_serverclass_name));

    /* 物理名情報ファイルCLOSE時のarg初期化 */
    memset(&iom_sub_prog_sts, 0x20, sizeof(iom_sub_prog_sts));
    memset(&iom_out,0x20,sizeof(iom_out));
    memset(&iom_inp,0x20,sizeof(iom_inp));

    /* 物理名情報ファイルCLOSE時の呼び出しarg設定 */
    memcpy(&iom_inp.io_timer,&t_timer_value.io_timer,sizeof(iom_inp.io_timer));

    /* IOモジュール呼び出し */
    return_code = COM_IOM(DEF_COM_IOM_FUNC_CLOSE,iom_sub_prog_sts,&iom_trc
                         ,&iom_fil,&iom_inp,&iom_out);


}
/****************************************************************************/
/*  FUNCTION        : 2.0.0  CMDI_main                                      */
/*  CALLING SEQ.    : void  CMDI_main(void)                                 */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 主処理                                                */
/****************************************************************************/
void CMDI_main(void)
{
    while(t_my_info.end_flag == DEF_FLG_OFF){
        /*起動モード判定*/
        if(t_my_info.startup_mode == DEF_INP_PROMPT_MODE){
            /*プロンプトコマンド入力処理呼び出し*/
            CMDI_input_prompt_cmd();
        }else{
            /*INファイルコマンド入力処理呼び出し*/
            CMDI_input_infile_cmd();
        }
        /*処理続行フラグ判定*/
        if(t_my_info.continue_flag == DEF_FLG_ON){
            /*コマンド判定処理呼び出し*/
            CMDI_select_command();
        }
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_finish                                     */
/*  CALLING SEQ.    : void  CMDI_finish(void)                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 終了処理                                              */
/****************************************************************************/
void CMDI_finish(void)
{
    //ローカル変数宣言
    lk_zac2001r_arg_1_def trace_if;

    // 終了メッセージ出力
    CMDI_output_msg(DEF_CODE_NORMAL_END);
    // バッファ出力
    fflush(stdout);

    //トレースモジュール終了処理
    memset ((char *)&trace_if, ' ', sizeof(trace_if));
    memcpy(trace_if.trace_info.prog_id,DEF_CMDIF_PROGRAM_ID , sizeof(DEF_CMDIF_PROGRAM_ID)-1);
    trace_if.func_flg = DEF_TRACE_FUNC_END;
    TRACEOUT((char *)&trace_if);

    //プロセス終了
    PROCESS_STOP_(, , DEF_PROC_NORMAL_END,t_my_info.completion_code);
}


/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_get_params                                 */
/*  CALLING SEQ.    : void  CMDI_get_params(void)                           */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : パラメータ取得処理                                    */
/****************************************************************************/
void CMDI_get_params(void)
{
    /* 変数宣言 */
    char buffer[80];
    short return_code;
    connection_id_def t_connection_id;

    /* 初期化 */
    memset(buffer,  NULL, sizeof(buffer));
    memset(&t_connection_id,0x20,sizeof(t_connection_id));

    /* PATHSENDタイマー取得 */
    return_code = get_param_by_name(DEF_PSEND_TIMER_10MSECOND,buffer
                                ,(short)sizeof(buffer));
    /* リターンコードチェック */
    if (return_code != 0){
        printf("get_param_by_name(PSEND-TIMER-10MSECOND) error :%d\n",return_code);
        CMDI_abend();
    }
    /* 取得したパラメータが数値であるかを確認する */
    if(CMDI_isStrDigit(buffer) == 0){
        printf("PSEND-TIMER-10MSECOND FORMAT ERROR\n");
        CMDI_abend();
    }
    /* 取得したパラメータを数値に変換し、グローバル変数に設定 */
    t_timer_value.pathsend_timer=atol(buffer);
    /* バッファ初期化 */
    memset(buffer,  NULL, sizeof(buffer));

    /* PATHSENDリトライ回数取得 */
    return_code = get_param_by_name(DEF_PSEND_RETRY_CNT,buffer
                                 ,(short)sizeof(buffer)-1);

    /* リターンコードチェック */
    if (return_code != 0){
        printf("get_param_by_name(PSEND-RETRY-CNT) error :%d\n",return_code);
        CMDI_abend();
    }
    /* 取得したパラメータが数値であるかを確認する */
    if(CMDI_isStrDigit(buffer) == 0){
        printf("PSEND-RETRY-CNT FORMAT ERROR\n");
        CMDI_abend();
    }
    /* 取得したパラメータがshortの最大値を超えていないか確認する */
    if(memcmp(buffer,DEF_SHORT_MAX,sizeof(DEF_SHORT_MAX)+1)>0){
        printf("PSEND-RETRY-CNT FORMAT ERROR\n");
        CMDI_abend();
    }

    /* 取得したパラメータをshortに変換し、グローバル変数に設定 */
    t_timer_value.pathsend_retry_num = (short) atoi(buffer);

    /* バッファ初期化 */
    memset(buffer,  NULL, sizeof(buffer));

    /* IOタイマー取得 */
    return_code = get_param_by_name(DEF_FILE_IO_TIMER_10MSECOND,buffer
                                ,(short)sizeof(buffer)-1);
    /* リターンコードチェック */
    if (return_code != 0){
        printf("get_param_by_name(FILE-IO-TIMER-10MSECOND) error :%d\n",return_code);
        CMDI_abend();
    }
    /* 取得したパラメータが数値であるかを確認する */
    if(CMDI_isStrDigit(buffer) == 0){
        printf("FILE-IO-TIMER-10MSECOND FORMAT ERROR\n");
        CMDI_abend();
    }

    /* 取得したパラメータをshortに変換し、グローバル変数に設定 */
    t_timer_value.io_timer = atol(buffer);

    /* バッファ初期化 */
    memset(buffer,  NULL, sizeof(buffer));

    /* 自環境情報の取得 */
    return_code = get_param_by_name(DEF_ENVIRONMENT_GROUP_ID,buffer
                                ,(short)sizeof(buffer)-1);

    /* リターンコードチェック */
    if (return_code != 0){
        printf("get_param_by_name(ENVIRONMENT-GROUP-ID) error :%d\n",return_code);
        CMDI_abend();
    }
    /* ハイフン区切りで取得 */
    return_code = CMDI_sep_identifier(buffer,&t_connection_id);

    /* リターンコードチェック */
    if (return_code != 0){
        printf("ENVIRONMENT-GROUP-ID FORMAT ERROR\n");
        CMDI_abend();
    }
    /* 自環境情報に値を格納する */
    memcpy(&t_my_info.my_env,&t_connection_id,sizeof(t_my_info.my_env));

    /* バッファ初期化 */
    memset(buffer,  NULL, sizeof(buffer));
    /* 運用監視端末PATHMON名の取得 */
    return_code = get_param_by_name(DEF_MSG_MON_NAME,buffer,(short)sizeof(buffer)-1);
    /* リターンコードチェック */
    if (return_code != 0){
        printf("get_param_by_name(MSG-MON-NAME) error :%d\n",return_code);
        CMDI_abend();
    }
    /* メッセージ出力PATHMON名に値を格納する */
    strncpy(t_my_info.ems_mon_name,buffer,sizeof(t_my_info.ems_mon_name)-1);

    /* バッファ初期化 */
    memset(buffer,  NULL, sizeof(buffer));
    /* 運用監視端末サーバクラス名の取得 */
    return_code = get_param_by_name(DEF_MSG_SRV_NAME,buffer,(short)sizeof(buffer)-1);
    /* リターンコードチェック */
    if (return_code != 0){
        printf("get_param_by_name(MSG-SRV-NAME) error :%d\n",return_code);
        CMDI_abend();
    }
    /* メッセージ出力PATHMON名に値を格納する */
    strncpy(t_my_info.ems_srv_name,buffer,sizeof(t_my_info.ems_srv_name)-1);

    
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_input_prompt_cmd                           */
/*  CALLING SEQ.    : void  CMDI_input_prompt_cmd(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : プロンプトコマンド入力処理                            */
/****************************************************************************/
void CMDI_input_prompt_cmd(void)
{
    /* 変数宣言 */
    short command_sep_num = 0;
    short return_code =0;
    char buffer[80];
    char *pt = NULL;
    int err_tmp = 0;
 
    /* 変数初期化 */
    memset(&t_command_sep,NULL,sizeof(command_sep_def));
    memset(buffer,NULL,sizeof(buffer));
    t_my_info.continue_flag = DEF_FLG_OFF;
 
    /* 入力受付 */
    printf(">> ");
    fflush(stdout);
 
    // 標準入力から読込
    pt = fgets(buffer,sizeof(buffer)-1,stdin) ;
    err_tmp = errno;
    if (err_tmp != 0) {
        // 標準入力のIOが失敗。Abendする。
        CMDI_abend();
    }
    else if (pt[0] == '\n') {
        // 空エンターの場合リターン
        return;
    }

    size_t buffer_len = strlen(buffer);
    //改行文字まで読み込めていない場合(80文字以上入力された場合)
    if(buffer[buffer_len-1] != '\n'){
        // 入力バッファの残留分をクリア
        while(getchar() != '\n');
    }

    // 改行文字を取り除く
    if (buffer_len > 0 && buffer[buffer_len - 1] == '\n'){
        buffer[buffer_len - 1] = NULL;
    }
    //コマンドを格納
    strncpy(t_command_sep.input_command,buffer,sizeof(t_command_sep.input_command)-1);
 
    /* OUTファイル出力モードの時、コマンド内容を出力する */
    if (t_my_info.output_mode == DEF_OUT_OUTFILE_MODE) {
        printf("%s\n",t_command_sep.input_command);
    }
    /* 入力コマンド分割精査処理 */
    CMDI_sep_check_command(t_command_sep.input_command);
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_input_infile_cmd                           */
/*  CALLING SEQ.    : void  CMDI_input_infile_cmd(void)                     */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : インファイルコマンド入力処理                          */
/****************************************************************************/
void CMDI_input_infile_cmd(void)
{
    /* 変数宣言 */
    short s_error;
    short command_sep_num = 0;

    /* 変数初期化 */
    memset(t_infile_info.buffer_command,NULL,sizeof(t_infile_info.buffer_command));
    memset(&t_command_sep,NULL,sizeof(command_sep_def));
    t_infile_info.buffer_command_length=sizeof(t_command_sep.input_command)-1;

    /* 1行ずつ読む */
    s_error = EDITREAD(t_infile_info.edit_controlblk
                    ,t_infile_info.buffer_command
                    ,t_infile_info.buffer_command_length
                    ,&t_infile_info.sequence_num);

    /* EDITREADのリターンコードチェック */
    if(s_error<0){
        /* EOF判定 */
        if(s_error == DEF_EDITREAD_EOF){
            t_my_info.end_flag = DEF_FLG_ON;
            t_my_info.continue_flag = DEF_FLG_OFF;
            return;
        }else{
            CMDI_abend();
        }
    }
    /* 入力コマンドを構造体に格納 */
    strncpy(t_command_sep.input_command,t_infile_info.buffer_command,sizeof(t_command_sep.input_command)-1);
    /* 改行時、読み飛ばす */
    if(strlen(t_command_sep.input_command)==0){
        t_my_info.continue_flag = DEF_FLG_OFF;
        return;
    }

    /* 入力コマンドを出力する */
    printf(">> %s\n",t_command_sep.input_command);

    /* 入力コマンド分割精査処理 */
    CMDI_sep_check_command(t_command_sep.input_command);

}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_sep_check_command                          */
/*  CALLING SEQ.    : void  CMDI_sep_check_command(void)                    */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 入力コマンド分割精査処理                              */
/****************************************************************************/
void CMDI_sep_check_command(char* input_command)
{
    /* 変数宣言 */
    char* wk_separated;

    /*コメントアウト判定*/
    if(memcmp(input_command,"==",2) == 0){
        t_my_info.continue_flag = DEF_FLG_OFF;
        return;
    }
    /* スペース区切りで分割 */
    for(short i = 0;i< 4; i++){
        /* 一回目の分割処理 */
        if(i == 0){
            wk_separated = strtok(input_command," ");
        /* 二回目以降の分割処理 */
        }else{
            wk_separated = strtok(NULL," ");
        }
        /* 結果がNULLでなければ値を格納する */
        if (wk_separated != NULL){
            strncpy(t_command_sep.separate_command[i],wk_separated,sizeof(t_command_sep.separate_command[i])-1);
        }else{
            break;
        }
    }



    /* オプション数精査処理 */
    /* 精査異常時 */
    if (strlen(t_command_sep.separate_command[3]) !=  0){
        /*画面出力メッセージ*/
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        /* INファイルコマンドモードの時、処理続行フラグをOFFにし、終了フラグをONにする */
        if(t_my_info.startup_mode == DEF_INP_INFILE_MODE){
            t_my_info.continue_flag = DEF_FLG_OFF;
            t_my_info.end_flag = DEF_FLG_ON;
            t_my_info.completion_code = DEF_COMPLETION_ERROR;
        /* プロンプトコマンドモードの時、処理続行フラグをOFFにする */
        }else{
            t_my_info.continue_flag = DEF_FLG_OFF;
        }
    /* 精査正常時 */
    }else{
        t_my_info.continue_flag = DEF_FLG_ON;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_select_command                             */
/*  CALLING SEQ.    : void  CMDI_select_command(void)                       */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : コマンド判定処理                                      */
/****************************************************************************/
void CMDI_select_command(void)
{
    /* 変数宣言 */
    short return_code;
    /* 各コマンド処理の呼び出し */
    if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_TCP_CONNECT) == 0){

        return_code = CMDI_open_tcp(t_command_sep.separate_command[1]
                                      ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_TCP_CLOSE) == 0){

        return_code = CMDI_close_tcp(t_command_sep.separate_command[1]
                                    ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_TCP_LISTEN) == 0){

        return_code = CMDI_listen_tcp(t_command_sep.separate_command[1]
                                     ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_INFO_TCP_STATE) == 0){

        return_code = CMDI_status_tcp(t_command_sep.separate_command[1]
                                         ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_SIGN_ON) == 0){

        return_code = CMDI_sign_on(t_command_sep.separate_command[1]
                                  ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_SIGN_OFF) == 0){

        return_code = CMDI_sign_off(t_command_sep.separate_command[1]
                                   ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_INFO_STATE) == 0){

        return_code = CMDI_status_service(t_command_sep.separate_command[1]
                                     ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_ECHO) == 0){

        return_code = CMDI_echo(t_command_sep.separate_command[1]
                               ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_INFO_ECHO) == 0){

        return_code = CMDI_status_echo(t_command_sep.separate_command[1]
                                    ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_KEY_REQUEST) == 0){

        return_code = CMDI_request_key(t_command_sep.separate_command[1]
                                      ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_KEY_PUSH) == 0){

        return_code = CMDI_push_key(t_command_sep.separate_command[1]
                                   ,t_command_sep.separate_command[2]);

    }
    else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_LOG_ROTATE) == 0){

        return_code = CMDI_rotate_log(t_command_sep.separate_command[1]
                                     ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_RELOAD_GFLIN) == 0){

        return_code = CMDI_reload_gflin(t_command_sep.separate_command[1]
                                       ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_RELOAD_GFNSW) == 0){

        return_code = CMDI_reload_gfnsw(t_command_sep.separate_command[1]
                                       ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_EXIT) == 0){

        return_code = CMDI_exit_command(t_command_sep.separate_command[1]
                                       ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_HELP) == 0){

        return_code = CMDI_display_help(t_command_sep.separate_command[1]
                                       ,t_command_sep.separate_command[2]);

    }else if(CMDI_strcmpi(t_command_sep.separate_command[0],DEF_DELAY) == 0){

        return_code = CMDI_delay_command(t_command_sep.separate_command[1]
                                        ,t_command_sep.separate_command[2]);

    }else{
        /*画面出力メッセージ*/
        CMDI_output_msg(DEF_CODE_COMMAND_NOT_FOUND);
        /* INファイルモード時、終了処理に遷移する */
        if(t_my_info.startup_mode == DEF_INP_INFILE_MODE){
            t_my_info.end_flag = DEF_FLG_ON;
            t_my_info.completion_code = DEF_COMPLETION_ERROR;
            return;
        }
    }
    /* INファイルモード時は異常応答が返ってきた時点でプロセスを終了する。 */
    if ((t_my_info.startup_mode == DEF_INP_INFILE_MODE) && (return_code == DEF_RET_NG)){
        t_my_info.end_flag = DEF_FLG_ON;
        t_my_info.completion_code = DEF_COMPLETION_ERROR;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_edit_ipc                                   */
/*  CALLING SEQ.    : void CMDI_edit_ipc(char*,c501_def*)                   */
/*  ARGUMENT        :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : IPC編集処理                                           */
/****************************************************************************/
void CMDI_edit_ipc(char* cmd_name,c501_def* pt_c501)
{

    /* 共通ヘッダ編集 */
    memcpy(pt_c501->common_header.interface_code,DEF_IPC_IFCD_CMD_REQ
           ,sizeof(pt_c501->common_header.interface_code));
    pt_c501->common_header.error_code = DEF_IPC_ERRCD_OK;
    memset(pt_c501->common_header.internal_error_code,'0',sizeof(pt_c501->common_header.internal_error_code));
    pt_c501->common_header.control_data_length = sizeof(pt_c501->command_info)+sizeof(pt_c501->record_count);
    /* コマンド情報編集 */
    memcpy(pt_c501->command_info.command_name,cmd_name
           ,sizeof(pt_c501->command_info.command_name));
    pt_c501->record_count = 0;

}
/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_edit_pathsend                              */
/*  CALLING SEQ.    : void edit_pathsend (COM_PSD_arg_1_def*                */
/*                   ,COM_PSD_arg_2_def*,c501_def*,short,short)             */
/*  ARGUMENT        :                                                       */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : PATHSEND編集処理                                      */
/****************************************************************************/
void CMDI_edit_pathsend(COM_PSD_arg_1_def* pt_psd_msg,COM_PSD_arg_2_def* pt_psd_trc,c501_def* pt_c501
                  ,short buffer_len,short max_len)
{
    memcpy(pt_psd_msg->pathmon_name,t_my_info.cmd_srv_domain_name,sizeof(pt_psd_msg->pathmon_name));
    memcpy(pt_psd_msg->serverclass_name,t_my_info.cmd_srv_serverclass_name,sizeof(pt_psd_msg->serverclass_name));
    memcpy(pt_psd_msg->msg_buf,pt_c501,sizeof(pt_psd_msg->msg_buf));
    pt_psd_msg->req_send_len = buffer_len;
    pt_psd_msg->receive_max_len = buffer_len+max_len;
    pt_psd_msg->send_timer_msec = t_timer_value.pathsend_timer;
    pt_psd_msg->retry_cnt = t_timer_value.pathsend_retry_num;
    memcpy(pt_psd_trc->prog_id,DEF_CMDIF_PROGRAM_ID,sizeof(pt_psd_trc->prog_id));

}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_open_tcp                                   */
/*  CALLING SEQ.    : short CMDI_open_tcp(char* scd_cmd,char* trd_cmd)      */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : オープンコマンド処理                                  */
/****************************************************************************/
short CMDI_open_tcp(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));

    /* オプション数精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_OPN,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));
    /* コマンドサーバへのPATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_close_tcp                                  */
/*  CALLING SEQ.    : short CMDI_close_tcp(char* scd_cmd,char* trd_cmd)     */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : クローズコマンド処理                                  */
/****************************************************************************/
short CMDI_close_tcp(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));

    /* オプション数精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);

    /* 識別子分割処理リターンコードチェック */
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_CLS,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* コマンドサーバへのPATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }

}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_listen_tcp                                 */
/*  CALLING SEQ.    : short CMDI_listen_tcp(char* scd_cmd,char* trd_cmd)    */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : リスナーコマンド処理                                  */
/****************************************************************************/
short CMDI_listen_tcp(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));

    /* オプション精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) == 0)){

        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    /* リターンコードチェック */
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    if (CMDI_strcmpi(trd_cmd,DEF_MODE_START) == 0){
        CMDI_edit_ipc(DEF_IPC_CMD_LSN_START,&t_c501);
    }else if(CMDI_strcmpi(trd_cmd,DEF_MODE_END) == 0){
        CMDI_edit_ipc(DEF_IPC_CMD_LSN_END,&t_c501);
    }else{
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* コネクション論理ID設定 */
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));
    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));

    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_status_tcp                                 */
/*  CALLING SEQ.    : short CMDI_status_tcp(char* scd_cmd,char* trd_cmd)    */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : コネクションステータス照会コマンド処理                */
/****************************************************************************/
short CMDI_status_tcp(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;
    r501_cn_st_def* pt_r501_cn_st;
    char group_name[6];
    char interface_name[6];
    char station_name[7];
    char connection_name[7];
    char connection_logical_name[19];
    char interface_name_tmp[5];

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    memset(group_name,NULL,sizeof(group_name));
    memset(interface_name,NULL,sizeof(interface_name));
    memset(station_name,NULL,sizeof(station_name));
    memset(connection_name,NULL,sizeof(connection_name));
    memset(interface_name_tmp,0x20,sizeof(interface_name_tmp));

    /* オプション精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) !=  0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    /* リターンコードチェック */
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_STS_DSP,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501)+ DEF_CMD_DATA_SIZE );

    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* レコードバッファの読み取り */
    pt_r501_cn_st = (r501_cn_st_def*)t_psd_msg.msg_buf;

    /* リターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    /* 共通ヘッダエラーコードチェック */
    }else if(pt_r501_cn_st->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501_cn_st->common_header.internal_error_code);
        return DEF_RET_NG;
    }

    /* 各レコードを繰り返し読み、標準出力する。 */
    for(short i = 0;i < (pt_r501_cn_st->record_count);i++){

        /* NULL終端にする */
        memcpy(group_name,pt_r501_cn_st->command_info.connection_logical_name.group_name
              ,sizeof(pt_r501_cn_st->command_info.connection_logical_name.group_name));
        memcpy(interface_name,pt_r501_cn_st->gclst_data[i].connection_logical_name.interface_name
              ,sizeof(pt_r501_cn_st->gclst_data[i].connection_logical_name.interface_name));
        memcpy(station_name,pt_r501_cn_st->gclst_data[i].connection_logical_name.station_name
              ,sizeof(pt_r501_cn_st->gclst_data[i].connection_logical_name.station_name));
        memcpy(connection_name,pt_r501_cn_st->gclst_data[i].connection_logical_name.connection_name
              ,sizeof(pt_r501_cn_st->gclst_data[i].connection_logical_name.connection_name));

        /* インタフェースIDが前のレコードと異なる場合、インタフェースヘッダとラベルを出力する。 */
        if(memcmp(pt_r501_cn_st->gclst_data[i].connection_logical_name.interface_name,interface_name_tmp,sizeof(interface_name_tmp))){
            memcpy(interface_name_tmp,pt_r501_cn_st->gclst_data[i].connection_logical_name.interface_name,sizeof(interface_name_tmp));
            
            /* 画面にテーブル形式で出力 */
            printf("\n%s",DEF_RABEL_INTERFACE_ID);
            printf(":%c-%c-%s-%s\n\n",pt_r501_cn_st->command_info.connection_logical_name.site_name
                               ,pt_r501_cn_st->command_info.connection_logical_name.nw_name
                               ,group_name
                               ,interface_name
                    );
            /* ラベルの表示 */
            connect_sts_display connect_sts_rabel;
            memset(&connect_sts_rabel,0x20,sizeof(connect_sts_rabel));
            memcpy(connect_sts_rabel.connection_id,DEF_RABEL_CONNECTION_ID,strlen(DEF_RABEL_CONNECTION_ID));
            memcpy(connect_sts_rabel.connect_sts,DEF_RABEL_CONNECT_STS,strlen(DEF_RABEL_CONNECT_STS));
            memcpy(connect_sts_rabel.disconnect_rsn,DEF_RABEL_DISCONNECT_RSN,strlen(DEF_RABEL_DISCONNECT_RSN));
            memcpy(connect_sts_rabel.ip_address_src,DEF_RABEL_IP_ADDRESS_SRC,strlen(DEF_RABEL_IP_ADDRESS_SRC));
            memcpy(connect_sts_rabel.port_num_src,DEF_RABEL_PORT_NUM_SRC,strlen(DEF_RABEL_PORT_NUM_SRC));
            memcpy(connect_sts_rabel.ip_address_dst,DEF_RABEL_IP_ADDRESS_DST,strlen(DEF_RABEL_IP_ADDRESS_DST));
            memcpy(connect_sts_rabel.port_num_dst,DEF_RABEL_PORT_NUM_DST,strlen(DEF_RABEL_PORT_NUM_DST));
            memcpy(connect_sts_rabel.err_code,DEF_RABEL_ERR_CODE,strlen(DEF_RABEL_ERR_CODE));
            memcpy(connect_sts_rabel.sts_update_time,DEF_RABEL_LAST_MODIFIED,strlen(DEF_RABEL_LAST_MODIFIED));

            connect_sts_rabel.end_null = NULL;
            printf("%s\n",&connect_sts_rabel);
        }
        /* 各レコードの構造体の初期処理 */
        connect_sts_display connect_sts_record[pt_r501_cn_st->record_count];
        memset(connect_sts_record,0x20,sizeof(connect_sts_record));

        /* ハイフンつなぎコネクション論理IDを格納する */
        sprintf(connection_logical_name,"%s-%s "
                ,station_name,connection_name);
        memcpy(connect_sts_record[i].connection_id,connection_logical_name
               ,strlen(connection_logical_name));

        /* 画面表示用構造体に値を設定する */
        memcpy(connect_sts_record[i].connect_sts,pt_r501_cn_st->gclst_data[i].connect_sts_info.connect_sts
               ,sizeof(pt_r501_cn_st->gclst_data[i].connect_sts_info.connect_sts));
        memcpy(connect_sts_record[i].disconnect_rsn,pt_r501_cn_st->gclst_data[i].connect_info.disconnect_rsn
               ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.disconnect_rsn));
        memcpy(connect_sts_record[i].disconnect_rsn,pt_r501_cn_st->gclst_data[i].connect_info.disconnect_rsn
               ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.disconnect_rsn));
        //コネクションステータスがCNの時、IPアドレス、ポート番号を表示する。
        if(memcmp(pt_r501_cn_st->gclst_data[i].connect_sts_info.connect_sts,DEF_CONNECT_STS_CONNECT
                 ,sizeof(pt_r501_cn_st->gclst_data[i].connect_sts_info.connect_sts))==0){
            memcpy(connect_sts_record[i].ip_address_src,pt_r501_cn_st->gclst_data[i].connect_info.ip_address_src
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.ip_address_src));
            connect_sts_record[i].port_num_src[0]=':';
            memcpy(connect_sts_record[i].port_num_src+1,pt_r501_cn_st->gclst_data[i].connect_info.port_num_src
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.port_num_src));
            memcpy(connect_sts_record[i].ip_address_dst,pt_r501_cn_st->gclst_data[i].connect_info.ip_address_dst
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.ip_address_dst));
            connect_sts_record[i].port_num_dst[0]=':';
            memcpy(connect_sts_record[i].port_num_dst+1,pt_r501_cn_st->gclst_data[i].connect_info.port_num_dst
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.port_num_dst));
        //コネクションステータスがLSかつ、リスナーの時、IPアドレス、ポート番号を表示する。
        }else if((memcmp(pt_r501_cn_st->gclst_data[i].connect_sts_info.connect_sts,DEF_CONNECT_STS_LISTEN
                 ,sizeof(pt_r501_cn_st->gclst_data[i].connect_sts_info.connect_sts))==0) 
                 && (memcmp(connection_name,"CL",2)==0)){
            memcpy(connect_sts_record[i].ip_address_src,pt_r501_cn_st->gclst_data[i].connect_info.ip_address_src
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.ip_address_src));
            connect_sts_record[i].port_num_src[0]=':';
            memcpy(connect_sts_record[i].port_num_src+1,pt_r501_cn_st->gclst_data[i].connect_info.port_num_src
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.port_num_src));
            memcpy(connect_sts_record[i].ip_address_dst,pt_r501_cn_st->gclst_data[i].connect_info.ip_address_dst
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.ip_address_dst));
            connect_sts_record[i].port_num_dst[0]=' ';
            memcpy(connect_sts_record[i].port_num_dst+1,pt_r501_cn_st->gclst_data[i].connect_info.port_num_dst
                   ,sizeof(pt_r501_cn_st->gclst_data[i].connect_info.port_num_dst));
        }
        //MMDDを表示項目に設定する。
        memcpy(connect_sts_record[i].sts_update_time+1
               ,pt_r501_cn_st->gclst_data[i].prc_sts_info.prc_sts_update_time+DEF_DATE_MONTH_OFFSET
               ,DEF_SIZE_MMDD);
        connect_sts_record[i].sts_update_time[DEF_DATE_MONTH_OFFSET+1]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(connect_sts_record[i].sts_update_time+1+DEF_SIZE_MMDD+1
               ,pt_r501_cn_st->gclst_data[i].prc_sts_info.prc_sts_update_time+DEF_DATE_MONTH_OFFSET+DEF_SIZE_MMDD
               ,DEF_SIZE_HHMMSS);
        connect_sts_record[i].end_null = NULL;
        /* レコードを出力する */
        printf("%s\n",&connect_sts_record[i]);

    }
    /* 内部エラーコードチェック(最大数超過) */
    if(memcmp(pt_r501_cn_st->common_header.internal_error_code,DEF_NERR_EXCEEDING_UPPER_LIMIT 
      ,sizeof(pt_r501_cn_st->common_header.internal_error_code)) == 0){
        CMDI_output_msg(DEF_CODE_EXCEEDING_UPPER_LIMIT);
    /* 正常時 */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
    }

    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_sign_on                                    */
/*  CALLING SEQ.    : short CMDI_sign_on(char* scd_cmd,char* trd_cmd)       */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 開局コマンド処理                                      */
/****************************************************************************/
short CMDI_sign_on(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;
    char cmd_name[5];

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    memset(cmd_name,NULL,sizeof(cmd_name));

    /* オプション数精査 */
    if(strlen(scd_cmd) == 0){

        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* モード精査 */
    if (CMDI_strcmpi(trd_cmd,DEF_MODE_FORCE) == 0){
        /* コマンド識別設定 */
        memcpy(cmd_name,DEF_IPC_CMD_CNT_OPN_ABS,sizeof(cmd_name));

    }else if(CMDI_strcmpi(trd_cmd,DEF_MODE_UPDATE) == 0){
        /* コマンド識別設定 */
        memcpy(cmd_name,DEF_IPC_CMD_CNT_OPN_UPD,sizeof(cmd_name));

    }else if(strlen(trd_cmd) == 0){
        /* コマンド識別設定 */
        memcpy(cmd_name,DEF_IPC_CMD_CNT_OPN,sizeof(cmd_name));

    }else{
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* インタフェース名指定かステーション名指定か識別子指定かを判別する */
    /* インタフェース名指定の場合 */
    if (memcmp(scd_cmd,"-I",2) == 0){
        /* 先頭二文字を省いたもの */
        char* interface_name = scd_cmd+2;
        if (strlen(interface_name)>sizeof(t_c501.command_info.interface_ext_name)){
            CMDI_output_msg(DEF_CODE_OPTION_ERROR);
            return DEF_RET_NG;
        }
        /* IPC編集 */
        memset(&t_c501,0x20,sizeof(t_c501));
        CMDI_edit_ipc(cmd_name,&t_c501);
        memcpy(t_c501.command_info.interface_ext_name,interface_name
               ,strlen(interface_name));
        
    /* ステーション名指定の場合 */
    }else if(memcmp(scd_cmd,"-S",2) == 0){
        /* 桁数精査 */
        char* station_name = scd_cmd+2;
        if (strlen(station_name)>sizeof(t_c501.command_info.station_ext_name)){
            CMDI_output_msg(DEF_CODE_OPTION_ERROR);
            return DEF_RET_NG;
        }
        /* IPC編集 */
        memset(&t_c501,0x20,sizeof(t_c501));
        CMDI_edit_ipc(cmd_name,&t_c501);
        memcpy(t_c501.command_info.station_ext_name,station_name
               ,strlen(station_name));
    /* 識別子指定の場合 */
    }else{

        /* 識別子分割処理呼び出し */
        wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
        /* リターンコードチェック */
        if(wk_return_code == DEF_RET_NG){
            CMDI_output_msg(DEF_CODE_OPTION_ERROR);
            return DEF_RET_NG;
        }
         /* IPC編集 */
        memset(&t_c501,0x20,sizeof(t_c501));
        CMDI_edit_ipc(cmd_name,&t_c501);
        memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
               ,sizeof(t_c501.command_info.connection_logical_name));
    }

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_sign_off                                   */
/*  CALLING SEQ.    : short CMDI_sign_off(char* scd_cmd,char* trd_cmd)      */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 閉局コマンド処理                                      */
/****************************************************************************/
short CMDI_sign_off(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;
    char cmd_name[5];

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    memset(cmd_name,NULL,sizeof(cmd_name));

    /* オプション数精査 */
    if(strlen(scd_cmd) == 0){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* モード精査 */
    if (CMDI_strcmpi(trd_cmd,DEF_MODE_FORCE) == 0){
        /* コマンド識別設定 */
        memcpy(cmd_name,DEF_IPC_CMD_CNT_CLS_ABS,sizeof(cmd_name));

    }else if(CMDI_strcmpi(trd_cmd,DEF_MODE_UPDATE) == 0){
        /* コマンド識別設定 */
        memcpy(cmd_name,DEF_IPC_CMD_CNT_CLS_UPD,sizeof(cmd_name));

    }else if(strlen(trd_cmd) == 0){
        /* コマンド識別設定 */
        memcpy(cmd_name,DEF_IPC_CMD_CNT_CLS,sizeof(cmd_name));

    }else{
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* インタフェース名指定かステーション名指定か識別子指定かを判別する */
    /* インタフェース名指定の場合 */
    if (memcmp(scd_cmd,"-I",2) == 0){
        /* 桁数精査 */
        char* interface_name = scd_cmd+2;
        if (strlen(interface_name)>sizeof(t_c501.command_info.interface_ext_name)){
            CMDI_output_msg(DEF_CODE_OPTION_ERROR);
            return DEF_RET_NG;
        }
        /* IPC編集 */
        memset(&t_c501,0x20,sizeof(t_c501));
        CMDI_edit_ipc(cmd_name,&t_c501);
        memcpy(t_c501.command_info.interface_ext_name,interface_name
               ,strlen(interface_name));
        
    /* ステーション名指定の場合 */
    }else if(memcmp(scd_cmd,"-S",2) == 0){
        /* 桁数精査 */
        char* station_name = scd_cmd+2;
        if (strlen(station_name)>sizeof(t_c501.command_info.station_ext_name)){
            CMDI_output_msg(DEF_CODE_OPTION_ERROR);
            return DEF_RET_NG;
        }
        /* IPC編集 */
        memset(&t_c501,0x20,sizeof(t_c501));
        CMDI_edit_ipc(cmd_name,&t_c501);
        memcpy(t_c501.command_info.station_ext_name,station_name,strlen(station_name));
    /* 識別子指定の場合 */
    }else{

        /* 識別子分割処理呼び出し */
        wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
        /* リターンコードチェック */
        if(wk_return_code == DEF_RET_NG){
            CMDI_output_msg(DEF_CODE_OPTION_ERROR);
            return DEF_RET_NG;
        }

         /* IPC編集 */
        memset(&t_c501,0x20,sizeof(t_c501));
        CMDI_edit_ipc(cmd_name,&t_c501);
        memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
              ,sizeof(t_c501.command_info.connection_logical_name));
    }
    

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);


    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_status_service                             */
/*  CALLING SEQ.    : short CMDI_status_service(char* scd_cmd,char* trd_cmd)*/
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 局状態照会コマンド処理                                */
/****************************************************************************/
short CMDI_status_service(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;
    r501_sta_st_def* pt_r501_sta_st;
    char group_name[6];
    char interface_name[6];
    char station_name[7];
    char connection_name[7];
    char connection_logical_name[19];

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    memset(group_name,NULL,sizeof(group_name));
    memset(interface_name,NULL,sizeof(interface_name));
    memset(station_name,NULL,sizeof(station_name));
    memset(connection_name,NULL,sizeof(connection_name));
    memset(connection_logical_name,NULL,sizeof(connection_logical_name));

    /* オプション精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    /* リターンコードチェック */
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_CNT_STS_DSP,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501)+DEF_CMD_DATA_SIZE);

    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);
    /* レコードバッファの読み取り */
    pt_r501_sta_st = (r501_sta_st_def*)t_psd_msg.msg_buf;
    /* リターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    /* 共通ヘッダエラーコードチェック */
    }else if(pt_r501_sta_st->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501_sta_st->common_header.internal_error_code);
        return DEF_RET_NG;
    }

    /* 画面にテーブル形式で出力 */
    printf("%s",DEF_RABEL_GROUP_ID);
    memcpy(group_name,pt_r501_sta_st->command_info.connection_logical_name.group_name
           ,sizeof(pt_r501_sta_st->command_info.connection_logical_name.group_name));
    printf(":%c-%c-%s\n\n",pt_r501_sta_st->command_info.connection_logical_name.site_name
                       ,pt_r501_sta_st->command_info.connection_logical_name.nw_name
                       ,group_name
            );
    /* ラベルの表示 */
    state_sts_display state_sts_rabel;
    memset(&state_sts_rabel,0x20,sizeof(state_sts_rabel));
    memcpy(state_sts_rabel.connection_id,DEF_RABEL_MANAGEMENT_ID,strlen(DEF_RABEL_MANAGEMENT_ID));
    memcpy(state_sts_rabel.station_sts,DEF_RABEL_STATION_STS,strlen(DEF_RABEL_STATION_STS));
    memcpy(state_sts_rabel.state_sts_update_time,DEF_RABEL_STATE_UPDATE_DATE,strlen(DEF_RABEL_STATE_UPDATE_DATE));
    memcpy(state_sts_rabel.interface_name,DEF_RABEL_INTERFACE_NAME,strlen(DEF_RABEL_INTERFACE_NAME));
    memcpy(state_sts_rabel.station_name,DEF_RABEL_STATION_NAME,strlen(DEF_RABEL_STATION_NAME));
    state_sts_rabel.end_null = NULL;
    printf("%s\n",&state_sts_rabel);

    /* 画面表示用構造体の初期化 */
    state_sts_display state_sts_record[pt_r501_sta_st->record_count];
    memset(state_sts_record,0x20,sizeof(state_sts_record));

    /* レコード出力 */
    for(short i = 0;i<pt_r501_sta_st->record_count;i++){
        /* NULL終端させる */
        memcpy(interface_name,pt_r501_sta_st->gcsst_data[i].connection_logical_name.interface_name
              ,sizeof(pt_r501_sta_st->gcsst_data[i].connection_logical_name.interface_name));
        memcpy(station_name,pt_r501_sta_st->gcsst_data[i].connection_logical_name.station_name
              ,sizeof(pt_r501_sta_st->gcsst_data[i].connection_logical_name.station_name));
        memcpy(connection_name,pt_r501_sta_st->gcsst_data[i].connection_logical_name.connection_name
              ,sizeof(pt_r501_sta_st->gcsst_data[i].connection_logical_name.connection_name));

        /* ハイフンつなぎコネクション論理IDを格納する */
        /* インタフェース単位レコード */
        if(station_name[0]==0x20){
            sprintf(connection_logical_name,"%s "
                   ,interface_name);
        /* ステーション単位レコード */
        }else if(connection_name[0]==0x20){
            sprintf(connection_logical_name,"%s-%s "
                   ,interface_name,station_name);
        /* コネクション単位レコード */
        }else{
            sprintf(connection_logical_name,"%s-%s-%s "
                   ,interface_name,station_name,connection_name);
        }
        memcpy(state_sts_record[i].connection_id,connection_logical_name,strlen(connection_logical_name));

        /* 画面表示用構造体に値を設定する */
        memcpy(state_sts_record[i].station_sts,pt_r501_sta_st->gcsst_data[i].state_sts_info.state_sts
               ,sizeof(pt_r501_sta_st->gcsst_data[i].state_sts_info.state_sts));
        memcpy(state_sts_record[i].interface_name,pt_r501_sta_st->gcsst_data[i].nw_id_info.nw_if
               ,sizeof(pt_r501_sta_st->gcsst_data[i].nw_id_info.nw_if));
        memcpy(state_sts_record[i].station_name,pt_r501_sta_st->gcsst_data[i].nw_id_info.nw_station
               ,sizeof(pt_r501_sta_st->gcsst_data[i].nw_id_info.nw_station));

        //YYMMDDを表示項目に設定する。
        memcpy(state_sts_record[i].state_sts_update_time
               ,pt_r501_sta_st->gcsst_data[i].state_sts_info.state_sts_update_time+DEF_DATE_YY_OFFSET
               ,DEF_SIZE_YYMMDD);
        state_sts_record[i].state_sts_update_time[DEF_SIZE_YYMMDD]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(state_sts_record[i].state_sts_update_time+DEF_SIZE_YYMMDD+1
               ,pt_r501_sta_st->gcsst_data[i].state_sts_info.state_sts_update_time+DEF_DATE_YY_OFFSET+DEF_SIZE_YYMMDD
               ,DEF_SIZE_HHMMSS);


        state_sts_record[i].end_null = NULL;

        /* 画面表示用構造体に値を設定する */
        printf("%s\n",&state_sts_record[i]);
    }

    /* 内部エラーコードチェック(最大数超過) */
    if(memcmp(pt_r501_sta_st->common_header.internal_error_code,DEF_NERR_EXCEEDING_UPPER_LIMIT 
        ,sizeof(pt_r501_sta_st->common_header.internal_error_code)) == 0){
        CMDI_output_msg(DEF_CODE_EXCEEDING_UPPER_LIMIT);
    /* 正常時 */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
    }
    return DEF_RET_OK;
}


/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_echo                                       */
/*  CALLING SEQ.    : short CMDI_echo(char* scd_cmd,char* trd_cmd)          */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : エコー送信コマンド処理                                */
/****************************************************************************/
short CMDI_echo(char* scd_cmd,char* trd_cmd)
{

    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));

    /* オプション数精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    /* リターンコードチェック */
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 必須入力不可チェック */
    if(memcmp(t_connection_id.interface_name," ",1) == 0)
    {
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_ECH_SND,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);


    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_status_echo                                  */
/*  CALLING SEQ.    : short CMDI_status_echo(char* scd_cmd,char* trd_cmd)     */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : エコーステータス照会コマンド処理                      */
/****************************************************************************/
short CMDI_status_echo(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;
    r501_echo_st_def* pt_r501_echo_st;
    char group_name[6];
    char interface_name[6];
    char station_name[7];
    char connection_name[7];
    char connection_logical_name[19];

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    memset(group_name,NULL,sizeof(group_name));
    memset(interface_name,NULL,sizeof(interface_name));
    memset(station_name,NULL,sizeof(station_name));
    memset(connection_name,NULL,sizeof(connection_name));


    /* オプション精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 必須入力不可チェック */
    if(memcmp(t_connection_id.interface_name," ",1) == 0 )
    {
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_ECH_STS_DSP,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501)+DEF_CMD_DATA_SIZE);

    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);
    /* レコードバッファの読み取り */
    pt_r501_echo_st = (r501_echo_st_def*)t_psd_msg.msg_buf;
    /* リターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    /* 共通ヘッダエラーコードチェック */
    }else if(pt_r501_echo_st->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501_echo_st->common_header.internal_error_code);
        return DEF_RET_NG;
    }
    /* 画面にテーブル形式で出力 */
    printf("%s",DEF_RABEL_GROUP_ID);
    memcpy(group_name,pt_r501_echo_st->command_info.connection_logical_name.group_name
           ,sizeof(pt_r501_echo_st->command_info.connection_logical_name.group_name));
    printf(":%c-%c-%s\n\n",pt_r501_echo_st->command_info.connection_logical_name.site_name
                       ,pt_r501_echo_st->command_info.connection_logical_name.nw_name
                       ,group_name
            );
    /* ラベル出力 */
    echo_sts_display echo_sts_rabel;
    memset(&echo_sts_rabel,0x20,sizeof(echo_sts_rabel));
    memcpy(echo_sts_rabel.connection_id,DEF_RABEL_MANAGEMENT_ID
          ,strlen(DEF_RABEL_MANAGEMENT_ID));
    memcpy(echo_sts_rabel.echo_init,DEF_RABEL_ECHO_INIT
          ,strlen(DEF_RABEL_ECHO_INIT));
    memcpy(echo_sts_rabel.echo_result,DEF_RABEL_ECHO_RESULT
          ,strlen(DEF_RABEL_ECHO_RESULT));
    memcpy(echo_sts_rabel.echo_last_request_time,DEF_RABEL_ECHO_LAST_REQUEST
          ,strlen(DEF_RABEL_ECHO_LAST_REQUEST));
    memcpy(echo_sts_rabel.echo_last_response_time,DEF_RABEL_ECHO_LAST_RESPONSE
          ,strlen(DEF_RABEL_ECHO_LAST_RESPONSE));
    memcpy(echo_sts_rabel.echo_last_success_time,DEF_RABEL_ECHO_LAST_SUCCESS
          ,strlen(DEF_RABEL_ECHO_LAST_SUCCESS));
    echo_sts_rabel.end_null = NULL;
    /* ラベル出力 */
    printf("%s\n",&echo_sts_rabel);

    /* 画面表示用構造体の初期化 */
    echo_sts_display echo_sts_record[pt_r501_echo_st->record_count * 2];
    memset(echo_sts_record,0x20,sizeof(echo_sts_record));

    /* レコード出力 */
    for(short i = 0;i<pt_r501_echo_st->record_count;i++){
        /* NULL終端させる */
        memcpy(interface_name,pt_r501_echo_st->gcest_data[i].connection_logical_name.interface_name
              ,sizeof(pt_r501_echo_st->gcest_data[i].connection_logical_name.interface_name));
        memcpy(station_name,pt_r501_echo_st->gcest_data[i].connection_logical_name.station_name
              ,sizeof(pt_r501_echo_st->gcest_data[i].connection_logical_name.station_name));
        memcpy(connection_name,pt_r501_echo_st->gcest_data[i].connection_logical_name.connection_name
              ,sizeof(pt_r501_echo_st->gcest_data[i].connection_logical_name.connection_name));

        /* ハイフンつなぎコネクション論理IDを格納する */
        /* インタフェース単位レコード */
        if(station_name[0]==0x20){
            sprintf(connection_logical_name,"%s "
                   ,interface_name);
        /* ステーション単位レコード */
        }else if(connection_name[0]==0x20){
            sprintf(connection_logical_name,"%s-%s "
                   ,interface_name,station_name);
        /* コネクション単位レコード */
        }else{
            sprintf(connection_logical_name,"%s-%s-%s "
                   ,interface_name,station_name,connection_name);
        }

        /* レコードを画面出力用構造体に格納する GFP契機レコード */
        memcpy(echo_sts_record[i * 2].connection_id,connection_logical_name
               ,strlen(connection_logical_name));
        memcpy(echo_sts_record[i * 2].echo_init,DEF_ECHO_INIT_GFP
               ,strlen(DEF_ECHO_INIT_GFP));
        memcpy(echo_sts_record[i * 2].echo_result
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_result
               ,sizeof(pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_result));

        //YYMMDDを表示項目に設定する。
        memcpy(echo_sts_record[i * 2].echo_last_request_time
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_start_time+DEF_DATE_YY_OFFSET
               ,DEF_SIZE_YYMMDD);
        echo_sts_record[i * 2].echo_last_request_time[DEF_SIZE_YYMMDD]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(echo_sts_record[i * 2].echo_last_request_time+DEF_SIZE_YYMMDD+1
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_start_time+DEF_DATE_YY_OFFSET+DEF_SIZE_YYMMDD
               ,DEF_SIZE_HHMMSS);

        //YYMMDDを表示項目に設定する。
        memcpy(echo_sts_record[i * 2].echo_last_response_time
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_end_time+DEF_DATE_YY_OFFSET
               ,DEF_SIZE_YYMMDD);
        echo_sts_record[i * 2].echo_last_response_time[DEF_SIZE_YYMMDD]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(echo_sts_record[i * 2].echo_last_response_time+DEF_SIZE_YYMMDD+1
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_end_time+DEF_DATE_YY_OFFSET+DEF_SIZE_YYMMDD
               ,DEF_SIZE_HHMMSS);

        //YYMMDDを表示項目に設定する。
        memcpy(echo_sts_record[i * 2].echo_last_success_time
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_ok_time+DEF_DATE_YY_OFFSET
               ,DEF_SIZE_YYMMDD);
        echo_sts_record[i * 2].echo_last_response_time[DEF_SIZE_YYMMDD]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(echo_sts_record[i * 2].echo_last_success_time+DEF_SIZE_YYMMDD+1
               ,pt_r501_echo_st->gcest_data[i].echo_info.cbs_echo_info.last_echo_ok_time+DEF_DATE_YY_OFFSET+DEF_SIZE_YYMMDD
               ,DEF_SIZE_HHMMSS);

        echo_sts_record[i * 2].end_null = NULL;

        /* レコードを出力する */
        printf("%s\n",&echo_sts_record[i * 2]);

        /* レコードを画面出力用構造体に格納する NW契機レコード */
        memcpy(echo_sts_record[i * 2 + 1].connection_id,connection_logical_name
               ,strlen(connection_logical_name));
        memcpy(echo_sts_record[i * 2 + 1].echo_init,DEF_ECHO_INIT_NW
               ,strlen(DEF_ECHO_INIT_NW));
        memcpy(echo_sts_record[i * 2 + 1].echo_result
               ,pt_r501_echo_st->gcest_data[i].echo_info.dst_echo_info.last_echo_result
               ,sizeof(pt_r501_echo_st->gcest_data[i].echo_info.dst_echo_info.last_echo_result));

        //YYMMDDを表示項目に設定する。
        memcpy(echo_sts_record[i * 2 + 1].echo_last_request_time
               ,pt_r501_echo_st->gcest_data[i].echo_info.dst_echo_info.last_echo_req_recv_time+DEF_DATE_YY_OFFSET
               ,DEF_SIZE_YYMMDD);
        echo_sts_record[i * 2 + 1].echo_last_request_time[DEF_SIZE_YYMMDD]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(echo_sts_record[i * 2 + 1].echo_last_request_time+DEF_SIZE_YYMMDD+1
               ,pt_r501_echo_st->gcest_data[i].echo_info.dst_echo_info.last_echo_req_recv_time+DEF_DATE_YY_OFFSET+DEF_SIZE_YYMMDD
               ,DEF_SIZE_HHMMSS);

        //YYMMDDを表示項目に設定する。
        memcpy(echo_sts_record[i * 2 + 1].echo_last_success_time
               ,pt_r501_echo_st->gcest_data[i].echo_info.dst_echo_info.last_echo_ok_time+DEF_DATE_YY_OFFSET
               ,DEF_SIZE_YYMMDD);
        echo_sts_record[i * 2 + 1].echo_last_request_time[DEF_SIZE_YYMMDD]=0x20;
        //hhmmssを表示項目に設定する。
        memcpy(echo_sts_record[i * 2 + 1].echo_last_success_time+DEF_SIZE_YYMMDD+1
               ,pt_r501_echo_st->gcest_data[i].echo_info.dst_echo_info.last_echo_ok_time+DEF_DATE_YY_OFFSET+DEF_SIZE_YYMMDD
               ,DEF_SIZE_HHMMSS);

        echo_sts_record[i * 2 + 1].end_null = NULL;

        /* レコードを出力する */
        printf("%s\n",&echo_sts_record[i * 2 + 1]);

    }

    /* 内部エラーコードチェック(最大数超過) */
    if(memcmp(pt_r501_echo_st->common_header.internal_error_code,DEF_NERR_EXCEEDING_UPPER_LIMIT 
      ,sizeof(pt_r501_echo_st->common_header.internal_error_code)) == 0){
        CMDI_output_msg(DEF_CODE_EXCEEDING_UPPER_LIMIT);
    /* 正常時 */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
    }

    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_request_key                                */
/*  CALLING SEQ.    : short CMDI_request_key(char* scd_cmd,char* trd_cmd)   */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 鍵交換依頼コマンド処理                                */
/****************************************************************************/
short CMDI_request_key(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    /* オプション数精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 必須入力不可チェック */
    if((memcmp(t_connection_id.interface_name," ",1) == 0 ) ||
      (memcmp(t_connection_id.connection_name," ",1) != 0 ))
    {
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_KEY_EXC_REQ,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_push_key                                   */
/*  CALLING SEQ.    : short CMDI_push_key(char* scd_cmd,char* trd_cmd)      */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 鍵交換実施コマンド処理                                */
/****************************************************************************/
short CMDI_push_key(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    /* オプション精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) != 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 必須入力不可チェック */
    if((memcmp(t_connection_id.interface_name," ",1) == 0 ) ||
      (memcmp(t_connection_id.connection_name," ",1) != 0 ))
    {
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_KEY_EXC,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_rotate_log                                 */
/*  CALLING SEQ.    : short CMDI_rotate_log(char* scd_cmd,char* trd_cmd)    */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : ログ切替コマンド処理                                  */
/****************************************************************************/
short CMDI_rotate_log(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,0x20,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));
    /* オプション数精査 */
    if((strlen(scd_cmd) == 0) || (strlen(trd_cmd) !=  0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 必須入力不可チェック */
    if((memcmp(t_connection_id.interface_name," ",1) !=  0 ) ||
       (memcmp(t_connection_id.station_name," ",1) !=  0 )   ||
       (memcmp(t_connection_id.connection_name," ",1) !=  0 ))
    {
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_LOG_FL_EXC,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
           ,sizeof(t_c501.command_info.connection_logical_name));

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));

    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);
    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_reload_gflin                               */
/*  CALLING SEQ.    : short CMDI_reload_gflin(char* scd_cmd,char* trd_cmd)  */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 接続構成変更処理                                      */
/****************************************************************************/
short CMDI_reload_gflin(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;
    short srv_cls_len;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));

    /* オプション数精査 */
    if((strlen(scd_cmd) == 0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* 識別子分割処理呼び出し */
    wk_return_code = CMDI_sep_identifier(scd_cmd,&t_connection_id);
    if(wk_return_code == DEF_RET_NG){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* 必須入力不可チェック */
    if(((memcmp(t_connection_id.interface_name," ",1) == 0) ||
        (memcmp(t_connection_id.connection_name," ",1) != 0)))
    {
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

     /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_FL_RE_READ_GFLIN,&t_c501);
    memcpy(&t_c501.command_info.connection_logical_name,&t_connection_id
            ,sizeof(t_c501.command_info.connection_logical_name));

    /*サーバクラス論理ID桁数チェック*/
    srv_cls_len = (short)strlen(trd_cmd);
    /*サーバクラス論理IDが8桁の時*/
    if(srv_cls_len == sizeof(t_c501.command_info.srv_cls_id)){
        memcpy(t_c501.command_info.srv_cls_id,trd_cmd,sizeof(t_c501.command_info.srv_cls_id));
    }else if(srv_cls_len == 0){
        //何もしない
    }else{
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_reload_gfnsw                               */
/*  CALLING SEQ.    : short CMDI_reload_gfnsw(char* scd_cmd,char* trd_cmd)  */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 東阪振分比率変更処理                                  */
/****************************************************************************/
short CMDI_reload_gfnsw(char* scd_cmd,char* trd_cmd)
{
    /* 変数宣言 */
    connection_id_def t_connection_id;
    c501_def t_c501;
    r501_def* pt_r501;
    short wk_return_code;
    COM_PSD_arg_1_def t_psd_msg;
    COM_PSD_arg_2_def t_psd_trc;
    COM_PSD_arg_3_def t_psd_out;

    /* ローカル変数の初期化 */
    memset(&t_connection_id,0x20,sizeof(t_connection_id));
    memset(&t_psd_msg,NULL,sizeof(t_psd_msg));
    memset(&t_psd_trc,NULL,sizeof(t_psd_trc));
    memset(&t_psd_out,NULL,sizeof(t_psd_out));

    /* オプション数精査 */
    if((strlen(scd_cmd) !=  0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }

     /* IPC編集 */
    memset(&t_c501,0x20,sizeof(t_c501));
    CMDI_edit_ipc(DEF_IPC_CMD_FL_RE_READ_GFNSW,&t_c501);

    /* PATHSEND編集 */
    CMDI_edit_pathsend(&t_psd_msg,&t_psd_trc,&t_c501,sizeof(t_c501),sizeof(t_c501));
    /* PATHSENDモジュール呼び出し */
    wk_return_code = COM_PSD(&t_psd_msg,&t_psd_trc,&t_psd_out,&t_psd_ems,&t_psd_add);

    /* メッセージバッファ読込 */
    pt_r501 = (r501_def*)t_psd_msg.msg_buf;
    /* PATHSENDモジュールリターンコードチェック */
    if(wk_return_code != 0){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("Guardian-errcode:%d\n",t_psd_out.guardian_errcode);
        return DEF_RET_NG;
    }
    /*共通ヘッダエラーコードチェック */
    if(pt_r501->common_header.error_code == DEF_IPC_ERRCD_NG){
        CMDI_output_msg(DEF_CODE_RESPONSE_ERROR);
        printf("internal-errcode:%s\n",pt_r501->common_header.internal_error_code);
        return DEF_RET_NG;
    /* 正常時メッセージ */
    }else{
        CMDI_output_msg(DEF_CODE_COMMAND_SUCCESS);
        return DEF_RET_OK;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_display_help                               */
/*  CALLING SEQ.    : short CMDI_display_help(char* scd_cmd,char* trd_cmd)  */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : HELPコマンド処理                                      */
/****************************************************************************/
short CMDI_display_help(char* scd_cmd,char* trd_cmd)
{
    /* オプション数精査 */
    if((strlen(scd_cmd) !=  0) || (strlen(trd_cmd) !=  0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }else{
        /* ヘルプコマンド出力 */
        printf(" %s\n",DEF_HELP_MSG_TCP_CONNECT);
        printf(" %s\n",DEF_HELP_MSG_TCP_CLOSE);
        printf(" %s\n",DEF_HELP_MSG_TCP_LISTEN);
        printf(" %s\n",DEF_HELP_MSG_INFO_TCP_STATE);
        printf(" %s\n",DEF_HELP_MSG_SIGN_ON_CN_ID);
        printf(" %s\n",DEF_HELP_MSG_SIGN_ON_IF_NAME);
        printf(" %s\n",DEF_HELP_MSG_SIGN_ON_ST_NAME);
        printf(" %s\n",DEF_HELP_MSG_SIGN_OFF_CN_ID);
        printf(" %s\n",DEF_HELP_MSG_SIGN_OFF_IF_NAME);
        printf(" %s\n",DEF_HELP_MSG_SIGN_OFF_ST_NAME);
        printf(" %s\n",DEF_HELP_MSG_INFO_STATE);
        printf(" %s\n",DEF_HELP_MSG_ECHO);
        printf(" %s\n",DEF_HELP_MSG_INFO_ECHO);
        printf(" %s\n",DEF_HELP_MSG_KEY_REQUEST);
        printf(" %s\n",DEF_HELP_MSG_KEY_PUSH);
        printf(" %s\n",DEF_HELP_MSG_LOG_ROTATE);
        printf(" %s\n",DEF_HELP_MSG_RELOAD_GFLIN);
        printf(" %s\n",DEF_HELP_MSG_RELOAD_GFNSW);
        printf(" %s\n",DEF_HELP_MSG_EXIT);
        printf(" %s\n",DEF_HELP_MSG_DELAY);
        printf(" %s\n",DEF_HELP_MSG_COMMENT);
    }
    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_exit_command                               */
/*  CALLING SEQ.    : short CMDI_exit_command(char* scd_cmd,char* trd_cmd)  */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : EXITコマンド処理                                      */
/****************************************************************************/
short CMDI_exit_command(char* scd_cmd,char* trd_cmd)
{
    /* オプション数精査 */
    if((strlen(scd_cmd) !=  0) || (strlen(trd_cmd) !=  0)){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }else{
        t_my_info.end_flag = DEF_FLG_ON;
        return DEF_RET_OK;

    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_delay_command                              */
/*  CALLING SEQ.    : short CMDI_delay_command(char* scd_cmd,char* trd_cmd) */
/*  ARGUMENT        : char*                                                 */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : DELAYコマンド処理                                     */
/****************************************************************************/
short CMDI_delay_command(char* scd_cmd,char* trd_cmd)
{
    /* オプション数精査 */
    if(strlen(scd_cmd) == 0 || strlen(trd_cmd) !=  0){
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    /* scd_cmdが数値であるかを確認する */
    if(CMDI_isStrDigit(scd_cmd)){
        /* scd_cmd秒ディレイ */
        PROCESS_DELAY_(atoi(scd_cmd)*100000);
    }else{
        CMDI_output_msg(DEF_CODE_OPTION_ERROR);
        return DEF_RET_NG;
    }
    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_sep_identifier                             */
/*  CALLING SEQ.    : short   CMDI_sep_identifier(char* ,connection_id_def*)*/
/*  ARGUMENT        : char*,connection_id_def*                              */
/*  RETURN CODE     : short                                                 */
/*  DESCRIPTION     : 識別子分割処理                                        */
/****************************************************************************/

short CMDI_sep_identifier(char* scd_cmd ,connection_id_def* t_connection_id)
{
    /* 変数宣言 */
    char separate_identifier[6][10];
    char* wk_str;
    short return_code;
    char* sep_result;

    /* 変数初期化 */
    memset(separate_identifier,NULL,sizeof(separate_identifier));
    wk_str=scd_cmd;

    /* 分割処理 */
    for(short i=0;i < 6; i++){
        /* 最初に見つかったハイフンのポインタを取得 */
        sep_result=strchr(wk_str,'-');
        /* ハイフンが見つからないとき、ループを抜ける */
        if (sep_result==NULL){
            /* 値をセットする */
            strncpy(separate_identifier[i],wk_str,sizeof(separate_identifier[i])-1);
            break;
        }
        /* ハイフン終わりの場合、エラー */
        if (strlen(sep_result+1) == 0){
            return DEF_RET_NG;
        }
        /* ハイフンが連続する場合、エラー */
        if (wk_str-sep_result==0){
            return DEF_RET_NG;
        }
        /* 分割する文字列に後半部を設定し、処理を続行する */
        *sep_result=NULL;
        strncpy(separate_identifier[i],wk_str,sizeof(separate_identifier[i])-1);
        wk_str=sep_result+1;
    }
    //分割を6回行った場合、エラー
    if(sep_result != NULL){
        return DEF_RET_NG;
    }

    /* 桁数精査し、構造体に格納する(サイト識別) */
    if(strlen(separate_identifier[0]) == sizeof(t_connection_id->site_name)){
        t_connection_id->site_name=separate_identifier[0][0];
    }else{
        return DEF_RET_NG;
    }
    /* 桁数精査し、構造体に格納する(NW識別) */
    if(strlen(separate_identifier[1]) == sizeof(t_connection_id->nw_name)){
        t_connection_id->nw_name=separate_identifier[1][0];
    }else{
        return DEF_RET_NG;
    }

    /* 桁数精査し、構造体に格納する(グループ識別) */
    if(strlen(separate_identifier[2]) == sizeof(t_connection_id->group_name)){
        memcpy(t_connection_id->group_name,separate_identifier[2]
                ,sizeof(t_connection_id->group_name));
    }else{
        return DEF_RET_NG;
    }
    /* 桁数精査し、構造体に格納する(インタフェース識別) */
    if(strlen(separate_identifier[3]) == sizeof(t_connection_id->interface_name)){

        memcpy(t_connection_id->interface_name,separate_identifier[3]
               ,sizeof(t_connection_id->interface_name));

    }else if(strlen(separate_identifier[3]) == 0){
        return DEF_RET_OK;
    }else{
        return DEF_RET_NG;
    }

    /* 桁数精査し、構造体に格納する(ステーション識別) */
    if(strlen(separate_identifier[4]) == sizeof(t_connection_id->station_name)){
        memcpy(t_connection_id->station_name,separate_identifier[4],sizeof(t_connection_id->station_name));
    }else if(strlen(separate_identifier[4]) == 0){
        return DEF_RET_OK;
    }else{
        return DEF_RET_NG;
    }

    /* 桁数精査し、構造体に格納する(コネクション識別) */
    if(strlen(separate_identifier[5]) == sizeof(t_connection_id->connection_name)){
        memcpy(t_connection_id->connection_name,separate_identifier[5],sizeof(t_connection_id->connection_name));
    }else if(strlen(separate_identifier[5]) == 0){
        return DEF_RET_OK;
    }else{
        return DEF_RET_NG;
    }
    return DEF_RET_OK;
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_output_msg                                 */
/*  CALLING SEQ.    : void  CMDI_output_msg(short)                          */
/*  ARGUMENT        : short                                                 */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 画面出力メッセージ処理                                */
/****************************************************************************/
void CMDI_output_msg(short msg_code)
{
    /* 出力メッセージ判定 */
    switch(msg_code){
        case DEF_CODE_COMMAND_SUCCESS:
            printf("%s\n",DEF_MSG_COMMAND_SUCCESS);
            break;
        case DEF_CODE_RESPONSE_ERROR:
            printf("%s\n",DEF_MSG_RESPONSE_ERROR);
            break;
        case DEF_CODE_TIME_OUT:
            printf("%s\n",DEF_MSG_TIME_OUT);
            break;
        case DEF_CODE_EXCEEDING_UPPER_LIMIT:
            printf("%s\n",DEF_MSG_EXCEEDING_UPPER_LIMIT);
            break;
        case DEF_CODE_COMMAND_NOT_FOUND:
            printf("%s\n",DEF_MSG_COMMAND_NOT_FOUND);
            break;
        case DEF_CODE_OPTION_ERROR:
            printf("%s\n",DEF_MSG_OPTION_ERROR);
            break;
        case DEF_CODE_ABEND:
            printf("%s\n",DEF_MSG_ABEND);
            break;
        case DEF_CODE_NORMAL_END:
            printf("%s\n",DEF_MSG_NORMAL_END);
            break;
        default:
            break;
    }
}

/****************************************************************************/
/*  FUNCTION        : 2.0.0 CMDI_abend                                      */
/*  CALLING SEQ.    : void  CMDI_abend(void)                                */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void CMDI_abend(void)
{
    // 画面出力メッセージ
    CMDI_output_msg(DEF_CODE_ABEND);
    // バッファ出力
    fflush(stdout);
    //コンプリッションコードを異常でセット
    t_my_info.completion_code = DEF_COMPLETION_ERROR;
    // プロセス終了
    PROCESS_STOP_(, , DEF_PROC_ABNORMAL_END,t_my_info.completion_code);
}
/****************************************************************************/
/*  FUNCTION        : 0.0.2  CMDI_strcmpi                                   */
/*  CALLING SEQ.    : void  CMDI_strcmpi(const char *, const char *)        */
/*  ARGUMENT        : 1. p1            (I) 文字列1                          */
/*                  : 2. p2            (I) 文字列2                          */
/*  RETURN CODE     : 一致 0 ; 不一致: other                                */
/*  DESCRIPTION     : 大文字小文字区別しないことを除けばstrcmpと同じ        */
/****************************************************************************/
int CMDI_strcmpi( const char *p1, const char *p2 )
{
    for( ; tolower( *p1 ) == tolower( *p2 ); p1++, p2++ )
    {
        if (*p1 == '\0')
            return 0;
    }
    return ( tolower( *p1 ) - tolower( *p2 ) );
}

/****************************************************************************/
/*  FUNCTION        : 0.1.3  CMDI_isStrDigit                                */
/*  CALLING SEQ.    : void  CMDI_isStrDigit(char *)                         */
/*  ARGUMENT        : 1. p             (I) 文字列                           */
/*  RETURN CODE     : 数字以外 0 ; 数字: 1                                  */
/*  DESCRIPTION     : 文字列pが数字なら非零、そうでなければ0                */
/****************************************************************************/
int CMDI_isStrDigit(char *p)
{
    int c;
    for ( ; *p; p++ )
    {
        c = *p;
        if (!isdigit(c))
            return 0;
    }

    return 1;
}
