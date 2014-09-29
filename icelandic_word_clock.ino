/* WORD CLOCK INSPIRED BY http://imgur.com/a/iMXmj for Kata */

#include <Wire.h>
#include <Adafruit_NeoPixel.h>



/* DEFINITIONS */

#define MODE_PIN 6
#define INCRIMENT_PIN 7

#define bday_strip_size 14
#define bday_strip_pin 12

//regsiter pins
#define CD4094_CLOCK    8
#define CD4094_DATA     9
#define CD4094_STROBE   10
#define CD4094_OUTPUT   11

// the following are the pin definitons for the ground of the respective words
#define HALF Display1=Display1 | (1<<0)
#define H_TIU Display1=Display1 | (1<<1)
#define TUTTUGU Display1=Display1 | (1<<2)
#define OG Display1=Display1 | (1<<3)
#define H_FIMM Display1=Display1 | (1<<5) 
#define FIMMTAN Display1=Display1 | (1<<4)
#define MINUTUR Display1=Display1 | (1<<6) 

#define IN Display2=Display2 | (1<<0)
#define YFIR Display2=Display2 | (1<<1)
#define THRJU Display2=Display2 | (1<<2)
#define ELLEFU Display2=Display2 | (1<<3)
#define ATTA Display2=Display2 | (1<<4)
#define EITT Display2=Display2 | (1<<5)
#define NIU Display2=Display2 | (1<<6)

#define SJO Display3=Display3 | (1<<0) 
#define SEX Display3=Display3 | (1<<1)
#define TOLF Display3=Display3 | (1<<2)
#define TVO Display3=Display3 | (1<<3)
#define L_TIU Display3=Display3 | (1<<4)
#define L_FIMM Display3=Display3 | (1<<5)
#define FJOGUR Display3=Display3 | (1<<6)

#define MODE_OPERATE 0
#define MODE_SET_YEAR 1
#define MODE_SET_MONTH 2
#define MODE_SET_DATE 3
#define MODE_SET_HOUR 4
#define MODE_SET_MINUTE 5

#define BUTTON_THRESHOLD 50
#define BUTTON_DECRIMENT_THRESHOLD 600

/* VARIABLE DECLERATIONS */

// these are the time variables, the rtc functions are in a seperate file. done this way to be able to easily plug in another source of time
int seconds;
int minutes;
int hours;
int day;
int date; // 0-31
int month;
int year; //0-99
int century;

//
byte Display1 = B00011000;
byte Display2 = B01000000;
byte Display3 = B00000001;

//
int mode = MODE_OPERATE;
unsigned int mode_pressdown_time = 0;
unsigned int incriment_pressdown_time = 0;
bool mode_release = false;
bool incriment_release = false;
bool decriment_release = false;


Adafruit_NeoPixel bday_strip = Adafruit_NeoPixel(bday_strip_size, bday_strip_pin, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(CD4094_CLOCK, OUTPUT);
  pinMode(CD4094_STROBE, OUTPUT);
  pinMode(CD4094_DATA, OUTPUT);
  pinMode(CD4094_OUTPUT, OUTPUT);

  pinMode(MODE_PIN, INPUT);
  pinMode(INCRIMENT_PIN, INPUT);
  digitalWrite(MODE_PIN, HIGH);
  digitalWrite(INCRIMENT_PIN, HIGH);

  digitalWrite(CD4094_OUTPUT, HIGH);

  bday_strip.begin();
  bday_strip.setBrightness(100);

  Serial.begin(9600);
  while (! Serial);
  Serial.println("application start");

  Wire.begin();// start IC2 interface for communicating with Chronodot RTC module
  clear_ESOC_bit();
  //set_rtc_date(6, 27, 9, 14); //COME BACK TO THIS, MAKE SURE YOU UNDERSTAND THE YEAR ASPECT OF THINGS BEFORE GIVING THIS TO KATA!!!!!!!!!!!!!!!
  //set_rtc_time(5, 50, 1);

  get_time();
  get_date();
  //minutes = 38;
}

void loop() {
  //get_date();
  //get_time();
  //fake_time_fastforward();
  //print_rtc_datetime();

  clear_leds();
  check_buttons();
  //MINUTUR;
  //IN;
  //YFIR;
  //THRJU;
  //NIU;
  //FJOGUR;
  //set_time_pins();
  //write_leds();
  //rainbowCycle(20);

  if(incriment_pressdown_time < 0 || mode_pressdown_time < 0) {
    Serial.println(incriment_pressdown_time);
  }
  if(mode != MODE_OPERATE) {
    program_time();
  }
  //Serial.println(Display1, BIN);
  //Serial.println(Display2, BIN);
  //Serial.println(Display3, BIN);

  cleanup_buttons();

  //delay(3000);

}

void check_buttons() {

  unsigned int now = millis();

  if(digitalRead(MODE_PIN) == LOW) {
    if(mode_pressdown_time == 0) {
      mode_pressdown_time = now;
    }
  } else {
    if(mode_pressdown_time > 0 && (now - mode_pressdown_time > BUTTON_THRESHOLD)) {
      mode_release = true;
    }
    mode_pressdown_time = 0;
  }

  if(digitalRead(INCRIMENT_PIN) == LOW) {
    if(incriment_pressdown_time == 0) {
      incriment_pressdown_time = now;
    }
  } else {
    if(incriment_pressdown_time > 0) {
      if(now - incriment_pressdown_time > BUTTON_DECRIMENT_THRESHOLD) {
        decriment_release = true;
      }
      if(now - incriment_pressdown_time > BUTTON_THRESHOLD && !decriment_release) {
        incriment_release = true;
      }
    }
    incriment_pressdown_time = 0;
  }

  if(mode_release) {
    Serial.println("MODE CHANGE");
    mode = generic_incriment(mode, 1, MODE_OPERATE, MODE_SET_MINUTE);
  }
  if(incriment_release) {
    Serial.println("INCRIMENT");
  }
  if(decriment_release) {
    Serial.println("DECRIMENT");
  }

}

void cleanup_buttons() {
  mode_release = false;
  incriment_release = false;
  decriment_release = false;
}

void fake_time_fastforward() {
  minutes = minute_incriment(minutes, 5);
  if(minutes == 0) {
    hours = hour_incriment(hours, 1);
  }
}

void program_time() {

  int incriment_amount = 0;

  if(incriment_release) {
    incriment_amount = 1;
  }
  if(decriment_release) {
    incriment_amount = -1;
  }

  if(mode == MODE_SET_YEAR) {
    year = generic_incriment(year, incriment_amount, 0, 99);
  }

  if(mode == MODE_SET_MONTH) {
    month = generic_incriment(month, incriment_amount, 1, 12);
    seconds = 0;
  }

  if(mode == MODE_SET_DATE) {
    date = generic_incriment(date, incriment_amount, 1, 31);
    seconds = 0;
  }

  if(mode == MODE_SET_HOUR) {
    hours = hour_incriment(hours, incriment_amount);
    seconds = 0;
  }

  if(mode == MODE_SET_MINUTE && incriment_amount != 0) {
    int minutes_incriment = 5 - (minutes % 5);
    if(incriment_amount < 0) {
      minutes_incriment -= 5;
      if(minutes_incriment == 0) {
        minutes_incriment = -5;
      }
    }
    minutes = minute_incriment(minutes, minutes_incriment);
    if(minutes == 59 && incriment_amount < 0) {
      minutes = 55;
    }
  }

}

void set_time_pins() {
  if(minutes > 4 && minutes < 10) {
    H_FIMM;
    MINUTUR;
  }

  if(minutes > 9 && minutes < 15) {
    H_TIU;
    MINUTUR;
  }

  if(minutes > 14 && minutes < 20) {
    FIMMTAN;
    MINUTUR;
  }

  if(minutes > 19 && minutes < 25) {
    TUTTUGU;
    MINUTUR;
  }

  if(minutes > 24 && minutes < 30) {
    TUTTUGU;
    OG;
    H_FIMM;
    MINUTUR;
  }

  if(minutes > 29 && minutes < 35) {
    HALF;
  }

  if(minutes > 34 && minutes < 40) {
    TUTTUGU;
    OG;
    H_FIMM;
    MINUTUR;
  }

  if(minutes > 39 && minutes < 45) {
    TUTTUGU;
    MINUTUR;
  }

  if(minutes > 44 && minutes < 50) {
    FIMMTAN;
    MINUTUR;
  }

  if(minutes > 49 && minutes < 55) {
    H_TIU;
    MINUTUR;
  }

  if(minutes > 54) {
    H_FIMM;
    MINUTUR;
  }

  int working_hour = hours;
  if (minutes > 29) {
    working_hour = hour_incriment(working_hour, 1);
  }
  if (working_hour > 12) {
    working_hour -= 12;
  }

  switch(working_hour) {
    case 1:
      EITT;
      break;
    case 2:
      TVO;
      break;
    case 3:
      THRJU;
      break;
    case 4:
      FJOGUR;
      break;
    case 5:
      L_FIMM;
      break;
    case 6:
      SEX;
      break;
    case 7:
      SJO;
      break;
    case 8:
      ATTA;
      break;
    case 9:
      NIU;
      break;
    case 10:
      L_TIU;
      break;
    case 11:
      ELLEFU;
      break;
    case 12:
      TOLF;
      break;
  }

  if ((minutes < 5) || (minutes > 29 && minutes < 35)) {
    //
  } else {
    if(minutes < 30) {
     YFIR;
    } else {
      IN;
    }
  }
}

void clear_leds() {
  Display1 = 0;
  Display2 = 0;
  Display3 = 0;
}

void write_leds() {

  //digitalWrite(CD4094_OUTPUT, LOW);
  digitalWrite(CD4094_STROBE, LOW);
  shiftOut(CD4094_DATA, CD4094_CLOCK, MSBFIRST, Display3);
  shiftOut(CD4094_DATA, CD4094_CLOCK, MSBFIRST, Display2);
  shiftOut(CD4094_DATA, CD4094_CLOCK, MSBFIRST, Display1);
  //digitalWrite(CD4094_OUTPUT, HIGH);
  digitalWrite(CD4094_STROBE, HIGH);
}

int hour_incriment(int working_hour, int incriment) {
  return generic_incriment(working_hour, incriment, 0, 23);
}

// assumes 5 minute incriments, sorta
int minute_incriment(int working_minute, int incriment) {
  return generic_incriment(working_minute, incriment, 0, 59);
}

// mostly assumes an incriment of 1 based on the roll over logic
int generic_incriment(int working_value, int incriment, int low_value, int high_value) {
  working_value += incriment;

  if(working_value > high_value) {
    working_value = low_value;
  }
  if(working_value < low_value) {
    working_value = high_value;
  }

  return working_value;
}

void get_date() {
  get_rtc_date();
}

void get_time() {
  get_rtc_time();
}
