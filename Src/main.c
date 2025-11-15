/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * Este código recebe uma curva de 60 pontos via porta serial e, em seguida, utiliza o código
  * C (gerado pelo m2cgen a partir do modelo Random Forest) para realizar a inferência
  * (determinação da concentração).
  *
  * Foram adicionados os arquivos randomforest.h e randomforest.h nos diretórios Inc e Src respectivamente.
  *
  * Sempre que o botão azul da placa F446RE é pressionado, ou sempre que um vetor chega na porta serial
  * o led2 presente na placa muda de estado.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <randomforest.h>
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NBYTES 512
#define N_FEATURES 60
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t uart_buffer[NBYTES];
volatile uint16_t rx_index = 0; 	// �?ndice do buffer, começa em 0
volatile uint8_t rx_complete = 0;	// Flag para indicar que o buffer está cheio
uint8_t rx_data;					// Variável temporária para armazenar o byte recebido
int flag = 0;

float input_vector[N_FEATURES];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

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
  __enable_irq();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  // Inicia a primeira recepção de APENAS 1 byte, armazenando-o temporariamente em rx_data
  HAL_UART_Receive_IT(&huart2, &rx_data, 1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if (rx_complete == 1)
	  {
	      HAL_UART_Transmit(&huart2, (uint8_t *)"rxcomplete\n", strlen("rxcomplete\n"), 100);
	      rx_complete = 0;
	      rx_index = 0;
	      HAL_UART_Transmit(&huart2, uart_buffer, strlen((char*)uart_buffer), 100);
	      HAL_UART_Transmit(&huart2, (uint8_t*)"\n", 1, 100);

	      char *token = strtok((char*)uart_buffer, ",");
	      int i = 0;
	      while (token != NULL && i < N_FEATURES)
	      {
	          input_vector[i] = strtof(token, NULL);  // converte "35" → 35.0f
	          token = strtok(NULL, ",");
	          i++;
	      }

	      float resultado = score(input_vector);

	      char msg[50];
	      sprintf(msg, "Resultado: %.4f\r\n", resultado);
	      HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);

	      HAL_UART_Receive_IT(&huart2, &rx_data, 1);
	  }

	  	  if(HAL_GPIO_ReadPin (GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET){
	  		  HAL_Delay(50);
	  		  if(HAL_GPIO_ReadPin (GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET){
	  			  flag = 1;
	  		  }
	  	  }

	  	  if(flag == 1 && HAL_GPIO_ReadPin (GPIOC, GPIO_PIN_13) == GPIO_PIN_SET){
	  		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	  		  HAL_UART_Transmit(&huart2, "Inverte_Led\n" , strlen("Inverte_Led\n"), 200);
	  		  flag = 0;

	  	  }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 120;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
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
  huart2.Init.BaudRate = 115200;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    // Verifica se a interrupção é da UART2
    if (huart->Instance == USART2)
    {

        // 1. Armazena o byte recebido (rx_data) na posição atual do buffer
        // (Isso é crucial, pois você quer a quebra de linha no buffer)
        uart_buffer[rx_index] = rx_data;

        // 2. Verifica as condições de parada ANTES de incrementar o índice
        //    (e antes de iniciar a próxima recepção)
        if (rx_index >= (NBYTES - 1) || rx_data == '\n')
        {
            // O buffer está cheio (use NBYTES - 1 para que o último byte seja armazenado
            // e o índice rx_index chegue a NBYTES, ou se chegou a quebra de linha).
            rx_complete = 1;
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

            // Opcional: Adicionar o terminador nulo ('\0') se for tratar a mensagem
            // como string C, e se houver espaço.
            if (rx_index < NBYTES) {
                 uart_buffer[rx_index + 1] = '\0';
            }

        }
        else
        {
            // 3. Incrementa o índice SOMENTE se for continuar a recepção
            rx_index++;

            // 4. Reinicia a recepção de mais 1 byte.
            HAL_UART_Receive_IT(&huart2, &rx_data, 1);
        }
    }
}
/* USER CODE END 4 */

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
