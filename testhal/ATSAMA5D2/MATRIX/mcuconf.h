/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef MCUCONF_H
#define MCUCONF_H

#define SAMA5D2x_MCUCONF

/*
 * HAL driver system settings.
 */
#define SAMA_HAL_IS_SECURE                  TRUE
#define SAMA_NO_INIT                        FALSE
#define SAMA_MOSCRC_ENABLED                 FALSE
#define SAMA_MOSCXT_ENABLED                 TRUE
#define SAMA_MOSC_SEL                       SAMA_MOSC_MOSCXT
#define SAMA_OSC_SEL                        SAMA_OSC_OSCXT
#define SAMA_MCK_SEL                        SAMA_MCK_PLLA_CLK
#define SAMA_MCK_PRES_VALUE                 1
#define SAMA_MCK_MDIV_VALUE                 3
#define SAMA_PLLA_MUL_VALUE                 83
#define SAMA_PLLADIV2_EN                    TRUE
#define SAMA_H64MX_H32MX_RATIO              2

/*
 * SPI driver system settings.
 */
#define SAMA_SPI_USE_SPI0                   FALSE
#define SAMA_SPI_USE_SPI1                   FALSE
#define SAMA_SPI_USE_FLEXCOM0               FALSE
#define SAMA_SPI_USE_FLEXCOM1               FALSE
#define SAMA_SPI_USE_FLEXCOM2               FALSE
#define SAMA_SPI_USE_FLEXCOM3               FALSE
#define SAMA_SPI_USE_FLEXCOM4               FALSE
#define SAMA_SPI_SPI0_DMA_IRQ_PRIORITY      4
#define SAMA_SPI_SPI1_DMA_IRQ_PRIORITY      4
#define SAMA_SPI_FLEXCOM0_DMA_IRQ_PRIORITY  4
#define SAMA_SPI_FLEXCOM1_DMA_IRQ_PRIORITY  4
#define SAMA_SPI_FLEXCOM2_DMA_IRQ_PRIORITY  4
#define SAMA_SPI_FLEXCOM3_DMA_IRQ_PRIORITY  4
#define SAMA_SPI_FLEXCOM4_DMA_IRQ_PRIORITY  4
#define SAMA_SPI_DMA_ERROR_HOOK(spip)       osalSysHalt("DMA failure")

/*
 * SERIAL driver system settings.
 */
#define SAMA_SERIAL_USE_UART0               FALSE
#define SAMA_SERIAL_USE_UART1               FALSE
#define SAMA_SERIAL_USE_UART2               FALSE
#define SAMA_SERIAL_USE_UART3               FALSE
#define SAMA_SERIAL_USE_UART4               FALSE
#define SAMA_SERIAL_USE_UART5               FALSE
#define SAMA_SERIAL_USE_FLEXCOM0            FALSE
#define SAMA_SERIAL_USE_FLEXCOM1            FALSE
#define SAMA_SERIAL_USE_FLEXCOM2            FALSE
#define SAMA_SERIAL_USE_FLEXCOM3            FALSE
#define SAMA_SERIAL_USE_FLEXCOM4            FALSE
#define SAMA_SERIAL_UART0_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART1_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART2_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART3_IRQ_PRIORITY      4
#define SAMA_SERIAL_UART4_IRQ_PRIORITY      4
#define SAMA_SERIAL_FLEXCOM0_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM1_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM2_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM3_IRQ_PRIORITY   4
#define SAMA_SERIAL_FLEXCOM4_IRQ_PRIORITY   4

/*
 * TC driver system settings.
 */
#define HAL_USE_TC                          FALSE
#define SAMA_USE_TC0                        FALSE
#define SAMA_USE_TC1                        FALSE
#define SAMA_TC0_IRQ_PRIORITY               2
#define SAMA_TC1_IRQ_PRIORITY               2

/*
 * UART driver system settings.
 */
#define SAMA_UART_USE_UART0                 FALSE
#define SAMA_UART_USE_UART1                 FALSE
#define SAMA_UART_USE_UART2                 FALSE
#define SAMA_UART_USE_UART3                 FALSE
#define SAMA_UART_USE_UART4                 FALSE
#define SAMA_UART_USE_FLEXCOM0              FALSE
#define SAMA_UART_USE_FLEXCOM1              FALSE
#define SAMA_UART_USE_FLEXCOM2              FALSE
#define SAMA_UART_USE_FLEXCOM3              FALSE
#define SAMA_UART_USE_FLEXCOM4              FALSE
#define SAMA_UART_UART0_IRQ_PRIORITY        4
#define SAMA_UART_UART1_IRQ_PRIORITY        4
#define SAMA_UART_UART2_IRQ_PRIORITY        4
#define SAMA_UART_UART3_IRQ_PRIORITY        4
#define SAMA_UART_UART4_IRQ_PRIORITY        4
#define SAMA_UART_FLEXCOM0_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM1_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM2_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM3_IRQ_PRIORITY     4
#define SAMA_UART_FLEXCOM4_IRQ_PRIORITY     4
#define SAMA_UART_UART0_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART1_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART2_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART3_DMA_IRQ_PRIORITY    4
#define SAMA_UART_UART4_DMA_IRQ_PRIORITY    4
#define SAMA_UART_FLEXCOM0_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM1_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM2_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM3_DMA_IRQ_PRIORITY 4
#define SAMA_UART_FLEXCOM4_DMA_IRQ_PRIORITY 4
#define SAMA_UART_DMA_ERROR_HOOK(uartp)     osalSysHalt("DMA failure")

#endif /* MCUCONF_H */
