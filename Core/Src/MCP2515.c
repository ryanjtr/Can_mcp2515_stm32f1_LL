/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "MCP2515.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"
/* SPI related variables */

#define SPI_CAN SPI1
#define SPI_TIMEOUT 10
#define MCP2515_CS_HIGH() LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_3)
#define MCP2515_CS_LOW() LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_3)

/* Prototypes */
static void SPI_Tx(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length);
static uint8_t SPI_Rx(void);
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length);

/* initialize MCP2515 */
bool MCP2515_Initialize(void)
{
  MCP2515_CS_HIGH();
  uint8_t loop = 10;
  do
  {
    if ((LL_SPI_IsActiveFlag_TXE(SPI_CAN) == 1 )&& (LL_SPI_IsActiveFlag_RXNE(SPI_CAN) && LL_SPI_IsActiveFlag_BSY(SPI_CAN)) == 0)
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* change mode as configuration mode */
bool MCP2515_SetConfigMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x80);

  uint8_t loop = 10;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x80)
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* change mode as normal mode */
bool MCP2515_SetNormalMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x00);

  uint8_t loop = 10;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x00)
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* Entering sleep mode */
bool MCP2515_SetSleepMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x20);

  uint8_t loop = 10;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x20)
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* MCP2515 SPI-Reset */
void MCP2515_Reset(void)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_RESET);

  MCP2515_CS_HIGH();
}

/* read single byte */
uint8_t MCP2515_ReadByte(uint8_t address)
{
  uint8_t retVal;

  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_READ);
  SPI_Tx(address);
  retVal = SPI_Rx();

  MCP2515_CS_HIGH();

  return retVal;
}

/* read buffer */
void MCP2515_ReadRxSequence(uint8_t instruction, uint8_t *data, uint8_t length)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);
  SPI_RxBuffer(data, length);

  MCP2515_CS_HIGH();
}

/* write single byte */
void MCP2515_WriteByte(uint8_t address, uint8_t data)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(address);
  SPI_Tx(data);

  MCP2515_CS_HIGH();
}

/* write buffer */
void MCP2515_WriteByteSequence(uint8_t startAddress, uint8_t endAddress, uint8_t *data)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(startAddress);
  SPI_TxBuffer(data, (endAddress - startAddress + 1));

  MCP2515_CS_HIGH();
}

/* write to TxBuffer */
void MCP2515_LoadTxSequence(uint8_t instruction, uint8_t *idReg, uint8_t dlc, uint8_t *data)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);
  SPI_TxBuffer(idReg, 4);
  SPI_Tx(dlc);
  SPI_TxBuffer(data, dlc);

  MCP2515_CS_HIGH();
}

/* write to TxBuffer(1 byte) */
void MCP2515_LoadTxBuffer(uint8_t instruction, uint8_t data)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);
  SPI_Tx(data);

  MCP2515_CS_HIGH();
}

/* request to send */
void MCP2515_RequestToSend(uint8_t instruction)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);

  MCP2515_CS_HIGH();
}

/* read status */
uint8_t MCP2515_ReadStatus(void)
{
  uint8_t retVal;

  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_READ_STATUS);
  retVal = SPI_Rx();

  MCP2515_CS_HIGH();

  return retVal;
}

/* read RX STATUS register */
uint8_t MCP2515_GetRxStatus(void)
{
  uint8_t retVal;

  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_RX_STATUS);
  retVal = SPI_Rx();

  MCP2515_CS_HIGH();

  return retVal;
}

/* Use when changing register value */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_BIT_MOD);
  SPI_Tx(address);
  SPI_Tx(mask);
  SPI_Tx(data);

  MCP2515_CS_HIGH();
}

/* SPI Tx wrapper function  */
static void SPI_Tx(uint8_t data)
{
  while (!LL_SPI_IsActiveFlag_TXE(SPI_CAN))
    ;
  LL_SPI_TransmitData8(SPI_CAN, data);
  while (!LL_SPI_IsActiveFlag_RXNE(SPI_CAN))
    ;
  LL_SPI_ReceiveData8(SPI_CAN);
}

/* SPI Tx wrapper function */
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
{
  for (int i = 0; i < length; ++i)
  {
    SPI_Tx(buffer[i]);
  }
}

/* SPI Rx wrapper function */
static uint8_t SPI_Rx(void)
{
  while (!LL_SPI_IsActiveFlag_TXE(SPI_CAN))
    ;
  LL_SPI_TransmitData8(SPI_CAN, 0xFF);
  while (!LL_SPI_IsActiveFlag_RXNE(SPI_CAN))
    ;
  return LL_SPI_ReceiveData8(SPI_CAN);
}

/* SPI Rx wrapper function */
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
{
  for (int i = 0; i < length; ++i)
  {
    buffer[i] = SPI_Rx();
  }
}
