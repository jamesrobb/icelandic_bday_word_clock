/* WORD CLOCK INSPIRED BY http://imgur.com/a/iMXmj for Kata */

#include <Wire.h>
#include <Adafruit_NeoPixel.h>



/* DEFINITIONS */
#define MODE_PIN 2
#define INCRIMENT_PIN 3

#define bday_strip_size 14
#define bday_strip_pin 12

//regsiter pins
#define CLOCK_PIN    8
#define DATA_PIN     9
#define LATCH_PIN   10
#define OUTPUT_PIN   11

// the following are the pin definitons for the ground of the respective words
#define HALF Display1=Display1 | (1<<1)
#define YFIR Display1=Display1 | (1<<2) // not wired in right order
#define H_TIU Display1=Display1 | (1<<3)
#define TUTTUGU Display1=Display1 | (1<<4)
#define OG Display1=Display1 | (1<<5)
#define FIMMTAN Display1=Display1 | (1<<6)
#define H_FIMM Display1=Display1 | (1<<7) 

#define EITT Display2=Display2 | (1<<1)
#define NIU Display2=Display2 | (1<<2)
#define ATTA Display2=Display2 | (1<<3)
#define THRJU Display2=Display2 | (1<<4)
#define ELLEFU Display2=Display2 | (1<<5)
#define MINUTUR Display2=Display2 | (1<<6) // not wired in right order
#define IN Display2=Display2 | (1<<7)

#define TVO Display3=Display3 | (1<<1)
#define TOLF Display3=Display3 | (1<<2)
#define SEX Display3=Display3 | (1<<3)
#define SJO Display3=Display3 | (1<<4)
#define FJOGUR Display3=Display3 | (1<<5)
#define L_FIMM Display3=Display3 | (1<<6)
#define L_TIU Display3=Display3 | (1<<7)

#define MODE_OPERATE 0
#define MODE_SET_YEAR 1
#define MODE_SET_MONTH 2
#define MODE_SET_DATE 3
#define MODE_SET_HOUR 4
#define MODE_SET_MINUTE 5

#define BUTTON_THRESHOLD 50
#define BUTTON_DECRIMENT_THRESHOLD 600

#define PROGRAM_DIGIT_DISPLAY 200
#define PROGRAM_DIGIT_DISPLAY_INBETWEEN 200

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

unsigned long mode_pressdown_time = 0;
unsigned long incriment_pressdown_time = 0;
bool mode_button_down = false;
bool incriment_button_down = false;
bool mode_release = false;
bool incriment_release = false;
bool decriment_release = false;

int program_first_digit = 0;
int program_second_digit = 0;
unsigned long program_first_digit_display_time = 0;
unsigned long program_second_digit_display_time = 0;

bool master_loop_break = false;

unsigned long last_loop = 0;

Adafruit_NeoPixel bday_strip = Adafruit_NeoPixel(bday_strip_size, bday_strip_pin, NEO_GRB + NEO_KHZ800);

void setup() {
  // shift register stuff
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(OUTPUT_PIN, OUTPUT);

  // programming pins
  pinMode(MODE_PIN, INPUT);
  pinMode(INCRIMENT_PIN, INPUT);
  digitalWrite(MODE_PIN, HIGH);
  digitalWrite(INCRIMENT_PIN, HIGH);

  digitalWrite(OUTPUT_PIN, LOW);

  bday_strip.begin();
  bday_strip.setBrightness(100);

  Serial.begin(9600);
  while (! Serial);
  Serial.println("application start");

  Wire.begin();// start IC2 interface for communicating with Chronodot RTC module
  clear_ESOC_bit();
  //set_rtc_date(6, 27, 9, 14);
  //set_rtc_time(5, 50, 1);

  get_time();
  get_date();
  clear_leds();
  clear_rainbow();
  write_leds();
  //word_test();
}

void loop() {
  //print_rtc_datetime();

  clear_leds();
  check_buttons();

  if(incriment_pressdown_time < 0 || mode_pressdown_time < 0) {
    Serial.println(incriment_pressdown_time);
  }
  if(mode != MODE_OPERATE) {
    program_time();
    set_program_time_pins();
    write_leds();
  } else {
    get_date();
    get_time();
    master_loop_break = false;

    set_time_pins();
    write_leds();
    //fake_time_fastforward();
  }

  //Serial.println(millis() - last_loop);

  last_loop = millis();
  cleanup_buttons();

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
        program_first_digit_display_time = 0;
        program_second_digit_display_time = 0;
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
    master_loop_break = true;
    mode_release = false;
    set_rtc_date(1, date, month, year);
    set_rtc_time(seconds, minutes, hours);
  }
  if(incriment_release) {
    //Serial.println("INCRIMENT");
  }
  if(decriment_release) {
    //Serial.println("DECRIMENT");
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

void number_to_pin(int number) { // primarily used for programming the time

  switch(number) {
    case 0:
    IN;
      break;
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
    program_first_digit = year / 10;
    program_second_digit = year % 10;
  }

  if(mode == MODE_SET_MONTH) {
    month = generic_incriment(month, incriment_amount, 1, 12);
    seconds = 0;
    program_first_digit = month / 10;
    program_second_digit = month % 10;
  }

  if(mode == MODE_SET_DATE) {
    date = generic_incriment(date, incriment_amount, 1, 31);
    seconds = 0;
    program_first_digit = date / 10;
    program_second_digit = date % 10;
  }

  if(mode == MODE_SET_HOUR) {
    hours = hour_incriment(hours, incriment_amount);
    seconds = 0;
    program_first_digit = hours / 10;
    program_second_digit = hours % 10;
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
    seconds = 0;
  }

  if(mode == MODE_SET_MINUTE) {
    program_first_digit = minutes / 10;
    program_second_digit = minutes % 10;
  }

}

void set_program_time_pins() {
  unsigned int now = millis();

  if(program_first_digit_display_time == 0) {
    program_first_digit_display_time = now;
    Serial.println("first digit");
  }
  number_to_pin(program_first_digit);

  if(program_first_digit_display_time > 0) {

    if(now - program_first_digit_display_time > PROGRAM_DIGIT_DISPLAY) {
      clear_leds();
      Serial.println("in between");
    }

    if(now - program_first_digit_display_time > PROGRAM_DIGIT_DISPLAY + PROGRAM_DIGIT_DISPLAY_INBETWEEN) {

      if(program_second_digit_display_time == 0) {
        program_second_digit_display_time = now;
      }

      number_to_pin(program_second_digit);
      Serial.println("second digit");

      if(now - program_second_digit_display_time > PROGRAM_DIGIT_DISPLAY) {
        clear_leds();
        Serial.println("after second");

        if(now - program_second_digit_display_time > PROGRAM_DIGIT_DISPLAY + (PROGRAM_DIGIT_DISPLAY_INBETWEEN * 3.5)) {
          program_first_digit_display_time = 0;
          program_second_digit_display_time = 0;
        }
      }
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
    case 0:
      TOLF;
      break;
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

void word_test() {
  int word_wait = 200;

  Serial.println("begin word test");

  clear_leds();
  HALF;
  write_leds();
  delay(word_wait);

  clear_leds();
  H_TIU;
  write_leds();
  delay(word_wait);

  clear_leds();
  TUTTUGU;
  write_leds();
  delay(word_wait);

  clear_leds();
  OG;
  write_leds();
  delay(word_wait);

  clear_leds();
  FIMMTAN;
  write_leds();
  delay(word_wait);

  clear_leds();
  H_FIMM;
  write_leds();
  delay(word_wait);

  clear_leds();
  MINUTUR;
  write_leds();
  delay(word_wait);

  clear_leds();
  IN;
  write_leds();
  delay(word_wait);

  clear_leds();
  YFIR;
  write_leds();
  delay(word_wait);

  clear_leds();
  THRJU;
  write_leds();
  delay(word_wait);

  clear_leds();
  ELLEFU;
  write_leds();
  delay(word_wait);

  clear_leds();
  ATTA;
  write_leds();
  delay(word_wait);

  clear_leds();
  EITT;
  write_leds();
  delay(word_wait);

  clear_leds();
  NIU;
  write_leds();
  delay(word_wait);

  clear_leds();
  SJO;
  write_leds();
  delay(word_wait);

  clear_leds();
  SEX;
  write_leds();
  delay(word_wait);

  clear_leds();
  TOLF;
  write_leds();
  delay(word_wait);

  clear_leds();
  TVO;
  write_leds();
  delay(word_wait);

  clear_leds();
  L_TIU;
  write_leds();
  delay(word_wait);

  clear_leds();
  L_FIMM;
  write_leds();
  delay(word_wait);

  clear_leds();
  FJOGUR;
  write_leds();
  delay(word_wait);

  Serial.println("end word test");
}

void write_leds() {

  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, Display3);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, Display2);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, Display1);
  digitalWrite(LATCH_PIN, HIGH);
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
