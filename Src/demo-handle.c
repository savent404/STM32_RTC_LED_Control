#include <stdio.h>
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "cmd-fun.h"
#include "cmsis_os.h"

#define USR_NAME "Savent"

/* Usr include */
#include "usart.h"

#ifndef USR_CLI_INPUT_SIZE
#define USR_CLI_INPUT_SIZE 64
#endif

#ifndef configCOMMAND_INT_MAX_OUTPUT_SIZE
#error "Define configCOMMAND_INT_MAX_OUTPUT_SIZE and configAPPLICATION_PROVIDES_cOutputBuffer first"
#endif

char cInputBuffer[ USR_CLI_INPUT_SIZE ] = "";
char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ] = "";

/* Usr var */

/* Usr func declear */

/**
  * @Brief: Console Handle
  * @Note:  String transform based on fput, fget in <stdio.h>
  */
void StartDefaultTask(void const * argument)
{
    uint32_t cpInput = 0;
    /* Hardware Init first */
    MX_USART1_UART_Init();
	
    CMD_Init();
    printf("\fCLI Interface Init ok\n");
    for(;;)
    {
        
        printf("@%s>>", USR_NAME);
        while (cpInput < USR_CLI_INPUT_SIZE) {
            cInputBuffer[cpInput] = getchar();
            if (cInputBuffer[cpInput] == 0x0D || cInputBuffer[cpInput] == '\n' || cInputBuffer[cpInput] == 0) {
                cInputBuffer[cpInput] = 0;
                printf("@%s>>", USR_NAME);
                puts(cInputBuffer);
                break;
            } else if (cInputBuffer[cpInput] == 0x7F) {
                cpInput -= 2;
            } cpInput += 1;
        }
        cpInput = 0;
        while (FreeRTOS_CLIProcessCommand(cInputBuffer, cOutputBuffer, sizeof(cOutputBuffer))!= pdFALSE){
            if (cOutputBuffer[0]) {
                cOutputBuffer[configCOMMAND_INT_MAX_OUTPUT_SIZE - 1] = 0;
                puts(cOutputBuffer);
                cOutputBuffer[0] = 0;
            }
        }
        if (cOutputBuffer[0]) {
            cOutputBuffer[configCOMMAND_INT_MAX_OUTPUT_SIZE - 1] = 0;
            puts(cOutputBuffer);
            cOutputBuffer[0] = 0;
        }
    }
}

/* Usr func */
