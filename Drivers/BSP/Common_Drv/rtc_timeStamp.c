#include "rtc_timeStamp.h"
#include "stm32h7xx.h"
#include <stdio.h>

RTC_HandleTypeDef RtcHandle;

uint8_t aShowTime[50] = {0}, aShowTimeStamp[50] = {0};
uint8_t aShowDate[50] = {0}, aShowDateStamp[50] = {0};

static void RTC_TimeStampConfig(void)
{
    RTC_DateTypeDef sdatestructure;
    RTC_TimeTypeDef stimestructure;
    /*
    if(HAL_OK != HAL_RTCEx_SetTimeStamp_IT(&RtcHandle, RTC_TIMESTAMPEDGE_RISING, RTC_TIMESTAMPPIN_DEFAULT))
    {
      printf("set time stamp IT error\r\n");
    }
    */
    sdatestructure.Year    = 24;
    sdatestructure.Month   = 10;
    sdatestructure.Date    = 10;
    sdatestructure.WeekDay = 4;
    
    if(HAL_RTC_SetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BIN) != HAL_OK)
    {
      printf("rtc set date error\r\n");
    }
    
    stimestructure.Hours =  18;
    stimestructure.Minutes = 7;
    stimestructure.Seconds = 0;
    stimestructure.SubSeconds = 0;
    stimestructure.TimeFormat = RTC_HOURFORMAT_24;
    stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    stimestructure.StoreOperation =  RTC_STOREOPERATION_RESET;
    
    if(HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN) != HAL_OK)
    {
      printf("rtc set time error\r\n");
    }
}

static void RTC_AlarmConfig(void)
{
  RTC_AlarmTypeDef salarmstructure;
  salarmstructure.Alarm = RTC_ALARM_A;
  salarmstructure.AlarmDateWeekDay = 10;
  salarmstructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  salarmstructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;                //屏蔽日期、小时和分钟
  salarmstructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;      //屏蔽亚秒
  salarmstructure.AlarmTime.Hours = 18;
  salarmstructure.AlarmTime.Minutes = 7;
  salarmstructure.AlarmTime.Seconds = 30;
  salarmstructure.AlarmTime.SubSeconds = 0;
  
  
  if(HAL_RTC_SetAlarm_IT(&RtcHandle,&salarmstructure,RTC_FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
    printf("rtc set alarm it error\r\n");
  }
  
  if(HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle, 0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    printf("rtc set wakeup timer error\r\n");
  }
}

/**
  * @brief  Display the current time and date.
  * @param  showtime : pointer to buffer
  * @param  showdate : pointer to buffer
  * @retval None
  */
void RTC_CalendarShow(void)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);

  /* Display time Format : hh:mm:ss */
  sprintf((char*)aShowTime,"%.2d:%.2d:%.2d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char*)aShowDate,"%.2d-%.2d-%.2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
  
  printf("Time : %s\r\n", aShowTime);
  printf("Date : %s\r\n", aShowDate);
}

void rtcTimeStampInit(void)
{
  RtcHandle.Instance = RTC;
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut         = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
  
  if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    printf("rtc init failed\r\n");
  }
  RTC_TimeStampConfig();
  RTC_AlarmConfig();
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
    RCC_OscInitTypeDef        RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
    
    /*开启备份区域访问*/
    HAL_PWR_EnableBkUpAccess();
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      printf("rtc osc config error\r\n");
    }
    
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      printf("set rtc clock source failed\r\n");
    }
    
    /*开启RTC时钟*/
    __HAL_RCC_RTC_ENABLE();
    
    //HAL_NVIC_SetPriority(TAMP_STAMP_IRQn, 5, 0);
    //HAL_NVIC_EnableIRQ(TAMP_STAMP_IRQn);
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 1,  0);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
    
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 1,  0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
                         
}


void RTC_Alarm_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&RtcHandle);
  
}

void RTC_WKUP_IRQHandler(void)
{
  HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  if(HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN)  != HAL_OK)
  {
    printf("error from RTC_WKUP_IRQHandler, HAL_RTC_GetTime error\r\n");
  }
  if(HAL_RTC_GetDate(hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    printf("error from RTC_WKUP_IRQHandler, HAL_RTC_GetDate error\r\n");
  }
  char strDate[60];
  sprintf(strDate, "WakeUp event: RTC Date= %04d-%02d-%02d\r\n", 2000+sDate.Year, sDate.Month, sDate.Date);
  printf("%s\r\n", strDate);
  
  char strTime[60];
  sprintf(strTime, "Wakeup event: RTC Time = %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
  printf("%s\r\n", strTime);
  
}

static uint32_t triggerCntA = 0;

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  triggerCntA++;
  printf("Alarm event: cnt = %d\r\n", triggerCntA);
}
#if 0
/**
  * @brief  This function handles Tamper interrupt request.
  * @param  None
  * @retval None
  */
void TAMP_STAMP_IRQHandler(void)
{
  HAL_RTCEx_TamperTimeStampIRQHandler(&RtcHandle);
}

/**
  * @brief  Timestamp callback
  * @param  hrtc : hrtc handle
  * @retval None
  */
void HAL_RTCEx_TimeStampEventCallback(RTC_HandleTypeDef *hrtc)
{


  RTC_DateTypeDef sTimeStampDateget;
  RTC_TimeTypeDef sTimeStampget;

  HAL_RTCEx_GetTimeStamp(&RtcHandle, &sTimeStampget, &sTimeStampDateget, RTC_FORMAT_BIN);

  /* Display time Format : hh:mm:ss */
  sprintf((char*)aShowTimeStamp,"%.2d:%.2d:%.2d", sTimeStampget.Hours, sTimeStampget.Minutes, sTimeStampget.Seconds);
  /* Display date Format : mm-dd */
  sprintf((char*)aShowDateStamp,"%.2d-%.2d-%.2d", sTimeStampDateget.Month, sTimeStampDateget.Date, 2024);
  
  printf("ISR Time : %s\r\n", aShowTimeStamp);
  printf("ISR Date : %s\r\n", aShowDateStamp);
}

#endif
