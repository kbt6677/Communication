/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････ GFP                                         */
/*        SUB-SYSTEM        ････ 通信制御                                    */
/*        PROGRAM-ID        ････ GFPCGXG0                                    */
/*        FUNCTION          ････ プロセス情報取得                            */
/*                                                                           */
/*        AUTHER            ････ ISYS Y.Kawasaki                             */
/*        PROGRAM-CALL      ････                                             */
/*        WRITTEN-DATE      ････ 2024-11-17                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*   版  修正者     修正日     修正内容                                      */
/*  ==== ========== ========== ============================================= */
/*  1.0  Y.Kawasaki 2024/11/27 (J0680)新規作成                               */
/*                                                                           */
/*****************************************************************************/
/* ################### */
/* # INCLUDE         # */
/* ################### */
/* STANDARD HEADER */
#include <stdio.h>    nolist
#include <stdlib.h>   nolist
#include <string.h>   nolist
#include <tal.h>      nolist
#include <cextdecs.h> nolist
#include <zspic>      nolist
#include <zfilc>      nolist
/* USER HEADER */
#include "GFPCGXG0.h" nolist
#include "vproc.h"    nolist      // vproc

/****************************************************************************/
/*  FUNCTION        : 0.0.1  COM_PRC                                        */
/*  CALLING SEQ.    : void  COM_PRC(procinfo_def* procinfo)                 */
/*  ARGUMENT        : 1. procinfo   (I/O) プロセス情報                      */
/*  RETURN CODE     : Guardianプロシージャ・コールの戻り値                  */
/*  DESCRIPTION     : 自・親プロセスハンドル、プロセス名を取得する          */
/****************************************************************************/
short COM_PRC(procinfo_def* procinfo)
{
    short ret;
    short nodename_len;

    // 初期化
    memset(procinfo, NULL, sizeof(procinfo_def));
    PROCESSHANDLE_NULLIT_(procinfo->my_phandle);
    PROCESSHANDLE_NULLIT_(procinfo->ans_phandle);
    // 自プロセス名取得
    ret = PROCESSHANDLE_GETMINE_(procinfo->my_phandle);
    if (ret) {
        memcpy(procinfo->err_pname, "PROCESSHANDLE_GETMINE_",
               sizeof("PROCESSHANDLE_GETMINE_")-1);
        return ret;
    }
    ret = PROCESSHANDLE_DECOMPOSE_(procinfo->my_phandle,
                                   ,
                                   ,
                                   ,
                                   procinfo->my_nodename,
                                   (short)sizeof(procinfo->my_nodename),
                                   &procinfo->my_nodename_len,
                                   procinfo->my_pname,
                                   (short)sizeof(procinfo->my_pname),
                                   &procinfo->my_pname_len);
    if (ret) {
        memcpy(procinfo->err_pname, "PROCESSHANDLE_DECOMPOSE_",
               sizeof("PROCESSHANDLE_DECOMPOSE_")-1);
        return ret;
    }

    // 親プロセス名取得
    ret = PROCESS_GETPAIRINFO_(procinfo->my_phandle,
                                 ,
                                 ,
                                 ,
                                 ,
                                 ,
                                 ,
                                procinfo->ans_phandle);
    // 4 シングルの名前付きプロセス ( コールしたプロセスである可能性もあります ) に関する情報が返された。
    // 5 コールしたプロセスが現在プライマリとなっているプロセス・ペアの情報が返された。
    if (ret != 4 && ret != 5) {
        memcpy(procinfo->err_pname, "PROCESS_GETPAIRINFO_",
               sizeof("PROCESS_GETPAIRINFO_")-1);
        return ret;
    }
    ret = PROCESSHANDLE_DECOMPOSE_(procinfo->ans_phandle,
                                   ,
                                   ,
                                   ,
                                   ,
                                   ,
                                   ,
                                   procinfo->ans_pname,
                                   (short)sizeof(procinfo->ans_pname),
                                   &procinfo->ans_pname_len);
    if (ret) {
        memcpy(procinfo->err_pname, "PROCESSHANDLE_DECOMPOSE_",
               sizeof("PROCESSHANDLE_DECOMPOSE_")-1);
    }

    return ret;
/*
PROCESSHANDLE_GETMINE_
0 情報は、正常に返されました。
3 パラメータ・アドレスが境界外
PROCESSHANDLE_DECOMPOSE_
操作の結果を示す、ファイル・システム・エラー番号です。
PROCESS_GETPAIRINFO_
0 プロセス・ペア ( コールするプロセスではありません ) の情報が返されました。
2 パラメータ・エラー。error-detail には、エラーが見つかった最初のパラメータの番号が含ま
れます。この数が 1 であれば、一番左のパラメータを指します。
3 境界エラー。error-detail には、エラーが見つかった最初のパラメータの番号が含まれます。
この数が 1 であれば、一番左のパラメータを指します。
4 シングルの名前付きプロセス ( コールしたプロセスである可能性もあります ) に関する情報が返
された。
5 コールしたプロセスが現在プライマリとなっているプロセス・ペアの情報が返された。
6 コールしたプロセスが現在バックアップとなっているプロセス・ペアの情報が返された。
7 情報は返されていない。プロセスには名前がありません ( コールしたプロセスである可能性もあ
ります )。
8 情報は返されていない。検索は完了しました。
9 指定されたプロセスは存在しない。
10 プロセスが置かれているノードとの通信ができない。
11 プロセスは I/O プロセスであるが、I/O プロセスを許可するオプションが選択されなかった。
13 その名前が予約名である、開始していない名前付きプロセスに関する限られた情報が返された。
*/
}

