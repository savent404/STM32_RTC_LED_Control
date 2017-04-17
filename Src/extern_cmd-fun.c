#include "cmd-fun.h"
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "cmsis_os.h"
#include "freertos.h"


BaseType_t CLI_getDate(char *pt, size_t size, const char *cmd) {
    RTC_DateTypeDef date;
    if (HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
        sprintf(pt, "Get RTC date Erorr");
    } else {
        sprintf(pt, "Date:%02d", date.Date);
    } return 0;
}

BaseType_t CLI_setDate(char *pt, size_t size, const char *cmd) {
    RTC_DateTypeDef date;
    int _date;
    sscanf(cmd, "%*s %d", &_date);
    date.Date = _date;
    date.Year = 0;
    date.WeekDay = RTC_WEEKDAY_MONDAY;
    date.Month = RTC_MONTH_JANUARY;
    if (_date <= 1 || _date > 31 || HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
        sprintf(pt, "Set RTC date Error");
    } else {
        sprintf(pt, "Set RTC date:%d", _date);
    } return 0;
}

BaseType_t CLI_getTime(char *pt, size_t size, const char *cmd) {
    RTC_TimeTypeDef time;
    if (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) {
        sprintf(pt, "Get RTC time Error");
    } else {
        sprintf(pt, "RTC time :%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
    } return 0;
}

BaseType_t CLI_setTime(char *pt, size_t size, const char *cmd) {
    RTC_TimeTypeDef time;
    int hours, min, sec;
    sscanf(cmd, "%*s %d:%d:%d", &hours, &min, &sec);
    time.Hours = hours;
    time.Minutes = min;
    time.Seconds = sec;
    if (hours < 0 || hours > 23 || min < 0 || min > 59 || sec < 0 || sec > 59 ||
        HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) {
        sprintf(pt, "Set RTC time Error");
    } else {
        sprintf(pt, "RTC time now:%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
    } return 0;
}

BaseType_t CLI_ledSwtich(char *pt, size_t size, const char *cmd) {
    char buf[5];
    sscanf(cmd, "%*s %s", buf);
    if (!strcmp(buf, "on")) {
        HAL_TIM_Base_Start(&htim1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
        sprintf(pt, "All LED ON");
    } else if (!strcmp(buf, "off")) {
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
        HAL_TIM_Base_Stop(&htim1);
        sprintf(pt, "ALL LED OFF");
    } else {
        sprintf(pt, "no args matched");
    } return 0;
}


typedef struct _sechlist {
    uint8_t date;
    uint8_t hour;
    uint8_t min;
    uint8_t option;
    struct _sechlist *next;
} SechList_Typedef;

static SechList_Typedef root = {
    0,0,0,0,NULL
};

static int sech_add(SechList_Typedef* root, uint8_t option, char *time) {
    SechList_Typedef *pt = root;
    SechList_Typedef *npt = NULL;
    int date, hour, min;
    //malloc
    npt = (SechList_Typedef*)malloc(sizeof(SechList_Typedef));
    if (npt == NULL) {
        return -1;
    } npt->next = NULL;
    // offset
    while (pt->next) {
        pt = pt->next;
    } pt->next = npt;
    // scanf
    sscanf(time, "%d@%d:%d", &date, &hour, &min);
    npt->date = date;
    npt->hour = hour;
    npt->min = min;
    npt->option = option;
    if (npt->hour > 23 || npt->min > 59) return -2;
    return 0;
}
static SechList_Typedef* sech_pos(SechList_Typedef* root, int offset) {
    SechList_Typedef* pt = root;
    while (offset-- && pt->next) {
        pt = pt->next;
    } return pt;
}
static int sech_sum(SechList_Typedef* root) {
    SechList_Typedef* pt = root;
    int cnt = 0;
    while (pt->next) {
        cnt += 1;
        pt = pt->next;
    } return cnt;
}
BaseType_t CLI_SechList(char *spt, size_t size, const char *cmd) {
    SechList_Typedef *pt = NULL;
    uint8_t cnt = sech_sum(&root);
    uint8_t pos = 0;
    if (cnt == 0) {
        sprintf(spt, "There has no Sech list");
        return 0;
    }
    sprintf(spt, "LED Turn-on:1/Turn-off:0\r\n");
    for (pos = 1; pos <= cnt; pos++) {
        pt = sech_pos(&root, pos);
        sprintf(spt + strlen(spt), "[%02d] %02d@%02d:%02d %01d\r\n",
        pos, pt->date, pt->hour, pt->min, pt->option);
    } return 0;
}

BaseType_t CLI_SechAdd(char *pt, size_t size, const char *cmd) {
    char opt[5];
    char time[20];
    uint8_t nopt = 0;
    sscanf(cmd, "%*s %s %s", opt, time);
    if (!strcmp(opt, "on")) {
        nopt = 1;
    } else if (!strcmp(opt, "off")){
        nopt = 0;
    } else {
        sprintf(pt, "no args matched");
        return 0;
    }
    switch(sech_add(&root, nopt, time)) {
        case -1: sprintf(pt, "Malloc error"); return 0;
        case -2: sprintf(pt, "Date format error"); return 0;
        case 0:  sprintf(pt, "Add ok"); return 0;
        default: return 0;
    }
}


static uint32_t Plus_cnt[4] = {0,0,0,0};
/**
  * @brief: GPIO中断Callback函数
  * @param: 中断引脚
  * @retval:NONE
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint32_t before[4] = {0,0,0,0};
    switch (GPIO_Pin) {
        case CH1_Pin: 
            if (HAL_GPIO_ReadPin(CH1_GPIO_Port, CH1_Pin) && before[0] < HAL_GetTick() + 600) {
                Plus_cnt[0] += 1;
                before[0] = HAL_GetTick();
            } break;
        case CH2_Pin: 
            if (HAL_GPIO_ReadPin(CH2_GPIO_Port, CH2_Pin) && before[1] < HAL_GetTick() + 600) {
                Plus_cnt[1] += 1;
                before[1] = HAL_GetTick();
            } break;
        case CH3_Pin: 
            if (HAL_GPIO_ReadPin(CH3_GPIO_Port, CH3_Pin) && before[2] < HAL_GetTick() + 600) {
                Plus_cnt[2] += 1;
                before[2] = HAL_GetTick();
            } break;
        case CH4_Pin:
            if (HAL_GPIO_ReadPin(CH4_GPIO_Port, CH4_Pin) && before[3] < HAL_GetTick() + 600) {
                Plus_cnt[3] += 1;
                before[3] = HAL_GetTick();
            } break;
    }
}

BaseType_t CLI_Plus(char *pt, size_t size, const char *cmd) {
    sprintf(pt, "Channel 1~4 cnt:\r\n"
                "CH1:%d\r\n"
                "CH2:%d\r\n"
                "CH3:%d\r\n"
                "CH4:%d", Plus_cnt[0],
                Plus_cnt[1],Plus_cnt[2],Plus_cnt[3]);
    return 0;
}
static osTimerId timer_id;
static void Timer_Callback(const void *arg);
osTimerDef(handle, Timer_Callback);
BaseType_t CLI_Start(char *pt, size_t size, const char *cmd) {
    static uint8_t init_flag = 0;
    if (!init_flag) {
        timer_id = osTimerCreate(osTimer(handle), osTimerPeriodic, &root);
        init_flag = 1;
    }

    osTimerStart(timer_id, 3000);
    sprintf(pt, "Task Start ok, reset counter");
    Plus_cnt[0] = 0;
    Plus_cnt[1] = 0;
    Plus_cnt[2] = 0;
    Plus_cnt[3] = 0;
    return 0;
}
static void Timer_Callback(const void *arg) {
    static SechList_Typedef *pt = (SechList_Typedef*)&root;
    static __IO uint32_t des,now;
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    
    printf("\033[s\033[100D");
    {
        uint8_t cnt = 20;
        while (cnt--) {
            printf("\033[1B\033[K");
        }
    }
    printf("DATE:%02d@%02d:%02d:%02d\r\nCH1:%5ud\r\n%CH2:%5ud\r\n%CH3:%5ud\r\n%CH4:%5ud",
        date.Date, time.Hours, time.Minutes, time.Seconds,
        Plus_cnt[0], Plus_cnt[1], Plus_cnt[2], Plus_cnt[3]
    );
    printf("\033[u");
    if (pt) {
        des = pt->date << 15 | pt->hour << 10 | pt->min << 5;
        now = date.Date << 15 | time.Hours << 10 | time.Minutes << 5 | time.Seconds;
        if (des <= now) {
            if (pt->option) {
                HAL_TIM_Base_Start(&htim1);
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
            } else {
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
                HAL_TIM_Base_Stop(&htim1);
            }
            if (pt->next == NULL) {
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
                HAL_TIM_Base_Stop(&htim1);
            } pt = pt->next;
        }
    }
}
