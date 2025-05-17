#ifndef HV518_cpp
#define HV518_cpp

#include "HV518.h"

HV518::HV518(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t numDigits, uint8_t numLines){
	initData(dataPin, clockPin, latchPin, 0, numDigits, numLines, true, false);
}

HV518::HV518(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t strobePin, uint8_t numDigits, uint8_t numLines){
	initData(dataPin, clockPin, latchPin, strobePin, numDigits, numLines, true, true);
}

HV518::HV518(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t numDigits, uint8_t numLines, bool leftAlignDisplay){
	initData(dataPin, clockPin, latchPin, 0, numDigits, numLines, leftAlignDisplay, false);
}

HV518::HV518(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t strobePin, uint8_t numDigits, uint8_t numLines, bool leftAlignDisplay){
	initData(dataPin, clockPin, latchPin, strobePin, numDigits, numLines, leftAlignDisplay, true);
}

void HV518::initData(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t strobePin, uint8_t numDigits, uint8_t numLines, bool leftAlignDisplay, bool hasStrobe){
	this->dataPin = dataPin;
	this->clockPin = clockPin;
	this->latchPin = latchPin;
	this->strobePin = strobePin;
	this->numDigits = numDigits;
	this->numLines = numLines;
	
	this->hasStrobe = hasStrobe;
	this->leftAlignDisplay = leftAlignDisplay;

	numRegisters = this->numLines / 8;
	displayState = new byte[numRegisters];
	
	initDisplay();
}

void HV518::initDisplay(){
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	if(hasStrobe){
		pinMode(strobePin, OUTPUT);
		analogWriteRange(255); // Use 8 bits for brightness
		digitalWrite(strobePin, LOW);
	}

	// Reset latch
	digitalWrite(clockPin, HIGH);
	digitalWrite(dataPin, LOW);
	digitalWrite(latchPin, LOW);
	digitalWrite(latchPin, HIGH);
	digitalWrite(latchPin, LOW);
}

byte HV518::getDigit(uint8_t pos){
	if(leftAlignDisplay && pos < numDigits){
		pos = numDigits - pos - 1;
	}
	return displayState[pos];
}

void HV518::setDigit(uint8_t pos, byte data){
	if(pos > numRegisters){
		return;
	}

	// Reverse position for digits when left aligned
	if(leftAlignDisplay && pos < numDigits){
		pos = numDigits - pos - 1;
	}

	displayState[pos] = data;
}

void HV518::updateDisplay(){
	for(uint8_t i = 0; i < numRegisters; i++){
		shiftOut(dataPin, clockPin, MSBFIRST, displayState[i]);
	}

	// Maybe replace with faster digital writing
	digitalWrite(latchPin, HIGH);
	digitalWrite(latchPin, LOW);
}

void HV518::clearDisplayMemory(){
	for(uint8_t i = 0; i < numRegisters; i++){
		displayState[i] = 0;
	}
}

void HV518::clearDisplay(){
	clearDisplayMemory();
	updateDisplay();
}

void HV518::setAllHigh(){
	for(uint8_t i = 0; i < numRegisters; i++){
		displayState[i] = 255;
	}
	updateDisplay();
}

void HV518::writeSingleDigit(uint8_t pos, uint8_t number){
	// Convert from ASCII
	// Not a problem because this should be a SINGLE digit (not 2 digits)
	if(number >= 48 && number <= 57){
		number -= 48;
	}

	if(pos < numDigits && number < 10 && number > -10){
		// Handle negatives
		if(number < 0){
			number *= -1; // Make the number positive for looking it up in the byte array

			if(leftAlignDisplay){
				setDigit(pos++, punctuation.hyphen);
			}
			else{
				setDigit(pos+1, punctuation.hyphen);
			}
		}

		setDigit(pos, pgm_read_byte_near(digits + number));
	}
}

void HV518::writeNumber(uint8_t pos, long number, int totalLength){
	char numChar[12]; // 12 is max length of a long (incl. negative)
	ltoa(number, numChar, 10);
	int numLen = strlen(numChar);

	if(leftAlignDisplay){
		if(totalLength > -1 && numLen < totalLength){
			int padLen = totalLength - numLen;
			for(int i = 0; i < padLen; i++){
				writeSingleDigit(pos++, 0);
			}
		}
		for(int i = 0; i < numLen; i++){
			if(numChar[i] == '-'){
				// Convert from ASCII
				writeSingleDigit(pos, (numChar[i + 1] - 48)* -1);
				i++; // Extra increment for negative
				pos += 2;
			}
			else{
				writeSingleDigit(pos++, numChar[i]);
			}
		}
	}
	else { // Right aligned display
		if(totalLength > -1 && numLen < totalLength){
			int padLen = totalLength - numLen;
			int padPos = pos;
			for(int i = 0; i < padLen; i++){
				writeSingleDigit((padPos++) + numLen, 0);
			}
		}
		for(int i = numLen - 1; i >= 0; i--){
			if(i == 1 && number < 0){
				// Convert from ASCII
				writeSingleDigit(pos, (numChar[i] - 48)* -1);
				pos += 2;
				i--;
			}
			else{
				writeSingleDigit(pos++, numChar[i]);
			}
		}
	}
}

void HV518::writeNumber(uint8_t pos, long number){
	writeNumber(pos, number, -1);
}

void HV518::writeString(uint8_t pos, String str){
	clearDisplayMemory();

	uint8_t firstPosition = 0;
	if(!leftAlignDisplay){
		firstPosition = numDigits;
	}

	for(uint8_t i = 0; i < str.length(); i++){
		char c;
		if(leftAlignDisplay){
			c = str.charAt(i);
		}
		else{
			// Go through the string backwards
			c = str.charAt(str.length() - i - 1);
		}

		if(c >= 48 && c <= 57){
			c -= 48;
			// OR with current data to keep period (if any)
			setDigit(pos++, pgm_read_byte_near(digits + c) | getDigit(pos));
		}
		else if(c >= 65 && c <= 90){
			c -= 65;
			setDigit(pos++, pgm_read_byte_near(letters + c) | getDigit(pos));
		}
		else if(c >= 97 && c <= 122){
			c -= 97;
			setDigit(pos++, pgm_read_byte_near(letters + c) | getDigit(pos));
		}
		else if(c == '-'){
			setDigit(pos++, punctuation.hyphen | getDigit(pos));
		}
		else if(c == '_'){
			setDigit(pos++, punctuation.underscore | getDigit(pos));
		}
		else if(c == '='){
			setDigit(pos++, punctuation.equal | getDigit(pos));
		}
		else if(c == ' '){
			pos++;
			continue;
		}
		else if(c == '.'){
			uint8_t prevPos = pos - 1;
			bool doneOne = false;

			if(!leftAlignDisplay){
				prevPos = pos;

				int nextIndex = str.length() - i - 2;
				if(nextIndex >= 0 && str.charAt(nextIndex) == '.'){
					setDigit(pos++, getDigit(prevPos) | punctuation.period);
					doneOne = true;
				}
			}
			
			if(!doneOne){
				if(pos == firstPosition || getDigit(prevPos) & punctuation.period > 0){
					setDigit(pos++, punctuation.period);
				}
				else{
					setDigit(prevPos, getDigit(prevPos) | punctuation.period);
				}
			}
		}
	}
}


	// duty = brightness, dispTime = how many ms the message should be displayed for

void HV518::displayWithAnodePWM(uint8_t duty, long dispTime){
	unsigned long startTime = millis();
	unsigned long endTime = startTime + dispTime;

	// Backup current display data			< ------- take note of this
	byte dataBak[numRegisters];
	memcpy(dataBak, displayState, numRegisters * sizeof(byte));

	// IIIIIIIIM GETTINGGGG ITTTT

	// these two commands are from the initializaiton up at the top of the file:

	// numRegisters = this->numLines / 8;
	// displayState = new byte[numRegisters];

	// each byte sent to the chip can only be 8 bits long
	// [numLines] in this context is the number of outputs being used on the chip
	// in my case, that would be, for ease of coding, 32.
	
	// some of the chip outputs won't actually be used (pin 1, pin 30, pin 31, pin 32)
	// why? good question
	// pin 1 on my VFD is for the filament. so all of the grid and anode pins are from 2 to 29.
	// pin 30 on the VFD is the other side of the filament.
	// so to make my job easier, i'm just making the chip output numbers match the vfd pin numbers, because that will make things significantly
	// easier to code.

	// back to my understanding of this code.
	// a "Register" in this context, is a single byte. a byte composed of 8 bits.
	// so if there are 32 outputs, or Lines, the amount of bytes needed to send to the chip, or Registers, would be 32 / 8. which 

	// OH MY GOD HOLD THE HORSES
	// petah
	// the horse is here
	// so
	// a byte variable is any number between 0 and 255
	// said byte variable can also be expressed in binary form
	// ex: the byte can be set to  B10101010  , which is 170
	// which i think could be a solution to my "how do i construct the data to go to the chip" problem
	// because:
	// i EXPECT there should be a way to convert an integer-style byte to a binary-style byte, (if they aren't already internally treated the same. but it's c++, so probably not.)
	// that means i should be able to use python to construct the binary, because python can actually handle it.
	
	// and then either:
	// python would create a binary list for each individual pinset, then send that to the teensy, it goes and displays it
	// OR:
	// make the python a little more complicated. but potentially make things faster for the teensy.

	// step 1:
	// sort the address list by anode pin. example of sorted list:
	// [26, 17], [27, 17], [28, 17], [29, 17], [19, 16], [20, 16], [19, 12], [25, 9]
	// ^                                       ^                   ^         ^
	// carats mark the starts of every "clump". each clump consists of addresses with the same anode pin.
	// then each clump is sorted by their grid pins, from smallest to largest. no technical reason, just to make it pretty.
	// 
	// then the large list consists of all of the clumps, starting with the biggest clump first, making its way to the smallest clump.
	// lets also say, for the sake of making it pretty, if there are clumps that are the same size, the clump with the lowest anode pin goes first.
	//
	// that way, i think it may be at least minimally faster. at least to me it feels more efficient.
	//
	// so then the python takes that list, and assembles a list of binary from it.
	//
	// because i'm lazy, i'm gonna use that binary from above.
	//
	// let's take a half step back real quick.
	//
	// so the data that goes into the chip is binary, and for simplicity for this example, let's say the chip has 8 outputs.
	//
	// imagine there are LEDs hooked up to every output as well.
	// 
	// so if i fed the chip this binary:  B10101010  every other LED would turn on, starting with the first LED being on.
	//
	// and if i fed it  B00000001  , only the last LED would be on.
	//
	// now pretend our chip has 16 outputs now.
	//
	// now we have to feed it two bytes in order to control all 16 outputs.
	//
	// so we'd feed it this: [ B10101010, B00000001 ]
	// 
	// so now the first 8 LED's would be staggering on/off, and then the next 7 would be off, and the last LED would be on.
	//
	// our chip has 32 outputs.
	//
	// so to stagger all 32 LEDs, i'd feed it [ B10101010, B10101010, B10101010, B10101010 ]
	//
	// now that we know how data is fed to the chip, lets take a half step forward again.
	//
	// assembling a list of binary.
	//
	// so im like 90 million percent sure that it is possible to convert a string of 1's and 0's to binary in python, since python isnt WEAK like c++
	//
	// no offense c++ dont brick my teensy please thanks <3
	//
	// so for every address in the list of addresses we organized earlier:
	//
	// create a list of 32 0's
	// (note: the 0's should be strings)
	//
	// select element number in list according to grid pin and set that element to '1'
	// (e.g. if the grid was 19, the 19th '0' would be flipped to a '1')
	//
	// select element number in list according to anode pin and set that element to '1'
	// (e.g. its the same thing as the grid pin but with the anode pin)
	//
	// next we take that list of 0's and we're gonna join them together into one big string
	//
	// now we have what im gonna call a CHUNKYBYTE because megabyte is already taken
	//
	// now we split this string into smaller strings consisting of 8 characters each
	//
	// congratulations! you now have 4 bytes!
	//
	// now use python's magical wizardry to convert each byte into an integer!
	// (the same way that B10101010 is equal to 170)
	//
	// now each chunk of that binary is reduced to a single integer, which can be converted BACK into that binary later!
	//
	// so now for every pin address in that list, it is now a list of four integers
	//
	// and sure, four integers is longer than the two we started out with, BUT!! we have just done pretty much nearly all of the work in python instead of
	// trying to deal with it in c++
	//
	// so now our list would look something similar to this:
	//
	// [ [12, 24, 100, 4], [32, 57, 250, 77] ]
	//
	// except actually, now that i think about it, it's going to look a lot simpler than that, because at least two of the bytes are going to be 0,
	// since only 2 bits out of 32 are 1. if i'm lucky then maybe both of those two bits will be in the same byte. im too lazy to verify that.
	//
	// anyway, this is a bit more accurate of a representation:
	//
	// [ [1, 0, 4, 0], [0, 8, 16, 0] ]
	//
	// yeah that's a lot simpler
	//
	// now to go and actually code all of this !!!



	uint8_t offTime = 0;
	uint8_t onTime = 1;
	if(duty == 0){				// this appears to just clear the display if brightness is 0
		onTime = 0;
		clearDisplay();			// the variable 'duty' is outwardly known as the brightness setting, which is a value between 0 and 255.
	}
	else if(duty < 255){
		offTime = 255 - duty;	// okay so it appears that brightness determines how long the display is on within an arbritrary length of time, 255.
								// think of it like an imperfect square wave. 1 full cycle of the wave would be 255. so setting duty to 255 means that
								// the display will be on for all 255 steps of the wave.
		onTime = duty;
	}

	uint8_t interlaceRatioOff = 0;
	if(onTime != 0){
		interlaceRatioOff = offTime / onTime;	// the ratio between time that the display is off vs time the display is on
	}
	uint8_t interlaceRatioOn = 0;
	if(offTime != 0){
		interlaceRatioOn = onTime / offTime;	// the ratio between time that the display is on vs time the display is off
	}

	// Loop until time is up
	while(millis() < endTime){

		if(offTime > onTime){			// IF: the display is OFF longer than it is ON


			for(uint8_t i = 0; i < offTime && millis() < endTime; i++){
				// loops either for the amount of time the display is off, OR until time ends (if it ends in the middle of the loop)

				clearDisplay();
				
					// if the off ratio is not 0, (meaning that there is time when the display is ON)
					// AND if the modulus of [i] % off ratio = 0 (example, the ratio is 50/50, this condition would be true half of the time)
					// AND if the time that it is on is over 0
					// AND if the time has not ended
				if(interlaceRatioOff != 0 && i % interlaceRatioOff == 0 && onTime > 0 && millis() < endTime){
					
					memcpy(displayState, dataBak, numRegisters * sizeof(byte));
					// this appears to copy the current displayState to memory, though i need to read up on this function a bit more
					
					// i have read up
					//
					// void *memcpy(void *dest, const void *src, size_t n);
					//
					// looks like the global variable displayState is the destination
					//
					// and the source is a variable called dataBak
					//
					// dataBak is a byte list defined at the top of this funcition
					// 
					// AHA
					// memcpy() literally just copies a variable onto another one
					// i dont know if it works for all data types but here it's used for byte lists
					// at the top of the function, dataBak is defined, and displayState is copied to it
					// and here it's retrieving the backup of displayState, and copying it back onto displayState.
					//
					// you also have to define how big the data is :( usual c++ :(

					updateDisplay();
				}
			}
		}

		else{						// IF: the display is ON longer than it is OFF

			for(uint8_t i = 0; i < onTime && millis() < endTime; i++){
				// loops either for the amount of time the display is off, OR until time ends (if it ends in the middle of the loop)
				
				memcpy(displayState, dataBak, numRegisters * sizeof(byte));	// this same function.... this call is the same as the one above

				updateDisplay();


					// if the on ratio is not 0, (meaning that there is time when the display is OFF)
					// AND if the modulus of [i] % on ratio = 0 (example, the ratio is 50/50, this condition would be true half of the time)
					// AND if the time that it is on is over 0
					// AND if the time has not ended
				if(interlaceRatioOn != 0 && i % interlaceRatioOn == 0 && offTime > 0 && millis() < endTime){
					
					clearDisplay();
				}
			}
		}

		// my only question: why is that if statement there? the one separating if the display is off longer than on / on longer than off.
		//
		//why??
		//
		// lets think...
		//
		// when the display is OFF more than it's on:
		// 
		// it clears & updates the display
		// then if the tick is on the modulus of the off ratio
		// it copies the backup of displayState back onto displayState
		// then it updates the display
		//
		//
		// when the display is ON more than it's off:
		//
		// it copies the backup of displayState back onto displayState
		// then it updates the display
		// then if the tick is on the modulus of the on ratio
		// it clears & updates the display
		//
		//
		// i UNDERSTAND NOW!!!
		// I GET IT!!!
		// i see how theyre different.
		// i mean theyre basically the same
		// its just that when its on more than its off, it starts by, yknow, turning on the display
		// and then the opposite is true as well, when it's off more, it'll start by clearing.
		//
		// however in my implementation i think i'm going to do it a bit differently.
		//
		// instead of constantly updating the shift register with new data (the clear function clears displayState and then sends that to the shift register)
		// i think that's what the STROBE pin on the shift register is for. it turns on/off all of the outputs. so i can most likely simplify things a whole lot
		// by only having to send the data once, and then use STROBE to determine if its output or not. rather than how this function does it, by making the shift register have to reset
		// all the latches to 0 when it clears, and then it has to reload all the data again, and so on.
		//
		// im gonna check with the datasheet to be sure.
		//
		// yup that checks out
		//
		// LE (latch enable) when HIGH takes the new data and puts it in the output buffers.
		// then, when LE is LOW, it allows the next line of data to be entered.
		//
		// then, the chip will only actually output the data in the output buffers when STR is LOW.




		delay(0); // Added to prevent watchdog issues
	}

	// Make sure the display state is restored
	memcpy(displayState, dataBak, numRegisters * sizeof(byte));
}

void HV518::displayWithAnodePWM(uint8_t duty, bool (*cont)()){
	// Backup current display data
	byte dataBak[numRegisters];
	memcpy(dataBak, displayState, numRegisters * sizeof(byte));

	uint8_t offTime = 0;
	uint8_t onTime = 1;
	if(duty == 0){
		onTime = 0;
		clearDisplay();
	}
	else if(duty < 255){
		offTime = 255 - duty;
		onTime = duty;
	}

	uint8_t interlaceRatioOff = 0;
	if(onTime != 0){
		interlaceRatioOff = offTime / onTime;
	}
	uint8_t interlaceRatioOn = 0;
	if(offTime != 0){
		interlaceRatioOn = onTime / offTime;
	}

	// Loop until time is up
	while((*cont)()){
		if(offTime > onTime){
			for(uint8_t i = 0; i < offTime && (*cont)(); i++){
				clearDisplay();
				if(interlaceRatioOff != 0 && i % interlaceRatioOff == 0 && onTime > 0 && (*cont)()){
					// On Display
					memcpy(displayState, dataBak, numRegisters * sizeof(byte));
					updateDisplay();
				}
			}
		}
		else{
			for(uint8_t i = 0; i < onTime && (*cont)(); i++){
				// On Display
				memcpy(displayState, dataBak, numRegisters * sizeof(byte));
				updateDisplay();
				if(interlaceRatioOn != 0 && i % interlaceRatioOn == 0 && offTime > 0 && (*cont)()){
					clearDisplay();
				}
			}
		}

		delay(0); // Added to prevent watchdog issues
	}

	// Make sure the display state is restored
	memcpy(displayState, dataBak, numRegisters * sizeof(byte));
}

void HV518::setBrightnessStrobePWM(uint8_t brightness){
	if(hasStrobe){
		if(brightness > 0 && brightness < 255){ // PWM
			analogWrite(strobePin, 255-brightness);
		}
		else if(brightness == 0){ // Steady-state off
			analogWrite(strobePin, 0);
			digitalWrite(strobePin, HIGH);
		}
		else{ // Steady-state on
			analogWrite(strobePin, 0);
			digitalWrite(strobePin, LOW);
		}
	}
}

byte* HV518::getDisplayMemory(){
	return displayState;
}

HV518::~HV518(){
	if(displayState){
		delete[] displayState;
	}
}

#endif /* HV518_cpp */