/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "bsp_ili9341_lcd.h"
#include "bsp_xpt2046_lcd.h"
#include "palette.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "UI.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
char data_rx[10];


LED_TypeDef LED_B={mode_off,"00"};
LED_TypeDef* LED_P=&LED_B;
TaskHandle_t LED_TaskHandle;
TaskHandle_t lvgl_TaskHandle;
SemaphoreHandle_t MutexSemaphore;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void lvgl_handler( void *pvParameters );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LED_TaskFunc(void* param)
{
  static int pwm=0;
  while(1)
  {
    LED_TypeDef* led_p=(LED_TypeDef*)(param);
    int Mode=led_p->Mode;
    if(Mode==mode_off)
    {
      __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,0);
    }
    else if(Mode==mode_shine)
    {
      __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,100);
      vTaskDelay(1000);
      __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,0);
      vTaskDelay(500);
    }
    else if(Mode==mode_pwm)
    {
      while(pwm++<100)
      {
         __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,pwm);
         vTaskDelay(10);
      }
      while(pwm--)
      {
        __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,pwm);
         vTaskDelay(5);
      }
    }
    else if(Mode==mode_hand)
    {
      int light=(int)(led_p->light[0]-'0')*10+(int)(led_p->light[0]-'0');
      
      __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,light);
      

    }
    else if(Mode==mode_on)
    {
      
      
      __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_4,100);
      

    }
    vTaskDelay(20);
  }

  
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  
  /* USER CODE BEGIN 1 */
  MutexSemaphore=xSemaphoreCreateMutex(); 
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */
  //LCD初始�?????
  ILI9341_Init();
  ILI9341_GramScan(6);
  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);
  //ILI9341_DispString_EN(0,0,"Hello World!");
  XPT2046_Init();

  //LVGL初始�?????
  lv_init();          	//lv 系统初始�?????
  lv_port_disp_init();    //lvgl 显示接口初始化，放在lv_init后面
	lv_port_indev_init();   //lvgl 输入接口初始化，放在 lv_init后面
	

  my_UIInit();

  //PWM和串口初始化
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
  HAL_UART_Receive_IT(&huart1,LED_P->light,2);


  //新建FreeRTOS任务
  xTaskCreate(LED_TaskFunc,"LED",100,LED_P,6,&LED_TaskHandle);
  xTaskCreate(lvgl_handler,"lvgl",1500,NULL,6,NULL);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  
  
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void lvgl_handler( void *pvParameters )
 {
   for( ;; )
   { 
   // HAL_UART_Transmit_IT(&huart1,"task",5);
     xSemaphoreTake(MutexSemaphore,portMAX_DELAY);  
     lv_task_handler();
     xSemaphoreGive(MutexSemaphore); 
     vTaskDelay(pdMS_TO_TICKS(20));
   }
 }
void vApplicationTickHook(void)
{
   
   lv_tick_inc(1);
   //lv_task_handler();
   //HAL_UART_Transmit(&huart1,"task",5,500);
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
