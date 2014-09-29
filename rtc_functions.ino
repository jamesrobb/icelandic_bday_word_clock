#include <Wire.h>

void get_rtc_date() {
  Wire.beginTransmission(104);
  Wire.write(3); //set register to 3 (day)
  Wire.endTransmission();
  Wire.requestFrom(104, 4); 
	
  day   = bcdToDec(Wire.read());
  date  = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year  = bcdToDec(Wire.read());
}

void set_rtc_date(int _day, int _date, int _month, int _year) {
  Wire.beginTransmission(104);
  Wire.write(3);
  Wire.write(decToBcd(_day));
  Wire.write(decToBcd(_date));
  Wire.write(decToBcd(_month));
  Wire.write(decToBcd(_year));
  Wire.endTransmission();
}

void get_rtc_time() {
  Wire.beginTransmission(104);
  Wire.write(0); //set register to 0
  Wire.endTransmission();
  Wire.requestFrom(104, 3);

  seconds = bcdToDec(Wire.read() & 0x7f);
  minutes = bcdToDec(Wire.read());
  hours = bcdToDec(Wire.read() & 0x3f);
}

void set_rtc_time(int _seconds, int _minutes, int _hours)
{
   Wire.beginTransmission(104);
   Wire.write(0);
   Wire.write(decToBcd(_seconds));
   Wire.write(decToBcd(_minutes));
   Wire.write(decToBcd(_hours));
   Wire.endTransmission();
}

byte decToBcd(byte val) {
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val) {
  return ( (val/16*10) + (val%16) );
}

void print_rtc_datetime() {
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.print(seconds);
  Serial.print("  ");

  Serial.print("day: ");
  Serial.print(day);
  Serial.print(" ");

  Serial.print("date: ");
  Serial.print(date);
  Serial.print(" ");

  Serial.print("month: ");
  Serial.print(month);
  Serial.print(" ");

  Serial.print("year: ");
  Serial.println(year);
}

void clear_ESOC_bit() {
  // need to make sure the device uses its battery when not being powered. only needs to be run once, but wont hurt to run multiple times
  Wire.beginTransmission(0x68); // address DS3231
  Wire.write(0x0E); // select register
  Wire.write(0b00011100); // write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();
}
