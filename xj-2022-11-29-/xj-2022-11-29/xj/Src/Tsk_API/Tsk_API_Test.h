
#ifndef API_TEST_H
#define API_TEST_H
#include "Utility.h"
#include "BSP_Gpio.h"
//#include "BSP_RTC.h"
#include "BSP_Pwm.h"
#include "BSP_ADc.h"
#include "BSP_buzzer.h"
#include "BSP_WatchDog.h"

void Input_test(void);

void Output_test(void);

void test_help(void);
void In_put_test(void *p);
int testFunProc(char *src,void *p);

#endif
