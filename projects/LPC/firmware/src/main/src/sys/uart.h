#pragma once

#include <stdint.h>
#include "cmsis/LPC43xx.h"

static inline void UART_Init(void)
{
  // Baudrate setup for PCLK=204MHz and BR=115200, (approximate) best fit :
  // initial try:
  // DL_est = 110.677, not integer ==>
  // Iteration with initial FR_est=1.5 gives a good fit for FR=1.538
  // DL_exact=71.96 ==> DL=72 ==> DLM=0, DLL=72
  // FR_exact=1.5372, FR_used (as per table) =1.538 ==> DivAddVal=7, MulVal=13
  // gives an actual baudrate of 115104, an error of -0.08%
  LPC_USART0->LCR = (1 << USART0_LCR_DLAB_Pos);  // enable access to DLM, DLL and FDR
  LPC_USART0->DLM = 0;
  LPC_USART0->DLL = 72;
  LPC_USART0->FDR = (13 << USART0_FDR_MULVAL_Pos) | (7 << USART0_FDR_DIVADDVAL_Pos);

  // 8bit, 1 stopBit, no parity
  LPC_USART0->LCR = (3 << USART0_LCR_WLS_Pos)  // 8 bits
      | (0 << USART0_LCR_SBS_Pos)              // 1 stopBit
      | (0 << USART0_LCR_PE_Pos)               // no parity
      | (0 << USART0_LCR_PS_Pos)               // odd parity (not used)
      | (0 << USART0_LCR_BC_Pos)               // no break
      | (0 << USART0_LCR_DLAB_Pos);            // disable access to DLM, DLL and FDR

  // Enable and Reset FIFOs
  LPC_USART0->FCR = (1 << USART0_FCR_FIFOEN_Pos)
      | (1 << USART0_FCR_RXFIFORES_Pos)
      | (1 << USART0_FCR_TXFIFORES_Pos)
      | (0 << USART0_FCR_DMAMODE_Pos)
      | (0 << USART0_FCR_RXTRIGLVL_Pos);
}

static inline uint32_t UART_GetAndClearLineStatusRegister(void)
{
  return LPC_USART0->LSR;
}

static inline int UART_ReceiveError(uint32_t const status)
{
  return (status & ((1 << USART0_LSR_OE_Pos) | (1 << USART0_LSR_PE_Pos) | (1 << USART0_LSR_FE_Pos) | (1 << USART0_LSR_BI_Pos) | (1 << USART0_LSR_RXFE_Pos))) != 0;
}

static inline int UART_DataAvailable(uint32_t const status)
{
  return (status & (1 << USART0_LSR_RDR_Pos)) != 0;
}

static inline int UART_ReadyToSend(uint32_t const status)
{
  return (status & (1 << USART0_LSR_TEMT_Pos)) != 0;
}

static inline void UART_SendByte(uint8_t const byte)
{
  LPC_USART0->THR = byte;
}

static inline uint8_t UART_ReceiveByte(void)
{
  return (uint8_t) LPC_USART0->RBR;
}
