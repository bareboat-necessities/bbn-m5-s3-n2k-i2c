# bbn-m5-s3-n2k-i2c
NMEA 2000 sensors for M5 atomS3-lite

## Flash

````
# shutdown signalk
sudo systemctl stop signalk

if [ -f bbn-flash-m5-s3-n2k-i2c.sh ]; then rm bbn-flash-m5-s3-n2k-i2c.sh; fi
wget https://raw.githubusercontent.com/bareboat-necessities/my-bareboat/refs/heads/master/m5stack-tools/bbn-flash-m5-s3-n2k-i2c.sh
chmod +x bbn-flash-m5-s3-n2k-i2c.sh
./bbn-flash-m5-s3-n2k-i2c.sh -p /dev/ttyACM0

````
