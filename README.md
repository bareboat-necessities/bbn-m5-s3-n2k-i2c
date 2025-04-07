# bbn-m5-s3-n2k-i2c
NMEA 2000 sensors for M5 atomS3-lite

## Hardware

- m5stack atomS3-lite: https://shop.m5stack.com/products/atoms3-lite-esp32s3-dev-kit
- m5stack ATOMIC PortABC Extension Base: https://shop.m5stack.com/products/atomic-portabc-extension-base
- m5stack CANBus Unit (CA-IS3050G) https://shop.m5stack.com/products/canbus-unitca-is3050g
- m5stack ENV III Unit with Temperature Humidity Air Pressure Sensor (SHT30+QMP6988)  https://shop.m5stack.com/products/env-iii-unit-with-temperature-humidity-air-pressure-sensor-sht30-qmp6988

## Accessories

- Waterproof ABS Plastic Electronic Enclosure Box Ip68, Clear Cover, Hinged with Base Plate and mounting brackets
- Panel mount NMEA-2000 connector with pig tails. M12 5 Pin Male Connector Cable, A Coded Straight Back Mount Cable
Waterproof Male Aviation Socket UnShielded Electrical Cable IP67 Sensor Receptacle Brand: FOWIUNYE https://www.amazon.com/Connector-Waterproof-UnShielded-Electrical-Receptacle/dp/B0CB6SCTJ1
- Cable glands
- M3 standoffs
- USB-C to USB-A cables with small support tang on USB-C end
- m5stack grove cables
- m5stack grove to Dupont cables: https://shop.m5stack.com/products/grove2dupont-conversion-cable-20cm-5pairs

## Flash

````
# shutdown signalk
sudo systemctl stop signalk

if [ -f bbn-flash-m5-s3-n2k-i2c.sh ]; then rm bbn-flash-m5-s3-n2k-i2c.sh; fi
wget https://raw.githubusercontent.com/bareboat-necessities/my-bareboat/refs/heads/master/m5stack-tools/bbn-flash-m5-s3-n2k-i2c.sh
chmod +x bbn-flash-m5-s3-n2k-i2c.sh
./bbn-flash-m5-s3-n2k-i2c.sh -p /dev/ttyACM0

````

## Other Bareboat Necessities Devices

Project Home: https://bareboat-necessities.github.io/

- Alarms Box: https://github.com/bareboat-necessities/bbn_alarms_A
- Engine Sensors Box: https://github.com/bareboat-necessities/bbn_sensors_hub_C
- Sensors Hub: https://github.com/bareboat-necessities/bbn_sensors_hub_AB
- NMEA N2K to USB: https://github.com/bareboat-necessities/bbn-m5-s3-n2k-usb
- Instruments Displays on esp32: https://github.com/bareboat-necessities/bbn-m5stack-tough
- Boat Heave Sensor: https://github.com/bareboat-necessities/bbn-wave-period-esp32
- I2C over USB for Linux: https://github.com/bareboat-necessities/bbn-i2c-over-usb

