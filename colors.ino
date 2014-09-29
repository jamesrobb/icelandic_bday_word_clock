
void rainbowCycle(uint8_t wait) {
	uint16_t i, j;

	for(j = 0; j < 256*3; j++) {
		
		for(i = 0; i < bday_strip_size; i++) {
			bday_strip.setPixelColor(i, Wheel(((i * 50 / bday_strip_size) + j) & 255));
			delayMicroseconds(550);
		}

		// for(i = 0; i < 3; i++) {
		// 	bday_strip.setPixelColor(i, Wheel(((i * 50 / bday_strip_size) + j) & 255));
		// 	delayMicroseconds(550);
		// }

		// for(i = 3; i < 10; i++) {
		// 	bday_strip.setPixelColor(i, Wheel((((i - 3) * 50 / bday_strip_size) + j) & 255));
		// 	delayMicroseconds(550);
		// }

		// for(i = 10; i < 15; i++) {
		// 	bday_strip.setPixelColor(i, Wheel((((i - 10) * 50 / bday_strip_size) + j) & 255));
		// 	delayMicroseconds(550);
		// }

		bday_strip.show();
		// delay(wait);
	}

}

uint32_t Wheel(byte WheelPos) {
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