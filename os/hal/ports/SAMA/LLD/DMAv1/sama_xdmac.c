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

/**
 * @file    DMAv1/sama_xdmac.c
 * @brief   Enhanced DMA helper driver code.
 *
 * @addtogroup SAMA_DMA
 * @details DMA sharing helper driver. In the SAMA the DMA channels are a
 *          dedicated resource, this driver allows to allocate and free DMA
 *          channels at runtime.
 * @{
 */

#include "hal.h"

/* The following macro is only defined if some driver requiring DMA services
   has been enabled.*/
#if defined(SAMA_DMA_REQUIRED) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
sama_dma_channel_t _sama_dma_channel_t[XDMAC_CHANNELS_TOT];

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/**
 * @brief   Associates a controller to a DMA channel.
 *
 * @param[in]   index        index of the channel
 * @return      xdmacp       pointer to DMA controller
 *
 * @notapi
 */
#define dmaControllerSelect(index)                             \
  index < (XDMAC_CONTROLLERS - 1) ? XDMAC0 : XDMAC1            \

/**
 * @brief   Associates ID number to controller.
 *
 * @param[in]   xdmacp       pointer to DMA controller
 * @return      ID_XDMACx    peripheral ID of DMA controller
 *
 * @notapi
 */
 #define dmaGetControllerId(xdmacp)                            \
   (Xdmac *) xdmacp == XDMAC0 ? ID_XDMAC0 : ID_XDMAC1

/**
 * @brief   Get content of Global Status register.
 *
 * @param[in]   xdmacp       pointer to DMA controller
 * @return      XDMAC_GS     content of Global Status register
 *
 * @notapi
 */
 #define dmaGetGlobal(xdmacp)       xdmacp->XDMAC_GS

/**
 * @brief   Get content of Global Interrupt Status register.
 *
 * @param[in]   xdmacp       pointer to DMA controller
 * @return      XDMAC_GIS    content of Global Interrupt Status register
 *
 * @notapi
 */
 #define dmaGetGlobalInt(xdmacp)    xdmacp->XDMAC_GIS

/**
 * @brief   Get content of Channel Interrupt Mask register.
 *
 * @param[in] dmachp      pointer to a sama_dma_channel_t structure
 * @return    XDMAC_CIMx  content of Channel Interrupt Mask register
 *
 * @notapi
 */
#define dmaGetChannelIntMask(dmachp)                                \
  (dmachp)->xdmac->XDMAC_CHID[(dmachp)->chid].XDMAC_CIM

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief XDMAC interrupt handler
 */
OSAL_IRQ_HANDLER(dmaHandler) {
  uint32_t cont;

  OSAL_IRQ_PROLOGUE();
  for (cont = 0; cont < XDMAC_CONTROLLERS; cont++) {
    uint32_t chan, gis, flags;

    Xdmac *xdmac = dmaControllerSelect(cont);

    /* Read Global Interrupt Status Register */
    gis = dmaGetGlobalInt(xdmac);

    if ((gis & 0xFFFF) == 0)
    /* There is no interrupt pending for this xdmac controller */
      continue;

    for (chan = 0; chan < XDMAC_CHANNELS; chan++) {
      sama_dma_channel_t *channel = &_sama_dma_channel_t[(cont * XDMAC_CHANNELS) + chan];
      bool pendingInt = FALSE;

      if (!(gis & (0x1 << chan)))
      /* There is no pending interrupt for this channel */
        continue;

      if (channel->state == SAMA_DMA_FREE)
      /* Channel is free */
        continue;

      uint32_t cis = dmaGetChannelInt(channel);

      if (cis & XDMAC_CIS_BIS) {
        if (!(dmaGetChannelIntMask(channel) & XDMAC_CIM_LIM)) {
          pendingInt = TRUE;
        }
      }

      if (cis & XDMAC_CIS_LIS) {
        pendingInt = TRUE;
      }

      if (cis & XDMAC_CIS_DIS) {
        pendingInt = TRUE;
      }
      flags = cis;

      /* Execute callback */
      if (pendingInt && channel->dma_func) {
        channel->dma_func(channel->dma_param,flags);
      }
    }
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   SAMA DMA helper initialization.
 *
 * @init
 */
void dmaInit(void) {

  uint8_t cont, chan;

#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX0, ID_XDMAC0, SECURE_PER);
  mtxConfigPeriphSecurity(MATRIX0, ID_XDMAC1, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */

  for (cont = 0; cont < XDMAC_CONTROLLERS; cont++) {

    Xdmac *xdmac = dmaControllerSelect(cont);

    for (chan = 0; chan < XDMAC_CHANNELS; chan++) {
      sama_dma_channel_t *channel = &_sama_dma_channel_t[(cont * XDMAC_CHANNELS) + chan];

      /* Initialization of the specific channel */
      channel->xdmac    = xdmac;
      channel->chid     = chan;
      channel->state    = SAMA_DMA_FREE;
      channel->dma_func = NULL;

      /* Clear interrupts */
      dmaGetChannelInt(channel);
    }

    uint32_t id = dmaGetControllerId(xdmac);
    /* set aic source handler */
    aicSetSourceHandler(id, dmaHandler);
  }
}

/**
 * @brief   Sets the number of transfers to be performed.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @pre     The channel must have been allocated using @p dmaChannelAllocate().
 * @post    After use the channel can be released using @p dmaChannelRelease().
 *
 * @param[in] dmastp    pointer to a sama_dma_channel_t structure
 * @param[in] size      value to be written in the XDMAC_CUBC register
 *
 * @special
 */
void dmaChannelSetTransactionSize(sama_dma_channel_t *dmachp, size_t n) {

uint32_t i;
uint32_t divisor;
  /* Single block single microblock */
  if (n <= XDMAC_MAX_BT_SIZE) {
    (dmachp)->xdmac->XDMAC_CHID[(dmachp)->chid].XDMAC_CUBC = XDMAC_CUBC_UBLEN(n);
  }
  /* Single block multiple microblocks */
  else {
   /* If n exceeds XDMAC_MAX_BT_SIZE, split the transfer in microblocks */
    for (i = 2; i < XDMAC_MAX_BT_SIZE; i++) {
      divisor = XDMAC_MAX_BT_SIZE / i;
      if (n % divisor)
        continue;
      if ((n / divisor) <= XDMAC_MAX_BLOCK_LEN) {
        (dmachp)->xdmac->XDMAC_CHID[(dmachp)->chid].XDMAC_CUBC = XDMAC_CUBC_UBLEN(i);
        (dmachp)->xdmac->XDMAC_CHID[(dmachp)->chid].XDMAC_CBC =
                                                   XDMAC_CBC_BLEN((n / divisor) - 1);
        break;
      }
    }
    osalDbgAssert(n == XDMAC_MAX_BT_SIZE, "unsupported DMA transfer size");
  }
}

/**
 * @brief   Assigns a DMA channel.
 * @details The channel is assigned and, if required, the DMA clock enabled.
 *          The function also enables the IRQ vector associated to the channel
 *          and initializes its priority.
 * @pre     The channel must not be already in use or an error is returned.
 * @post    The channel is allocated and the default ISR handler redirected
 *          to the specified function.
 * @post    The channel ISR vector is enabled and its priority configured.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] priority  IRQ priority mask for the DMA stream
 * @param[in] func      handling function pointer, can be @p NULL
 * @param[in] param     a parameter to be passed to the handling function
 * @return              A pointer to sama_dma_channel_t structure if channel is
 *                      assigned or NULL.
 *
 * @special
 */
sama_dma_channel_t* dmaChannelAllocate(uint32_t priority,
                                       sama_dmaisr_t func,
                                       void *param) {

  sama_dma_channel_t *channel = NULL;
  uint8_t id;
  uint8_t chan;
  for (chan = 0; chan < XDMAC_CHANNELS_TOT; chan++) {
    channel = &_sama_dma_channel_t[chan];
    if (channel->state != SAMA_DMA_FREE) {
      channel = NULL;
    }
    else {
      break;
    }
  }

  if (channel != NULL) {
  /* Marks the channel as allocated.*/
    channel->state = SAMA_DMA_NOT_FREE;
    channel->dma_func = func;
    channel->dma_param = param;
    id = dmaGetControllerId(channel->xdmac);

  /* Setting aic */
    aicSetSourcePriority(id, priority);
    aicEnableInt(id);

  /* Enabling DMA clocks required by the current channel set.*/
    if (id == ID_XDMAC0) {
      pmcEnableXDMAC0();
    }
    else {
      pmcEnableXDMAC1();
    }

  /* Enable channel interrupt */
    channel->xdmac->XDMAC_CHID[channel->chid].XDMAC_CIE =  XDMAC_CIE_BIE;
    channel->xdmac->XDMAC_GIE = XDMAC_GIE_IE0 << (channel->chid);
  }
  return channel;
}

/**
 * @brief   Releases a DMA channel.
 * @details The stream is channel and, if required, the DMA clock disabled.
 *          Trying to release a unallocated channel is an illegal operation
 *          and is trapped if assertions are enabled.
 * @pre     The channel must have been allocated using @p dmaChannelAllocate().
 * @post    The channel is again available.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] dmachp    pointer to a sama_dma_channel_t structure
 *
 * @special
 */
void dmaChannelRelease(sama_dma_channel_t *dmachp) {

  osalDbgCheck(dmachp != NULL);
  uint8_t id;
  /* Check if the channel is free.*/
  osalDbgAssert(dmachp->state != SAMA_DMA_FREE,
                "not allocated");
  id = dmaGetControllerId(dmachp->xdmac);
  /* Disables the associated IRQ vector.*/
  aicDisableInt(id);

  /* Disables channel */
  dmaChannelDisable(dmachp);

  /* Marks the stream as not allocated.*/
  (dmachp)->state = SAMA_DMA_FREE;

}

#endif /* SAMA_DMA_REQUIRED */

/** @} */
