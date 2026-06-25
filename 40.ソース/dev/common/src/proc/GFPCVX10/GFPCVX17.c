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
/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */

/* USER HEADER     */
#include "GFPCVX1G.h" nolist
#include "GFPCVX1E.h" nolist

/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/

/****************************************************************************/
/*   内部変数定義                                                           */
/****************************************************************************/

/* 関数のﾌﾟﾛﾄﾀｲﾌﾟ宣言 */
#include "GFPCVX1P.h"
/****************************************************************************/
/*  FUNCTION        : 1.1.0  AbNormal_End                                   */
/*  CALLING SEQ.    : void AbNormal_End ( void )                            */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 異常終了処理                                          */
/****************************************************************************/
void AbNormal_End(void)
{
    if (myinfo.my_name[0] == 0) {
        message_output(DEF_EVT_PROC_ABNORMAL_END,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_NOMAL,"@X",DEF_GFPCVX10,DEF_VAR_STOP);
    } else {
        message_output(DEF_EVT_PROC_ABNORMAL_END,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_NOMAL,"@X",myinfo.my_name,DEF_VAR_STOP);
    }
    PROCESS_STOP_(,,DEF_ABNORMAL_TERMINATION);
} /*end of AbNormal_End*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_LOAD                                      */
/*  CALLING SEQ.    : void CNSV_LOAD ( void )                               */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 読込処理                                              */
/****************************************************************************/
short CNSV_LOAD(void)
{
short s_rc;
short s_tbl;
short s_idx1,s_idx2;
short s_grp_id;
char  cha_group_info[DEF_GROUP_len_station];
sc_info_def     *sc_info_wk;

    s_tbl = (short)(myinfo.cf_idx ^ 1);
    /*ロード処理開始メッセージ*/
    s_rc = CNSV_GFPHI_open();           /*物理名情報ファイルオープン*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        return (false);
    }
    s_rc = CNSV_GFPHI_load();           /*物理名情報ファイル読込*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力*/
        s_rc = CNSV_GFPHI_close();      /*物理名情報ファイルクローズ*/
        return (false);
    }

    s_rc = CNSV_GFLIN_Rebuild(s_tbl,myinfo.cf_idx); /*回線管理ファイル読込再構築*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力*/
        s_rc = CNSV_GFPHI_close();      /*物理名情報ファイルクローズ*/
        return (false);
    }
    s_rc = CNSV_GFNWI_load(s_tbl);      /*N/W情報ファイル*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力*/
        s_rc = CNSV_GFPHI_close();      /*物理名情報ファイルクローズ*/
        return (false);
    }
    s_rc = CNSV_load_server_info(s_tbl); /*サーバー情報取得*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        s_rc = CNSV_GFPHI_close();      /*物理名情報ファイルクローズ*/
        return (false);
    }
    s_rc = CNSV_GFPHI_close();          /*物理名情報ファイルクローズ*/
    if (s_rc != true) {
        /*エラー処理、メッセージ出力後正常停止*/
        return (false);
    }

    if (cf[s_tbl].sc_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","LINE DEF",DEF_VAR_STOP);
        return (false);
    }
    if (cf[s_tbl].if_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI DEF",DEF_VAR_STOP);
        return (false);
    }
    if (cf[s_tbl].lc_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","Listener DEF",DEF_VAR_STOP);
        return (false);
    }
    if (cf[s_tbl].ob_use == 0) {
        /*コンフィグ取得エラー登録件数チェック*/
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_PHSIC_INFO,"","Outbound DEF",DEF_VAR_STOP);
        return (false);
    }

    sc_info_wk = sc_info;       /*テーブル入換、カレント⇔編集用(処理正常の場合そのままカレント)*/
    sc_info = sc_info2;
    sc_info2 = sc_info_wk;

    for ( s_idx1 = 0; s_idx1 < cf[s_tbl].sc_use; s_idx1++ ) {
        memcpy((char *)&sc_info[s_idx1],(char *)&sc_info2[s_idx1],sizeof(sc_info_def));
    }
    for (s_idx1 = 0; s_idx1 < cf[s_tbl].sc_use; s_idx1++ ) {
        /*インターフェースorステーション管理テーブル、ステーション名で紐づけ*/
        for (s_idx2 = 0; s_idx2 < cf[s_tbl].if_use; s_idx2++ ) {
            if (memcmp(cf[s_tbl].sc_conf[s_idx1].station_name,cf[s_tbl].st_conf[s_idx2].station_name,sizeof(cf[s_tbl].sc_conf[s_idx1].station_name)) == 0) {
                sc_info[s_idx1].station_index = s_idx2;
                s_idx2 = cf[s_tbl].if_use;
            }
        }
        /*リスナー管理テーブル、リスナーコネクション識別で紐づけ*/
        for (s_idx2 = 0; s_idx2 < cf[s_tbl].lc_use; s_idx2++ ) {
            if (memcmp(cf[s_tbl].sc_conf[s_idx1].lc_sc_sign,cf[s_tbl].lc_conf[s_idx2].lc_sc_sign,sizeof(cf[s_tbl].sc_conf[s_idx1].lc_sc_sign)) == 0) {
                sc_info[s_idx1].listner_index = s_idx2;
                s_idx2 = cf[s_tbl].if_use;
            }
        }
        /*Outbound電文振分管理テーブル、*/
        /*※コネクションテーブルとの紐づけは不要*/
    }

    for (s_idx1 = 0; s_idx1 < cf[s_tbl].lc_use; s_idx1++ ) {
        /*インターフェースorステーション管理テーブル、ステーション名で紐づけ*/
        for (s_idx2 = 0; s_idx2 < cf[s_tbl].if_use; s_idx2++ ) {
            if (memcmp(cf[s_tbl].lc_conf[s_idx1].interface_name,cf[s_tbl].st_conf[s_idx2].interface_name,sizeof(cf[s_tbl].lc_conf[s_idx1].interface_name)) == 0) {
                lc_info[s_idx1].station_index = s_idx2;
                s_idx2 = cf[s_tbl].if_use;
            }
        }
    }

    /*グルーピング設定*/
    memcpy(cha_group_info,(char *)&cf[s_tbl].sc_conf[0].site_name,DEF_GROUP_len_station);   /*先頭のテーブルの値で初期化*/
    if (cf[s_tbl].connect_num_mng_lyr == 'I') {            /*コネクション数管理単位 "I"：インタフェース単位*/
        for ( s_idx1 = 0, s_grp_id = 0; s_idx1 < cf[s_tbl].sc_use; s_idx1++ ) {
            if (memcmp((char *)&cf[s_tbl].sc_conf[s_idx1].site_name,cha_group_info,DEF_GROUP_len_interface) == 0) {
                sc_info[s_idx1].group_no = s_grp_id;
            } else {
                memcpy(cha_group_info,(char *)&cf[s_tbl].sc_conf[s_idx1].site_name,DEF_GROUP_len_interface);
                s_grp_id++;
                sc_info[s_idx1].group_no = s_grp_id;
            }
        }
    } else if (cf[s_tbl].connect_num_mng_lyr == 'S') {     /*コネクション数管理単位 "S"：ステーション単位*/
        for ( s_idx1 = 0, s_grp_id = 0; s_idx1 < cf[s_tbl].sc_use; s_idx1++ ) {
            if (memcmp((char *)&cf[s_tbl].sc_conf[s_idx1].site_name,cha_group_info,DEF_GROUP_len_station) == 0) {
                sc_info[s_idx1].group_no = s_grp_id;
            } else {
                memcpy(cha_group_info,(char *)&cf[s_tbl].sc_conf[s_idx1].site_name,DEF_GROUP_len_station);
                s_grp_id++;
                sc_info[s_idx1].group_no = s_grp_id;
            }
        }
    } else {                                                /*コネクション数管理単位 その他(スペース)：管理対象外*/
    }

    for ( s_idx1 = 0; s_idx1 < cf[s_tbl].lc_use; s_idx1++ ) {   /*従属コネクション数クリア*/
        lc_info[s_idx1].manage_count = 0;
    }
    for ( s_idx1 = 0; s_idx1 < cf[s_tbl].sc_use; s_idx1++ ) {   /*従属コネクション数取得*/
        lc_info[sc_info[s_idx1].listner_index].manage_count++;
    }

    /*ロード処理終了メッセージ*/
    message_output(DEF_EVT_CONF_RE_READ,DEF_MSGTTKB_NORMAL,DEF_NERR_NOMAL,"@X","",DEF_VAR_STOP);

    myinfo.cf_idx = (short)(myinfo.cf_idx ^ 1);  /*コンフィグテーブルのインデックスを入れ替え*/

    return (true);
} /*end of CNSV_LOAD*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFPHI_open                                */
/*  CALLING SEQ.    : void CNSV_GFPHI_open ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 物理名情報ファイルオープン処理                        */
/****************************************************************************/
short CNSV_GFPHI_open(void)
{
fileio_def       gfphi_io;

    /* IOモジュール情報初期化 */
    memset((char *)&gfphi_io, ' ', sizeof(gfphi_io));
    gfphi_io.arg4.file_no = DEF_FILE_CLOSED;
    gfphi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_io.arg5.part_key_position = 0;
    gfphi_io.arg5.part_key_len = 0;
    gfphi_io.arg5.key_len = 0;
    gfphi_io.arg5.compare_len = 0;
    gfphi_io.arg5.positioning_mode = 0;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = 0;
    gfphi_io.arg5.rec_len = 0;
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 物理名情報ファイルオープン                   */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_OPEN, sizeof(DEF_COM_IOM_FUNC_OPEN)-1);
    memcpy(gfphi_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gfphi_io.arg3.file_id, DEF_FL_PHSIC_INFO, sizeof(DEF_FL_PHSIC_INFO)-1);
    memcpy(gfphi_io.arg3.file_name, myinfo.GFPHI_name, myinfo.GFPHI_name_len);
    memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_OPEN, sizeof(DEF_TRACE_OPEN)-1);
    memcpy(gfphi_io.arg4.file_id, DEF_GFPHI, sizeof(DEF_GFPHI)-1);
    memcpy(gfphi_io.arg4.file_name, myinfo.GFPHI_name, myinfo.GFPHI_name_len);

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_OPN_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"OPEN","",gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }
    myinfo.GFPHI_fd = gfphi_io.arg4.file_no;
    return (true);
} /*end of CNSV_GFPHI_open*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFPHI_close                               */
/*  CALLING SEQ.    : void CNSV_GFPHI_close ( void )                        */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 物理名情報ファイルクローズ処理                        */
/****************************************************************************/
short CNSV_GFPHI_close(void)
{
fileio_def       gfphi_io;

    /* IOモジュール情報初期化 */
    memset((char *)&gfphi_io, ' ', sizeof(gfphi_io));
    gfphi_io.arg4.file_no = myinfo.GFPHI_fd;
    gfphi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_io.arg5.part_key_position = 0;
    gfphi_io.arg5.part_key_len = 0;
    gfphi_io.arg5.key_len = 0;
    gfphi_io.arg5.compare_len = 0;
    gfphi_io.arg5.positioning_mode = 0;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = 0;
    gfphi_io.arg5.rec_len = 0;
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 物理名情報ファイルクローズ                   */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_CLOSE, sizeof(DEF_COM_IOM_FUNC_CLOSE)-1);
    memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_CLOSE, sizeof(DEF_TRACE_CLOSE)-1);
    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    gfphi_io.arg6.guardian_errcode = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","COM_IOM",gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }
    myinfo.GFPHI_fd = DEF_FILE_CLOSED;
    return (true);
} /*end of CNSV_GFPHI_close*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFPHI_load                                */
/*  CALLING SEQ.    : void CNSV_GFPHI_load ( void )                         */
/*  ARGUMENT        : void                                                  */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 物理名情報ファイル読込処理                            */
/****************************************************************************/
short CNSV_GFPHI_load(void)
{
fileio_def       gfphi_io;
db_gfphi_def     *gfphi_rec;
short           s_len;
char            ach_report_key[40+1];

    gfphi_rec = (db_gfphi_def *)&gfphi_io.arg6.rec_area[0];

    /* IOモジュール情報初期化 */
    memset((char *)&gfphi_io, ' ', sizeof(gfphi_io));
    gfphi_io.arg4.file_no = myinfo.GFPHI_fd;
    gfphi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_io.arg5.part_key_position = 0;
    gfphi_io.arg5.part_key_len = 0;
    gfphi_io.arg5.key_len = 0;
    gfphi_io.arg5.compare_len = 0;
    gfphi_io.arg5.positioning_mode = 0;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = 0;
    gfphi_io.arg5.rec_len = 0;
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 物理名情報ファイルREAD                       */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gfphi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gfphi_io.arg5.key_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.compare_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Exact;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = myinfo.file_io_timer;
    gfphi_io.arg5.rec_len = db_gfphi_def_Size;

    /*自プロセス情報取得*/
    gfphi_rec->pri_key.site_id = myinfo.site;
    gfphi_rec->pri_key.nw_id = myinfo.network;
    memcpy(gfphi_rec->pri_key.grp_id, myinfo.group, sizeof(myinfo.group));
/*    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind, DEF_SC_CON_SVR, sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind)); */
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,myinfo.server_class_name, sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,myinfo.server_class_num,sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind,DEF_SC_NAME_DEFAULT,sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, DEF_SC_NUM_DEFAULT, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, DEF_SC_DUP_DEFAULT, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num));

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X",DEF_FL_PHSIC_INFO,DEF_SC_CON_SVR,"my Process info",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","COM_IOM",gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }
    memcpy(myinfo.pathmon_name,gfphi_rec->srv_cls_info.pathmon_name,sizeof(myinfo.pathmon_name));
    CNSV_set_null(myinfo.pathmon_name,sizeof(myinfo.pathmon_name));
    memcpy(myinfo.my_server_class,gfphi_rec->srv_cls_info.srv_cls_name,sizeof(myinfo.my_server_class));
    CNSV_set_null(myinfo.my_server_class,sizeof(myinfo.my_server_class));

    /*N/W情報ファイル名取得*/
    gfphi_rec->pri_key.site_id = myinfo.site;
    gfphi_rec->pri_key.nw_id = myinfo.network;
    memcpy(gfphi_rec->pri_key.grp_id, myinfo.group, sizeof(myinfo.group));
    memset((char *)&gfphi_rec->pri_key.srv_cls_key, '}', sizeof(gfphi_rec->pri_key.srv_cls_key));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_NW_INFO, strlen(DEF_FL_NW_INFO));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X",DEF_FL_PHSIC_INFO,DEF_FL_NW_INFO,"GFNWI-record",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        memset(ach_report_key,'\0',sizeof(ach_report_key));
        memcpy(ach_report_key,(char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"READ",ach_report_key,gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    FILENAME_SCAN_(gfphi_rec->prc_file_info.prc_file_name, sizeof(gfphi_rec->prc_file_info.prc_file_name),&s_len);
    myinfo.GFNWI_name_len = s_len;
    memcpy(myinfo.GFNWI_name, gfphi_rec->prc_file_info.prc_file_name, myinfo.GFNWI_name_len);

    /*回線管理ファイル名取得*/
    gfphi_rec->pri_key.site_id = myinfo.site;
    gfphi_rec->pri_key.nw_id = myinfo.network;
    memcpy(gfphi_rec->pri_key.grp_id, myinfo.group, sizeof(myinfo.group));
    memset((char *)&gfphi_rec->pri_key.srv_cls_key, '}', sizeof(gfphi_rec->pri_key.srv_cls_key));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_LIN_MG, strlen(DEF_FL_LIN_MG));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X",DEF_FL_PHSIC_INFO,DEF_FL_LIN_MG,"GFLIN-record",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        memset(ach_report_key,'\0',sizeof(ach_report_key));
        memcpy(ach_report_key,(char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"READ",ach_report_key,gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    FILENAME_SCAN_(gfphi_rec->prc_file_info.prc_file_name,sizeof(gfphi_rec->prc_file_info.prc_file_name),&s_len);
    myinfo.GFLIN_name_len = s_len;
    memcpy(myinfo.GFLIN_name, gfphi_rec->prc_file_info.prc_file_name, myinfo.GFLIN_name_len);

    /*回線ステータスファイル名取得*/
    gfphi_rec->pri_key.site_id = myinfo.site;
    gfphi_rec->pri_key.nw_id = myinfo.network;
    memcpy(gfphi_rec->pri_key.grp_id, myinfo.group, sizeof(myinfo.group));
    memset((char *)&gfphi_rec->pri_key.srv_cls_key, '}', sizeof(gfphi_rec->pri_key.srv_cls_key));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind, DEF_FL_LIN_STS, strlen(DEF_FL_LIN_STS));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, "0000", 4);
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", 4);

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X",DEF_FL_PHSIC_INFO,DEF_FL_LIN_STS,"GFLIN-record",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        memset(ach_report_key,'\0',sizeof(ach_report_key));
        memcpy(ach_report_key,(char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"READ",ach_report_key,gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    FILENAME_SCAN_(gfphi_rec->prc_file_info.prc_file_name, sizeof(gfphi_rec->prc_file_info.prc_file_name),&s_len);
    myinfo.GCLST_name_len = s_len;
    memcpy(myinfo.GCLST_name, gfphi_rec->prc_file_info.prc_file_name, myinfo.GCLST_name_len);

    /*コマンドサーバー情報取得*/
    gfphi_rec->pri_key.site_id = myinfo.site;
    gfphi_rec->pri_key.nw_id = myinfo.network;
    memcpy(gfphi_rec->pri_key.grp_id, myinfo.group, sizeof(myinfo.group));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,DEF_SC_CMD_SRV, sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,"0000",sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind,DEF_SC_NAME_DEFAULT,sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, DEF_SC_NUM_DEFAULT, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, DEF_SC_DUP_DEFAULT, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num));

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X",DEF_FL_PHSIC_INFO,DEF_SC_CMD_SRV,"SCNCMDSV-record",DEF_VAR_STOP);
        AbNormal_End();
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","COM_IOM",gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }
    if (gfphi_rec->srv_cls_info.domain_name[0] != ' ') {    /*ドメイン指定有り*/
        memset(ci_info->ps_manage.pathmon_name,' ',sizeof(ci_info->ps_manage.pathmon_name));
        memcpy(ci_info->ps_manage.pathmon_name,gfphi_rec->srv_cls_info.domain_name,sizeof(gfphi_rec->srv_cls_info.domain_name));
    } else {                                                /*PATHMON指定*/
        memcpy(ci_info->ps_manage.pathmon_name,gfphi_rec->srv_cls_info.pathmon_name,sizeof(ci_info->ps_manage.pathmon_name));
    }
    memcpy(ci_info->ps_manage.server_class,gfphi_rec->srv_cls_info.srv_cls_name,sizeof(ci_info->ps_manage.server_class));
    CNSV_set_null(ci_info->ps_manage.pathmon_name,sizeof(ci_info->ps_manage.pathmon_name));
    ci_info->ps_manage.pathmon_name_len = (short)strlen(ci_info->ps_manage.pathmon_name);
    CNSV_set_null(ci_info->ps_manage.server_class,sizeof(ci_info->ps_manage.server_class));
    ci_info->ps_manage.server_class_len = (short)strlen(ci_info->ps_manage.server_class);
    return (true);
} /*end of CNSV_GFPHI_load*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFLIN_load                                */
/*  CALLING SEQ.    : void CNSV_GFLIN_load ( short )                        */
/*  ARGUMENT        : テーブル番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線管理ファイル読込処理                              */
/****************************************************************************/
short CNSV_GFLIN_load(short tbl_no)
{
fileio_def      gflin_io;
db_gflin_def    *gflin_rec;
short           s_idx;
short           s_loop;
char            ach_report_key[40+1];
struct
{
    char        site_id;        /*比較対象*/
    char        nw_id;          /*比較対象*/
    char        grp_id[5];      /*比較対象*/
    char        if_id[5];       /*リスナー情報取得時比較対象*/
    char        station_id[6];  /*リスナー情報取得時比較対象*/
    char        connect_id[6];  /*リスナー情報取得時比較対象*/
} compare_key;
struct
{
    char        srv_cls_kind[8];
    char        srv_cls_num[4];
} alt_key;

    gflin_rec = (db_gflin_def *)&gflin_io.arg6.rec_area[0];
    /* IOモジュール情報初期化 */
    memset((char *)&gflin_io, ' ', sizeof(gflin_io));
    gflin_io.arg4.file_no = DEF_FILE_CLOSED;
    gflin_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gflin_io.arg5.part_key_position = 0;
    gflin_io.arg5.part_key_len = 0;
//    gflin_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
//    gflin_io.arg5.compare_len = (1+1+5);
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_A1,sizeof(DEF_COM_IOM_KEYTYPE_A1)-1);
    gflin_io.arg5.key_len = (8+4);
    gflin_io.arg5.compare_len = (8+4);
    gflin_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Generic;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.file_io_timer;
    gflin_io.arg5.rec_len = db_gflin_def_Size;
    gflin_io.arg6.guardian_errcode = 0;
    gflin_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 回線管理ファイルオープン                     */
    /*----------------------------------------------*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_OPEN, sizeof(DEF_COM_IOM_FUNC_OPEN)-1);
    memcpy(gflin_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gflin_io.arg3.file_id, DEF_FL_LIN_MG, sizeof(DEF_FL_LIN_MG)-1);
    memcpy(gflin_io.arg3.file_name, myinfo.GFLIN_name, myinfo.GFLIN_name_len);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_OPEN, sizeof(DEF_TRACE_OPEN)-1);
    memcpy(gflin_io.arg4.file_id, DEF_GFLIN, sizeof(DEF_GFLIN)-1);
    memcpy(gflin_io.arg4.file_name, myinfo.GFLIN_name, myinfo.GFLIN_name_len);

    COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
    if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"OPEN","",gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    memset((short *)&compare_key,0,sizeof(compare_key));
    compare_key.site_id = myinfo.site;
    compare_key.nw_id = myinfo.network;
    memcpy(compare_key.grp_id,myinfo.group,sizeof(compare_key.grp_id));

    /*担当回線管理レコード取り込み*/
    /*回線管理ファイル読込(キー指定)*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
//    memcpy(gflin_io.arg5.key_value, (char *)&compare_key, sizeof(gflin_rec->pri_key));
    memcpy(alt_key.srv_cls_kind,myinfo.server_class_name,sizeof(alt_key.srv_cls_kind));
    memcpy(alt_key.srv_cls_num,myinfo.server_class_num,sizeof(alt_key.srv_cls_num));
    memcpy(gflin_io.arg5.key_value, (char *)&alt_key, sizeof(alt_key));

    /*■読み込み処理 (1) コネクション管理テーブルを設定する*/
    /*回線管理ファイル読込(ALTキーに自プロセスサーバークラスを設定しREAD)*/
    for (s_loop = 0, s_idx = 0; s_loop == 0; ) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;

        COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gflin_io.sub_prog_sts)) == 0) {
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
            memset(ach_report_key,'\0',sizeof(ach_report_key));
            memcpy(ach_report_key,(char *)&alt_key, sizeof(alt_key));
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"READ",ach_report_key,gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
            AbNormal_End();
        }
        /*対象テーブル(ALTキー値とサーバークラス論理名、論理IDが同じ)のみ取り込む*/
//        if (memcmp(myinfo.server_class_name,gflin_rec->alt1_key_info.srv_cls_id.srv_cls_kind,sizeof(myinfo.server_class_name)) == 0 &&
//            memcmp(myinfo.server_class_num,gflin_rec->alt1_key_info.srv_cls_id.srv_cls_num,sizeof(myinfo.server_class_num)) == 0) {
        if (memcmp((char *)&compare_key,(char *)&gflin_rec->pri_key,(1+1+5)) == 0) {
            cf[tbl_no].sc_conf[s_idx].site_name = gflin_rec->pri_key.site_id;
            cf[tbl_no].sc_conf[s_idx].nw_name = gflin_rec->pri_key.nw_id;
            memcpy(cf[tbl_no].sc_conf[s_idx].group_name,gflin_rec->pri_key.grp_id,sizeof(cf[tbl_no].sc_conf[s_idx].group_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].interface_name,gflin_rec->pri_key.if_id,sizeof(cf[tbl_no].sc_conf[s_idx].interface_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].station_name,gflin_rec->pri_key.station_id,sizeof(cf[tbl_no].sc_conf[s_idx].station_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].src_connection_name,gflin_rec->pri_key.connect_id,sizeof(cf[tbl_no].sc_conf[s_idx].src_connection_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].lc_sc_sign,gflin_rec->alt2_key_info.lst_connect_id,sizeof(cf[tbl_no].sc_conf[s_idx].lc_sc_sign));
            memcpy(cf[tbl_no].sc_conf[s_idx].tcpip_name,gflin_rec->tcpip_prc_name,sizeof(cf[tbl_no].sc_conf[s_idx].tcpip_name)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].tcpip_name,sizeof(cf[tbl_no].sc_conf[s_idx].tcpip_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].local_ipaddr,gflin_rec->ip_adress_src,sizeof(cf[tbl_no].sc_conf[s_idx].local_ipaddr)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].local_ipaddr,sizeof(cf[tbl_no].sc_conf[s_idx].local_ipaddr));
            memcpy(cf[tbl_no].sc_conf[s_idx].local_port_no,gflin_rec->port_num_src,sizeof(cf[tbl_no].sc_conf[s_idx].local_port_no)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].local_port_no,sizeof(cf[tbl_no].sc_conf[s_idx].local_port_no));
            memcpy(cf[tbl_no].sc_conf[s_idx].remote_ipaddr,gflin_rec->ip_adress_dst,sizeof(cf[tbl_no].sc_conf[s_idx].remote_ipaddr)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].remote_ipaddr,sizeof(cf[tbl_no].sc_conf[s_idx].remote_ipaddr));
            memcpy(cf[tbl_no].sc_conf[s_idx].remote_port_no,gflin_rec->port_num_dst,sizeof(cf[tbl_no].sc_conf[s_idx].remote_port_no)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].remote_port_no,sizeof(cf[tbl_no].sc_conf[s_idx].remote_port_no));
            cf[tbl_no].sc_conf[s_idx].use_on_off = gflin_rec->invalid_flg;
            cf[tbl_no].sc_use++;
            s_idx++;
            if (s_idx > DEF_MAX_CONNECTION) {
                message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","GFLIN record over",DEF_VAR_STOP);
                return (false);
            }
        }
        /*次レコードREAD用*/
        memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, sizeof(DEF_COM_IOM_FUNC_NEXTREAD)-1);
        memset(gflin_io.arg3.file_io_type,0x20,sizeof(gflin_io.arg3.file_io_type));
        memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_READ, sizeof(DEF_TRACE_READ)-1);
    }

    /*リスナー情報取得、サイト、ネットワーク、グループでキー指定、読んだ内容のALTキー1がリスナー「SCNLISTN」*/
    gflin_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
    gflin_io.arg5.compare_len = (1+1+5);
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI,sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gflin_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Generic;
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memset((short *)&compare_key,0,sizeof(compare_key));
    compare_key.site_id = myinfo.site;
    compare_key.nw_id = myinfo.network;
    memcpy(compare_key.grp_id,myinfo.group,sizeof(compare_key.grp_id));
    memcpy(gflin_io.arg5.key_value, (char *)&compare_key, sizeof(gflin_rec->pri_key));

    /*■読み込み処理 (2)、リスナー管理情報を設定する*/
    /*回線管理ファイル読込(プロセスサーバークラスを設定しREAD)*/
    cf[tbl_no].lc_use = 0;
    for (s_loop = 0, s_idx = 0; s_loop == 0; ) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;

        COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gflin_io.sub_prog_sts)) == 0) {
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
            memset(ach_report_key,'\0',sizeof(ach_report_key));
            memcpy(ach_report_key,(char *)&alt_key, sizeof(alt_key));
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"READ",ach_report_key,gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
            AbNormal_End();
        }
        /*リスナー判定、読んだ内容のALTキー1がリスナー「SCNLISTN」であるか確認*/
        if (memcmp((char *)&gflin_rec->alt1_key_info.srv_cls_id,DEF_SC_LISTEN,sizeof(DEF_SC_LISTEN)-1) == 0) {
            /*読み込んだ情報をリスナー定義情報に設定*/
            cf[tbl_no].lc_conf[s_idx].site_name = gflin_rec->pri_key.site_id;
            cf[tbl_no].lc_conf[s_idx].nw_name = gflin_rec->pri_key.nw_id;
            memcpy(cf[tbl_no].lc_conf[s_idx].group_name,gflin_rec->pri_key.grp_id,sizeof(cf[tbl_no].lc_conf[s_idx].group_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].lc_sc_sign,gflin_rec->pri_key.connect_id,sizeof(cf[tbl_no].lc_conf[s_idx].lc_sc_sign));
            memcpy(cf[tbl_no].lc_conf[s_idx].interface_name,(char *)&gflin_rec->pri_key.if_id,sizeof(cf[tbl_no].lc_conf[s_idx].interface_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].serverclass_logical_name,(char *)&gflin_rec->alt1_key_info.srv_cls_id.srv_cls_kind,sizeof(cf[tbl_no].lc_conf[s_idx].serverclass_logical_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].serverclass_logical_num,(char *)&gflin_rec->alt1_key_info.srv_cls_id.srv_cls_num,sizeof(cf[tbl_no].lc_conf[s_idx].serverclass_logical_num));
            memcpy(cf[tbl_no].lc_conf[s_idx].tcpip_name,(char *)&gflin_rec->tcpip_prc_name,sizeof(gflin_rec->tcpip_prc_name));
            CNSV_set_null(cf[tbl_no].lc_conf[s_idx].tcpip_name,sizeof(cf[tbl_no].lc_conf[s_idx].tcpip_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].local_ipaddr,(char *)&gflin_rec->ip_adress_src,sizeof(gflin_rec->ip_adress_src));
            CNSV_set_null(cf[tbl_no].lc_conf[s_idx].local_ipaddr,sizeof(cf[tbl_no].lc_conf[s_idx].local_ipaddr));
            memcpy(cf[tbl_no].lc_conf[s_idx].local_port_no,(char *)&gflin_rec->port_num_src,sizeof(gflin_rec->port_num_src));
            CNSV_set_null(cf[tbl_no].lc_conf[s_idx].local_port_no,sizeof(cf[tbl_no].lc_conf[s_idx].local_port_no));
            cf[tbl_no].lc_conf[s_idx].validity_flag = gflin_rec->invalid_flg;
            /*読み込んだ情報をリスナー管理情報に設定*/
            lc_info[s_idx].site_name = gflin_rec->pri_key.site_id;
            lc_info[s_idx].nw_name = gflin_rec->pri_key.nw_id;
            memcpy(lc_info[s_idx].group_name,gflin_rec->pri_key.grp_id,sizeof(cf[tbl_no].lc_conf[s_idx].group_name));
            memcpy(lc_info[s_idx].connect_id,gflin_rec->pri_key.connect_id,sizeof(lc_info[s_idx].connect_id));

            s_idx++;
            cf[tbl_no].lc_use++;
            if (cf[tbl_no].lc_use > DEF_MAX_LISTNER) {
                message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","GFLIN record over",DEF_VAR_STOP);
                return (false);
            }
        }
        /*次レコードREAD用*/
        memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, sizeof(DEF_COM_IOM_FUNC_NEXTREAD)-1);
        memset(gflin_io.arg3.file_io_type,0x20,sizeof(gflin_io.arg3.file_io_type));
        memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_READ, sizeof(DEF_TRACE_READ)-1);
    }

    /*----------------------------------------------*/
    /* 回線管理ファイルクローズ                     */
    /*----------------------------------------------*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_CLOSE, sizeof(DEF_COM_IOM_FUNC_CLOSE)-1);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_CLOSE, sizeof(DEF_TRACE_CLOSE)-1);
    memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
    gflin_io.arg6.guardian_errcode = 0;

    COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
    if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"CLOSE","",gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    return (true);
} /*end of CNSV_GFLIN_load*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFLIN_key_read                            */
/*  CALLING SEQ.    : shortCNSV_GFLIN_key_read ( char *, short, short,short)*/
/*  ARGUMENT        : 比較キー                                              */
/*                  : 比較キー長                                            */
/*                  : 編集先cfテーブルインデックス                          */
/*                  : 格納開始位置                                          */
/*                  : 読込件数 (out)                                        */
/*  RETURN CODE     : 処理結果 true / false                                 */
/*  DESCRIPTION     : 回線管理読み込み処理                                  */
/****************************************************************************/
short CNSV_GFLIN_key_read(char *key, short compare_len,short tbl_no,short start_idx,short *readcnt)
{
short s_idx;
short s_count;

    /*担当回線管理レコード取り込み*/
    /*回線管理ファイル読込(キー指定)*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    gflin_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
    gflin_io.arg5.compare_len = compare_len;
    memset(gflin_io.arg5.key_value,' ',sizeof(gflin_io.arg5.key_value));
    memcpy(gflin_io.arg5.key_value, (char *)key,compare_len);

    /*回線管理ファイル読込(ALTキーに自プロセスサーバークラスを設定しREAD)*/
    s_count = 0;
    s_idx = start_idx;
    for ( ; ; ) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;

        COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gflin_io.sub_prog_sts)) == 0) {
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"READ",key,gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
            return (false);
        }
        /*対象テーブル(ALTキー値とサーバークラス論理名、論理IDが同じ)のみ取り込む*/
        if (memcmp(myinfo.server_class_name,gflin_rec->alt1_key_info.srv_cls_id.srv_cls_kind,sizeof(myinfo.server_class_name)) == 0 &&
            memcmp(myinfo.server_class_num,gflin_rec->alt1_key_info.srv_cls_id.srv_cls_num,sizeof(myinfo.server_class_num)) == 0) {
            cf[tbl_no].sc_conf[s_idx].site_name = gflin_rec->pri_key.site_id;
            cf[tbl_no].sc_conf[s_idx].nw_name = gflin_rec->pri_key.nw_id;
            memcpy(cf[tbl_no].sc_conf[s_idx].group_name,gflin_rec->pri_key.grp_id,sizeof(cf[tbl_no].sc_conf[s_idx].group_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].interface_name,gflin_rec->pri_key.if_id,sizeof(cf[tbl_no].sc_conf[s_idx].interface_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].station_name,gflin_rec->pri_key.station_id,sizeof(cf[tbl_no].sc_conf[s_idx].station_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].src_connection_name,gflin_rec->pri_key.connect_id,sizeof(cf[tbl_no].sc_conf[s_idx].src_connection_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].lc_sc_sign,gflin_rec->alt2_key_info.lst_connect_id,sizeof(cf[tbl_no].sc_conf[s_idx].lc_sc_sign));
            memcpy(cf[tbl_no].sc_conf[s_idx].tcpip_name,gflin_rec->tcpip_prc_name,sizeof(cf[tbl_no].sc_conf[s_idx].tcpip_name)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].tcpip_name,sizeof(cf[tbl_no].sc_conf[s_idx].tcpip_name));
            memcpy(cf[tbl_no].sc_conf[s_idx].local_ipaddr,gflin_rec->ip_adress_src,sizeof(cf[tbl_no].sc_conf[s_idx].local_ipaddr)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].local_ipaddr,sizeof(cf[tbl_no].sc_conf[s_idx].local_ipaddr));
            memcpy(cf[tbl_no].sc_conf[s_idx].local_port_no,gflin_rec->port_num_src,sizeof(cf[tbl_no].sc_conf[s_idx].local_port_no)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].local_port_no,sizeof(cf[tbl_no].sc_conf[s_idx].local_port_no));
            memcpy(cf[tbl_no].sc_conf[s_idx].remote_ipaddr,gflin_rec->ip_adress_dst,sizeof(cf[tbl_no].sc_conf[s_idx].remote_ipaddr)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].remote_ipaddr,sizeof(cf[tbl_no].sc_conf[s_idx].remote_ipaddr));
            memcpy(cf[tbl_no].sc_conf[s_idx].remote_port_no,gflin_rec->port_num_dst,sizeof(cf[tbl_no].sc_conf[s_idx].remote_port_no)-1);
            CNSV_set_null(cf[tbl_no].sc_conf[s_idx].remote_port_no,sizeof(cf[tbl_no].sc_conf[s_idx].remote_port_no));
            cf[tbl_no].sc_conf[s_idx].use_on_off = gflin_rec->invalid_flg;
            s_count++;
            s_idx++;
            if (s_idx > DEF_MAX_CONNECTION) {
                message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","GFLIN record over",DEF_VAR_STOP);
                return (false);
            }
        }
        /*次レコードREAD用*/
        memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, sizeof(DEF_COM_IOM_FUNC_NEXTREAD)-1);
        memset(gflin_io.arg3.file_io_type,0x20,sizeof(gflin_io.arg3.file_io_type));
        memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_READ, sizeof(DEF_TRACE_READ)-1);
    }

    *readcnt = s_count; /*読込処理件数*/
    return (true);

} /*end of CNSV_GFLIN_key_read*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFLIN_Rebuild                             */
/*  CALLING SEQ.    : void CNSV_GFLIN_Rebuild ( short , short )             */
/*  ARGUMENT        : テーブル番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線管理ファイル読込再定義処理                        */
/****************************************************************************/
short CNSV_GFLIN_Rebuild(short tbl_no,short Reference)
{
short           s_idx, s_idx2;
short           s_loop;
short           s_result, s_skip, s_compare_len, s_read_count;
char            ach_report_key[40+1];
struct
{
    char        site_id;        /*比較対象*/
    char        nw_id;          /*比較対象*/
    char        grp_id[5];      /*比較対象*/
    char        if_id[5];       /*リスナー情報取得時比較対象*/
    char        station_id[6];  /*リスナー情報取得時比較対象*/
    char        connect_id[6];  /*リスナー情報取得時比較対象*/
} compare_key;
struct
{
    char        srv_cls_kind[8];
    char        srv_cls_num[4];
} alt_key;

    gflin_rec = (db_gflin_def *)&gflin_io.arg6.rec_area[0];
    /* IOモジュール情報初期化 */
    memset((char *)&gflin_io, ' ', sizeof(gflin_io));
    gflin_io.arg4.file_no = DEF_FILE_CLOSED;
    gflin_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gflin_io.arg5.part_key_position = 0;
    gflin_io.arg5.part_key_len = 0;
    gflin_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
    gflin_io.arg5.compare_len = DEF_GFLIN_PKEY_LEN;
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI,sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gflin_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Generic;
    gflin_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gflin_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gflin_io.arg5.io_timer = myinfo.file_io_timer;
    gflin_io.arg5.rec_len = db_gflin_def_Size;
    gflin_io.arg6.guardian_errcode = 0;
    gflin_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 回線管理ファイルオープン                     */
    /*----------------------------------------------*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_OPEN, sizeof(DEF_COM_IOM_FUNC_OPEN)-1);
    memcpy(gflin_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gflin_io.arg3.file_id, DEF_FL_LIN_MG, sizeof(DEF_FL_LIN_MG)-1);
    memcpy(gflin_io.arg3.file_name, myinfo.GFLIN_name, myinfo.GFLIN_name_len);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_OPEN, sizeof(DEF_TRACE_OPEN)-1);
    memcpy(gflin_io.arg4.file_id, DEF_GFLIN, sizeof(DEF_GFLIN)-1);
    memcpy(gflin_io.arg4.file_name, myinfo.GFLIN_name, myinfo.GFLIN_name_len);

    COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
    if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"OPEN","",gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
        return (false);
    }

    memset((short *)&compare_key,0,sizeof(compare_key));
    compare_key.site_id = myinfo.site;
    compare_key.nw_id = myinfo.network;
    memcpy(compare_key.grp_id,myinfo.group,sizeof(compare_key.grp_id));

    /*コネクション定義情報の更新、コマンドで指定されたコネクションのみファイルから取り込む残りはテーブルから設定*/
    if (cf[myinfo.cf_idx].connect_num_mng_lyr == 'S') { /*比較長設定*/
        s_compare_len = DEF_GROUP_len_station;
    } else {
        s_compare_len = DEF_GROUP_len_interface;
    }
    cf[tbl_no].sc_use = 0;
    for (s_idx = 0, s_idx2 = 0, s_skip = false, s_result = true; s_idx < cf[Reference].sc_use ; s_idx++ ) {
        if (memcmp((char *)&cf[Reference].sc_conf[s_idx].site_name,(char *)&c502->command_info.connection_logical_name,s_compare_len) == 0){     /*コマンドで指定された対象*/
            if ( s_skip == 0 ) {    /*未処理の場合*/
                s_result =  CNSV_GFLIN_key_read((char *)&c502->command_info.connection_logical_name,s_compare_len,tbl_no,s_idx,&s_read_count);
                if (s_result == false) break;
                cf[tbl_no].sc_use += s_read_count;
                s_idx2 += s_read_count; /*読み込み件数更新*/
                s_skip = true;      /*処理済としスキップ条件設定*/
            }
        } else {    /*その他、そのまま展開*/
            memcpy((char *)&cf[tbl_no].sc_conf[s_idx2],(char *)&cf[Reference].sc_conf[s_idx],DEF_len_sc_conf);
            cf[tbl_no].sc_use++;
            s_idx2++;
        }
        if (s_idx2 > DEF_MAX_CONNECTION) {
            message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","GFLIN record over",DEF_VAR_STOP);
            s_result = false;
            break;
        }
    }

    if (s_result == false) {
        /*----------------------------------------------*/
        /* 回線管理ファイルクローズ                     */
        /*----------------------------------------------*/
        memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_CLOSE, sizeof(DEF_COM_IOM_FUNC_CLOSE)-1);
        memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_CLOSE, sizeof(DEF_TRACE_CLOSE)-1);
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"CLOSE","",gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
            AbNormal_End();
        }
        return (false);
    }

    /*リスナー情報取得、サイト、ネットワーク、グループでキー指定、読んだ内容のALTキー1がリスナー「SCNLISTN」*/
    gflin_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
    gflin_io.arg5.compare_len = (1+1+5);
    memcpy(gflin_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI,sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gflin_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Generic;
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memset((short *)&compare_key,0,sizeof(compare_key));
    compare_key.site_id = myinfo.site;
    compare_key.nw_id = myinfo.network;
    memcpy(compare_key.grp_id,myinfo.group,sizeof(compare_key.grp_id));
    memcpy(gflin_io.arg5.key_value, (char *)&compare_key, sizeof(gflin_rec->pri_key));

    /*■読み込み処理 (2)、リスナー管理情報を設定する*/
    /*回線管理ファイル読込(プロセスサーバークラスを設定しREAD)*/
    cf[tbl_no].lc_use = 0;
    for (s_loop = 0, s_idx = 0, s_result = true; s_loop == 0; ) {
        memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
        gflin_io.arg6.guardian_errcode = 0;
        gflin_io.arg6.rec_len = 0;

        COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gflin_io.sub_prog_sts)) == 0) {
            break;
        }
        if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
            memset(ach_report_key,'\0',sizeof(ach_report_key));
            memcpy(ach_report_key,(char *)&alt_key, sizeof(alt_key));
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"READ",ach_report_key,gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
            s_result = false;
            break;
        }
        /*リスナー判定、読んだ内容のALTキー1がリスナー「SCNLISTN」であるか確認*/
        if (memcmp((char *)&gflin_rec->alt1_key_info.srv_cls_id,DEF_SC_LISTEN,sizeof(DEF_SC_LISTEN)-1) == 0) {
            /*読み込んだ情報をリスナー定義情報に設定*/
            cf[tbl_no].lc_conf[s_idx].site_name = gflin_rec->pri_key.site_id;
            cf[tbl_no].lc_conf[s_idx].nw_name = gflin_rec->pri_key.nw_id;
            memcpy(cf[tbl_no].lc_conf[s_idx].group_name,gflin_rec->pri_key.grp_id,sizeof(cf[tbl_no].lc_conf[s_idx].group_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].lc_sc_sign,gflin_rec->pri_key.connect_id,sizeof(cf[tbl_no].lc_conf[s_idx].lc_sc_sign));
            memcpy(cf[tbl_no].lc_conf[s_idx].interface_name,(char *)&gflin_rec->pri_key.if_id,sizeof(cf[tbl_no].lc_conf[s_idx].interface_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].serverclass_logical_name,(char *)&gflin_rec->alt1_key_info.srv_cls_id.srv_cls_kind,sizeof(cf[tbl_no].lc_conf[s_idx].serverclass_logical_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].serverclass_logical_num,(char *)&gflin_rec->alt1_key_info.srv_cls_id.srv_cls_num,sizeof(cf[tbl_no].lc_conf[s_idx].serverclass_logical_num));
            memcpy(cf[tbl_no].lc_conf[s_idx].tcpip_name,(char *)&gflin_rec->tcpip_prc_name,sizeof(gflin_rec->tcpip_prc_name));
            CNSV_set_null(cf[tbl_no].lc_conf[s_idx].tcpip_name,sizeof(cf[tbl_no].lc_conf[s_idx].tcpip_name));
            memcpy(cf[tbl_no].lc_conf[s_idx].local_ipaddr,(char *)&gflin_rec->ip_adress_src,sizeof(gflin_rec->ip_adress_src));
            CNSV_set_null(cf[tbl_no].lc_conf[s_idx].local_ipaddr,sizeof(cf[tbl_no].lc_conf[s_idx].local_ipaddr));
            memcpy(cf[tbl_no].lc_conf[s_idx].local_port_no,(char *)&gflin_rec->port_num_src,sizeof(gflin_rec->port_num_src));
            CNSV_set_null(cf[tbl_no].lc_conf[s_idx].local_port_no,sizeof(cf[tbl_no].lc_conf[s_idx].local_port_no));
            cf[tbl_no].lc_conf[s_idx].validity_flag = gflin_rec->invalid_flg;
            /*読み込んだ情報をリスナー管理情報に設定*/
            lc_info[s_idx].site_name = gflin_rec->pri_key.site_id;
            lc_info[s_idx].nw_name = gflin_rec->pri_key.nw_id;
            memcpy(lc_info[s_idx].group_name,gflin_rec->pri_key.grp_id,sizeof(cf[tbl_no].lc_conf[s_idx].group_name));
            memcpy(lc_info[s_idx].connect_id,gflin_rec->pri_key.connect_id,sizeof(lc_info[s_idx].connect_id));
            s_idx++;
            cf[tbl_no].lc_use++;
        }
        if (cf[tbl_no].lc_use > DEF_MAX_LISTNER) {
            message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_LIN_MG,"","GFLIN record over",DEF_VAR_STOP);
            s_result = false;
            break;
        }
        /*次レコードREAD用*/
        memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, sizeof(DEF_COM_IOM_FUNC_NEXTREAD)-1);
        memset(gflin_io.arg3.file_io_type,0x20,sizeof(gflin_io.arg3.file_io_type));
        memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_READ, sizeof(DEF_TRACE_READ)-1);
    }

    /*----------------------------------------------*/
    /* 回線管理ファイルクローズ                     */
    /*----------------------------------------------*/
    memcpy(gflin_io.func_type, DEF_COM_IOM_FUNC_CLOSE, sizeof(DEF_COM_IOM_FUNC_CLOSE)-1);
    memcpy(gflin_io.arg3.file_io_type, DEF_TRACE_CLOSE, sizeof(DEF_TRACE_CLOSE)-1);
    memset(gflin_io.sub_prog_sts, ' ', sizeof(gflin_io.sub_prog_sts));
    gflin_io.arg6.guardian_errcode = 0;

    COM_IOM(gflin_io.func_type,gflin_io.sub_prog_sts,&gflin_io.arg3,&gflin_io.arg4,&gflin_io.arg5,&gflin_io.arg6);
    if (memcmp(gflin_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gflin_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_MG,"CLOSE","",gflin_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }
    return (s_result);
} /*end of CNSV_GFLIN_Rebuild*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GFNWI_load                                */
/*  CALLING SEQ.    : void CNSV_GFNWI_load ( short )                        */
/*  ARGUMENT        : テーブル番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : NW情報ファイル読込処理                                */
/****************************************************************************/
short CNSV_GFNWI_load(short tbl_no)
{
fileio_def      gfnwi_io;
db_gfnwi_def    *gfnwi_rec;
char            ch_buf[64];
long            l_val;
short           s_err;
short           s_idx1,s_idx2,s_col;
short           s_loop;
short           s_len;
struct
{
    char        site_id;        /*比較対象①②*/
    char        nw_id;          /*比較対象①②*/
    char        grp_id[5];      /*比較対象①*/
    char        if_id[5];
    char        station_id[6];
    char        connect_id[6];
} compare_key;
char            ach_report_key[40+1];
    gfnwi_rec = (db_gfnwi_def *)&gfnwi_io.arg6.rec_area[0];
    /* IOモジュール情報初期化 */
    memset((char *)&gfnwi_io, ' ', sizeof(gfnwi_io));
    gfnwi_io.arg4.file_no = DEF_FILE_CLOSED;
    gfnwi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfnwi_io.arg5.part_key_position = 0;
    gfnwi_io.arg5.part_key_len = 0;
    gfnwi_io.arg5.key_len = (1+1+5+5+6);
    gfnwi_io.arg5.compare_len = (1+1+5);        /*グループ識別までを指定*/
    gfnwi_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Generic;
    gfnwi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfnwi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfnwi_io.arg5.io_timer = myinfo.file_io_timer;
    gfnwi_io.arg5.rec_len = db_gfnwi_def_Size;
    gfnwi_io.arg6.guardian_errcode = 0;
    gfnwi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* NW情報ファイルオープン                       */
    /*----------------------------------------------*/
    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_OPEN, sizeof(DEF_COM_IOM_FUNC_OPEN)-1);
    memcpy(gfnwi_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gfnwi_io.arg3.file_id, DEF_FL_NW_INFO, sizeof(DEF_FL_NW_INFO)-1);
    memcpy(gfnwi_io.arg3.file_name, myinfo.GFNWI_name, myinfo.GFNWI_name_len);
    memcpy(gfnwi_io.arg3.file_io_type, DEF_TRACE_OPEN, sizeof(DEF_TRACE_OPEN)-1);
    memcpy(gfnwi_io.arg4.file_id, DEF_GFNWI, sizeof(DEF_GFNWI)-1);
    memcpy(gfnwi_io.arg4.file_name, myinfo.GFNWI_name, myinfo.GFNWI_name_len);

    COM_IOM(gfnwi_io.func_type,gfnwi_io.sub_prog_sts,&gfnwi_io.arg3,&gfnwi_io.arg4,&gfnwi_io.arg5,&gfnwi_io.arg6);
    if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_OPN_ERR,"@X@X@X@X@X@5","","",DEF_FL_NW_INFO,"OPEN","",gfnwi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    /*NW情報ファイルキー指定(第①条件)*/
    gfnwi_io.arg5.compare_len = (1+1+5);  /*サイト識別、NW識別、グループ識別*/
    memset((short *)&compare_key,0,sizeof(compare_key));
    compare_key.site_id = myinfo.site;
    compare_key.nw_id = myinfo.network;
    memcpy(compare_key.grp_id,myinfo.group,sizeof(compare_key.grp_id));
//    memcpy(compare_key.if_id,"}}}}}",5);
//    memcpy(compare_key.station_id,"}}}}}}",6);
//    memcpy(compare_key.connect_id,"}}}}}}",6);

    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gfnwi_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gfnwi_io.arg5.key_value, (char *)&compare_key, sizeof(gfnwi_rec->pri_key));

    /*NW情報ファイル読み込み*/
    for (s_loop = 0, s_idx1 = 0; s_loop == 0; ) {
        memset(gfnwi_io.sub_prog_sts, ' ', sizeof(gfnwi_io.sub_prog_sts));
        gfnwi_io.arg6.guardian_errcode = 0;
        gfnwi_io.arg6.rec_len = 0;

        COM_IOM(gfnwi_io.func_type,gfnwi_io.sub_prog_sts,&gfnwi_io.arg3,&gfnwi_io.arg4,&gfnwi_io.arg5,&gfnwi_io.arg6);
        if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfnwi_io.sub_prog_sts)) == 0) {
            break;
        }
        if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
            memset(ach_report_key,'\0',sizeof(ach_report_key));
            memcpy(ach_report_key,(char *)&compare_key, sizeof(gfnwi_rec->pri_key));
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_NW_INFO,"READ",ach_report_key,gfnwi_io.arg6.guardian_errcode,DEF_VAR_STOP);
            AbNormal_End();
        }
        /*グループ単位レコード*/
        /*インターフェース識別、ステーション識別共に指定なし*/
        if (memcmp(gfnwi_rec->pri_key.if_id,"}}}}}",sizeof(gfnwi_rec->pri_key.if_id)) == 0 &&
            memcmp(gfnwi_rec->pri_key.station_id,"}}}}}}",sizeof(gfnwi_rec->pri_key.station_id)) == 0) {
            memcpy(cf[tbl_no].nw_kubun,gfnwi_rec->nw_id_info.nw_kubun,sizeof(cf[tbl_no].nw_kubun)); /*NW区分*/
            cf[tbl_no].connect_num_mng_lyr = gfnwi_rec->mng_lyr_info.connect_num_mng_lyr;           /*コネクション数管理単位*/
        } else if (memcmp(gfnwi_rec->pri_key.station_id,"}}}}}}",sizeof(gfnwi_rec->pri_key.station_id)) == 0) {
            /*インターフェース単位レコード*/
            /*ステーション識別指定なし*/
            /*インターフェース名からエントリーを検索*/
            for (s_idx2 = 0; s_idx2 < cf[tbl_no].if_use; s_idx2++) {
                if (memcmp(cf[tbl_no].st_conf[s_idx2].nw_if,gfnwi_rec->nw_id_info.nw_if,sizeof(cf[tbl_no].st_conf[s_idx2].nw_if)) == 0) {

                    /*インタフェース識別、インタフェース名。インタフェース識別、インタフェース名、電文項目位置情報を取得*/
                    cf[tbl_no].st_conf[s_idx2].site_name = gfnwi_rec->pri_key.site_id;
                    cf[tbl_no].st_conf[s_idx2].nw_name = gfnwi_rec->pri_key.nw_id;
                    memcpy(cf[tbl_no].st_conf[s_idx2].group_name,gfnwi_rec->pri_key.grp_id,sizeof(cf[tbl_no].st_conf[s_idx1].group_name));
                    memcpy(cf[tbl_no].st_conf[s_idx2].interface_name,gfnwi_rec->pri_key.if_id,sizeof(cf[tbl_no].st_conf[s_idx1].interface_name));
                    memcpy(cf[tbl_no].st_conf[s_idx2].nw_if,gfnwi_rec->nw_id_info.nw_if,sizeof(cf[tbl_no].st_conf[s_idx1].nw_if));

                    /*電文項目位置情報*/
                    memcpy(ch_buf,gfnwi_rec->denbun_item_lct_info.data_len_start_lct,sizeof(gfnwi_rec->denbun_item_lct_info.data_len_start_lct));
                    ch_buf[sizeof(gfnwi_rec->denbun_item_lct_info.data_len_start_lct)] = 0;
                    cf[tbl_no].st_conf[s_idx2].length_pos = (short)atoi(ch_buf);

                    memcpy(ch_buf,gfnwi_rec->denbun_item_lct_info.data_len_size,sizeof(gfnwi_rec->denbun_item_lct_info.data_len_size));
                    ch_buf[sizeof(gfnwi_rec->denbun_item_lct_info.data_len_size)] = 0;
                    cf[tbl_no].st_conf[s_idx2].length_size = (short)atoi(ch_buf);

                    if (memcmp(gfnwi_rec->denbun_item_lct_info.data_len_attribute,DEF_DATA_LEN_ATTR_BIN3,sizeof(gfnwi_rec->denbun_item_lct_info.data_len_attribute)) == 0) {
                        cf[tbl_no].st_conf[s_idx2].length_attr = DEF_textlen_type_BIN;
                    } else if (memcmp(gfnwi_rec->denbun_item_lct_info.data_len_attribute,DEF_DATA_LEN_ATTR_BCD3,sizeof(gfnwi_rec->denbun_item_lct_info.data_len_attribute)) == 0) {
                        cf[tbl_no].st_conf[s_idx2].length_attr = DEF_textlen_type_BCD;
                    } else if (memcmp(gfnwi_rec->denbun_item_lct_info.data_len_attribute,DEF_DATA_LEN_ATTR_ASC3,sizeof(gfnwi_rec->denbun_item_lct_info.data_len_attribute)) == 0) {
                        cf[tbl_no].st_conf[s_idx2].length_attr = DEF_textlen_type_ASCII;
                    } else if (memcmp(gfnwi_rec->denbun_item_lct_info.data_len_attribute,DEF_DATA_LEN_ATTR_EBC3,sizeof(gfnwi_rec->denbun_item_lct_info.data_len_attribute)) == 0) {
                        cf[tbl_no].st_conf[s_idx2].length_attr = DEF_textlen_type_EBCDIC;
                    } else {    /*■設定エラー*/
                        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI len-attr",DEF_VAR_STOP);
                        return (false);
                    }

                    if (gfnwi_rec->denbun_item_lct_info.data_len_include_id == DEF_DATA_LEN_INCLUDE_OUT) {
                        cf[tbl_no].st_conf[s_idx2].length_id = DEF_datalen_outside;
                    } else if (gfnwi_rec->denbun_item_lct_info.data_len_include_id == DEF_DATA_LEN_INCLUDE_IN) {
                        cf[tbl_no].st_conf[s_idx2].length_id = DEF_datalen_inside;
                    } else {    /*■設定エラー*/
                        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI data-len-include",DEF_VAR_STOP);
                        return (false);
                    }

                    memcpy(ch_buf,gfnwi_rec->denbun_item_lct_info.denbun_start_lct,sizeof(gfnwi_rec->denbun_item_lct_info.denbun_start_lct));
                    ch_buf[sizeof(gfnwi_rec->denbun_item_lct_info.denbun_start_lct)] = 0;
                    cf[tbl_no].st_conf[s_idx2].text_pos = (short)atoi(ch_buf);

                    /*コネクション後処理情報*/
                    memcpy(cf[tbl_no].st_conf[s_idx2].connect_after,gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind,sizeof(cf[tbl_no].st_conf[s_idx1].connect_after));
                    if (memcmp(gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind,DEF_CONNECT_NEXT_OPN_SEND,sizeof(gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind)) == 0) {
                        /*開局送信*/
                        cf[tbl_no].st_conf[s_idx2].connect_data_len = 0;
                    } else if (memcmp(gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind,DEF_CONNECT_NEXT_DATA_SEND,sizeof(gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind)) == 0) {
                        /*特定データ送信*/
                        memset(cf[tbl_no].st_conf[s_idx2].connect_data,'\0',sizeof(cf[tbl_no].st_conf[s_idx2].connect_data));
                        for (s_col = 0,s_len = 0;s_col < sizeof(gfnwi_rec->connect_nxt_prc_info.spc_data);s_col++) {
                            if ((gfnwi_rec->connect_nxt_prc_info.spc_data[s_col] == 0x20) || (gfnwi_rec->connect_nxt_prc_info.spc_data[s_col] == 0)) {
                                s_len = s_col;
                                s_col = (short)sizeof(gfnwi_rec->connect_nxt_prc_info.spc_data);
                            }
                        }
                        if ((s_len == 0) || (s_len & 1)) {  /*設定無しまたは奇数*/  /*■設定エラー*/
                            message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI connect-data",DEF_VAR_STOP);
                            return (false);
                        }
                        cf[tbl_no].st_conf[s_idx2].connect_data_len = s_len / 2;
                        s_err = CHAR2HEX(gfnwi_rec->connect_nxt_prc_info.spc_data,cf[tbl_no].st_conf[s_idx2].connect_data,s_len);
                        if (s_err == false) {   /*■設定エラー*/
                            message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI hex-data",DEF_VAR_STOP);
                            return (false);
                        }
                    } else if (memcmp(gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind,DEF_CONNECT_NEXT_NO,sizeof(gfnwi_rec->connect_nxt_prc_info.connect_nxt_prc_kind)) == 0) {
                        /*初期値*/
                    } else {    /*■設定エラー*/
                        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI connect_next_kind",DEF_VAR_STOP);
                        return (false);
                    }

                    /*伝送制御タイマー情報*/
                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_tmr_info.send_wait_tmr,sizeof(gfnwi_rec->trans_cntrl_tmr_info.send_wait_tmr));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_tmr_info.send_wait_tmr)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].send_wait_timer = l_val;

                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_tmr_info.nxt_data_recv_wait,sizeof(gfnwi_rec->trans_cntrl_tmr_info.nxt_data_recv_wait));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_tmr_info.nxt_data_recv_wait)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].recv_wait_timer = l_val;

                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_tmr_info.non_comm_monitor,sizeof(gfnwi_rec->trans_cntrl_tmr_info.non_comm_monitor));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_tmr_info.non_comm_monitor)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].idle_timer = l_val;

                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_srt,sizeof(gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_srt));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_srt)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].short_retry_timer = l_val;

                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lng,sizeof(gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lng));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_tmr_info.line_fail_rtr_num_lng)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].long_retry_timer = l_val;

                    /*伝送制御カウンター情報*/
                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_srt,sizeof(gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_srt));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_srt)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].short_retry_count = l_val;

                    memcpy(ch_buf,gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lng,sizeof(gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lng));
                    ch_buf[sizeof(gfnwi_rec->trans_cntrl_cnt_info.line_fail_rtr_num_lng)] = 0;
                    l_val = atol(ch_buf);
                    cf[tbl_no].st_conf[s_idx2].long_retry_count = l_val;

                }
            }

        } else {    /*その他*/
            /*ステーション単位レコード*/
            /*インタフェース識別、インタフェース名。インタフェース識別、インタフェース名を取得*/
            cf[tbl_no].st_conf[s_idx1].site_name = gfnwi_rec->pri_key.site_id;
            cf[tbl_no].st_conf[s_idx1].nw_name = gfnwi_rec->pri_key.nw_id;
            memcpy(cf[tbl_no].st_conf[s_idx1].group_name,gfnwi_rec->pri_key.grp_id,sizeof(cf[tbl_no].st_conf[s_idx1].group_name));
            memcpy(cf[tbl_no].st_conf[s_idx1].interface_name,gfnwi_rec->pri_key.if_id,sizeof(cf[tbl_no].st_conf[s_idx1].interface_name));

            memcpy(cf[tbl_no].st_conf[s_idx1].nw_if,gfnwi_rec->nw_id_info.nw_if,sizeof(cf[tbl_no].st_conf[s_idx1].nw_if));
            memcpy(cf[tbl_no].st_conf[s_idx1].nw_station,gfnwi_rec->nw_id_info.nw_station,sizeof(cf[tbl_no].st_conf[s_idx1].nw_station));

            memcpy(cf[tbl_no].st_conf[s_idx1].station_name,gfnwi_rec->pri_key.station_id,sizeof(cf[tbl_no].st_conf[s_idx1].station_name));
            cf[tbl_no].if_use++;
            s_idx1++;
            if (s_idx1 >= DEF_MAX_interface) {  /*コンフィグエラー、テーブルフル*/
                message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_SYSTEM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFNWI station record over",DEF_VAR_STOP);
                return (false);
            }
        }
        /*次レコードREAD設定*/
        memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, sizeof(DEF_COM_IOM_FUNC_NEXTREAD)-1);
        memset(gfnwi_io.arg3.file_io_type,0x20,sizeof(gfnwi_io.arg3.file_io_type));
        memcpy(gfnwi_io.arg3.file_io_type, DEF_TRACE_READ, sizeof(DEF_TRACE_READ)-1);
    }

    /*----------------------------------------------*/
    /* NW情報ファイルクローズ                       */
    /*----------------------------------------------*/
    memcpy(gfnwi_io.func_type, DEF_COM_IOM_FUNC_CLOSE, sizeof(DEF_COM_IOM_FUNC_CLOSE)-1);
    memcpy(gfnwi_io.arg3.file_io_type, DEF_TRACE_CLOSE, sizeof(DEF_TRACE_CLOSE)-1);
    memset(gfnwi_io.sub_prog_sts, ' ', sizeof(gfnwi_io.sub_prog_sts));
    gfnwi_io.arg6.guardian_errcode = 0;

    COM_IOM(gfnwi_io.func_type,gfnwi_io.sub_prog_sts,&gfnwi_io.arg3,&gfnwi_io.arg4,&gfnwi_io.arg5,&gfnwi_io.arg6);
    if (memcmp(gfnwi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfnwi_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_NW_INFO,"CLOSE","",gfnwi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }

    return (true);
} /*end of CNSV_GFNWI_load*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  GCLST_open                                     */
/*  CALLING SEQ.    : void CNSV_GCLST_open ( void )                         */
/*  ARGUMENT        : スレッド番号                                          */
/*                    変更する値                                            */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイルオープン処理                    */
/****************************************************************************/
short CNSV_GCLST_open(void)
{
fileio_def       gclst_io;

    /* IOモジュール情報初期化 */
    memset((char *)&gclst_io, ' ', sizeof(gclst_io));
    gclst_io.arg4.file_no = DEF_FILE_CLOSED;
    gclst_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gclst_io.arg5.part_key_position = 0;
    gclst_io.arg5.part_key_len = 0;
    gclst_io.arg5.key_len = 0;
    gclst_io.arg5.compare_len = 0;
    gclst_io.arg5.positioning_mode = 0;
    gclst_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gclst_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gclst_io.arg5.io_timer = 0;
    gclst_io.arg5.rec_len = 0;
    gclst_io.arg6.guardian_errcode = 0;
    gclst_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 回線ステータスファイルオープン               */
    /*----------------------------------------------*/
    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_OPEN, sizeof(DEF_COM_IOM_FUNC_OPEN)-1);
    memcpy(gclst_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gclst_io.arg3.file_id, DEF_FL_LIN_STS, sizeof(DEF_FL_LIN_STS)-1);
    memcpy(gclst_io.arg3.file_name, myinfo.GCLST_name, myinfo.GCLST_name_len);
    memcpy(gclst_io.arg3.file_io_type, DEF_TRACE_OPEN, sizeof(DEF_TRACE_OPEN)-1);
    memcpy(gclst_io.arg4.file_id, DEF_GCLST, sizeof(DEF_GCLST)-1);
    memcpy(gclst_io.arg4.file_name, myinfo.GCLST_name, myinfo.GCLST_name_len);

    COM_IOM(gclst_io.func_type,gclst_io.sub_prog_sts,&gclst_io.arg3,&gclst_io.arg4,&gclst_io.arg5,&gclst_io.arg6);
    if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gclst_io.sub_prog_sts)) != 0) {
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_OPN_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_STS,"OPEN","",gclst_io.arg6.guardian_errcode,DEF_VAR_STOP);
        return (false);
    }
    myinfo.GCLST_fd = gclst_io.arg4.file_no;
    return (true);
} /*end of CNSV_GCLST_open*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  GCLST_load                                     */
/*  CALLING SEQ.    : void CNSV_GCLST_load ( short )                        */
/*  ARGUMENT        : スレッド番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイル読み込み処理                    */
/****************************************************************************/
short CNSV_GCLST_load(short tbl_no)
{
fileio_def       gclst_io;
db_gclst_def     *gclst_rec;
short           s_idx;
char            ach_report_key[40+1];
    gclst_rec = (db_gclst_def *)&gclst_io.arg6.rec_area[0];
    /* IOモジュール情報初期化 */
    memset((char *)&gclst_io, ' ', sizeof(gclst_io));
    gclst_io.arg4.file_no = myinfo.GCLST_fd;
    gclst_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gclst_io.arg5.part_key_position = 0;
    gclst_io.arg5.part_key_len = 0;
    gclst_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
    gclst_io.arg5.compare_len = DEF_GFLIN_PKEY_LEN;
    gclst_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Exact;
    gclst_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gclst_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gclst_io.arg5.io_timer = myinfo.file_io_timer;
    gclst_io.arg5.rec_len = db_gclst_def_Size;
    gclst_io.arg6.guardian_errcode = 0;
    gclst_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 回線ステータスファイル読み込み               */
    /*----------------------------------------------*/
    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gclst_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gclst_io.arg3.file_id, DEF_FL_LIN_STS, sizeof(DEF_FL_LIN_STS)-1);
    memcpy(gclst_io.arg3.file_name, myinfo.GCLST_name, myinfo.GCLST_name_len);
    memcpy(gclst_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gclst_io.arg4.file_id, DEF_GCLST, sizeof(DEF_GCLST)-1);
    memcpy(gclst_io.arg4.file_name, myinfo.GCLST_name, myinfo.GCLST_name_len);

    myinfo.sc_info_use_cnt = 0;
    for (s_idx = 0;s_idx < cf[tbl_no].sc_use;s_idx++) {
        memset(gclst_io.sub_prog_sts, ' ', sizeof(gclst_io.sub_prog_sts));
        gclst_io.arg6.guardian_errcode = 0;
        gclst_io.arg6.rec_len = 0;
        memcpy((char *)&gclst_io.arg5.key_value,(char *)&cf[tbl_no].sc_conf[s_idx].site_name,DEF_GFLIN_PKEY_LEN);

        COM_IOM(gclst_io.func_type,gclst_io.sub_prog_sts,&gclst_io.arg3,&gclst_io.arg4,&gclst_io.arg5,&gclst_io.arg6);
        if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gclst_io.sub_prog_sts)) == 0) {
            return (true);
        }
        if (memcmp(gclst_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gclst_io.sub_prog_sts)) != 0) {
            memset(ach_report_key,'\0',sizeof(ach_report_key));
            memcpy(ach_report_key,(char *)&cf[tbl_no].sc_conf[s_idx].site_name,DEF_GFLIN_PKEY_LEN);
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_LIN_STS,"READ",ach_report_key,gclst_io.arg6.guardian_errcode,DEF_VAR_STOP);
            return (false);
        }
        memcpy((char *)&sc_info[s_idx].site_name,(char *)&gclst_rec->pri_key.site_id,DEF_GFLIN_PKEY_LEN);
        memcpy((char *)&sc_info[s_idx].status_info,(char *)&gclst_rec->connect_sts_info,sizeof(sc_info[s_idx].status_info));
        memcpy((char *)&sc_info[s_idx].process_info,(char *)&gclst_rec->prc_sts_info,sizeof(sc_info[s_idx].process_info));
        memcpy((char *)&sc_info[s_idx].connection_info.error_code,(char *)&gclst_rec->connect_info.err_code,sizeof(sc_info[s_idx].connection_info.error_code));
        memcpy((char *)&sc_info[s_idx].connection_info.disconnect_reason,(char *)&gclst_rec->connect_info.disconnect_rsn,sizeof(sc_info[s_idx].connection_info.disconnect_reason));
        myinfo.sc_info_use_cnt++;
    }
    return (true);
} /*end of CNSV_GCLST_load*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_GCLST_update                              */
/*  CALLING SEQ.    : void GCLST_update ( short , short , short )           */
/*  ARGUMENT        : スレッド番号                                          */
/*                  : コネクションステータス                                */
/*                  : プロセスステータス                                    */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 回線ステータスファイル更新処理                        */
/****************************************************************************/
short CNSV_GCLST_update(short thread,short connection_state,short process_state)
{
short           s_err;
struct
{
    char        site_id;
    char        nw_id;
    char        grp_id[5];
    char        if_id[5];
    char        station_id[6];
    char        connect_id[6];
} p_key;
long    TXID;
char    text_work[64];
    gclst_recin = (db_gclst_def *)&gclst_io.arg6.rec_area[0];
    gclst_recout = (db_gclst_def *)&gclst_io.arg5.rec_area[0];
    /* IOモジュール情報初期化 */
    memset((char *)&gclst_io, ' ', sizeof(gclst_io));
    gclst_io.arg4.file_no = myinfo.GCLST_fd;
    gclst_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gclst_io.arg5.part_key_position = 0;
    gclst_io.arg5.part_key_len = 0;
    gclst_io.arg5.key_len = DEF_GFLIN_PKEY_LEN;
    gclst_io.arg5.compare_len = DEF_GFLIN_PKEY_LEN;
    gclst_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Exact;
    gclst_io.arg5.lock_flg = DEF_COM_IOM_LOCK;
    gclst_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gclst_io.arg5.io_timer = myinfo.file_io_timer;
    gclst_io.arg5.rec_len = db_gclst_def_Size;
    gclst_io.arg6.guardian_errcode = 0;
    gclst_io.arg6.rec_len = 0;

    memcpy(gclst_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gclst_io.arg3.prog_id, DEF_GFPCVX10, sizeof(DEF_GFPCVX10)-1);
    memcpy(gclst_io.arg3.file_id, DEF_FL_LIN_STS, sizeof(DEF_FL_LIN_STS)-1);
    memcpy(gclst_io.arg3.file_name, myinfo.GCLST_name, myinfo.GCLST_name_len);
    memcpy(gclst_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gclst_io.arg4.file_id, DEF_GCLST, sizeof(DEF_GCLST)-1);
    memcpy(gclst_io.arg4.file_name, myinfo.GCLST_name, myinfo.GCLST_name_len);

    /*キー編集*/
    p_key.site_id = cf[myinfo.cf_idx].sc_conf[thread].site_name;
    p_key.nw_id = cf[myinfo.cf_idx].sc_conf[thread].nw_name;
    memcpy(p_key.grp_id,cf[myinfo.cf_idx].sc_conf[thread].group_name,sizeof(p_key.grp_id));
    memcpy(p_key.if_id,cf[myinfo.cf_idx].sc_conf[thread].interface_name,sizeof(p_key.if_id));
    memcpy(p_key.station_id,cf[myinfo.cf_idx].sc_conf[thread].station_name,sizeof(p_key.station_id));
    memcpy(p_key.connect_id,cf[myinfo.cf_idx].sc_conf[thread].src_connection_name,sizeof(p_key.connect_id));
    memcpy(gclst_io.arg5.key_value,(char *)&p_key,sizeof(p_key));

    s_err = COM_TMF(DEF_COM_TMF_BEGIN,&TXID,DEF_GFPCVX10);
    if (s_err != 0) {
        message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_TMF_ERR,"@X@5","COM_TMF",s_err,DEF_VAR_STOP);
        AbNormal_End();
    }

    COM_IOM(gclst_io.func_type,gclst_io.sub_prog_sts,&gclst_io.arg3,&gclst_io.arg4,&gclst_io.arg5,&gclst_io.arg6);
    if (memcmp(gclst_io.sub_prog_sts,DEF_COM_IOM_NO_ERR,sizeof(gclst_io.sub_prog_sts)) != 0) {
        memset(text_work,'\0',sizeof(text_work));
        memcpy(text_work,(char *)&p_key,sizeof(p_key));
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",myinfo.GCLST_name,"READ",text_work,gclst_io.arg6.guardian_errcode,DEF_VAR_STOP);
        s_err = COM_TMF(DEF_COM_TMF_ABORT,&TXID,DEF_GFPCVX10);
        if (s_err != 0) {
            message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_TMF_ERR,"@X@5","COM_TMF",s_err,DEF_VAR_STOP);
        }
        AbNormal_End();
    }

    COM_SDT(2,(COM_SDT_arg_2_def *)&myinfo.ts_char,(COM_SDT_arg_3_def *)&myinfo.ts_short,&myinfo.ts_64);
    memcpy((char *)gclst_recout, (char *)gclst_recin, sizeof(db_gclst_def));
    if (process_state == DEF_GCLST_proc_state_close) {  /*プロセス状態クローズの場合、ポートも切断にする*/
        connection_state = DEF_GCLST_con_state_close;
    }
    /*コネクションステータス更新*/
    switch (connection_state)
    {
    case DEF_GCLST_con_state_nochange: /*無変更*/
        break;
    case DEF_GCLST_con_state_close: /*切断*/
        memcpy(gclst_recout->connect_sts_info.connect_sts,DEF_CONNECT_STS_DISCONN,sizeof(gclst_recout->connect_sts_info.connect_sts));
        memcpy(gclst_recout->connect_sts_info.connect_sts_update_time,(char *)&myinfo.ts_char,sizeof(gclst_recout->connect_sts_info.connect_sts_update_time));
        break;
    case DEF_GCLST_con_state_listen: /*接続待ち*/
        memcpy(gclst_recout->connect_sts_info.connect_sts,DEF_CONNECT_STS_LISTEN,sizeof(gclst_recout->connect_sts_info.connect_sts));
        memcpy(gclst_recout->connect_sts_info.connect_sts_update_time,(char *)&myinfo.ts_char,sizeof(gclst_recout->connect_sts_info.connect_sts_update_time));
        break;
    case DEF_GCLST_con_state_open: /*接続*/
        memcpy(gclst_recout->connect_sts_info.connect_sts,DEF_CONNECT_STS_CONNECT,sizeof(gclst_recout->connect_sts_info.connect_sts));
        memcpy(gclst_recout->connect_sts_info.connect_sts_update_time,(char *)&myinfo.ts_char,sizeof(gclst_recout->connect_sts_info.connect_sts_update_time));
        break;
    case DEF_GCLST_con_state_reconnect: /*再接続*/
        memcpy(gclst_recout->connect_sts_info.connect_sts,DEF_CONNECT_STS_RECONNECT,sizeof(gclst_recout->connect_sts_info.connect_sts));
        memcpy(gclst_recout->connect_sts_info.connect_sts_update_time,(char *)&myinfo.ts_char,sizeof(gclst_recout->connect_sts_info.connect_sts_update_time));
        break;
    default:
        /*内部エラー処理*/
        AbNormal_End();
        break;
    }
    /*プロセスステータス更新*/
    switch (process_state)
    {
    case DEF_GCLST_proc_state_nocgange: /*無変更*/
        break;
    case DEF_GCLST_proc_state_close:
        memcpy(gclst_recout->prc_sts_info.prc_sts ,DEF_PROC_STS_CLS,sizeof(gclst_recout->connect_sts_info.connect_sts));
        memcpy(gclst_recout->prc_sts_info.prc_sts_update_time,(char *)&myinfo.ts_char,sizeof(gclst_recout->prc_sts_info.prc_sts_update_time));
        break;
    case DEF_GCLST_proc_state_open:
        memcpy(gclst_recout->prc_sts_info.prc_sts ,DEF_PROC_STS_OPN,sizeof(gclst_recout->connect_sts_info.connect_sts));
        memcpy(gclst_recout->prc_sts_info.prc_sts_update_time,(char *)&myinfo.ts_char,sizeof(gclst_recout->prc_sts_info.prc_sts_update_time));
        break;
    default:
        /*内部エラー処理*/
        AbNormal_End();
        break;
    }

    /*IPアドレス情報、切断情報の更新*/
    memset((char *)&gclst_recout->connect_info.ip_adress_src,' ',sizeof(gclst_recout->connect_info.ip_adress_src));
    memcpy((char *)&gclst_recout->connect_info.ip_adress_src,(char *)&cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr,strlen(cf[myinfo.cf_idx].sc_conf[thread].local_ipaddr));
    memset((char *)&gclst_recout->connect_info.port_num_src,' ',sizeof(gclst_recout->connect_info.port_num_src));
    memcpy((char *)&gclst_recout->connect_info.port_num_src,(char *)&cf[myinfo.cf_idx].sc_conf[thread].local_port_no,strlen(cf[myinfo.cf_idx].sc_conf[thread].local_port_no));
    memset((char *)&gclst_recout->connect_info.ip_adress_dst,' ',sizeof(gclst_recout->connect_info.ip_adress_dst));
    memcpy((char *)&gclst_recout->connect_info.ip_adress_dst,(char *)&cf[myinfo.cf_idx].sc_conf[thread].remote_ipaddr,strlen(cf[myinfo.cf_idx].sc_conf[thread].remote_ipaddr));
    memset((char *)&gclst_recout->connect_info.port_num_dst,' ',sizeof(gclst_recout->connect_info.port_num_dst));
    memcpy((char *)&gclst_recout->connect_info.port_num_dst,(char *)&cf[myinfo.cf_idx].sc_conf[thread].remote_port_no,strlen(cf[myinfo.cf_idx].sc_conf[thread].remote_port_no));
    memcpy((char *)&gclst_recout->connect_info.err_code,(char *)&sc_info[thread].connection_info.error_code,sizeof(gclst_recout->connect_info.err_code));
    memcpy((char *)&gclst_recout->connect_info.disconnect_rsn,(char *)&sc_info[thread].connection_info.disconnect_reason,sizeof(gclst_recout->connect_info.disconnect_rsn));

    memcpy(gclst_io.func_type,DEF_COM_IOM_FUNC_UPDATE,sizeof(gclst_io.func_type));
    memcpy(gclst_io.arg3.file_io_type, DEF_TRACE_REWRITE, sizeof(DEF_TRACE_REWRITE)-1);
    gclst_io.arg5.lock_flg = DEF_COM_IOM_LOCK;
    COM_IOM(gclst_io.func_type,gclst_io.sub_prog_sts,&gclst_io.arg3,&gclst_io.arg4,&gclst_io.arg5,&gclst_io.arg6);
    if (memcmp(gclst_io.sub_prog_sts,DEF_COM_IOM_NO_ERR,sizeof(gclst_io.sub_prog_sts)) != 0) {
        /*エラー処理*/
        memset(text_work,'\0',sizeof(text_work));
        memcpy(text_work,(char *)&p_key,sizeof(p_key));
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",myinfo.GCLST_name,"REWRITE",text_work,gclst_io.arg6.guardian_errcode,DEF_VAR_STOP);
        s_err = COM_TMF(DEF_COM_TMF_ABORT,&TXID,DEF_GFPCVX10);
        if (s_err != 0) {
            message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_TMF_ERR,"@X@5","COM_TMF",s_err,DEF_VAR_STOP);
        }
        AbNormal_End();
    }

    memcpy((char *)&sc_info[thread].status_info,(char *)&gclst_recout->connect_sts_info,sizeof(sc_info[thread].status_info));
    memcpy((char *)&sc_info[thread].process_info,(char *)&gclst_recout->prc_sts_info,sizeof(sc_info[thread].process_info));
    memcpy((char *)&sc_info[thread].connection_info.error_code,(char *)&gclst_recout->connect_info.err_code,sizeof(sc_info[thread].connection_info.error_code));
    memcpy((char *)&sc_info[thread].connection_info.disconnect_reason,(char *)&gclst_recout->connect_info.disconnect_rsn,sizeof(sc_info[thread].connection_info.disconnect_reason));

    s_err = COM_TMF(DEF_COM_TMF_END,&TXID,DEF_GFPCVX10);
    if (s_err != 0) {
        message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_TMF_ERR,"@X@5","COM_TMF",s_err,DEF_VAR_STOP);
        AbNormal_End();
    }

    return (true);
} /*end of CNSV_GCLST_update*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_load_server_info                          */
/*  CALLING SEQ.    : void CNSV_load_server_info ( short )                  */
/*  ARGUMENT        : テーブル番号                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : サーバー情報取得処理                                  */
/****************************************************************************/
short CNSV_load_server_info(short tbl_no)
{
fileio_def      gfphi_io;
db_gfphi_def    *gfphi_rec;
short           s_idx, s_idx2;
short           s_loop, s_find;
struct
{
    char        site_id;            /*比較対象*/
    char        nw_id;              /*比較対象*/
    char        grp_id[5];          /*比較対象*/
    char        srv_cls_kind[8];    /*比較対象*/
    char        srv_cls_num[4];
    char        srv_cls_mlt_num[4];
    char        prc_file_kind[8];
    char        prc_file_num[4];
    char        prc_file_mlt_num[4];
} compare_key;
char            ach_report_key[40+1];

    gfphi_rec = (db_gfphi_def *)&gfphi_io.arg6.rec_area[0];

    /* IOモジュール情報初期化 */
    memset((char *)&gfphi_io, ' ', sizeof(gfphi_io));
    gfphi_io.arg4.file_no = myinfo.GFPHI_fd;
    gfphi_io.arg5.part_key_type = DEF_COM_IOM_PARTITION_KEY_NOT;
    gfphi_io.arg5.part_key_position = 0;
    gfphi_io.arg5.part_key_len = 0;
    gfphi_io.arg5.key_len = 0;
    gfphi_io.arg5.compare_len = 0;
    gfphi_io.arg5.positioning_mode = 0;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = 0;
    gfphi_io.arg5.rec_len = 0;
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    /*----------------------------------------------*/
    /* 物理名情報ファイルREAD                       */
    /*----------------------------------------------*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gfphi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gfphi_io.arg5.key_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.compare_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Exact;
    gfphi_io.arg5.lock_flg = DEF_COM_IOM_NOLOCK;
    gfphi_io.arg5.asc_desc_type = DEF_COM_IOM_ASCEND;
    gfphi_io.arg5.io_timer = myinfo.file_io_timer;
    gfphi_io.arg5.rec_len = db_gfphi_def_Size;

    /*リスナー情報、サーバクラス論理ID.サーバクラス種類(srv_cls_kind)迄設定*/
    gfphi_rec->pri_key.site_id = cf[tbl_no].sc_conf->site_name;
    gfphi_rec->pri_key.nw_id = cf[tbl_no].sc_conf->nw_name;
    memcpy(gfphi_rec->pri_key.grp_id, cf[tbl_no].sc_conf->group_name, sizeof(gfphi_rec->pri_key.grp_id));

    for ( s_idx = 0; s_idx < cf[tbl_no].lc_use; s_idx++) {
        if (s_idx >= DEF_MAX_LISTNER) { /*リスナー定義数オーバー*/
            message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_NW_INFO,"","GFPHI LISTNER record over",DEF_VAR_STOP);
            return (false);
        }
        memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
        gfphi_io.arg6.guardian_errcode = 0;
        gfphi_io.arg6.rec_len = 0;
        memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,cf[tbl_no].lc_conf[s_idx].serverclass_logical_name, sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
        memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,cf[tbl_no].lc_conf[s_idx].serverclass_logical_num,sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
        memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num));
        memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind,DEF_PRC_LISTEN,sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind));
        memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, cf[tbl_no].lc_conf[s_idx].serverclass_logical_num, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num));
        memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, "0000", sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num));
        memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));

        COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
        if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
            /*レコード無しは設定が不整合*/
            message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X",DEF_FL_NW_INFO,"","PSNLISTN record empty",DEF_VAR_STOP);
            AbNormal_End();
        }
        if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
            message_output(DEF_EVT_COMMON_MOD_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@5","COM_IOM",gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
            AbNormal_End();
        }
        memcpy(cf[tbl_no].lc_conf[s_idx].pathmon_name,gfphi_rec->srv_cls_info.pathmon_name,sizeof(cf[tbl_no].lc_conf[s_idx].pathmon_name));
        memcpy(cf[tbl_no].lc_conf[s_idx].serverclass_name,gfphi_rec->srv_cls_info.srv_cls_name,sizeof(cf[tbl_no].lc_conf[s_idx].serverclass_name));
        memcpy(cf[tbl_no].lc_conf[s_idx].process_name,gfphi_rec->prc_file_info.prc_file_name,sizeof(cf[tbl_no].lc_conf[s_idx].process_name));
        CNSV_set_null(cf[tbl_no].lc_conf[s_idx].process_name,sizeof(cf[tbl_no].lc_conf[s_idx].process_name));
        cf[tbl_no].lc_conf[s_idx].process_name_len = (short)strlen(cf[tbl_no].lc_conf[s_idx].process_name);
        cf[tbl_no].lc_conf[s_idx].validity_flag = gfphi_rec->invalid_flg;

        lc_info[s_idx].process_name_len = cf[tbl_no].lc_conf[s_idx].process_name_len;
        memcpy(lc_info[s_idx].process_name,cf[tbl_no].lc_conf[s_idx].process_name,cf[tbl_no].lc_conf[s_idx].process_name_len);

    }

    /*リスナープロセス情報設定*/
    for ( s_loop = 0, s_idx = 0; s_idx < cf[tbl_no].lc_use; s_idx++ ) {
        for ( s_find = false, s_idx2 = 0; s_idx2 < s_loop ; s_idx2++ ) {
            if (memcmp(cf[tbl_no].lc_conf[s_idx].process_name,lp_info[s_idx2].process_name,cf[tbl_no].lc_conf[s_idx].process_name_len) == 0) {
                /*登録済*/
                lc_info[s_idx].mng_no = s_idx2;
                s_find = true;
                break;
            }
        }
        if (s_find == false) {
            for ( s_idx2 = 0; s_idx2 <= s_loop; s_idx2++ ) {
                if (lp_info[s_idx2].process_name[0] == 0) {  /*未登録エントリー*/
                    lp_info[s_idx2].site_name = cf[tbl_no].lc_conf[s_idx].site_name;
                    lp_info[s_idx2].nw_name = cf[tbl_no].lc_conf[s_idx].nw_name;
                    memcpy(lp_info[s_idx2].group_name,cf[tbl_no].lc_conf[s_idx].group_name,5);
                    memcpy(lp_info[s_idx2].pathmon_name,cf[tbl_no].lc_conf[s_idx].pathmon_name,sizeof(lp_info[s_idx2].pathmon_name));
                    memcpy(lp_info[s_idx2].server_class,cf[tbl_no].lc_conf[s_idx].serverclass_name,sizeof(lp_info[s_idx].server_class));
                    lp_info[s_idx2].process_name_len = cf[tbl_no].lc_conf[s_idx].process_name_len;
                    memcpy(lp_info[s_idx2].process_name,cf[tbl_no].lc_conf[s_idx].process_name,cf[tbl_no].lc_conf[s_idx].process_name_len);
                    lc_info[s_idx].mng_no = s_idx2;
                    s_loop++;
                    break;
                }
            }
        }
    }

    myinfo.lp_info_use_cnt = 0;
    for ( s_idx = 0; s_idx < cf[tbl_no].lc_use; s_idx++ ) { /*再読み込み追加対応、使用中テーブルカウント*/
        if (lp_info[s_idx].process_name[0] != 0) {
            myinfo.lp_info_use_cnt++;
        }
    }

    /*Inbound電文振分*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gfphi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gfphi_io.arg5.key_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.compare_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Exact;
    gfphi_rec->pri_key.site_id = cf[tbl_no].sc_conf->site_name;
    gfphi_rec->pri_key.nw_id = cf[tbl_no].sc_conf->nw_name;
    memcpy(gfphi_rec->pri_key.grp_id, cf[tbl_no].sc_conf->group_name, sizeof(gfphi_rec->pri_key.grp_id));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,DEF_SC_FURI_I, sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,"0000",sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num));
    memcpy(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num, "0000", sizeof(gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind,DEF_SC_NAME_DEFAULT,sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num, DEF_SC_NUM_DEFAULT, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_num));
    memcpy(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num, DEF_SC_DUP_DEFAULT, sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_mlt_num));

    memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
    memcpy(gfphi_io.arg5.key_value, (char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
    gfphi_io.arg6.guardian_errcode = 0;
    gfphi_io.arg6.rec_len = 0;

    COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
        message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_PHSIC_INFO,"","GFPHI Inbound record empty",DEF_VAR_STOP);
        return (false);
    }
    if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
        memset(ach_report_key,'\0',sizeof(ach_report_key));
        memcpy(ach_report_key,(char *)&gfphi_rec->pri_key, sizeof(gfphi_rec->pri_key));
        message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"READ",ach_report_key,gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
        AbNormal_End();
    }
    ib_info->site_name = gfphi_rec->pri_key.site_id;
    ib_info->nw_name = gfphi_rec->pri_key.nw_id;
    memcpy(ib_info->group_name,gfphi_rec->pri_key.grp_id,sizeof(ib_info->group_name));

    if (gfphi_rec->srv_cls_info.domain_name[0] != ' ') {    /*ドメイン指定有り*/
        memset(ib_info->pathmon_name,' ',sizeof(ib_info->pathmon_name));
        memcpy(ib_info->pathmon_name,gfphi_rec->srv_cls_info.domain_name,sizeof(gfphi_rec->srv_cls_info.domain_name));
    } else {                                                /*PATHMON指定*/
        memcpy(ib_info->pathmon_name,gfphi_rec->srv_cls_info.pathmon_name,sizeof(ib_info->pathmon_name));
    }
    CNSV_set_null(ib_info->pathmon_name,sizeof(ib_info->pathmon_name));
    ib_info->pathmon_name_len = (short)strlen(ib_info->pathmon_name);

    memcpy(ib_info->serverclass_name,gfphi_rec->srv_cls_info.srv_cls_name,sizeof(ib_info->serverclass_name));
    CNSV_set_null(ib_info->serverclass_name,sizeof(ib_info->serverclass_name));
    ib_info->serverclass_name_len = (short)strlen(ib_info->serverclass_name);

    /*Outbound電文振分(冗長化レコード)*/
    memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_STARTREAD, sizeof(DEF_COM_IOM_FUNC_STARTREAD)-1);
    memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_START, sizeof(DEF_TRACE_START)-1);
    memcpy(gfphi_io.arg5.key_type, DEF_COM_IOM_KEYTYPE_PRI, sizeof(DEF_COM_IOM_KEYTYPE_PRI)-1);
    gfphi_io.arg5.key_len = DEF_GFPHI_PKEY_LEN;
    gfphi_io.arg5.compare_len = (1+1+5+8);      /*比較長*/
    gfphi_io.arg5.positioning_mode = DEF_POSITIONING_MODE_Generic;
    memset((char *)&compare_key,'\0',sizeof(compare_key));
    compare_key.site_id = cf[tbl_no].sc_conf->site_name;
    compare_key.nw_id = cf[tbl_no].sc_conf->nw_name;
    memcpy(compare_key.grp_id,cf[tbl_no].sc_conf->group_name,sizeof(compare_key.grp_id));
    memcpy(compare_key.srv_cls_kind,DEF_SC_FURI_O,sizeof(compare_key.srv_cls_kind));
    memcpy(compare_key.srv_cls_num,"0000",sizeof(compare_key.srv_cls_num));
    memcpy(compare_key.srv_cls_mlt_num,"0000",sizeof(compare_key.srv_cls_mlt_num));
    memcpy(gfphi_io.arg5.key_value, (char *)&compare_key, sizeof(compare_key));

    for (s_loop = 0, s_idx = 0; s_loop == 0; ) {
        memset(gfphi_io.sub_prog_sts, ' ', sizeof(gfphi_io.sub_prog_sts));
        gfphi_io.arg6.guardian_errcode = 0;
        gfphi_io.arg6.rec_len = 0;

        COM_IOM(gfphi_io.func_type,gfphi_io.sub_prog_sts,&gfphi_io.arg3,&gfphi_io.arg4,&gfphi_io.arg5,&gfphi_io.arg6);
        if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_EOF_ERR, sizeof(gfphi_io.sub_prog_sts)) == 0) {
            return (true);
        }
        if (memcmp(gfphi_io.sub_prog_sts, DEF_COM_IOM_NO_ERR, sizeof(gfphi_io.sub_prog_sts)) != 0) {
            memset(ach_report_key,'\0',sizeof(ach_report_key));
            memcpy(ach_report_key,(char *)&compare_key, sizeof(gfphi_rec->pri_key));
            message_output(DEF_EVT_FILE_IO_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_FILE_IO_ERR,"@X@X@X@X@X@5","","",DEF_FL_PHSIC_INFO,"READ",ach_report_key,gfphi_io.arg6.guardian_errcode,DEF_VAR_STOP);
            AbNormal_End();
        }
        if (memcmp((char *)&gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind,DEF_PRC_FURI_O,sizeof(gfphi_rec->pri_key.prc_file_key.prc_file_id.prc_file_kind)) == 0) {
            cf[tbl_no].ob_conf[s_idx].site_name = gfphi_rec->pri_key.site_id;
            cf[tbl_no].ob_conf[s_idx].nw_name = gfphi_rec->pri_key.nw_id;
            memcpy(cf[tbl_no].ob_conf[s_idx].group_name,gfphi_rec->pri_key.grp_id,sizeof(cf[tbl_no].ob_conf[s_idx].group_name));
            memcpy(cf[tbl_no].ob_conf[s_idx].srv_cls_key.srv_cls_id.srv_cls_kind,gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_kind,sizeof(cf[tbl_no].ob_conf[s_idx].srv_cls_key.srv_cls_id.srv_cls_kind));
            memcpy(cf[tbl_no].ob_conf[s_idx].srv_cls_key.srv_cls_id.srv_cls_num,gfphi_rec->pri_key.srv_cls_key.srv_cls_id.srv_cls_num,sizeof(cf[tbl_no].ob_conf[s_idx].srv_cls_key.srv_cls_id.srv_cls_num));
            memcpy(cf[tbl_no].ob_conf[s_idx].srv_cls_key.srv_cls_id.srv_cls_redundan_num,gfphi_rec->pri_key.srv_cls_key.srv_cls_mlt_num,sizeof(cf[tbl_no].ob_conf[s_idx].srv_cls_key.srv_cls_id.srv_cls_redundan_num));
            memcpy(cf[tbl_no].ob_conf[s_idx].pathmon_name,gfphi_rec->srv_cls_info.pathmon_name,sizeof(cf[tbl_no].ob_conf[s_idx].pathmon_name));
            memcpy(cf[tbl_no].ob_conf[s_idx].server_class,gfphi_rec->srv_cls_info.srv_cls_name,sizeof(cf[tbl_no].ob_conf[s_idx].server_class));
            memcpy(cf[tbl_no].ob_conf[s_idx].process_name,gfphi_rec->prc_file_info.prc_file_name,sizeof(cf[tbl_no].ob_conf[s_idx].process_name));
            CNSV_set_null(cf[tbl_no].ob_conf[s_idx].process_name,sizeof(cf[tbl_no].ob_conf[s_idx].process_name));
            cf[tbl_no].ob_conf[s_idx].process_name_len = (short)strlen(cf[tbl_no].ob_conf[s_idx].process_name);
            cf[tbl_no].ob_conf[s_idx].validity_flag = gfphi_rec->invalid_flg;
            cf[tbl_no].ob_use++;
            s_idx++;
            if (s_idx > DEF_MAX_OUTBOUND) {
                message_output(DEF_EVT_CONFIG_ERR,DEF_MSGTTKB_GYOM_ERR,DEF_NERR_PRM_RD_ERR_INV,"@X@X@X",DEF_FL_PHSIC_INFO,"","GFPHI Outbound record over",DEF_VAR_STOP);
                return (false);
            }
        }
        /*次レコードREAD用*/
        memcpy(gfphi_io.func_type, DEF_COM_IOM_FUNC_NEXTREAD, sizeof(DEF_COM_IOM_FUNC_NEXTREAD)-1);
        memset(gfphi_io.arg3.file_io_type,0x20,sizeof(gfphi_io.arg3.file_io_type));
        memcpy(gfphi_io.arg3.file_io_type, DEF_TRACE_READ, sizeof(DEF_TRACE_READ)-1);
    }

    return (true);
} /*end of CNSV_load_server_info*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_sc_table_search                           */
/*  CALLING SEQ.    : short CNSV_sc_table_search ( char * ,short )          */
/*  ARGUMENT        : 検索するキー情報                                      */
/*                  : 比較する長さ                                          */
/*  RETURN CODE     : テーブル番号                                          */
/*  DESCRIPTION     : コネクションテーブルサーチ                            */
/****************************************************************************/
short CNSV_sc_table_search(char *key,short compare_len)
{
short left = 0;
short right;
short mid;
int cmp;

    right = cf[myinfo.cf_idx].sc_use -1;
    while (left <= right) {
        mid = left + (right - left) / 2;
        cmp = memcmp((char *)&cf[myinfo.cf_idx].sc_conf[mid].site_name,key,compare_len);
        if (cmp == 0) {
            return mid; /*指定キーが見つかった場合*/
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return (DEF_Not_Found);         /*指定キーなし*/
} /*end of CNSV_sc_table_search*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_stop_check                                */
/*  CALLING SEQ.    : void CNSV_stop_check ( short )                        */
/*  ARGUMENT        : チェック前OPEN数                                      */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : オープナー数をサマリーし停止条件を判定                */
/****************************************************************************/
void CNSV_stop_check(short before_count)
{
//-short   idx,idx1,sum;
//-    for ( idx = 0,sum = 0;idx < DEF_MAX_NODE; idx++ ) {     /*OPEN数サマリー*/
//-        if (openers[idx].node_name[0] != 0) {           /*登録有り*/
//-            for (idx1=0;idx1 < 16; idx1++) {
//-                sum += openers[idx].cpus[idx1].opener_count;
//-            }
//-        }
//-    }
//-    if (before_count == 0) {        /*OPENされるまえにCPUダウン等を受信した場合、停止しない*/
//-        return;
//-    }
//-    if (sum == 0) myinfo.end_flag = -1;                 /*オープナーが0の場合終了条件を設定*/
} /*end of CNSV_stop_check*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_create_list                               */
/*  CALLING SEQ.    : void CNSV_create_list ( Event_List_def * , int )      */
/*  ARGUMENT        : リスト                                                */
/*                  : 作成するノード数                                      */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスト作成処理                                        */
/****************************************************************************/
void CNSV_create_list(Event_List_def *list, int numNodes)
{
    list->list_count = numNodes;
    list->head = NULL;
    list->tail = NULL;
    for (int i = 0; i < numNodes; i++) {
        Event_Node_def *newNode = (Event_Node_def *)malloc(sizeof(Event_Node_def));
        newNode->next = NULL;
        newNode->compo = 0;
        newNode->thread = 0;
        newNode->event = 0;
        newNode->len = 0;
        newNode->text = 0;
        newNode->event_time = 0;
        if (list->tail) {
            list->tail->next = newNode;
        } else {
            list->head = newNode;
        }
        list->tail = newNode;
    }
} /*end of CNSV_create_list*/
/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_add_list                                  */
/*  CALLING SEQ.    : void CNSV_add_list ( Event_List_def *,                */
/*                          Event_List_def *, short , short , short ,       */
/*                          short , short , short ,char * )                 */
/*  ARGUMENT        : 空きリスト                                            */
/*                  : 追加するリスト                                        */
/*                  : コンポーネント                                        */
/*                  : スレッド                                              */
/*                  : イベント                                              */
/*                  : オプション1                                           */
/*                  : オプション2                                           */
/*                  : 追加テキスト長                                        */
/*                  : 追加テキストポインター                                */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : データ追加処理                                        */
/****************************************************************************/
void CNSV_add_list(Event_List_def *freeList, Event_List_def *usedList, short compo, short thread, short event, short opt1, short opt2, short len,char *text)
{
    Event_Node_def *nodeToMove = freeList->head;
    if (freeList->head == NULL) return; /*空きリストが空の場合*/

    freeList->head = freeList->head->next;
    if (freeList->head == NULL) {
        freeList->tail = NULL;
    }

    freeList->list_count--;
    if (freeList->list_count < 0) {
        AbNormal_End();
    }
    nodeToMove->next = NULL;
    nodeToMove->compo = compo;
    nodeToMove->thread = thread;
    nodeToMove->event = event;
    nodeToMove->option1 = opt1;
    nodeToMove->option2 = opt2;
    nodeToMove->len = len;
    nodeToMove->text = text;
    nodeToMove->event_time = JULIANTIMESTAMP();
    usedList->list_count++;
    if (usedList->tail) {
        usedList->tail->next = nodeToMove;
    } else {
        usedList->head = nodeToMove;
    }
    usedList->tail = nodeToMove;
} /*end of CNSV_add_list*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_remove_list                               */
/*  CALLING SEQ.    : void CNSV_remove_list ( Event_List_def *,             */
/*                              Event_List_def *,Event_Node_def * )         */
/*  ARGUMENT        : 開放するリスト                                        */
/*                  : 空きリスト                                            */
/*                  : イベント内容                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : リスト削除処理                                        */
/*                  : 削除したリストの内容を呼び出し元に返す                */
/*                  : リストの付け替えのみ内容はクリアしない                */
/****************************************************************************/
void CNSV_remove_list(Event_List_def *usedList, Event_List_def *freeList,Event_Node_def *node)
{
    Event_Node_def *nodeToMove = usedList->head;
    if (usedList->head == NULL) return; /*使用中リストが空の場合*/

    usedList->head = usedList->head->next;
    if (usedList->head == NULL) {
        usedList->tail = NULL;
    }
    usedList->list_count--;
    if (usedList->list_count < 0) {
        AbNormal_End();
    }

    node->compo      = nodeToMove->compo;
    node->thread     = nodeToMove->thread;
    node->event      = nodeToMove->event;
    node->option1    = nodeToMove->option1;
    node->option2    = nodeToMove->option2;
    node->len        = nodeToMove->len;
    node->text       = nodeToMove->text;
    node->event_time = 0;

    nodeToMove->next = NULL;
    if (freeList->tail) {
        freeList->tail->next = nodeToMove;
    } else {
        freeList->head = nodeToMove;
    }
    freeList->tail = nodeToMove;
    freeList->list_count++;
} /*end of CNSV_remove_list*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_create_buff_list                          */
/*  CALLING SEQ.    : void CNSV_create_buff_list ( buff_List_def *, int )   */
/*  ARGUMENT        : バッファーリスト                                      */
/*                  : 作成するノード数                                      */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : バッファーリスト作成処理                              */
/****************************************************************************/
void CNSV_create_buff_list(buff_list_def *list, int numNodes)
{
    list->list_count = numNodes;
    list->head = NULL;
    list->tail = NULL;
    for (int i = 0; i < numNodes; i++) {
        buff_node_def *newNode = (buff_node_def *)malloc(sizeof(buff_node_def));
        newNode->next = NULL;
        if (list->tail) {
            list->tail->next = newNode;
        } else {
            list->head = newNode;
        }
        list->tail = newNode;
    }
} /*end of CNSV_create_list*/

/****************************************************************************/
/*  FUNCTION        : 1.1.0  message_output                                 */
/*  CALLING SEQ.    : void message_output (short ,short ,char *,char *,...) */
/*  ARGUMENT        : メッセージ番号                                        */
/*                  : メッセージ通知区分                                    */
/*                  : 内部エラーコード                                      */
/*                  : フォーマット                                          */
/*                  : 可変パラメータ                                        */
/*                  : ：                                                    */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : メッセージ編集出力処理                                */
/****************************************************************************/
void message_output (short msg_no,short MSGTTKB,char *inter_errcd,char *format,...)
{
char    text[99];
char    fmt[99];
char    msg_num[6];
char    *ep,*sp;
short   var,vcnt = 0,cnt,idx,param_cnt = 0,loop_flg = 1;
unsigned short port_no;
struct  sockaddr_in ip_info;
va_list ap;
oggz1in_def z1_in;

    memset((char *)&z1_in,0x20,sizeof(z1_in));
    z1_in.subrcd = '0';
    memcpy(z1_in.uytrminf.proctimer,myinfo.uytrmtimer,sizeof(z1_in.uytrminf.proctimer));
    memcpy(z1_in.uytrminf.uytrmmon,myinfo.uytrmmon,sizeof(z1_in.uytrminf.uytrmmon));
    memcpy(z1_in.uytrminf.uytrmmonlen,myinfo.uytrmmonlen,sizeof(z1_in.uytrminf.uytrmmonlen));
    memcpy(z1_in.uytrminf.uytrmsrv,myinfo.uytrmsrv,sizeof(z1_in.uytrminf.uytrmsrv));
    memcpy(z1_in.uytrminf.uytrmsrvlen,myinfo.uytrmsrvlen,sizeof(z1_in.uytrminf.uytrmsrvlen));
    z1_in.emsinf.rcd = '0';
    sprintf(msg_num,"%05d",msg_no);
    memcpy(z1_in.emsinf.msgid,msg_num,sizeof(z1_in.emsinf.msgid));
    switch (MSGTTKB)
    {
        case DEF_MSGTTKB_NORMAL:        /*メッセージ通知区分(正常)*/
            z1_in.emsinf.emsgkinf.msgttkb = '*';
            break;
        case DEF_MSGTTKB_SYSTEM_ERR:    /*メッセージ通知区分(システムエラー)*/
            z1_in.emsinf.emsgkinf.msgttkb = 'S';
            break;
        case DEF_MSGTTKB_WARNING:       /*メッセージ通知区分(警告)*/
            z1_in.emsinf.emsgkinf.msgttkb = 'W';
            break;
        case DEF_MSGTTKB_GYOM_ERR:      /*メッセージ通知区分(業務エラー)*/
            z1_in.emsinf.emsgkinf.msgttkb = 'E';
            break;
        default:
            break;
    }
    memcpy(z1_in.emsinf.emsgkinf.sysnm,DEF_EMS_SYSNM_GFP,sizeof(z1_in.emsinf.emsgkinf.sysnm));
    memcpy(z1_in.emsinf.emsgkinf.srv_kbn,DEF_EMS_SRV_KBN_COM,sizeof(z1_in.emsinf.emsgkinf.srv_kbn));
    memcpy(z1_in.emsinf.emsgkinf.h_nw_kbn,cf[myinfo.cf_idx].nw_kubun,sizeof(z1_in.emsinf.emsgkinf.h_nw_kbn));
    memcpy(z1_in.emsinf.emsgkinf.s_nw_kbn,"  ",sizeof(z1_in.emsinf.emsgkinf.s_nw_kbn));
    memcpy(z1_in.emsinf.emsgkinf.prgid,DEF_GFPCVX10,sizeof(DEF_GFPCVX10)-1);
    memcpy(z1_in.emsinf.emsgkinf.trmnm,myinfo.my_name,myinfo.my_name_len);
    memcpy(z1_in.emsinf.emsgkinf.inter_errcd,inter_errcd,sizeof(z1_in.emsinf.emsgkinf.inter_errcd));

    memset(fmt,'\0',sizeof(fmt));
    memcpy(fmt,format,strlen(format));
    for (cnt = 0,idx = 0; idx < strlen(fmt); idx++) if (fmt[idx] == 0x40) cnt++;
    va_start(ap,format);
    if (myinfo.server_class_name[0] != '\0') {
        memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,myinfo.server_class_name,sizeof(myinfo.server_class_name)+sizeof(myinfo.server_class_num));
        param_cnt++;
    }

    sp = fmt;
    while (sp[0] && loop_flg) {
        switch (sp[0]) {
            case '@':
                sp++;
                break;
            case 'T':       /*tcp/ip process name*/
                ep = (char *)va_arg(ap,char *);
                memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,ep,strlen(ep));
                sp++;
                param_cnt++;
                vcnt++;
                break;
            case 'A':       /*ip address*/
                ip_info.sin_addr.s_addr = (in_addr_t)va_arg(ap,in_addr_t);
                ep = inet_ntoa(ip_info.sin_addr);
                memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,ep,strlen(ep));
                sp++;
                param_cnt++;
                vcnt++;
                break;
            case 'P':       /*port num*/
                port_no = (unsigned short)va_arg(ap,short);
                sprintf(text,"%05u",port_no);
                memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,text,strlen(text));
                sp++;
                param_cnt++;
                vcnt++;
                break;
            case '5':       /*Error code,pathsend error code*/
                var = (short)va_arg(ap,short);
                sprintf(text,"%05u",var);
                memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,text,strlen(text));
                sp++;
                param_cnt++;
                vcnt++;
                break;
            case 'X':       /*text*/
                ep = (char *)va_arg(ap,char *);
                memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,ep,strlen(ep));
                sp++;
                param_cnt++;
                vcnt++;
                break;
            case 'I':       /*IPC HDR*/
                ep = (char *)va_arg(ap,char *);
                memset(text,' ',sizeof(text));
                HEX2CHAR(ep,text,24);
                memcpy((char *)z1_in.emsinf.emsnninf.msgtbl[param_cnt].msgtbl_vl,text,48);
                sp++;
                param_cnt++;
                vcnt++;
                break;
            default:
                sp++;
              break;
        }
       if (vcnt==cnt) break;
    }
    va_end(ap);

    GFPOGGZ1((oggz1in_def *)&z1_in);

} /* end of message_output */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  BCD2CHAR                                       */
/*  CALLING SEQ.    : void BCD2CHAR ( char *, char *, short )               */
/*  ARGUMENT        : BCDデータ                                             */
/*                  : ASCIIデータ格納先                                     */
/*                  : BCDデータサイズ                                       */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : BCDデータASCII変換処理                                */
/****************************************************************************/
//-void BCD2CHAR(unsigned char *bcd_p, char *ascii_p,short s_len)
//-{
//-    short s_count;
//-    const char ToNUM_tbl[16] = {"0123456789******"};    /* ニューメリック変換テーブル */
//-
//-    for (s_count = 0; s_count < s_len; s_count++) {
//-        ascii_p[s_count*2]   = ToNUM_tbl[bcd_p[s_count] >> 4];
//-        ascii_p[s_count*2+1] = ToNUM_tbl[bcd_p[s_count] & 0x0f];
//-    }
//-} /* end of BCD2CHAR */

/****************************************************************************/
/*  FUNCTION        : 1.1.0  EBCNUM2CHAR                                    */
/*  CALLING SEQ.    : short EBCNUM2CHAR ( char *, char *, short )           */
/*  ARGUMENT        : EBCDICデータ                                          */
/*                  : ASCIIデータ格納先                                     */
/*                  : EBCDICデータサイズ                                    */
/*  RETURN CODE     : 0:正常 -1:異常                                        */
/*  DESCRIPTION     : EBCDIC数字ASCII変換処理                               */
/****************************************************************************/
short EBCNUM2CHAR(unsigned char *ebcdic_p, char *ascii_p,short s_len)
{
    short s_count;
    for (s_count = 0;s_count < s_len; s_count++) {
        if (ebcdic_p[s_count] < 0xf0 || ebcdic_p[s_count] > 0xf9) {
            return -1;
        } else {
            ascii_p[s_count] = ebcdic_p[s_count] - 0xc0;
        }
    }
    return 0;
}

/****************************************************************************/
/*  FUNCTION        : 1.1.0  CNSV_set_null                                  */
/*  CALLING SEQ.    : void CNSV_set_null ( char *, short )                  */
/*  ARGUMENT        : 編集する項目                                          */
/*                  : エリアサイス                                          */
/*  RETURN CODE     : void                                                  */
/*  DESCRIPTION     : 文字列終端Null値設定処理                              */
/****************************************************************************/
void CNSV_set_null(char *dt,int len)
{
int     ix;
    for (ix = len-1;ix > 0; ix--) {
        if (dt[ix] == 0x20 || dt[ix] == 0x00) {
            dt[ix] = 0x00;
        } else return;
    }
} /*end of CNSV_set_null*/
