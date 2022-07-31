#include <SPI.h>
#include <NRFLite.h>
#include "CEC_Device.h"

#define DEBUG 0
#define NRF_ADDR 1
#define NRF_CE_PIN 7
#define NRF_CSN_PIN 8

#define CEC_IN_LINE 4
#define CEC_OUT_LINE 3

#define report(X) do { report ## X (); } while (0)

#define phy1 ((_physicalAddress >> 8) & 0xFF)
#define phy2 ((_physicalAddress >> 0) & 0xFF)

class MyCEC: public CEC_Device {
  public:
    MyCEC(int physAddr): CEC_Device(physAddr, CEC_IN_LINE, CEC_OUT_LINE) { }
    
    void reportPhysAddr()    { unsigned char frame[4] = { 0x84, phy1, phy2, 0x04 };        TransmitFrame(0x0F, frame, sizeof(frame)); } // Report physical address
    void reportStreamState() { unsigned char frame[3] = { 0x82, phy1, phy2 };              TransmitFrame(0x0F, frame, sizeof(frame)); } // Report stream state (playing)
    void reportPowerState()  { unsigned char frame[2] = { 0x90, 0x00 };                    TransmitFrame(0x00, frame, sizeof(frame)); } // Report power state (on)
    void reportCECVersion()  { unsigned char frame[2] = { 0x9E, 0x04 };                    TransmitFrame(0x00, frame, sizeof(frame)); } // Report CEC version (v1.3a)
    void reportOSDName()     { unsigned char frame[7] = { 0x47, 'C','L','A','U','D','E' }; TransmitFrame(0x00, frame, sizeof(frame)); } // FIXME: name hardcoded
    void reportVendorID()    { unsigned char frame[4] = { 0x87, 0x00, 0xF1, 0x0E };        TransmitFrame(0x00, frame, sizeof(frame)); } // Report fake vendor ID
    
    void OnReceive(int source, int dest, unsigned char* buffer, int count) {
      if (count == 0) return;
      switch (buffer[0]) {        
        case 0x36:
          /* standby */
          break;

        case 0x83:
          report(PhysAddr);
          break;

        case 0x86:
          if (buffer[1] == phy1 && buffer[2] == phy2) {
            report(StreamState);
          }
          break;

        case 0x8F:
          report(PowerState);
          break;

        case 0x9F:
          report(CECVersion);
          break;

        case 0x46:
          report(OSDName);
          break;

        case 0x8C:
          report(VendorID);
          break;

        default:
          CEC_Device::OnReceive(source,dest,buffer,count);
          break;
      }
    }
};

MyCEC cecDevice(0x1000);

unsigned long lastImageViewOn = millis();
bool srcChanged = true;

NRFLite nrf;

struct RadioData
{
  char action[16];
  char value[16];
};

RadioData radioData;

void setup()
{
  if (DEBUG) {
    Serial.begin(9600);
  }
  delay(1000);

  nrf.init(NRF_ADDR, NRF_CE_PIN, NRF_CSN_PIN);

  cecDevice.Initialize(CEC_LogicalDevice::CDT_PLAYBACK_DEVICE);

  if (DEBUG) {
    Serial.println("Showtime !");
  }
}

void loop()
{
  uint8_t packetSize = nrf.hasData();

  if (sizeof(RadioData) == packetSize) {
    nrf.readData(&radioData);

    String action = String(radioData.action);
    String value = String(radioData.value);

    if (DEBUG) {
      Serial.print("Received <");
      Serial.print(action);
      Serial.print(":");
      Serial.print(value);
      Serial.println(">");
      delay(5000);
    }

    if (action.equals("workday_start")) {
      tvPower(true);
    } else if (action.equals("workday_end")) {
      tvPower(false);
    }
  }

  if (!srcChanged && (lastImageViewOn + 15000) < millis()) {
    changeSrc();
  }

  cecDevice.Run();
}

void tvPower(bool state)
{
  unsigned char buffer[2] = { 0x00, 0x00 };

  if (state) {
    if (DEBUG) {
      Serial.println("CEC: Image view on");
    }

    srcChanged = false;
    lastImageViewOn = millis();

    // Image view on
    buffer[0] = 0x04;
    cecDevice.TransmitFrame(0, buffer, 1);

    return;
  }

  if (DEBUG) {
    Serial.println("CEC: standby");
  }

  // Standby
  buffer[0] = 0x36;
  cecDevice.TransmitFrame(0, buffer, 1);
}

void changeSrc()
{
  srcChanged = true;

  // Active Source, HDMI 2
  unsigned char buffer[3] = { 0x82, 0x20, 0x00 };

  if (DEBUG) {
    Serial.println("CEC: Active source");
  }

  cecDevice.TransmitFrame(15, buffer, 3);
}
