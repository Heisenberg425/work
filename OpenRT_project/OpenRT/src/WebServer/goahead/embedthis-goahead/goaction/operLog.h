#ifndef __OPERLOG_H_
#define __OPERLOG_H_

#include "login_info.h"
/*********************************************************************************************************
** ????: operLogInsert
** ????: ????????
** ? ?  : wp                 ?????
**           iModuleIndex       ?????? @enum OPEN_RT_MODULE_INDEX
**           pLogContent        ??????
**           iResult            ???? 0????,?0 ????
** ? ?  : NONE
** ? ?  : NONE
*********************************************************************************************************/
void operLogInsert (Webs  *wp, INT  iModuleIndex, char  *pLogContent, INT  iResult);

#endif /* __OPERLOG_H_ */
