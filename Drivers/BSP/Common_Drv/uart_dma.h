#ifndef __UART_DMA_H
#define __UART_DMA_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"

#define USARTx                           USART6
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART6_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART6_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART6_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_6
#define USARTx_TX_GPIO_PORT              GPIOC
#define USARTx_TX_AF                     GPIO_AF7_USART6
#define USARTx_RX_PIN                    GPIO_PIN_7
#define USARTx_RX_GPIO_PORT              GPIOC
#define USARTx_RX_AF                     GPIO_AF7_USART6

/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_STREAM             DMA2_Stream7
#define USARTx_RX_DMA_STREAM             DMA2_Stream1

/* Definition for USARTx's DMA Request */
#define USARTx_TX_DMA_REQUEST            DMA_REQUEST_USART6_TX
#define USARTx_RX_DMA_REQUEST            DMA_REQUEST_USART6_RX

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn               DMA2_Stream7_IRQn
#define USARTx_DMA_RX_IRQn               DMA2_Stream1_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA2_Stream7_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA2_Stream1_IRQHandler

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      USART6_IRQn
#define USARTx_IRQHandler                USART6_IRQHandler

/* Size of Transmission buffer */
#define TXBUFFERSIZE                     (sizeof(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                     TXBUFFERSIZE

void uartDmaInit(void);
void uartDmaTest(void);
void HAL_UART_MspInit(UART_HandleTypeDef *huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart);
#endif
