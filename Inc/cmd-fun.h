#ifndef _CMD_FUN_H_
#define _CMD_FUN_H_

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include <stdint.h>
/* Public Function */
extern void CMD_Init(void);

/* Static CMD functions */
static BaseType_t CLI_HelloWorld(char* pt, size_t size, const char *cmd);
static BaseType_t CLI_Clear(char *pt, size_t size, const char *cmd);
static BaseType_t CLI_CPU(char *pt, size_t size, const char *cmd);
static BaseType_t CLI_Heap(char *pt, size_t size, const char *cmd);
static BaseType_t CLI_ThreadList(char *pt, size_t size, const char *cmd);

/* Extern CMD function */
extern BaseType_t CLI_getDate(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_setDate(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_getTime(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_setTime(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_ledSwitch(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_SechList(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_SechAdd(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_ledSwtich(char *pt, size_t size, const char *cmd);
extern BaseType_t CLI_Plus(char *pt, size_t size, const char *cmd);
#endif

