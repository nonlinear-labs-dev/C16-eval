/******************************************************************************/
/** @file		nl_usb_midi.c
    @date		2014-12-11
    @brief    	Functions for the USB-MIDI driver
	@ingroup	nl_drv_modules
    @author		Nemanja Nikodijevic [2014-12-11]

*******************************************************************************/
#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_descmidi.h"
#include "usb/driver/nl_usb_core.h"
#include "usb/driver/nl_usb_core_circular_buffers.h"
#include "cmsis/LPC43xx.h"
#include "cmsis/core_cm4.h"
#include "cmsis/core_cmFunc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wpadded"

typedef struct
{
  MidiReceiveComplete_Callback ReceiveCallback;
  int                          suspendReceive;
  int                          primed;
} UsbMidi_t;

static UsbMidi_t usbMidi[2];

struct _rxBuffer
{
  uint8_t *buffer;
  uint16_t index;
  uint16_t bulkSize;
} static rx[2] = {
  {
      .buffer   = (uint8_t *) USB_BUFFER_HOST_TO_BRIDGE,
      .index    = 0,
      .bulkSize = USB_HS_BULK_SIZE,
  },
  {
      .buffer   = (uint8_t *) USB_BUFFER_BRIDGE_TO_HOST,
      .index    = 0,
      .bulkSize = USB_FS_BULK_SIZE,
  },
};

/******************************************************************************/
/** @brief		Endpoint 1 Callback (Data read from Host)
    @param[in]	event	Event that triggered the interrupt
*******************************************************************************/

static inline void primeReceive(uint8_t const port)
{
  if (!usbMidi[port].suspendReceive)
  {
    if (!usbMidi[port].primed)
    {
      USB_ReadReqEP(port, 0x01, &(rx[port].buffer[rx[port].index]), rx[port].bulkSize);
      usbMidi[port].primed = 1;
    }
  }
}

void USB_MIDI_primeReceive(uint8_t const port)
{
  __disable_irq();
  primeReceive(port);
  __enable_irq();
}

static void Handler_ReadFromHost(uint8_t const port, uint32_t const event)
{
  switch (event)
  {
    case USB_EVT_OUT_NAK:  // host has data --> get a transfer running to collect it
      primeReceive(port);
      break;

    case USB_EVT_OUT:  // transfer finished successfully --> hand data over to application
    {
      uint16_t length = (uint16_t) USB_ReadEP(port, 0x01);
      if (usbMidi[port].ReceiveCallback)
      {
        usbMidi[port].ReceiveCallback(port, &(rx[port].buffer[rx[port].index]), length);
        usbMidi[port].primed = 0;
        rx[port].index       = (uint16_t)((rx[port].index + length) % USB_BUFFER_BRIDGE_HOST_SIZE);
      }
      // prepare the next potential transfer right now to avoid extra NAK phase later
      primeReceive(port);
      break;
    }
  }
}

static void EndPoint1_ReadFromHost_0(uint8_t const port, uint32_t const event)
{
  Handler_ReadFromHost(0, event);
}

static void EndPoint1_ReadFromHost_1(uint8_t const port, uint32_t const event)
{
  Handler_ReadFromHost(1, event);
}

/******************************************************************************/
/** @brief    Function that initializes USB MIDI driver for USB0 controller
*******************************************************************************/
void USB_MIDI_Init(uint8_t const port)
{
  usbMidi[port].suspendReceive = 0;
  usbMidi[port].primed         = 0;
  /** assign descriptors */
  USB_Core_Device_Descriptor_Set(port, (const uint8_t *) (port == 0) ? USB0_MIDI_DeviceDescriptor : USB1_MIDI_DeviceDescriptor);
  USB_Core_Device_FS_Descriptor_Set(port, (const uint8_t *) USB_MIDI_FSConfigDescriptor);
  USB_Core_Device_HS_Descriptor_Set(port, (const uint8_t *) USB_MIDI_HSConfigDescriptor);
  USB_Core_Device_String_Descriptor_Set(port, (const uint8_t *) (port == 0) ? USB0_MIDI_StringDescriptor : USB1_MIDI_StringDescriptor);
  USB_Core_Device_Device_Quali_Descriptor_Set(port, (const uint8_t *) USB_MIDI_DeviceQualifier);
  /** assign callbacks */
  USB_Core_Endpoint_Callback_Set(port, 1, (port == 0) ? EndPoint1_ReadFromHost_0 : EndPoint1_ReadFromHost_1);
  USB_Core_Init(port);
}

void USB_MIDI_DeInit(uint8_t const port)
{
  usbMidi[port].suspendReceive = 0;
  usbMidi[port].primed         = 0;
  USB_Core_DeInit(port);
  USB_Core_DeInit(port);
}

/******************************************************************************/
/** @brief    Function that configures the USB MIDI driver
 *  @param[in]	midircv		Pointer to the callback function for
 *  						the MIDI received data
*******************************************************************************/
void USB_MIDI_Config(uint8_t const port, MidiReceiveComplete_Callback midircv)
{
  usbMidi[port].ReceiveCallback = midircv;
}

/******************************************************************************/
/** @brief		Checks whether the USB-MIDI is connected and configured
    @return		1 - Success ; 0 - Failure
*******************************************************************************/
uint32_t USB_MIDI_IsConfigured(uint8_t const port)
{
  return USB_Core_IsConfigured(port);
}

uint32_t USB_MIDI_ConfigStatus(uint8_t const port)
{
  return USB_Core_ConfigStatus(port);
}

/******************************************************************************/
/** @brief		Send MIDI buffer
    @param[in]	buff	Pointer to data buffer
    @param[in]	cnt		Amount of bytes to send
    @param[in]	imm		Immediate only
    @return		Number of bytes written - Success ; -1 - Failure
*******************************************************************************/
int32_t USB_MIDI_Send(uint8_t const port, uint8_t const *const buff, uint32_t const cnt)
{
  if (USB_Core_ReadyToWrite(port, 0x82))
  {
    if (cnt)
      USB_WriteEP(port, 0x82, (uint8_t *) buff, (uint32_t) cnt);
    return (int32_t) cnt;
  }
  return -1;
}

/******************************************************************************/
/** @brief		Get the amount of bytes left to be sent
    @return		Amount of bytes to be sent from the buffer
*******************************************************************************/
int32_t USB_MIDI_BytesToSend(uint8_t const port)
{
  return USB_Core_BytesToSend(port, 0x82);
}

/******************************************************************************/
/** @brief		Suspend further receives
    @param[in]	suspend	!= 0 --> suspended, == 0 --> normal
*******************************************************************************/
void USB_MIDI_SuspendReceive(uint8_t const port, uint8_t const suspend)
{
  usbMidi[port].suspendReceive = suspend;
}

/******************************************************************************/
/** @brief		Kill any active transmit
*******************************************************************************/
void USB_MIDI_KillTransmit(uint8_t const port)
{
  USB_ResetEP(port, 0x82);
}

void USB_MIDI_ClearReceive(uint8_t const port)
{
  usbMidi[port].primed = 0;
}

// EOF
