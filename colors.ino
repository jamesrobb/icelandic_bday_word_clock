
void rainbow_cycle(uint8_t wait) {
	uint16_t i, j;

	for(j = 0; j < 256*3; j++) {
		
		for(i = 0; i < NUM_BDAY_LEDS; i++) {
			leds.setPixelColor(BDAY[i], wheel(((i * 256 / NUM_BDAY_LEDS) + j) & 255));
			//delayMicroseconds(550);

			// checks to see if break variable is set so we can exit and go into programming mode
			check_buttons();
			if(master_loop_break) {
				return;
			}
			//
		}


		leds.show();
		delay(wait);
		
		// possibly an unneccesary hack, allows time to update while this function is running
		//get_date();
		//get_time();

		//set_time_pins();
		//write_leds();
		// end hack
	}

}

//Theatre-style crawling lights with rainbow effect
void theater_rainbow_chase(uint8_t wait) {
	for (int j=0; j < 256; j++) { // cycle all 256 colors in the wheel

		for (int q=0; q < 3; q++) {

			for (int i=0; i < leds.numPixels(); i=i+3) {
				leds.setPixelColor(i+q, wheel( (i+j) % 255)); //turn every third pixel on
			}

			leds.show();
			delay(wait);

			for (int i=0; i < leds.numPixels(); i=i+3) {
				leds.setPixelColor(i+q, 0); //turn every third pixel off
			}
		}
	}
}

void clear_rainbow() {
	uint16_t i;

	for(i = 0; i < NUM_BDAY_LEDS; i++) {
		leds.setPixelColor(BDAY[i], 0, 0, 0);
	}

	//eds.show();
}

uint32_t wheel(byte WheelPos) {
	WheelPos = 255 - WheelPos;

	if(WheelPos < 85) {
		return leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	else if(WheelPos < 170) {
		WheelPos -= 85;
		return leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	else {
		WheelPos -= 170;
		return leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}