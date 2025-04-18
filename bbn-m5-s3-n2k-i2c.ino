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
#include <ReactESP.h>  // https://github.com/mairas/ReactESP

using namespace reactesp;
ReactESP app;

#define ESP32_CAN_TX_PIN gpio_num_t(5)  // Set CAN TX port to 5 for M5ATOM-S3 CANBUS
#define ESP32_CAN_RX_PIN gpio_num_t(6)  // Set CAN RX port to 6 for M5ATOM-S3 CANBUS

#define CAN_TX_PIN ESP32_CAN_TX_PIN
#define CAN_RX_PIN ESP32_CAN_RX_PIN

#define I2C_FREQ 400000UL

#include "NMEA2000_esp32.h"
#include <N2kMessages.h>

int NodeAddress;  // To store last Node Address
tNMEA2000* nmea2000;

Preferences preferences;  // Nonvolatile storage on ESP32 - To store LastDeviceAddress

#include "i2c_sensors.h"

static const char* firmware_tag = "bbn-m5-s3-n2k-i2c";

#define ENABLE_DEBUG_LOG 0  // Debug log

// Set the information for other bus devices, which messages we support

const unsigned long TransmitMessages[] PROGMEM = { 
  130310L,  // Outside Environmental parameters
  0
};

void debug_log(char* str) {
#if ENABLE_DEBUG_LOG == 1
  Serial.println(str);
#endif
}

void setup() {
  AtomS3.begin(true);
  AtomS3.dis.setBrightness(100);

  Wire1.begin(G38, G39, I2C_FREQ);

  // Init USB serial port
  Serial.begin(38400);
  delay(10);
  gen_nmea0183_msg("$BBTXT,01,01,01,FirmwareTag: %s", firmware_tag);

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

  i2c_sensors_scan();
}

void loop() {
  AtomS3.update();
  
  app.tick();

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
