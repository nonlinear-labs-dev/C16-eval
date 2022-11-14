#pragma once

#include "usb/usbWriter_MidiSysex.h"
#include "usb/usbWriter_Bridge.h"
#include "usb/driver/nl_usb_descmidi.h"
#include "usb/driver/nl_usb_midi.h"
namespace Task
{

  static void initBridges(UsbWriter::BridgeWriter& bridgeToHost, UsbWriter::BridgeWriter& hostToBridge);

  class UsbProcess : public Task::Task
  {
    using Task::Task;

   private:
    UsbWriter::MidiSysexWriter& m_sensorAndKeyEventWriter;
    UsbWriter::BridgeWriter&    m_bridgeToHostWriter;
    UsbWriter::BridgeWriter&    m_hostToBridgeWriter;

   public:
    constexpr UsbProcess(UsbWriter::MidiSysexWriter& midiWriter,
                         UsbWriter::BridgeWriter&    bridgeToHostWriter,
                         UsbWriter::BridgeWriter&    hostToBridgeWriter)
        : m_sensorAndKeyEventWriter(midiWriter)
        , m_bridgeToHostWriter(bridgeToHostWriter)
        , m_hostToBridgeWriter(hostToBridgeWriter)
    {
      initBridges(bridgeToHostWriter, hostToBridgeWriter);
    };

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    // run unconditionally
    inline void run(void) const
    {
      m_sensorAndKeyEventWriter.process();
      m_bridgeToHostWriter.process();
      m_hostToBridgeWriter.process();
    };

  };  // class UsbProcess

  // ---- helper functions ----

  static UsbWriter::BridgeWriter* pBridgeToHostAccess;
  static UsbWriter::BridgeWriter* pHostToBridgeAccess;

  // receive from Bridge Callbacks, calling the object functions
  static void Receive_IRQ_Callback_USB1(uint8_t const port, uint8_t const* const buff, uint32_t const len)
  {
    pBridgeToHostAccess->onReceive(buff, (uint16_t) len);
  }

  static void Receive_IRQ_Callback_USB0(uint8_t const port, uint8_t const* const buff, uint32_t const len)
  {
    pHostToBridgeAccess->onReceive(buff, (uint16_t) len);
  }

  static void initBridges(void)
  {
    USB_MIDI_Config(0, Receive_IRQ_Callback_USB0);
    USB_MIDI_Config(1, Receive_IRQ_Callback_USB1);
    USB_MIDI_SetupDescriptors();
    USB_MIDI_Init(0);
    USB_MIDI_Init(1);
  }

  static void initBridges(UsbWriter::BridgeWriter& bridgeToHost, UsbWriter::BridgeWriter& hostToBridge)
  {
    pBridgeToHostAccess = &bridgeToHost;
    pHostToBridgeAccess = &hostToBridge;
    initBridges();
  }

}  // namespace
