/*****************************************************************************/
/*****                    <<     SOURCE PROGRAM      >>                  *****/
/*****************************************************************************/
/*                                                                           */
/*        SYSTEM            ････                                             */
/*        SUB-SYSTEM        ････                                             */
/*        PROGRAM-ID        ････                                             */
/*        FUNCTION          ････                                             */
/*                                                                           */
/*        AUTHER            ････                                             */
/*        PROGRAM-CALL      ････                                             */
/*                                                                           */
/*        WRITTEN-DATE      ････ yyyy-mm-dd                                  */
/*                                                                           */
/*        UPDATE            ････                                             */
/*                                                                           */
/*  版   修正者      修正日     修正内容                                     */
/*  ==== ========== ========== ============================================= */
/*  1.0  xxxxxxxx   YYYY/MM/DD (xxxxx)新規作成                               */
#ifndef COM_LIMIT
#define COM_LIMIT

/*ｺﾈｸｼｮﾝ制御*/
/*ﾘｽﾅｰ*/
#define LSN_MAX_ports 200 /*受信ﾎﾟｰﾄ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define LSN_MAX_connection 30  /*ｺﾈｸｼｮﾝ制御(ｻｰﾊﾞ)ﾌﾟﾛｾｽ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define LSN_MAX_interface 80 /*ｲﾝﾀｰﾌｪｰｽorｽﾃｰｼｮﾝ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/

/*ｺﾈｸｼｮﾝ制御(ｻｰﾊﾞ)*/
#define CCS_MAX_connection 200 /*ｺﾈｸｼｮﾝ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCS_MAX_listners 30  /*ﾘｽﾅｰﾌﾟﾛｾｽ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCS_MAX_interface 80  /*ｲﾝﾀｰﾌｪｰｽorｽﾃｰｼｮﾝ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCS_MAX_outbound 100 /*電文振分(outbound)ﾌﾟﾛｾｽ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCS_MAX_inbound 50  /*電文振分(inbound)同時PATHSEND数MAX*/
#define CCS_MAX_recvqueue 50  /*受信ｷｭｰ数MAX*/
#define CCS_MAX_sendqueue 50  /*送信ｷｭｰ数MAX*/

/*ｺﾈｸｼｮﾝ制御(ｸﾗｲｱﾝﾄ)*/
#define CCC_MAX_connection 200 /*ｺﾈｸｼｮﾝ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCC_MAX_interface 80  /*ｲﾝﾀｰﾌｪｰｽorｽﾃｰｼｮﾝ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCC_MAX_outbound 100 /*電文振分(outbound)ﾌﾟﾛｾｽ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/
#define CCC_MAX_inbound 50  /*電文振分(inbound)同時PATHSEND数MAX*/
#define CCC_MAX_recvqueue 50  /*受信ｷｭｰ数MAX*/
#define CCC_MAX_sendqueue 50  /*送信ｷｭｰ数MAX*/

/*電文振分*/
/*電文振分(inbound)*/
#define IB_MAX_inbound  20  /*電文中継(inbound)管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/

/*電文振分(outbound)*/
#define OB_MAX_connection  60  /*ｺﾈｸｼｮﾝ制御ﾌﾟﾛｾｽ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/

/*電文中継*/
/*電文中継(PUT)*/
#define PUT_MAX_table 30  /*ｷｭｰﾌｧｲﾙ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/

/*電文中継(GET)*/
#define GET_MAX_tabe 30  /*ｷｭｰﾌｧｲﾙ管理ﾃｰﾌﾞﾙ ｴﾚﾒﾝﾄ数*/

/*制御電文処理*/
/*制御電文振分*/
/*局状態制御*/
/*ｴｺｰ制御*/
/*鍵交換制御*/
/*ｶｯﾄｵｰﾊﾞｰ制御*/
/*SAF送信制御*/
/*通知電文制御*/

/*共通処理*/
/*ｺﾏﾝﾄﾞｻｰﾊﾞ*/
/*ﾛｸﾞ出力*/
/*ﾀｲﾏｰ制御*/
/*LCN採番*/
#define LCN_MIN_count 100  /*連番採番範囲MIN*/
#define LCN_MAX_count 8191 /*連番採番範囲MAX*/

/*ATALLA振分*/
/*運用監視端末出力*/

#define MAX_TEXT_BUF_LEN 9999   /*ﾈｯﾄﾜｰｸ電文の最大長*/
#define MAX_QUEUE_BUF_LEN 10400 /*ｷｭｰﾌｧｲﾙの最大長*/

#endif /*COM_LIMIT*/
