#ifndef BLT_H
#define BLT_H
#include"main.h"




typedef unsigned short     int uint16;
typedef unsigned          char uint8;
typedef unsigned           int uint32;

typedef enum
{
	STANDBY_MODE,
	GUN_INSERTION_AND_UNAUTHORIZED,
	GUN_INSERTION_AND_AUTHORIZED,
	GUN_INSERTION_AND_CHARGING_START,
	NON_SHUTDOWN_FAULT,
	SHUTDOWN_FAULT,
	BOOT,
}Lamp_signal;



#endif