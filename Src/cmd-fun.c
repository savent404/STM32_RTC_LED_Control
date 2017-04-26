#include "cmd-fun.h"

/* Usr Include */
#include <stdio.h>
#include <string.h>
#include "cpu_utils.h"
#include "cmsis_os.h"

/* CLI Command Structure define */
static const CLI_Command_Definition_t CLI_Definition_All[] = {
  {
    "hello",
    "\r\nhello:\r\n Hello World\r\n\r\n",
    CLI_HelloWorld,
    0
  },
	{
		"clear",
		"\r\nclear:\r\n clean the console\r\n\r\n",
		CLI_Clear,
		0
	},
  {
    "cpu",
    "\r\ncpu:\r\n show CPU usage\r\n\r\n",
    CLI_CPU,
    0
  },
  {
    "heap",
    "\r\nheap:\r\n show system OS heap size\r\n\r\n",
    CLI_Heap,
    0,
  },
  {
    "thread",
    "\r\nthread:\r\n list thread in system\r\n\r\n",
    CLI_ThreadList,
    0,
  },
  {
    "setDate",
    "\r\nsetDate:\r\n setDate day\r\n Set Date now\r\n\r\n",
    CLI_setDate,
    1,
  },
  {
    "getDate",
    "\r\ngetDate:\r\n getDate\r\n Get Date now\r\n\r\n",
    CLI_getDate,
    0,
  },
  {
    "getTime",
    "\r\ngetTime:\r\n getTime\r\n Get Time now\r\n\r\n",
    CLI_getTime,
    0,
  },
  {
    "setTime",
    "\r\nsetTime:\r\n setTime hou:min:sec\r\n\r\n",
    CLI_setTime,
    1,
  },
  {
    "led",
    "\r\nled:\r\n led on|off\r\n\r\n",
    CLI_ledSwtich,
    1,
  },
  {
    "cnt",
    "\r\ncnt:\r\n get Channel 1~4’s number\r\n\r\n",
    CLI_Plus,
    0,
  },
  {
    "start",
    "\r\nstart:\r\n Start handle\r\n\r\n",
    CLI_Start,
    0,
  },
  {
      "LED",
      "\r\nLED:\r\n Strike mode\r\n\r\n",
      CLI_Injected,
      0
  },
  {
      "ledT",
      "\r\nledT:\r\n Set led switch period(H)\r\n\r\n",
      CLI_LEDPeriod,
      1,
  },
  {
      "cntT",
      "\r\ncntT:\r\n Set Cnt period(M)\r\n\r\n",
      CLI_CNTPeriod,
      1,
  },
};
/* Public Function */
void CMD_Init(void) {
  size_t cnt = sizeof(CLI_Definition_All) / sizeof(CLI_Command_Definition_t);
  size_t i;
  for (i = 0; i < cnt; i++) {
    FreeRTOS_CLIRegisterCommand(&CLI_Definition_All[i]);
  }
}

/* Usr Command Function define */
static BaseType_t CLI_HelloWorld(char* pt, size_t size, const char *cmd) {
	sprintf(pt, "Hello World\r\n");
	return 0;
}

static BaseType_t CLI_Clear(char *pt, size_t size, const char *cmd) {
	sprintf(pt, "\f");
	return 0;
}

static BaseType_t CLI_CPU(char *pt, size_t size, const char *cmd) {
  sprintf(pt, "CPU usage:%d%%\r\n", osGetCPUUsage());
  return 0;
}

static BaseType_t CLI_Heap(char *pt, size_t size, const char *cmd) {
  size_t ever_free = xPortGetMinimumEverFreeHeapSize();
  size_t free = xPortGetFreeHeapSize();
  sprintf(pt, "System Never used heap::%dByte(%.2fKb)\tfree for now::%dByte(%.2fKb)\r\n",
          ever_free, (float)ever_free/1024, free, (float)free/1024);
  return 0;
}

static BaseType_t CLI_ThreadList(char *pt, size_t size, const char *cmd) {
	osThreadList((uint8_t*)pt);
  if (!*pt) {
    sprintf(pt, "Function is disabled"
    "\r\ntry set [configUSE_TRACE_FACILITY]&[configUSE_STATS_FORMATTING_FUNCTIONS] to 1\n");
  }
  return 0;
}
