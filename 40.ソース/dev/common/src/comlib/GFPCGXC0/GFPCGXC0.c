/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ CBS                                         */
/*        SUB-SYSTEM        ････ GFP通信制御                                 */
/*        PROGRAM-ID        ････ GFPCGXC0                                    */
/*        FUNCTION          ････ 共通モジュール                              */
/*                               サーバー停止判定モジュール                  */
/*                                                                           */
/*                               システムメッセージの内容をもとに呼び元の    */
/*                               親プログラムのサーバー停止判定の条件となる  */
/*                               「オープナーカウンタ」のカウントアップ      */
/*                               、カウントダウン管理を行う共通サブルーチン。*/
/*                               本共通サブルーチン内ではノード・CPU単位に   */
/*                               管理されたオープン数を合計し                */
/*                              「オープナーカウンタ」に設定する。           */
/*                                                                           */
/*        AUTHER            ････ HAS T.Fukunaga                              */
/*        PROGRAM-CALL      ････                                             */
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

/****************************************************************************/
/*   INCLUDE定義                                                            */
/****************************************************************************/
/* STANDARD HEADER */
#include <stdbool.h>  nolist
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <cextdecs.h> nolist
#include <tal.h>      nolist
#include <clurdec.h>  nolist
#include "/G/system/zsysdefs/zsysc" nolist

/* USER HEADER     */
#include "GFPCGXC0.h"

/* vproc関数の宣言 */
#include "vproc.h"
/****************************************************************************/
/*   DEFINE定義                                                             */
/****************************************************************************/
#define   DEF_COM_STP_NODE_NUM        10             /* 最大ノード数            */
#define   DEF_COM_STP_CPU_NUM         16             /* 最大CPU数               */
#define   DEF_COM_STP_NODE_NAME_LEN    8             /* ノード名長              */
#define   DEF_COM_STP_TBL_NO_HIT       0             /* テーブル未存在          */
#define   DEF_COM_STP_TBL_HIT          1             /* テーブル存在            */
/****************************************************************************/
/*   内部関数宣言                                                           */
/****************************************************************************/

/****************************************************************************/
/*  FUNCTION        : 1.0.0  COM_STP_INIT                                   */
/*  CALLING SEQ.    : void   COM_STP_INIT(struct *)                         */
/*  ARGUMENT        : 1.open_info       (O)   オープナー情報                */
/*  RETURN CODE     :                                                       */
/*  DESCRIPTION     : 監視対象ノード、監視対象CPUの設定および               */
/*                    オープナー情報の初期化を行う                          */
/****************************************************************************/
void  COM_STP_INIT(COM_STP_arg_1_def *open_info)
{
    short  idx_n;                     /* オープナー情報テーブルNODE INDEX  */
    short  idx_c;                     /* オープナー情報テーブルCPU INDEX   */
    short  proc_handle[10];           /* 自プロセスハンドル                */
    short  node_name_len;             /* 自ノード名長                      */
    
    /* 監視対象ノードの設定 */
    MONITORNET(1);              /* 他ノード状態監視 */
    /* 監視対象CPUの設定 */
    MONITORCPUS(0xffff);        /* 全CPU監視        */
    /* 親プロセス・ハンドルの取得 */
    PROCESSHANDLE_NULLIT_(proc_handle);
    PROCESS_GETPAIRINFO_(proc_handle            /*  processhandle   */
                                   ,                    /*  pair            */
                                ,                    /*  pair maxlen     */
                                ,                    /*  pair length     */
                                ,                    /*  pri proc handle */
                                ,                    /*  bak proc handle */
                                ,                    /*  search inde     */
                                ,open_info->ancst_handle); /*  ancst handle    */
    /* オープナー情報テーブルの初期化 */
    for(idx_n=0; idx_n < DEF_COM_STP_NODE_NUM; idx_n++){
        if(idx_n == 0){
            NODENUMBER_TO_NODENAME_(-1
                                   ,open_info->openers[0].node_name
                                   ,sizeof(open_info->openers[0].node_name)
                                   ,&node_name_len);
        } else {
            memset(open_info->openers[idx_n].node_name,' ',sizeof(open_info->openers[idx_n].node_name));
        }
        open_info->openers[idx_n].node_open_count = 0;
        for(idx_c=0; idx_c < DEF_COM_STP_CPU_NUM; idx_c++) {
            open_info->openers[idx_n].cpu_open_count[idx_c] = 0;
        }
    }
    return;

} /* End of COM_STP_INIT */

/****************************************************************************/
/*  FUNCTION        : 1.2.0  COM_STP_JUDGE                                  */
/*  CALLING SEQ.    : short COM_STP_JUDGE(struct *, char *)                 */
/*  ARGUMENT        : 1.open_info            (I/O) オープナー情報           */
/*                  : 2.sys_msg              (I)   システムメッセージ       */
/*  RETURN CODE     : 0 :正常（停止不要)  1:正常(停止要)                    */
/*                  : -1:テーブル空き無し                                   */
/*  DESCRIPTION     : システムメッセージにもとに、オープナーカウンタの      */
/*                    算出を行う                                            */
/****************************************************************************/
short COM_STP_JUDGE(COM_STP_arg_1_def *open_info
                  , char *sys_msg )
{
    short  idx_n = 0;                 /* オープナー情報テーブルNODE INDEX  */
    short  idx_c = 0;                 /* オープナー情報テーブルCPU INDEX   */
    short  idx_free = -1;             /* オープナー情報テーブル空きINDEX   */
    short  ret;
    short  hit_flg = 0;               /* オープナー情報テーブルHITフラグ   */
    short  cpu_num;                   /* オープナープロセス稼働CPU         */
    char   node_name[8];              /* オープナープロセス稼働ノード名    */
    short  node_name_len;             /* オープナープロセス稼働ノード名長  */
    char   proc_name[6];              /* オープナープロセス名              */
    short  proc_name_len;             /* オープナープロセス名長            */
    short  all_open_count = 0;        /* 全オープン数                      */
    
    zsys_ddl_smsg_cpudown_def       *cpu_dwn_p
                              = (zsys_ddl_smsg_cpudown_def *)sys_msg;
    zsys_ddl_smsg_remotecpudown_def *rmt_cpu_dwn_p
                               = (zsys_ddl_smsg_remotecpudown_def *)sys_msg;
    zsys_ddl_smsg_nodedown_def      *node_dwn_p
                              = (zsys_ddl_smsg_nodedown_def *)sys_msg;
    zsys_ddl_receiveinformation_def receive_info;
    
    /* オープナープロセス情報の取得 */
    /* システムメッセージにより処理を振分 */
    switch((cpu_dwn_p->z_msgnumber)) {
    case ZSYS_VAL_SMSG_OPEN:            /* オープンメッセージ or       */
    case ZSYS_VAL_SMSG_CLOSE:           /* クローズメッセージ          */
        /* オープナーのプロセスハンドル情報の取得 */
        FILE_GETRECEIVEINFO_((short *)&receive_info);
        /* オープナープロセスの稼働ノード,稼働CPU,プロセス名を取得 */
        PROCESSHANDLE_DECOMPOSE_((short *)&receive_info.z_sender
                                ,&cpu_num,,
                                ,node_name,sizeof(node_name),&node_name_len
                                ,proc_name,sizeof(proc_name),&proc_name_len );
        /* オープナープロセスの判定 */
        ret = PROCESSHANDLE_COMPARE_(open_info->ancst_handle, (short *)&receive_info.z_sender);
        if(ret == 0) {
            /* オープナープロセスがPATHMON以外の場合 */
            if(memcmp(proc_name,"$ZL",3) != 0){
                /*オープナープロセスがLINKMON以外の場合 */
                return(DEF_COM_STP_RTN_OK_NO_STOP);   /* 正常 サーバ停止処理不要 */
            }
        }
        break;
    case ZSYS_VAL_SMSG_CPUDOWN:         /* CPUダウンメッセージ         */
        /* ダウン対象CPU番号の取得 */
        cpu_num = cpu_dwn_p->z_cpunumber;
        memcpy(node_name,open_info->openers[0].node_name,sizeof(node_name));
        node_name_len = (short)sizeof(node_name);
        break;
    case ZSYS_VAL_SMSG_REMOTECPUDOWN:   /* リモートCPUダウンメッセージ */
        /* ダウン対象ノード名の取得 */
        NODENUMBER_TO_NODENAME_(rmt_cpu_dwn_p->z_nodenumber
                               ,node_name
                               ,sizeof(node_name)
                               ,&node_name_len);
        /* ダウン対象CPU番号の取得 */
        cpu_num = rmt_cpu_dwn_p->z_cpunumber;
        break;
    case ZSYS_VAL_SMSG_NODEDOWN:        /* ノードダウンメッセージ      */
        /* ダウン対象ノード名の取得 */
        NODENUMBER_TO_NODENAME_(node_dwn_p->z_nodenumber
                               ,node_name
                               ,sizeof(node_name)
                               ,&node_name_len);
        break;
    default:
        return(DEF_COM_STP_RTN_OK_NO_STOP);  /* 正常 サーバ停止処理不応 */
    } /* End of Switch */

    /* オープナー情報テーブルの検索 */
    while(hit_flg == DEF_COM_STP_TBL_NO_HIT && idx_n < DEF_COM_STP_NODE_NUM) {
        if(memcmp(open_info->openers[idx_n].node_name,node_name,node_name_len) == 0) {
            /* オープナー情報テーブルに同一ノードが存在する場合 */
            switch(cpu_dwn_p->z_msgnumber) {
            case ZSYS_VAL_SMSG_OPEN   :         /* オープンメッセージ          */
                open_info->openers[idx_n].node_open_count++;
                open_info->openers[idx_n].cpu_open_count[cpu_num]++;
                break;
            case ZSYS_VAL_SMSG_CLOSE  :         /* クローズメッセージ          */
                open_info->openers[idx_n].node_open_count--;
                open_info->openers[idx_n].cpu_open_count[cpu_num]--;
                break;
            case ZSYS_VAL_SMSG_CPUDOWN:         /* CPUダウンメッセージ         */
                if(open_info->openers[idx_n].cpu_open_count[cpu_num] != 0){
                    open_info->openers[idx_n].node_open_count = 
                        open_info->openers[idx_n].node_open_count - 
                            open_info->openers[idx_n].cpu_open_count[cpu_num];
                    open_info->openers[idx_n].cpu_open_count[cpu_num] = 0;
                }else{
                    return(DEF_COM_STP_RTN_OK_NO_STOP);
                }
                break;
            case ZSYS_VAL_SMSG_REMOTECPUDOWN:   /* リモートCPUダウンメッセージ */
                if(open_info->openers[idx_n].cpu_open_count[cpu_num] != 0){
                    open_info->openers[idx_n].node_open_count = 
                        open_info->openers[idx_n].node_open_count - 
                            open_info->openers[idx_n].cpu_open_count[cpu_num];
                    open_info->openers[idx_n].cpu_open_count[cpu_num] = 0;
                }else{
                    return(DEF_COM_STP_RTN_OK_NO_STOP);
                }
                break;
            case ZSYS_VAL_SMSG_NODEDOWN:        /* ノードダウンメッセージ      */
                open_info->openers[idx_n].node_open_count = 0;
                for(idx_c=0; idx_c < DEF_COM_STP_CPU_NUM; idx_c++) {
                    open_info->openers[idx_n].cpu_open_count[idx_c] = 0;
                }
                break;
            }
            hit_flg = DEF_COM_STP_TBL_HIT;
        }else {
            if(open_info->openers[idx_n].node_name[0] == ' ' && idx_free == -1) {
                /* オープナー情報テーブル空き有り、且つ空きINDEX未設定 */
                idx_free = idx_n;
            }
            idx_n++;
        }
    } /* End of While */
    if(cpu_dwn_p->z_msgnumber == ZSYS_VAL_SMSG_OPEN && 
                              hit_flg == DEF_COM_STP_TBL_NO_HIT){
        /* オープンメッセージ、且つオープナー情報テーブルに同一ノード名無し*/
        if(idx_free == -1){
            /* オープナー情報テーブル空き無しの場合 */
            return(DEF_COM_STP_RTN_NG_TBL_FULL);   /* オープナー情報テーブル空き無し */
        } else {
            /* オープナー情報テーブル空き有りの場合 */
            /* 空きレコードにノード名、open_coun(0)を設定 */
            memcpy(open_info->openers[idx_free].node_name,node_name,node_name_len);
            open_info->openers[idx_free].cpu_open_count[cpu_num] = 1;
            open_info->openers[idx_free].node_open_count         = 1;
        }
    }
    if(cpu_dwn_p->z_msgnumber != ZSYS_VAL_SMSG_OPEN && 
                              hit_flg == DEF_COM_STP_TBL_NO_HIT){
        /* オープンメッセージ以外、且つオープナー情報テーブルに同一ノード名無し*/
        return(DEF_COM_STP_RTN_OK_NO_STOP); /* 正常 停止無 */
    }
    /* オープン数の算出     */
    for(idx_n=0; idx_n < DEF_COM_STP_NODE_NUM; idx_n++){
        if(open_info->openers[idx_n].node_open_count > 0) {
            all_open_count = all_open_count + open_info->openers[idx_n].node_open_count;
        } else {
            /* ノード名のクリア（自ノード以外） */
            if(idx_n != 0){
                memset(open_info->openers[idx_n].node_name,' ',DEF_COM_STP_NODE_NAME_LEN);
            }
        }
    }
    /* オープン数の判定（サーバ停止有無 )  */
    if(all_open_count <= 0){
        return(DEF_COM_STP_RTN_OK_STOP);    /* 正常 停止有 */
    } else {
        return(DEF_COM_STP_RTN_OK_NO_STOP); /* 正常 停止無 */
    }

} /* End of COM_STP_JUDGE */
