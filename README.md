# bbn-m5-s3-n2k-i2c
NMEA 2000 sensors for M5 atomS3-lite

## Hardware

- m5stack atomS3-lite: https://shop.m5stack.com/products/atoms3-lite-esp32s3-dev-kit
- m5stack ATOMIC PortABC Extension Base: https://shop.m5stack.com/products/atomic-portabc-extension-base
- m5stack CANBus Unit(CA-IS3050G) https://shop.m5stack.com/products/canbus-unitca-is3050g

## Accessories

- Waterproof ABS Plastic Enclosure Box Electronic Ip67 Flanged, Clear Cover, Size 100x68x50 (mm)  https://www.aliexpress.us/item/3256806147195874.html
- Panel mount NMEA-2000 connector with pig tails. M12 5 Pin Male Connector Cable, A Coded Straight Back Mount Cable
Waterproof Male Aviation Socket UnShielded Electrical Cable IP67 Sensor Receptacle Brand: FOWIUNYE https://www.amazon.com/Connector-Waterproof-UnShielded-Electrical-Receptacle/dp/B0CB6SCTJ1
- 

## Flash

````
# shutdown signalk
sudo systemctl stop signalk

if [ -f bbn-flash-m5-s3-n2k-i2c.sh ]; then rm bbn-flash-m5-s3-n2k-i2c.sh; fi
wget https://raw.githubusercontent.com/bareboat-necessities/my-bareboat/refs/heads/master/m5stack-tools/bbn-flash-m5-s3-n2k-i2c.sh
chmod +x bbn-flash-m5-s3-n2k-i2c.sh
./bbn-flash-m5-s3-n2k-i2c.sh -p /dev/ttyACM0

````
