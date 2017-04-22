#include "cmd-fun.h"
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "cmsis_os.h"
#include "freertos.h"

static int32_t LED_Period_Hour = 12;
static int32_t CNT_Period_Min = 1;
static osTimerId timer_id;
static osTimerId cnt_timer_id;
static void Timer_Callback(const void *arg);
static void Cnt_Timer_Callback(const void *arg);
osTimerDef(cnt_handle, Cnt_Timer_Callback);
osTimerDef(handle, Timer_Callback);

struct Plus_Structure {
    uint8_t date;
    uint8_t hour;
    uint8_t min;
    uint32_t ch[4];
    struct Plus_Structure *next;
};
static struct Plus_Structure root = {
    0, 0, 0,
    0, 0, 0, 0,
    NULL
};

static uint32_t Plus_cnt[4] = {0, 0, 0, 0};

static void led_on(void) {
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}

static void led_off(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
    HAL_TIM_Base_Stop(&htim1);
}

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
        led_on();
        sprintf(pt, "All LED ON");
    } else if (!strcmp(buf, "off")) {
        led_off();
        sprintf(pt, "ALL LED OFF");
    } else {
        sprintf(pt, "no args matched");
    } return 0;
}

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

BaseType_t CLI_Plus(char *spt, size_t size, const char *cmd) {
    struct Plus_Structure *pt = root.next;
    uint32_t cnt = 1;
    while (pt) {
        printf("[%d]\t%01d@%02d:%02d\t%d\t%d\t%d\r\n",
            cnt++, pt->date, pt->hour, pt->min,
            pt->ch[0], pt->ch[1], pt->ch[3]);
        pt = pt->next;
    } return 0;
}

BaseType_t CLI_LEDPeriod(char *pt, size_t size, const char *cmd) {
    int n;
    sscanf(cmd, "%*s %d", &n);
    if (n > 0) {
        sprintf(pt, "Set LED Period OK:%d", n);
        LED_Period_Hour = n;
    } else {
        sprintf(pt, "Period can't be %d", n);
    } return 0;
}

BaseType_t CLI_CNTPeriod(char *pt, size_t size, const char *cmd) {
    int n;
    sscanf(cmd, "%*s %d", &n);
    if (n > 0) {
        sprintf(pt, "Set CNT Period OK:%d", n);
        CNT_Period_Min = n;
    } else {
        sprintf(pt, "Period cnt't be %d", n);
    } return 0;
}

BaseType_t CLI_Start(char *pt, size_t size, const char *cmd) {
    static uint8_t init_flag = 0;
    if (!init_flag) {
        timer_id = osTimerCreate(osTimer(handle), osTimerPeriodic, &root);
        cnt_timer_id = osTimerCreate(osTimer(cnt_handle), osTimerPeriodic, NULL);
        init_flag = 1;
    }
    osTimerStart(timer_id, LED_Period_Hour*60*60*500);
    osTimerStart(cnt_timer_id, CNT_Period_Min*60*1000);
    sprintf(pt, "Task Start ok, reset counter");
    Plus_cnt[0] = 0;
    Plus_cnt[1] = 0;
    Plus_cnt[2] = 0;
    Plus_cnt[3] = 0;
    led_on();

    return 0;
}

static void Cnt_Timer_Callback(const void *arg) {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    struct Plus_Structure *pt = (struct Plus_Structure*)pvPortMalloc(sizeof(struct Plus_Structure));
    pt->next = NULL;
    struct Plus_Structure *ppt = &root;
    while (ppt->next) {
        ppt = ppt->next;
    } ppt->next = pt;
    pt->date = date.Date;
    pt->hour = time.Hours;
    pt->min = time.Minutes;
    pt->ch[0] = Plus_cnt[0];
    pt->ch[1] = Plus_cnt[1];
    pt->ch[2] = Plus_cnt[2];
    pt->ch[3] = Plus_cnt[3];
    
}
static void Timer_Callback(const void *arg) {
    static uint8_t buf = 1;
    if (buf) {
        buf = 0;
        led_off();
    } else {
        buf = 1;
        led_on();
    }
}
