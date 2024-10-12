#include "uart_dma.h"
#include "main.h"
#include "stm32h7xx_it.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

bool UartReady = false;
UART_HandleTypeDef UartHandle;
UART_HandleTypeDef huart7;
uint8_t aTxBuffer[] = " *****UART DMA COMMUNICATION***** ";
/* Buffer used for reception :
   Size should be a Multiple of cache line size (32 bytes) */
uint8_t aRxBuffer[RXBUFFERSIZE];

void uartDmaInit(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn,1,0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /*##-1- Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART configured as follows:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = None
      - BaudRate = 9600 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance        = USARTx;

  UartHandle.Init.BaudRate   = 250000;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  
  UartHandle.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  UartHandle.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  
  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }  
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  
  huart7.Instance = UART7;
  huart7.Init.BaudRate = 250000;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_2;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  if(HAL_UART_Init(&huart7) != HAL_OK)
  {
    printf("uart7 init failed\r\n");
  }
  
}

static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;

static DMA_HandleTypeDef hdma_uart7_tx;
static DMA_HandleTypeDef hdma_uart7_rx;

#if 1
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{

  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  if(huart->Instance == UART7)
  {
    __HAL_RCC_UART7_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_UART7;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    hdma_uart7_rx.Instance = DMA2_Stream3;
    hdma_uart7_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart7_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart7_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart7_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart7_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart7_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart7_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_uart7_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if(HAL_DMA_Init(&hdma_uart7_rx) != HAL_OK)
    {
      printf("rx dma init failed\r\n");
    }
    __HAL_LINKDMA(huart, hdmarx, hdma_uart7_rx);
    
    hdma_uart7_tx.Instance = DMA2_Stream2;
    hdma_uart7_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart7_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart7_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart7_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart7_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart7_tx.Init.Mode = DMA_NORMAL;
    hdma_uart7_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart7_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if(HAL_DMA_Init(&hdma_uart7_tx) != HAL_OK)
    {
      printf("tx dma init failed\r\n");
    }
    __HAL_LINKDMA(huart, hdmatx, hdma_uart7_tx);
    
    HAL_NVIC_SetPriority(UART7_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(UART7_IRQn);
    
  }
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();

  /* Enable USARTx clock */
  USARTx_CLK_ENABLE();

  /* Enable DMA clock */
  DMAx_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = USARTx_TX_DMA_STREAM;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
  hdma_tx.Init.Request             = USARTx_TX_DMA_REQUEST;

  HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the UART handle */
  __HAL_LINKDMA(huart, hdmatx, hdma_tx);

  /* Configure the DMA handler for reception process */
  hdma_rx.Instance                 = USARTx_RX_DMA_STREAM;
  hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_rx.Init.Mode                = DMA_NORMAL;
  hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;
  hdma_rx.Init.Request             = USARTx_RX_DMA_REQUEST;

  HAL_DMA_Init(&hdma_rx);

  /* Associate the initialized DMA handle to the the UART handle */
  __HAL_LINKDMA(huart, hdmarx, hdma_rx);
    
  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (USART6_TX) */
  HAL_NVIC_SetPriority(USARTx_DMA_TX_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USARTx_DMA_TX_IRQn);
    
  /* NVIC configuration for DMA transfer complete interrupt (USART6_RX) */
  HAL_NVIC_SetPriority(USARTx_DMA_RX_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USARTx_DMA_RX_IRQn);
  
  /* NVIC for USART, to catch the TX complete */
  HAL_NVIC_SetPriority(USARTx_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}
#endif

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{

  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure USARTx Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure USARTx Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
   
  /*##-3- Disable the DMA #####################################################*/
  /* De-Initialize the DMA channel associated to reception process */
  if(huart->hdmarx != 0)
  {
    HAL_DMA_DeInit(huart->hdmarx);
  }
  /* De-Initialize the DMA channel associated to transmission process */
  if(huart->hdmatx != 0)
  {
    HAL_DMA_DeInit(huart->hdmatx);
  }  
  
  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(USARTx_DMA_TX_IRQn);
  HAL_NVIC_DisableIRQ(USARTx_DMA_RX_IRQn);
}

void uartDmaTest(void)
{

  if(HAL_UART_Transmit_DMA(&UartHandle, (uint8_t*)aTxBuffer, TXBUFFERSIZE)!= HAL_OK)
  {
    printf("uart  transmit dma  failed\r\n");
  }
  while (UartReady != true)
  {
  }
  UartReady = false;
  /*##-4- Put UART peripheral in reception process ###########################*/ 
  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    //Error_Handler();
  }  
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    //Error_Handler();
  }
  //__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_IDLE);
  if(HAL_UART_Receive_DMA(&UartHandle, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
  {
    Error_Handler();
  }


  
  /*##-5- Wait for the end of the transfer ###################################*/  
  while (UartReady != true)
  {
  }

  /* Reset transmission flag */
  UartReady = false;

  /* Invalidate cache prior to access by CPU */
  SCB_InvalidateDCache_by_Addr ((uint32_t *)aRxBuffer, RXBUFFERSIZE);
}

/**
  * @brief  This function handles UART interrupt request.  
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA  
  *         used for USART data transmission     
  */
void USART6_IRQHandler(void)
{

  HAL_UART_IRQHandler(&UartHandle);
}

void UART7_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle);
}

void DMA2_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart7_tx);
}

void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart7_rx);
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of DMA Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  if(UartHandle->Instance == USART6)
  {
    UartReady = true;
    
  }
  /* Turn LED2 on: Transfer in transmission process is correct */
  //BSP_LED_On(LED2); 

  
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  //memset(aRxBuffer, 0, RXBUFFERSIZE);
  //if(HAL_UART_Receive_DMA(UartHandle, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
  //{
   // Error_Handler();
  //}
  if(UartHandle->Instance == USART6)
  {
    UartReady = true;
  }
  /* Turn LED2 off: Transfer in reception process is correct */
  //BSP_LED_Off(LED2);
  

  
}

/**
  * @brief  This function handles DMA interrupt request.  
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA  
  *         used for USART data transmission     
  */
void USARTx_DMA_RX_IRQHandler(void)
{
  
  UartReady = true;
  HAL_DMA_IRQHandler(UartHandle.hdmarx);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  printf("uart error, errorCode = %d\r\n", huart->ErrorCode);
}

/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA  
  *         used for USART data reception    
  */
void USARTx_DMA_TX_IRQHandler(void)
{
  UartReady = true;
  HAL_DMA_IRQHandler(UartHandle.hdmatx);
}

/*********************************串口重定向***********************************************/
#include <LowLevelIOInterface.h>
int iar9x_putc(int x);
#pragma module_name = "?__write"
 
 
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int iar9x_putc(int ch)
#endif /* __GNUC__ */
 
 
/*
* If the __write implementation uses internal buffering, uncomment
* the following line to ensure that we are called with "buffer" as 0
* (i.e. flush) when the application terminates.
*/
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    size_t nChars = 0;
    if (buffer == 0)
    {
        /* This means that we should flush internal buffers. Since we don't we just return
        * (Remember, "handle" == -1 means that all handles should be flushed.) */
        return 0;
    }
    /* Only writes to "standard out" and "standard err", return failure for other handles*/
    if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR)
    {
        return _LLIO_ERROR;
    }
    for (/* Empty */; size != 0; --size)
    {
        if (iar9x_putc(*buffer++) < 0)
        {
            return _LLIO_ERROR;
        }
        ++nChars;
    }
    return nChars;
}
 
 
/*** @brief Retargets the C library printf function to the USART. ***/
PUTCHAR_PROTOTYPE
{
 /* Place your implementation of fputc here */
 /* e.g. write a character to the USART and Loop until the end of transmission */
 HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
 return ch;
}

/***************************************************************************************************/
