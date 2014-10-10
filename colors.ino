
void rainbow_cycle(uint8_t wait) {
	uint16_t i, j;

	for(j = 0; j < 256*3; j++) {
		
		for(i = 0; i < bday_strip_size; i++) {
			bday_strip.setPixelColor(i, wheel(((i * 50 / bday_strip_size) + j) & 255));
			delayMicroseconds(550);

			// checks to see if break variable is set so we can exit and go into programming mode
			check_buttons();
			if(master_loop_break) {
				return;
			}
			//
		}


		bday_strip.show();
		delay(wait);
		
		// possibly an unneccesary hack, allows time to update while this function is running
		get_date();
		get_time();

		set_time_pins();
		write_leds();
		// end hack
	}

}

void clear_rainbow() {
	uint16_t i;

	for(i = 0; i < bday_strip_size; i++) {
		bday_strip.setPixelColor(i, 0, 0, 0);
	}

	bday_strip.show();
}

uint32_t wheel(byte WheelPos) {
	if(WheelPos < 85) {
		return bday_strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	else if(WheelPos < 170) {
		WheelPos -= 85;
		return bday_strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	else {
		WheelPos -= 170;
		return bday_strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}