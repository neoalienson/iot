import serial
import time
import urllib2
import ConfigParser
import json

url = "https://api.thingspeak.com/update?api_key={}&field1={}&field2={}"

class MHZ14A():
    packet = [0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79]
    zero = [0xff, 0x87, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf2]

    def __init__(self,ser="/dev/serial0"):
      self.serial = serial.Serial(ser,9600,timeout=1)
      time.sleep(2)


    def get(self):
      retry = 5
      while True:
        retry -= 1
        if retry == 0: return -1, -1
        self.serial.write(bytearray(self.packet))
        res = self.serial.read(size=9)
        res = bytearray(res)
        if len(res) == 9:
          checksum = 0xff & (~(res[1]+res[2]+res[3]+res[4]+res[5]+res[6]+res[7])+1)
          if res[8] == checksum:
            co2 = (res[2]<<8)|res[3]
            humidity = res[4]
            return co2, humidity
        time.sleep(2)

    def close(self):
      self.serial.close


def main():
  config = ConfigParser.RawConfigParser()
  config.read("config.ini")
  apikey = config.get('thingspeak', 'api_key')

  co2_device = MHZ14A("/dev/serial0")
  co2, humidity = co2_device.get()
  if co2 > 0:
    print(json.dumps({"co2": co2, "humidity": humidity}))
  co2_device.close()

  try:
    if co2 > 0:
      urllib2.urlopen(url.format(apikey, co2, humidity)).read()
  except:
    pass

if __name__ == '__main__':
    main()
