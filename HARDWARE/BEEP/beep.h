#ifndef _BEEP_H
#define _BEEP_H
#include "sys.h"

#define BEEP PFout(8) //蜂鸣器控制IO
void BEEP_Init(void); //蜂鸣器初始化函数
#endif
