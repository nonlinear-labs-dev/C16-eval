/******************************************************************************/
/** @file		nl_usb_descmidi.c
    @date		2013-01-23
    @brief    	Descriptors for the USB-MIDI driver
	@ingroup	nl_drv_modules
    @author		Nemanja Nikodijevic [2013-01-23]
*******************************************************************************/
#include "cmsis/LPC43xx.h"
#include "cmsis/lpc_types.h"

#include "usb/driver/nl_usbd.h"
#include "usb/driver/nl_usb_core.h"
#include "usb/driver/nl_usb_descmidi.h"
#include "sys/flash.h"

/* USB Standard Device Descriptor */
uint8_t const USB0_MIDI_DeviceDescriptor[] = {
  USB_DEVICE_DESC_SIZE,       /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(BCDUSB_2_0),          /* bcdUSB */
  0x00,                       /* bDeviceClass */
  0x00,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  USB_MAX_PACKET0,            /* bMaxPacketSize */
  WBVAL(VENDOR_ID),           /* idVendor */
  WBVAL(PRODUCT_ID_HS),       /* idProduct */
  WBVAL(BCD_DEVICE),          /* bcdDevice */
  0x01,                       /* iManufacturer */
  0x02,                       /* iProduct */
  0x00,                       /* iSerialNumber */
  0x01,                       /* bNumConfigurations */
};

uint8_t const USB1_MIDI_DeviceDescriptor[] = {
  USB_DEVICE_DESC_SIZE,       /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(BCDUSB_1_1),          /* bcdUSB */
  0x00,                       /* bDeviceClass */
  0x00,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  USB_MAX_PACKET0,            /* bMaxPacketSize */
  WBVAL(VENDOR_ID),           /* idVendor */
  WBVAL(PRODUCT_ID_FS),       /* idProduct */
  WBVAL(BCD_DEVICE),          /* bcdDevice */
  0x01,                       /* iManufacturer */
  0x02,                       /* iProduct */
  0x00,                       /* iSerialNumber */
  0x01,                       /* bNumConfigurations */
};

/** USB FSConfiguration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class */
uint8_t const USB_MIDI_FSConfigDescriptor[] = {
  /** Configuration 1 */
  USB_CONFIGURATION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(CONFIG_BLOCK_SIZE),
  0x02,                    /* bNumInterfaces */
  0x01,                    /* bConfigurationValue: 0x01 is used to select this configuration */
  0x00,                    /* iConfiguration: no string to describe this configuration */
  USB_CONFIG_SELF_POWERED, /* bmAttributes */
  USB_CONFIG_POWER_MA(0),  /* bMaxPower, device power consumption is 200 mA */
                           /** Interface 0: Standard Audio Control interface */
  USB_INTERFACE_DESC_SIZE,
  USB_INTERFACE_DESCRIPTOR_TYPE,
  0x00,                       /* bInterfaceNumber: 0 */
  0x00,                       /* bAlternateSetting */
  0x00,                       /* bNumEndPoints: no endpoints */
  USB_DEVICE_CLASS_AUDIO,     /* bInterfaceClass: Audio */
  USB_SUBCLASS_AUDIO_CONTROL, /* bInterfaceSubClass: AudioControl */
  0x00,                       /* bInterfaceProtocol: no protocol */
  0x00,                       /* iInterface: no string desc */
                              /** CS Interface descriptor - Audio Control */
  USB_CS_AUDIO_CONTROL_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_HEADER_SUBTYPE,
  WBVAL(BCDADC_1_0),
  WBVAL(USB_CS_AUDIO_CONTROL_TOTAL),
  0x01,
  0x01,
  /** Interface 1: standard midistreaming interface descriptor */
  USB_INTERFACE_DESC_SIZE,
  USB_INTERFACE_DESCRIPTOR_TYPE,
  0x01,                        /* bInterfaceNumber: 1 */
  0x00,                        /* bAlternateSetting */
  0x02,                        /* bNumEndPoints: 2 */
  USB_DEVICE_CLASS_AUDIO,      /* bInterfaceClass: Audio */
  USB_SUBCLASS_MIDI_STREAMING, /* bInterfaceSubclass: MIDI Streaming */
  0x00,                        /* bInterfaceProtocol: no protocol */
  0x00,                        /* iInterface: no string desc */
                               /** CS Interface descriptor - MIDI Streaming */
  USB_CS_MIDI_STREAMING_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_HEADER_SUBTYPE,
  WBVAL(BCDADC_1_0),
  WBVAL(USB_CS_MIDI_STREAMING_TOTAL),
  /** MIDI IN jack - embedded */
  USB_MIDI_IN_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_IN_JACK_SUBTYPE,
  USB_MIDI_JACK_EMBEDDED,
  0x01,
  0x00,
  /** MIDI IN jack - external */
  USB_MIDI_IN_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_IN_JACK_SUBTYPE,
  USB_MIDI_JACK_EXTERNAL,
  0x02,
  0x00,
  /** MIDI OUT jack - embedded */
  USB_MIDI_OUT_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_OUT_JACK_SUBTYPE,
  USB_MIDI_JACK_EMBEDDED,
  0x03,
  0x01,
  0x02,
  0x01,
  0x00,
  /** MIDI OUT jack - external */
  USB_MIDI_OUT_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_OUT_JACK_SUBTYPE,
  USB_MIDI_JACK_EXTERNAL,
  0x04,
  0x01,
  0x01,
  0x01,
  0x00,
  /** Bulk OUT - standard endpoint descriptor */
  USB_AUDIO_ENDPOINT_DESC_SIZE,
  USB_ENDPOINT_DESCRIPTOR_TYPE,
  USB_ENDPOINT_OUT(MIDI_EP_OUT),
  USB_ENDPOINT_TYPE_BULK,
  WBVAL(USB_FS_BULK_SIZE),
  0x00,
  0x00,
  0x00,
  /** Bulk OUT - class-specific endpoint descriptor */
  USB_MIDI_ENDPOINT_DESC_SIZE,
  USB_CS_ENDPOINT_DESC_TYPE,
  USB_MIDI_EP_GENERAL_SUBTYPE,
  0x01,
  0x01,
  /** BULK IN - standard endpoint descriptor */
  USB_AUDIO_ENDPOINT_DESC_SIZE,
  USB_ENDPOINT_DESCRIPTOR_TYPE,
  USB_ENDPOINT_IN(MIDI_EP_IN),
  USB_ENDPOINT_TYPE_BULK,
  WBVAL(USB_FS_BULK_SIZE),
  0x00,
  0x00,
  0x00,
  /** Bulk IN - class-specific endpoint descriptor */
  USB_MIDI_ENDPOINT_DESC_SIZE,
  USB_CS_ENDPOINT_DESC_TYPE,
  USB_MIDI_EP_GENERAL_SUBTYPE,
  0x01,
  0x03,
  /* Terminator */
  0
};

// see https://www.microchip.com/forums/m493322.aspx#493408  on patching the virtual cables

/* USB HSConfiguration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
#define CS_MS_IFC_DESC_LEN (7             /* this descriptor                */ \
                            + (9 + 6) * 1 /* Ext OUT - Emb IN               */ \
                            + (9 + 6) * 3 /* Ext IN - Emb OUT               */ \
                            + (9 + 5)     /* Bulk OUT End point descriptors */ \
                            + (9 + 7))    /* Bulk IN End point descriptors  */

uint8_t const USB_MIDI_HSConfigDescriptor[] = {
  /** Configuration 1 */
  USB_CONFIGURATION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(9 + 9 + 9 + 9 + CS_MS_IFC_DESC_LEN),
  0x02,                    /* bNumInterfaces */
  0x01,                    /* bConfigurationValue: 0x01 is used to select this configuration */
  0x00,                    /* iConfiguration: no string to describe this configuration */
  USB_CONFIG_SELF_POWERED, /* bmAttributes */
  USB_CONFIG_POWER_MA(0),  /* bMaxPower, device power consumption is 200 mA */

  /** Interface 0: Standard Audio Control interface */
  USB_INTERFACE_DESC_SIZE,
  USB_INTERFACE_DESCRIPTOR_TYPE,
  0x00,                       /* bInterfaceNumber: 0 */
  0x00,                       /* bAlternateSetting */
  0x00,                       /* bNumEndPoints: no endpoints */
  USB_DEVICE_CLASS_AUDIO,     /* bInterfaceClass: Audio */
  USB_SUBCLASS_AUDIO_CONTROL, /* bInterfaceSubClass: AudioControl */
  0x00,                       /* bInterfaceProtocol: no protocol */
  0x00,                       /* iInterface: no string desc */

  /** CS Interface descriptor - Audio Control */
  USB_CS_AUDIO_CONTROL_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_HEADER_SUBTYPE,
  WBVAL(BCDADC_1_0),
  WBVAL(USB_CS_AUDIO_CONTROL_TOTAL),
  0x01,
  0x01,

  /** Interface 1: standard midistreaming interface descriptor */
  USB_INTERFACE_DESC_SIZE,
  USB_INTERFACE_DESCRIPTOR_TYPE,
  0x01,                        /* bInterfaceNumber: 1 */
  0x00,                        /* bAlternateSetting */
  0x02,                        /* bNumEndPoints: 2 */
  USB_DEVICE_CLASS_AUDIO,      /* bInterfaceClass: Audio */
  USB_SUBCLASS_MIDI_STREAMING, /* bInterfaceSubclass: MIDI Streaming */
  0x00,                        /* bInterfaceProtocol: no protocol */
  0x00,                        /* iInterface: no string desc */

  /** CS Interface descriptor - MIDI Streaming */
  USB_CS_MIDI_STREAMING_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_HEADER_SUBTYPE,
  WBVAL(BCDADC_1_0),
  WBVAL(CS_MS_IFC_DESC_LEN),

  //
  // Patch Board
  //

  // Outputs

  // --- Ext OUT Jack 01 -- Emb IN Jack 02 ---
  /** 01 MIDI OUT jack - external */
  USB_MIDI_OUT_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_OUT_JACK_SUBTYPE,
  USB_MIDI_JACK_EXTERNAL,
  0x01,  // bJackID
  0x01,  // bNrInputPins
  0x02,  // BaSourceID(2)
  0x01,  // BaSourcePin(1)
  0x00,
  /** 02 MIDI IN jack - embedded */
  USB_MIDI_IN_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_IN_JACK_SUBTYPE,
  USB_MIDI_JACK_EMBEDDED,
  0x02,  // bJackID
  0x00,  // iJack

  // Inputs

  // --- Ext IN Jack 11 -- Emb OUT Jack 12 ---
  /** 11 MIDI IN jack - external */
  USB_MIDI_IN_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_IN_JACK_SUBTYPE,
  USB_MIDI_JACK_EXTERNAL,
  0x11,  // bJackID
  0x00,  // iJack
  /** 12 MIDI OUT jack - embedded */
  USB_MIDI_OUT_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_OUT_JACK_SUBTYPE,
  USB_MIDI_JACK_EMBEDDED,
  0x12,  // bJackID
  0x01,  // bNrInputPins
  0x11,  // BaSourceID(1)
  0x01,  // BaSourcePin(1)
  0x00,

  // --- Ext IN Jack 13 -- Emb OUT Jack 14 ---
  /** 11 MIDI IN jack - external */
  USB_MIDI_IN_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_IN_JACK_SUBTYPE,
  USB_MIDI_JACK_EXTERNAL,
  0x13,  // bJackID
  0x00,  // iJack
  /** 12 MIDI OUT jack - embedded */
  USB_MIDI_OUT_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_OUT_JACK_SUBTYPE,
  USB_MIDI_JACK_EMBEDDED,
  0x14,  // bJackID
  0x01,  // bNrInputPins
  0x13,  // BaSourceID(1)
  0x01,  // BaSourcePin(1)
  0x00,

  // --- Ext IN Jack 15 -- Emb OUT Jack 16 ---
  /** 11 MIDI IN jack - external */
  USB_MIDI_IN_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_IN_JACK_SUBTYPE,
  USB_MIDI_JACK_EXTERNAL,
  0x15,  // bJackID
  0x00,  // iJack
  /** 12 MIDI OUT jack - embedded */
  USB_MIDI_OUT_JACK_SIZE,
  USB_CS_INTERFACE_DESC_TYPE,
  USB_MIDI_OUT_JACK_SUBTYPE,
  USB_MIDI_JACK_EMBEDDED,
  0x16,  // bJackID
  0x01,  // bNrInputPins
  0x15,  // BaSourceID(1)
  0x01,  // BaSourcePin(1)
  0x00,

  /** Bulk OUT - standard endpoint descriptor */
  USB_AUDIO_ENDPOINT_DESC_SIZE,
  USB_ENDPOINT_DESCRIPTOR_TYPE,
  USB_ENDPOINT_OUT(MIDI_EP_OUT),
  USB_ENDPOINT_TYPE_BULK,
  WBVAL(USB_HS_BULK_SIZE),
  0x00,
  0x00,
  0x00,

  // Emb IN Jack 0x02  ----+----- bulk OUT endpoint

  /** Bulk OUT - class-specific endpoint descriptor */
  5,  // bLength
  USB_CS_ENDPOINT_DESC_TYPE,
  USB_MIDI_EP_GENERAL_SUBTYPE,
  0x01,  // bNumEmbMIDIJack
  0x02,  // BaAssocJackID(2)

  /** BULK IN - standard endpoint descriptor */
  USB_AUDIO_ENDPOINT_DESC_SIZE,
  USB_ENDPOINT_DESCRIPTOR_TYPE,
  USB_ENDPOINT_IN(MIDI_EP_IN),
  USB_ENDPOINT_TYPE_BULK,
  WBVAL(USB_HS_BULK_SIZE),
  0x00,
  0x00,
  0x00,

  // Emb OUT Jack 0x12  ----+
  // Emb OUT Jack 0x14  ----+----- bulk IN endpoint
  // Emb OUT Jack 0x16  ----+

  /** Bulk IN - class-specific endpoint descriptor */
  7,  //bLength
  USB_CS_ENDPOINT_DESC_TYPE,
  USB_MIDI_EP_GENERAL_SUBTYPE,
  0x03,  // bNumEmbMIDIJack
  0x12,  // BaAssocJackID(12)
  0x14,  // BaAssocJackID(14)
  0x16,  // BaAssocJackID(16)

  /* Terminator */
  0
};

//
//-------------------------------------------------------------------------
//

/* USB Device Qualifier */
uint8_t const USB_MIDI_DeviceQualifier[] = {
  USB_DEVICE_QUALI_SIZE,                /* bLength */
  USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */             /* bcdUSB */
  0x00,                                 /* bDeviceClass */
  0x00,                                 /* bDeviceSubClass */
  0x00,                                 /* bDeviceProtocol */
  USB_MAX_PACKET0,                      /* bMaxPacketSize0 */
  0x01,                                 /* bNumOtherSpeedConfigurations */
  0x00                                  /* bReserved */
};

// clang format off
/* USB String Descriptor (optional) */
uint8_t USB0_MIDI_StringDescriptor[] = {
  /* Index 0x00: LANGID Codes */
  0x04,                           /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
  WBVAL(0x0409), /* US English */ /* wLANGID */
  /* Index 0x01: Manufacturer */
  (36 * 2 + 2),               /* bLength (36 x 16bitChar + Type + length) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'N', 0, 'o', 0, 'n', 0, 'l', 0, 'i', 0, 'n', 0, 'e', 0, 'a', 0, 'r', 0,
  'L', 0, 'a', 0, 'b', 0, 's', 0,
  '(', 0, 'w', 0, 'w', 0, 'w', 0, '.', 0, 'n', 0, 'o', 0, 'n', 0, 'l', 0, 'i', 0, 'n', 0, 'e', 0, 'a', 0, 'r', 0,
  '-', 0, 'l', 0, 'a', 0, 'b', 0, 's', 0, '.', 0, 'd', 0, 'e', 0, ')', 0,
  /* Index 0x02: Product */
  (25 * 2 + 2),               /* ???? bLength ( 24 Char + Type + length) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'N', 0, 'L', 0, 'L', 0, '-', 0, 'L', 0, 'P', 0, 'C', 0, '-', 0, 'B', 0, 'r', 0, 'i', 0, 'd', 0, 'g', 0, 'e', 0, ' ', 0,
  '(', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, ')', 0,
  0, 0
};

uint8_t USB1_MIDI_StringDescriptor[] = {
  /* Index 0x00: LANGID Codes */
  0x04,                           /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
  WBVAL(0x0409), /* US English */ /* wLANGID */
  /* Index 0x01: Manufacturer */
  (36 * 2 + 2),               /* bLength (36 x 16bitChar + Type + length) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'N', 0, 'o', 0, 'n', 0, 'l', 0, 'i', 0, 'n', 0, 'e', 0, 'a', 0, 'r', 0,
  'L', 0, 'a', 0, 'b', 0, 's', 0,
  '(', 0, 'w', 0, 'w', 0, 'w', 0, '.', 0, 'n', 0, 'o', 0, 'n', 0, 'l', 0, 'i', 0, 'n', 0, 'e', 0, 'a', 0, 'r', 0,
  '-', 0, 'l', 0, 'a', 0, 'b', 0, 's', 0, '.', 0, 'd', 0, 'e', 0, ')', 0,
  /* Index 0x02: Product */
  (18 * 2 + 2),               /* ???? bLength ( 17 Char + Type + length) */
  USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
  'N', 0, 'L', 0, 'L', 0, '-', 0, 'C', 0, '1', 0, '6', 0, ' ', 0,
  '(', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, 'x', 0, ')', 0,
  0, 0
};
// clang format on

void USB_MIDI_SetupDescriptors(void)
{
  char const    hexToCharTable[16] = "0123456789ABCDEF";
  iapUniqueId_t id;
  if (iapGetUniqueId(&id))
  {
    uint32_t idVal = id.data[0] ^ id.data[1] ^ id.data[2] ^ id.data[3];
    for (int i = 0; i < 8; i++, idVal >>= 4)
    {
      unsigned index;
      index                             = sizeof(USB0_MIDI_StringDescriptor) - 20 + (unsigned) (2 * i);
      USB0_MIDI_StringDescriptor[index] = hexToCharTable[idVal & 0xF];

      index                             = sizeof(USB1_MIDI_StringDescriptor) - 20 + (unsigned) (2 * i);
      USB1_MIDI_StringDescriptor[index] = hexToCharTable[idVal & 0xF];
    }
  }
}
