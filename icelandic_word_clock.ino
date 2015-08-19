/* WORD CLOCK INSPIRED BY http://imgur.com/a/iMXmj for Kata */

#include <Wire.h>
#include <Adafruit_NeoPixel.h>

/* DEFINITIONS */
#define MODE_PIN 3
#define INCRIMENT_PIN 4

#define NUM_LEDS 130
#define NUM_BDAY_LEDS 17
#define LEDS_PIN 2

// the following are the pin definitons for the ground of the respective words
/*#define HALF Display1=Display1 | (1<<1)
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
*/

// letter array
#define LA(A) A, sizeof(A) / sizeof(int)

int BDAY[] = {58, 59, 60, 47, 46, 45, 44, 43, 42, 41, 40, 33, 34, 35, 36, 37, 38};

int KLUKKAN[] = {117, 118, 119, 120, 121, 122, 123};
int ER[] = {126, 127};

int HALF[] = {113, 114, 115, 116};
int YFIR[] =  {74, 75, 76, 77};
int IN[] = {73};
int MINUTUR[] = {65, 66, 67, 68, 69, 70, 71};
int H_TIU[] = {105, 106, 107};
int TUTTUGU[] = {92, 93, 94, 95, 96, 97, 98};
int OG[] = {100, 101};
int FIMMTAN[] = {84, 85, 86, 87, 88, 89, 90};
int H_FIMM[] = {79, 80, 81, 82}; 

int EITT[] = {29, 30, 31, 32};
int NIU[] = {26, 27, 28};
int ATTA[] = {48, 49, 50, 51};
int THRJU[] = {61, 62, 63, 64};
int ELLEFU[] = {52, 53, 54, 55, 56, 57};

int TVO[] = {13, 14, 15};
int TOLF[] = {19, 20, 21, 22};
int SEX[] = {23, 24, 25};
int SJO[] = {16, 17, 18};
int FJOGUR[] = {5, 4, 3, 2, 1, 0};
int L_FIMM[] = {12, 11, 10, 9};
int L_TIU[] = {8, 7, 6};

#define MODE_OPERATE 0
#define MODE_SET_YEAR 1
#define MODE_SET_MONTH 2
#define MODE_SET_DATE 3
#define MODE_SET_HOUR 4
#define MODE_SET_MINUTE 5
#define MODE_TEST_BDAY 6

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
int brightness;
int led_states[NUM_LEDS];

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

Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS, LEDS_PIN, NEO_GRB + NEO_KHZ800);


void setup() {

	// programming pins
	pinMode(MODE_PIN, INPUT);
	pinMode(INCRIMENT_PIN, INPUT);
	digitalWrite(MODE_PIN, HIGH);
	digitalWrite(INCRIMENT_PIN, HIGH);

	leds.begin();

	Serial.begin(9600);
	while (! Serial);
	Serial.println("application start");

	Wire.begin();// start IC2 interface for communicating with Chronodot RTC module
	clear_ESOC_bit();
	// set_rtc_date(1, 07, 10, 15);
	// set_rtc_time(0, 21, 10);

	get_time();
	get_date();
	clear_leds();
	clear_rainbow();
	set_brightness();
	write_leds();
	word_test();
}

void loop() {

	bool birthday_condition = false;

	//print_rtc_datetime();

	clear_leds();
	check_buttons();

	if(incriment_pressdown_time < 0 || mode_pressdown_time < 0) {
	//Serial.println(incriment_pressdown_time);
	}
	if(mode != MODE_OPERATE) {
		program_time();

		if(mode != MODE_TEST_BDAY) {
			clear_rainbow();
			set_program_time_pins();
		} else {
			clear_leds();
		}
		write_leds();
		//Serial.println(mode);
	} else {

		get_date();
		get_time();
		master_loop_break = false;

		write_to_leds(LA(KLUKKAN));
		write_to_leds(LA(ER));
		set_time_pins();
		write_leds();

		// determine if we want to turn on birthday lights. first is just a general test, second is for illumination on xmas eve, third is for actual birthday
		if(year == 14) {
			if(hours % 2 == 0) {
				birthday_condition = true;
			}
			if(month == 10 && date > 23) {
				birthday_condition = false;
			}
		}

		if(month == 10 && date == 8) {
			birthday_condition = true;
		}

		if(birthday_condition) {
			rainbow_cycle(5);
		} else {
			clear_rainbow();
		}

		// fake_time_fastforward();
		// delay(800);
	}

	//Serial.println(millis() - last_loop);
	//last_loop = millis();
	cleanup_buttons();
	set_brightness();

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
			clear_leds();
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
		mode = generic_incriment(mode, 1, MODE_OPERATE, MODE_TEST_BDAY);
		
		master_loop_break = true;
		mode_release = false;

		set_rtc_date(1, date, month, year);
		set_rtc_time(0, minutes, hours);

		program_first_digit_display_time = 0;
		program_second_digit_display_time = 0;
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

void number_to_pin(int number) { // primarily used when programming/setting the time

	switch(number) {
	  case 0:
	    write_to_leds(LA(IN));
	    break;
	  case 1:
	    write_to_leds(LA(EITT));
	    break;
	  case 2:
	    write_to_leds(LA(TVO));
	    break;
	  case 3:
	    write_to_leds(LA(THRJU));
	    break;
	  case 4:
	    write_to_leds(LA(FJOGUR));
	    break;
	  case 5:
	    write_to_leds(LA(L_FIMM));
	    break;
	  case 6:
	    write_to_leds(LA(SEX));
	    break;
	  case 7:
	    write_to_leds(LA(SJO));
	    break;
	  case 8:
	    write_to_leds(LA(ATTA));
	    break;
	  case 9:
	    write_to_leds(LA(NIU));
	    break;
	  case 10:
	    write_to_leds(LA(L_TIU));
	    break;
	  case 11:
	    write_to_leds(LA(ELLEFU));
	    break;
	  case 12:
	    write_to_leds(LA(TOLF));
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

	if(mode == MODE_TEST_BDAY) {
	master_loop_break = false;
	clear_leds();
	write_leds();
	rainbow_cycle(5);
	}

}

void set_brightness() {
	if (hours >= 22 || hours <= 6) {
		brightness = 64;
	} else {
		brightness = 128;
	}
}

void set_program_time_pins() {
	unsigned int now = millis();

	if(program_first_digit_display_time == 0) {
	program_first_digit_display_time = now;
	//Serial.println("first digit");
	}
	number_to_pin(program_first_digit);

	if(program_first_digit_display_time > 0) {

	if(now - program_first_digit_display_time > PROGRAM_DIGIT_DISPLAY) {
		clear_leds();
		//Serial.println("in between");
	}

	if(now - program_first_digit_display_time > PROGRAM_DIGIT_DISPLAY + PROGRAM_DIGIT_DISPLAY_INBETWEEN) {

		if(program_second_digit_display_time == 0) {
		program_second_digit_display_time = now;
		}

		number_to_pin(program_second_digit);
		//Serial.println("second digit");

		if(now - program_second_digit_display_time > PROGRAM_DIGIT_DISPLAY) {
		clear_leds();
		//Serial.println("after second");

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
	  write_to_leds(LA(H_FIMM));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 9 && minutes < 15) {
	  write_to_leds(LA(H_TIU));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 14 && minutes < 20) {
	  write_to_leds(LA(FIMMTAN));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 19 && minutes < 25) {
	  write_to_leds(LA(TUTTUGU));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 24 && minutes < 30) {
	  write_to_leds(LA(TUTTUGU));
	  write_to_leds(LA(OG));
	  write_to_leds(LA(H_FIMM));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 29 && minutes < 35) {
	  write_to_leds(LA(HALF));
	}

	if(minutes > 34 && minutes < 40) {
	  write_to_leds(LA(TUTTUGU));
	  write_to_leds(LA(OG));
	  write_to_leds(LA(H_FIMM));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 39 && minutes < 45) {
	  write_to_leds(LA(TUTTUGU));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 44 && minutes < 50) {
	  write_to_leds(LA(FIMMTAN));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 49 && minutes < 55) {
	  write_to_leds(LA(H_TIU));
	  write_to_leds(LA(MINUTUR));
	}

	if(minutes > 54) {
	  write_to_leds(LA(H_FIMM));
	  write_to_leds(LA(MINUTUR));
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
	    write_to_leds(LA(TOLF));
	    break;
	  case 1:
	    write_to_leds(LA(EITT));
	    break;
	  case 2:
	    write_to_leds(LA(TVO));
	    break;
	  case 3:
	    write_to_leds(LA(THRJU));
	    break;
	  case 4:
	    write_to_leds(LA(FJOGUR));
	    break;
	  case 5:
	    write_to_leds(LA(L_FIMM));
	    break;
	  case 6:
	    write_to_leds(LA(SEX));
	    break;
	  case 7:
	    write_to_leds(LA(SJO));
	    break;
	  case 8:
	    write_to_leds(LA(ATTA));
	    break;
	  case 9:
	    write_to_leds(LA(NIU));
	    break;
	  case 10:
	    write_to_leds(LA(L_TIU));
	    break;
	  case 11:
	    write_to_leds(LA(ELLEFU));
	    break;
	  case 12:
	    write_to_leds(LA(TOLF));
	    break;
	}

	if ((minutes < 5) || (minutes > 29 && minutes < 35)) {
	  // do nothing here
	} else {
		if(minutes < 30) {
			write_to_leds(LA(YFIR));
		} else {
	  		write_to_leds(LA(IN));
	  	}
	}
}



void clear_leds() {
	uint16_t i;
	bool skip_color_set = false;

	for(i = 0; i < NUM_LEDS; i++) {
		led_states[i] = 0;
	}

	for(i = 0; i < NUM_LEDS; i++) {
		skip_color_set = false;
		
		for(int j = 0; j < NUM_BDAY_LEDS; j++) {
			if(BDAY[j] == i) {
				skip_color_set = true;
			}
		}

		if(!skip_color_set) {
			leds.setPixelColor(i, 0, 0, 0);
		}
	}

	// leds.show();
}

void word_test() {
	int word_wait = 150;

	Serial.println("begin word test");

	clear_leds();
	write_to_leds(LA(KLUKKAN));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(ER));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(HALF));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(H_TIU));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(TUTTUGU));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(OG));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(FIMMTAN));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(H_FIMM));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(MINUTUR));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(IN));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(YFIR));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(THRJU));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(ELLEFU));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(ATTA));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(EITT));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(NIU));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(TVO));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(SJO));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(TOLF));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(SEX));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(L_FIMM));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(L_TIU));
	write_leds();
	delay(word_wait);

	clear_leds();
	write_to_leds(LA(FJOGUR));
	write_leds();
	delay(word_wait);

	Serial.println("end word test");
}

void write_leds() {
	for(int i = 0; i < NUM_LEDS; i++) {
		if(led_states[i] == 1) {
			leds.setPixelColor(i, brightness * 255 / 255, brightness * 255 / 255, brightness * 255 / 255);
		}
	}

	leds.show();

	// digitalWrite(LATCH_PIN, LOW);
	// shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, Display3);
	// shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, Display2);
	// shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, Display1);
	// digitalWrite(LATCH_PIN, HIGH);
}

void write_to_leds(int letters[], int num_letters) {
	for(int i = 0; i < num_letters; i++) {
		led_states[letters[i]] = 1;
	}
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
