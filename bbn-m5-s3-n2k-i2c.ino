/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// NMEA2000 Temperature and Barometric Pressure
// Based on Version 0.2, 29.08.2020, AK-Homberger
//
// Edited by mgrouch for m5atomS3 with sht30 and qmp6988 sensors

// For Can CAIS3050G module. Powered by USB
// Can isolated (so connect only H and L)

#include <M5AtomS3.h>
#include <Arduino.h>
#include <Wire.h>
#include <M5UnitENV.h>
#include <Preferences.h>
#include <esp_mac.h>

#define ESP32_CAN_TX_PIN gpio_num_t(5)  // Set CAN TX port to 5 for M5ATOM-S3 CANBUS
#define ESP32_CAN_RX_PIN gpio_num_t(6)  // Set CAN RX port to 6 for M5ATOM-S3 CANBUS

#define CAN_TX_PIN ESP32_CAN_TX_PIN
#define CAN_RX_PIN ESP32_CAN_RX_PIN

#include "NMEA2000_esp32.h"
#include <N2kMessages.h>

SHT3X sht30;
QMP6988 qmp6988;

#define ENABLE_DEBUG_LOG 0  // Debug log

int NodeAddress;  // To store last Node Address

tNMEA2000* nmea2000;

double Temperature = 0;
double BarometricPressure = 0;
double Humidity = 0;

Preferences preferences;  // Nonvolatile storage on ESP32 - To store LastDeviceAddress

// Set the information for other bus devices, which messages we support

const unsigned long TransmitMessages[] PROGMEM = { 130310L,  // Outside Environmental parameters
                                                   0
                                                 };
// Send time offsets
#define TempSendOffset 0

#define SlowDataUpdatePeriod 250  // Time between CAN Messages sent

static bool led_state = false;

void ToggleLed() {
  if (led_state) {
    AtomS3.dis.drawpix(0x00ff00);
  } else {
    AtomS3.dis.drawpix(0x000000);
  }
  AtomS3.update();
  led_state = !led_state;
}

void debug_log(char* str) {
#if ENABLE_DEBUG_LOG == 1
  Serial.println(str);
#endif
}

void setup() {
  AtomS3.begin(true);
  AtomS3.dis.setBrightness(100);

  // Init USB serial port
  Serial.begin(115200);
  delay(10);

  if (!qmp6988.begin(&Wire1, QMP6988_SLAVE_ADDRESS_L, G38, G39, 400000U)) {
    while (1) {
      Serial.println("Couldn't find QMP6988");
      delay(500);
    }
  }

  if (!sht30.begin(&Wire1, SHT3X_I2C_ADDR, G38, G39, 400000U)) {
    while (1) {
      Serial.println("Couldn't find SHT3X");
      delay(500);
    }
  }

  // instantiate the NMEA2000 object
  nmea2000 = new tNMEA2000_esp32(CAN_TX_PIN, CAN_RX_PIN);

  // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega

  nmea2000->SetN2kCANMsgBufSize(8);
  nmea2000->SetN2kCANReceiveFrameBufSize(250);
  nmea2000->SetN2kCANSendFrameBufSize(250);

  uint8_t chipid[6];
  uint32_t id = 0;
  int i = 0;
  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

  // Set product information
  nmea2000->SetProductInformation("00001",                          // Manufacturer's Model serial code
                                  100,                              // Manufacturer's product code
                                  "BBN Env Sensor Module m5a-S3L",  // Manufacturer's Model ID
                                  "1.0.2.25 (2023-05-27)",          // Manufacturer's Software version code
                                  "1.0.2.0 (2023-05-27)"            // Manufacturer's Model version
                                 );
  // Set device information
  nmea2000->SetDeviceInformation(id,   // Unique number. Use e.g. Serial number.
                                 130,  // Device function=Temperature. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                 75,   // Device class=Sensor Communication Interface. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                 2046  // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                                );

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below

  //nmea2000->SetForwardType(tNMEA2000::fwdt_Text);  // Show in clear text. Leave uncommented for default Actisense format.

  preferences.begin("nvs", false);                          // Open nonvolatile storage (nvs)
  NodeAddress = preferences.getInt("LastNodeAddress", 37);  // Read stored last NodeAddress, default 35
  preferences.end();
  //Serial.printf("NodeAddress=%d\n", NodeAddress);

  nmea2000->SetMode(tNMEA2000::N2km_NodeOnly, NodeAddress);
  // Disable all msg forwarding to USB (=Serial)
  nmea2000->EnableForward(false);
  nmea2000->ExtendTransmitMessages(TransmitMessages);

  nmea2000->Open();

  delay(200);
}

bool IsTimeToUpdate(unsigned long NextUpdate) {
  return (NextUpdate < millis());
}

unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset = 0) {
  return millis() + Period + Offset;
}

void SetNextUpdate(unsigned long& NextUpdate, unsigned long Period) {
  while (NextUpdate < millis()) NextUpdate += Period;
}

void SendN2kTempPressure(void) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, TempSendOffset);
  tN2kMsg N2kMsg;

  if (IsTimeToUpdate(SlowDataUpdated)) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    if (qmp6988.update()) {
      BarometricPressure = qmp6988.calcPressure();
    }
    if (sht30.update()) {       // Obtain the data of SHT30.
      Temperature = sht30.cTemp;  // Store the temperature obtained from SHT30.
      Humidity = sht30.humidity;  // Store the humidity obtained from the SHT30.
    } else {
      Temperature = 0, Humidity = 0;
    }

    ToggleLed();
    Serial.printf("Temperature: %3.1f °C - Barometric Pressure: %6.0f Pa\n", Temperature, BarometricPressure);

    SetN2kPGN130310(N2kMsg, 0, N2kDoubleNA, CToKelvin(Temperature), BarometricPressure);
    nmea2000->SendMsg(N2kMsg);
    SetN2kPGN130313(N2kMsg, 0, 0, tN2kHumiditySource::N2khs_InsideHumidity, Humidity);
    nmea2000->SendMsg(N2kMsg);
  }
}

void loop() {
  AtomS3.update();
  
  SendN2kTempPressure();

  nmea2000->ParseMessages();
  int SourceAddress = nmea2000->GetN2kSource();
  if (SourceAddress != NodeAddress) {  // Save potentially changed Source Address to NVS memory
    NodeAddress = SourceAddress;       // Set new Node Address (to save only once)
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    //Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }

  // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
  if (Serial.available()) {
    Serial.read();
  }
}
