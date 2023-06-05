/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */ //here
#include <ros.h>
#include <std_msgs/String.h>
#include <string>
#include <cstring>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */ //here
#define GRIPPER_OPENED_ANG 0
#define GRIPPER_CLOSED_ANG 78 // calculate for pwm 170
#define ART_DOWN_ANG 0
#define ART_UP_ANG 90
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* Definitions for TaskADC */
osThreadId_t TaskADCHandle;
const osThreadAttr_t TaskADC_attributes = {
  .name = "TaskADC",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GripperOpenTask */
osThreadId_t GripperOpenTaskHandle;
const osThreadAttr_t GripperOpenTask_attributes = {
  .name = "GripperOpenTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};
/* Definitions for GripperClose */
osThreadId_t GripperCloseHandle;
const osThreadAttr_t GripperClose_attributes = {
  .name = "GripperClose",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for GripperGoToTask */
osThreadId_t GripperGoToTaskHandle;
const osThreadAttr_t GripperGoToTask_attributes = {
  .name = "GripperGoToTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};
/* Definitions for TaskRosserial */
osThreadId_t TaskRosserialHandle;
const osThreadAttr_t TaskRosserial_attributes = {
  .name = "TaskRosserial",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for ButtonTask */
osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
  .name = "ButtonTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ArtUpTask */
osThreadId_t ArtUpTaskHandle;
const osThreadAttr_t ArtUpTask_attributes = {
  .name = "ArtUpTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal4,
};
/* Definitions for ArtDownTask */
osThreadId_t ArtDownTaskHandle;
const osThreadAttr_t ArtDownTask_attributes = {
  .name = "ArtDownTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal5,
};
/* Definitions for ArtGoToTask */
osThreadId_t ArtGoToTaskHandle;
const osThreadAttr_t ArtGoToTask_attributes = {
  .name = "ArtGoToTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal6,
};
/* USER CODE BEGIN PV */ // here
int adc1, adc2;
float ang[2];
volatile uint16_t adc_result[2];
const int adc_channel_count = sizeof(adc_result) / sizeof(adc_result[0]);
int gripper_go_to;
int art_go_to;
int gripper, art;
// Flags
int gripper_instruction_flag = 0;
int art_instruction_flag = 0;
bool gripper_ang_response = false;
bool art_ang_response = false;
bool button_pressed = false;
bool emergency_stop = false;


// Ros variables
ros::NodeHandle nh;

void str_act_msg(const std_msgs::String& msg);

std_msgs::String str_msg;
ros::Publisher chatter("chatter", &str_msg);
ros::Subscriber<std_msgs::String> stm32_comms("gripper_action", &str_act_msg);
std::string hello = "STM32 to Jetson!";
std::string global_msg = "";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void StartTaskADC(void *argument);
void StartGripperOpenTask(void *argument);
void StartGripperCloseTask(void *argument);
void StartGripperGoToTask(void *argument);
void StartTaskRosserial(void *argument);
void StartButtonTask(void *argument);
void StartArtUpTask(void *argument);
void StartArtDownTask(void *argument);
void StartArtGoToTask(void *argument);

/* USER CODE BEGIN PFP */
void setup(void);
void loop(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  setup();
  TIM2->CCR1 = 60;
  TIM2->CCR2 = 60;
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskADC */
  TaskADCHandle = osThreadNew(StartTaskADC, NULL, &TaskADC_attributes);

  /* creation of GripperOpenTask */
  GripperOpenTaskHandle = osThreadNew(StartGripperOpenTask, NULL, &GripperOpenTask_attributes);

  /* creation of GripperClose */
  GripperCloseHandle = osThreadNew(StartGripperCloseTask, NULL, &GripperClose_attributes);

  /* creation of GripperGoToTask */
  GripperGoToTaskHandle = osThreadNew(StartGripperGoToTask, NULL, &GripperGoToTask_attributes);

  /* creation of TaskRosserial */
  TaskRosserialHandle = osThreadNew(StartTaskRosserial, NULL, &TaskRosserial_attributes);

  /* creation of ButtonTask */
  ButtonTaskHandle = osThreadNew(StartButtonTask, NULL, &ButtonTask_attributes);

  /* creation of ArtUpTask */
  ArtUpTaskHandle = osThreadNew(StartArtUpTask, NULL, &ArtUpTask_attributes);

  /* creation of ArtDownTask */
  ArtDownTaskHandle = osThreadNew(StartArtDownTask, NULL, &ArtDownTask_attributes);

  /* creation of ArtGoToTask */
  ArtGoToTaskHandle = osThreadNew(StartArtGoToTask, NULL, &ArtGoToTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 64;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2499;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Push_Button_Pin */
  GPIO_InitStruct.Pin = Push_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Push_Button_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */ //here
void get_servo_pos_liveExpression(){
		gripper =  TIM2->CCR1;
		art = TIM2->CCR2;
}
float pwm_to_ang(int n){
	float out = 0;
	if (n == 0){
		out = (180.0/240.0)*((float)TIM2->CCR1 - 60);
	}
	if(n == 1){
		out = (180.0/240.0)*((float)TIM2->CCR2 - 60);
	}
	return out < 0 ? 0 : out;
}
float ang_to_pwm(int ang){
	return ang*(240.0/180.0)+60;

}
void move_servo_to(int n, int pwm){
	if (n == 0){
		TIM2->CCR1 = pwm;
	}
	else if(n == 1){
		TIM2->CCR2 = pwm;
	}
}
void move_servo_fw(int n){
	if (n == 0){
		TIM2->CCR1 += 1;
	}
	else if(n == 1){
		TIM2->CCR2 += 1;
	}
}
void move_servo_bw(int n){
	if (n == 0){
		TIM2->CCR1 -= 1;
	}
	else if(n == 1){
		TIM2->CCR2 -= 1;
	}
}

// ROS serial functions
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  nh.getHardware()->flush();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  nh.getHardware()->reset_rbuf();
}

void setup(void)
{
  nh.initNode();
  nh.advertise(chatter);
  nh.subscribe(stm32_comms);
}

void loop(void)
{
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  const char* str = hello.c_str();
  str_msg.data = str;
  chatter.publish(&str_msg);
  nh.spinOnce();
}

void str_act_msg(const std_msgs::String& msg){
	hello = msg.data;
	global_msg = msg.data;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTaskADC */
/**
  * @brief  Function implementing the TaskADC thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskADC */
void StartTaskADC(void *argument)
{
  /* USER CODE BEGIN 5 */ //here
  /* Infinite loop */
  for(;;)
  {
	  if (gripper_instruction_flag != 0 || art_instruction_flag !=0){
		  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_result, adc_channel_count);
		  osDelay(100);
	  }
	  else{
		  osDelay(500);
	  }


  }
  osThreadTerminate(NULL);

  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartGripperOpenTask */
/**
* @brief Function implementing the GripperOpenTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGripperOpenTask */
void StartGripperOpenTask(void *argument)
{
  /* USER CODE BEGIN StartGripperOpenTask */
  /* Infinite loop */
  for(;;)
	{
		if (!emergency_stop && gripper_instruction_flag == 1){
			do{
				move_servo_bw(0);
				osDelay(50);
				ang[0] = pwm_to_ang(0);
			}
			while(ang[0] > GRIPPER_OPENED_ANG);
			gripper_instruction_flag = 0;
		}
		else{
		  osDelay(200);
		}
  }
  osThreadTerminate(NULL);
  /* USER CODE END StartGripperOpenTask */
}

/* USER CODE BEGIN Header_StartGripperCloseTask */
/**
* @brief Function implementing the GripperClose thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGripperCloseTask */
void StartGripperCloseTask(void *argument)
{
  /* USER CODE BEGIN StartGripperCloseTask */
  /* Infinite loop */
  for(;;)
  {
	if (!emergency_stop && gripper_instruction_flag == 2){
			do{
				move_servo_fw(0);
				osDelay(50);
				ang[0] = pwm_to_ang(0);
			}
			while(ang[0] < GRIPPER_CLOSED_ANG && !button_pressed);
			gripper_instruction_flag = 0;
		}
		else{
		  osDelay(200);
		}
  }
  osThreadTerminate(NULL);

  /* USER CODE END StartGripperCloseTask */
}

/* USER CODE BEGIN Header_StartGripperGoToTask */
/**
* @brief Function implementing the GripperGoToTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGripperGoToTask */
void StartGripperGoToTask(void *argument)
{
  /* USER CODE BEGIN StartGripperGoToTask */
  /* Infinite loop */
  for(;;)
  {
	  if (!emergency_stop && gripper_instruction_flag == 3){
			move_servo_to(0, ang_to_pwm(gripper_go_to));
			osDelay(500);
			ang[0] = pwm_to_ang(0);
			gripper_instruction_flag = 0;
		}
		else{
		  osDelay(200);
		}
  }
  osThreadTerminate(NULL);

  /* USER CODE END StartGripperGoToTask */
}

/* USER CODE BEGIN Header_StartTaskRosserial */
/**
* @brief Function implementing the TaskRosserial thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskRosserial */
void StartTaskRosserial(void *argument)
{
  /* USER CODE BEGIN StartTaskRosserial */
  /* Infinite loop */
  for(;;)
  {
	loop();
	get_servo_pos_liveExpression();
	if (global_msg[0] == 'g'){
		if(global_msg[1] == 'o'){
			gripper_instruction_flag = 1;
		}
		else if (global_msg[1] == 'c'){
			gripper_instruction_flag = 2;

		}
	}
	else if (global_msg[0] == 'p'){
		gripper_instruction_flag = 3;
		gripper_go_to = (int)global_msg[1];

	}
	else if (global_msg[0] == 'a'){
		if(global_msg[1] == 'd'){
			art_instruction_flag = 1;
		}
		else if (global_msg[1] == 'u'){
			art_instruction_flag = 2;
		}
	}
	global_msg = "";
	osDelay(300);
  }
  osThreadTerminate(NULL);
  /* USER CODE END StartTaskRosserial */
}

/* USER CODE BEGIN Header_StartButtonTask */
/**
* @brief Function implementing the ButtonTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void *argument)
{
  /* USER CODE BEGIN StartButtonTask */
  /* Infinite loop */
  for(;;)
  {
	button_pressed = HAL_GPIO_ReadPin(Push_Button_GPIO_Port, Push_Button_Pin);
    osDelay(100);
  }
  /* USER CODE END StartButtonTask */
}

/* USER CODE BEGIN Header_StartArtUpTask */
/**
* @brief Function implementing the ArtUpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartArtUpTask */
void StartArtUpTask(void *argument)
{
  /* USER CODE BEGIN StartArtUpTask */
  /* Infinite loop */
  for(;;)
  {
  	if (!emergency_stop && art_instruction_flag == 2){
  			do{
  				move_servo_fw(1);
  				osDelay(50);
  				ang[1] = pwm_to_ang(1);
  			}
  			while(ang[1] < ART_UP_ANG);
  			art_instruction_flag = 0;
  		}
  		else{
  		  osDelay(200);
  		}
    }
  /* USER CODE END StartArtUpTask */
}

/* USER CODE BEGIN Header_StartArtDownTask */
/**
* @brief Function implementing the ArtDownTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartArtDownTask */
void StartArtDownTask(void *argument)
{
  /* USER CODE BEGIN StartArtDownTask */
  /* Infinite loop */
  for(;;)
	{
		if (!emergency_stop && art_instruction_flag == 1){
			do{
				move_servo_bw(1);
				osDelay(50);
				ang[1] = pwm_to_ang(1);
			}
			while(ang[1] > ART_DOWN_ANG);
			art_instruction_flag = 0;
		}
		else{
		  osDelay(200);
		}
	}
  osThreadTerminate(NULL);
  /* USER CODE END StartArtDownTask */
}

/* USER CODE BEGIN Header_StartArtGoToTask */
/**
* @brief Function implementing the ArtGoToTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartArtGoToTask */
void StartArtGoToTask(void *argument)
{
  /* USER CODE BEGIN StartArtGoToTask */
  /* Infinite loop */
  for(;;)
  {
  	  if (!emergency_stop && art_instruction_flag == 3){
  			move_servo_to(1, ang_to_pwm(art_go_to));
  			osDelay(2000);
  			ang[1] = pwm_to_ang(1);
  			gripper_instruction_flag = 0;
  		}
  		else{
  		  osDelay(200);
  		}
    }
  osThreadTerminate(NULL);

  /* USER CODE END StartArtGoToTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM5 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM5) {
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
