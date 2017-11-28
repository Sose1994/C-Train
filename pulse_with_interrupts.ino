#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int lcd_key = 0;
int adc_key_in = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

char OUTPIN = 5;
char whichBit = 0;
char prevHighOrLow = 0;
char highOrLow = 0;
char counter = 0;
char testing = true;


unsigned char isRunAgain = false;
unsigned char bitMask = 0x80;
char state = 0;
//-------------------------------------------------------------------------------------------------------------//
unsigned char trainAddress = 40; //0x24 = 36, 0x28 = 40
//unsigned char trainOrder = 0x40; // 0x40 = 01000000
unsigned char trainDirection = 0x0;
unsigned char trainSpeed = 8;
//unsigned char trainChecksum = 0x0;

//-------------------------------------------------------------------------------------------------------------//

unsigned char accessoryNumber = 101;
//unsigned char accessoryAddress = 0;
unsigned char accessoryDirection = 1;
unsigned char accessoryOutput = 0;
unsigned char accessorySwitch = 0;
//unsigned char accessoryOrder = 0;
unsigned char packetType = 0;

char sound = false;
char horn1 = false;
char horn2 = false;
char bell  = false;
char light = false;


struct Packet
{
	unsigned char address;
	unsigned char order;
	unsigned char checksum;
};

enum functionMode
{
	ADDRESS,
	DIRECTION,
	SPEED,
	SOUND,
	HORN1,
	HORN2,
	BELL,
	LIGHT
};

enum functionMode currentFunctionMode;

struct Packet baselinePacket;
struct Packet idlePacket;
struct Packet accessoryPacket;
struct Packet soundPacket;

struct Packet *pointerCurrentPacket = &baselinePacket;

void timer2_setup();

int read_LCD_buttons()
{
	adc_key_in = analogRead(0);

	if (adc_key_in > 1000) return btnNONE;
	if (adc_key_in < 50)   return btnRIGHT;
	if (adc_key_in < 195)  return btnUP;
	if (adc_key_in < 380)  return btnDOWN;
	if (adc_key_in < 555)  return btnLEFT;
	if (adc_key_in < 790)  return btnSELECT;
	return btnNONE;
}

/*void selectFunction()
{
	switch (current)
	{
		case ADDRESS:
			noInterrupts();
			lcd.print("ADDRESS F");

			if (lcd_key == btnSELECT)
			{
				current = DIRECTION;
				break;
			}
			interrupts();
		break;

		case DIRECTION:
			noInterrupts();
			lcd.print("DIRECTION");
			interrupts();
		break;
	}
}*/

void buttonsPushed()
{
	switch (lcd_key)
	{
		case btnUP: //Trainspeed goes up by 1
      		switch (currentFunctionMode)
      		{
      			case SPEED:
      					if (trainSpeed == 30)
						{
							trainSpeed = 30;
						}
						else
						{
							trainSpeed = trainSpeed + 1;
							packetType = 0;
							baselineSetup();
						}
			 
						noInterrupts();
			      		lcd.setCursor(0,0);
					  	lcd.print("UP            ");
					  	lcd.setCursor(0, 1);
						lcd.print(trainSpeed);
						interrupts();

						Serial.print("UP ");
						Serial.println(trainSpeed);
      					break;
				case ADDRESS:

						if (trainAddress == 253)
						{
							trainAddress = 253;
						}
						else
						{
							trainAddress = trainAddress + 1;
							packetType = 0;
							baselineSetup();
						}
			 
						noInterrupts();
			      		lcd.setCursor(0,0);
					  	lcd.print("UP            ");
					  	lcd.setCursor(0, 1);
						lcd.print(trainAddress);
						interrupts();

						Serial.print("UP ");
						Serial.println(trainAddress);
      					break;
				case SOUND: 
						
						sound = true; horn1 = false; horn2 = false; bell = false; light = false;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("ON     ");
						packetType = 3;
						soundSetup();
						interrupts();
				
				case HORN1:
						sound = false; horn1 = true; horn2 = false; bell = false; light = false;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("ON     ");
						packetType = 3;
						soundSetup();
						interrupts();
						
				case HORN2:
						sound = false; horn1 = false; horn2 = true; bell = false; light = false;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("ON     ");
						packetType = 3;
						soundSetup();
						interrupts();
					
				case BELL:
						sound = false; horn1 = false; horn2 = false; bell = true; light = false;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("ON     ");
						packetType = 3;
						soundSetup();
						interrupts();
				case LIGHT:
						sound = false; horn1 = false; horn2 = false; bell = false; light = true;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("ON     ");
						packetType = 3;
						soundSetup();
						interrupts();
      		}
			break;

		case btnDOWN: //Trainspeed goes down by 1			
		  	switch (currentFunctionMode)
		  	{
		  		case SPEED:
		  				if (trainSpeed == 0)
						{
							trainSpeed = 0;
						}
						else 
						{
							trainSpeed = trainSpeed - 1;
							packetType = 0;
							baselineSetup();
						}
					
						noInterrupts();
					  	lcd.setCursor(0, 0);
						lcd.print("DOWN           ");
						lcd.setCursor(0, 1);
						lcd.print(trainSpeed);
						interrupts();


						Serial.print("DOWN ");
						Serial.println(trainSpeed);
						break;
				case ADDRESS:
						if (trainAddress == 253)
						{
							trainAddress = 253;
						}
						else
						{
							trainAddress = trainAddress + 1;
							packetType = 0;
							baselineSetup();
						}
			 
						noInterrupts();
			      		lcd.setCursor(0,0);
					  	lcd.print("UP            ");
					  	lcd.setCursor(0, 1);
						lcd.print(trainAddress);
						interrupts();

						Serial.print("UP ");
						Serial.println(trainAddress);
      					break;
				case SOUND: 
						sound = false; horn1 = false; horn2 = false; bell = false; light = false;
						packetType = 3;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("OFF     ");
						soundSetup();
						interrupts();
						
				case HORN1:
						sound = false; horn1 = false; horn2 = false; bell = false; light = false;
						packetType = 3;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("OFF     ");
						soundSetup();
						interrupts();
						
				case HORN2:
						sound = false; horn1 = false; horn2 = false; bell = false; light = false;
						packetType = 3;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("OFF     ");
						soundSetup();
						interrupts();
						
				case BELL:
						sound = false; horn1 = false; horn2 = false; bell = false; light = false;
						packetType = 3;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("OFF     ");
						soundSetup();
						interrupts();
						
				case LIGHT:
						sound = false; horn1 = false; horn2 = false; bell = false; light = false;
						packetType = 3;
						noInterrupts();
						lcd.setCursor(0, 1);
						lcd.print("OFF     ");
						soundSetup();
						interrupts();
						

		  	}
			break;

		case btnLEFT:

			switch (currentFunctionMode)
			{
				case DIRECTION:
							trainDirection = 1;
							baselineSetup();

							noInterrupts();
							lcd.setCursor(0, 0);
							lcd.print("FORWARDS    ");
							interrupts();
							Serial.println("FORWARDS   ");
							break;
			}
			break;

		case btnRIGHT:
			
			switch (currentFunctionMode)
			{
				case DIRECTION:
							trainDirection = 0;
							baselineSetup();

							noInterrupts();
							lcd.setCursor(0, 0);
							lcd.print("BACKWARDS         ");
							interrupts();
							Serial.println("BACKWARDS        ");
							break;
						}
			break;

		case btnSELECT: //Iterate through the different things to do
			noInterrupts();
			lcd.setCursor(0, 0);
			lcd.print("SELECT             ");
			//Serial.println("SPEED");

			switch (currentFunctionMode)
			{
				case ADDRESS:
						lcd.setCursor(0, 0);
						lcd.print("DIRECTION           ");
						pointerCurrentPacket = &baselinePacket;
						currentFunctionMode = DIRECTION;	
						break;
				case DIRECTION:
						lcd.setCursor(0, 0);
						lcd.print("SPEED         ");
						pointerCurrentPacket = &baselinePacket;
						currentFunctionMode = SPEED;
						break;

				case SPEED:
						lcd.setCursor(0, 0);
						lcd.print("SOUND            ");
						pointerCurrentPacket = &soundPacket;
						currentFunctionMode = SOUND;
						break; 

				case SOUND:
						lcd.setCursor(0, 0);
						lcd.print("HORN1        ");
						pointerCurrentPacket = &soundPacket;
						currentFunctionMode = HORN1;
						break;

				case HORN1:
						lcd.setCursor(0, 0);
						lcd.print("HORN2         ");
						pointerCurrentPacket = &soundPacket;
						currentFunctionMode = HORN2;
						break;

				case HORN2:
						lcd.setCursor(0, 0);
						lcd.print("BELL        ");
						pointerCurrentPacket = &soundPacket;
						currentFunctionMode = BELL;
						break;

				case BELL:
						lcd.setCursor(0, 0);
						lcd.print("LIGHT        ");
						pointerCurrentPacket = &soundPacket;
						currentFunctionMode = LIGHT;
						break;

				case LIGHT:
						lcd.setCursor(0, 0);
						lcd.print("ADDRESS        ");
						pointerCurrentPacket = &baselinePacket;
						currentFunctionMode = ADDRESS;
						break;
			}
			interrupts();

			break;

		case btnNONE:
			break;
	}
}

void idleSetup()
{
	idlePacket.address = 0xFF;
	idlePacket.order = 0x0;
	idlePacket.checksum = 0xFF;
}

void accessorySetup()
{
	unsigned char accessoryOrder = 0;
	unsigned char accessoryAddress = 0;

	if (accessoryNumber % 4 == 0)
	{
		accessoryAddress = accessoryNumber/4;
		accessoryOutput = accessoryNumber % 4;
	}
	else
	{
		accessoryAddress = accessoryNumber / 4 +1;
		accessoryOutput = accessoryNumber % 4 -1;
	}
	accessoryAddress |= 0x80;
	
	accessoryOrder |= 0xF8;
	accessoryOrder |= (accessoryOutput << 1);
	accessoryOrder |= accessoryDirection;

	accessoryPacket.address = accessoryAddress;
	accessoryPacket.order = accessoryOrder;
	accessoryPacket.checksum = accessoryPacket.address ^ accessoryPacket.order;
}

void baselineSetup()
{
	unsigned char DCCspeed = 0;
	unsigned char trainOrder = 0x40;
 	DCCspeed = trainSpeed;


	if (trainDirection == 1)
	{
		trainOrder |= 0x20; //0010 0000
	}
	
	if (1 & DCCspeed == 1)
	{
		DCCspeed |= 0x20;
	}
	DCCspeed >>= 1;
	trainOrder |= DCCspeed;


	baselinePacket.address = trainAddress;
	baselinePacket.order = trainOrder;
	baselinePacket.checksum = baselinePacket.address ^ baselinePacket.order;
}

void soundSetup()
{
	unsigned char temporaryOrder = 0x80;

	if (sound) temporaryOrder |= 0x1;
	if (horn1) temporaryOrder |= 0x2;
	if (horn2) temporaryOrder |= 0x4;
	if (bell)  temporaryOrder |= 0x8;
	if (light) temporaryOrder |= 0x10;

	soundPacket.address = trainAddress;
	soundPacket.order = temporaryOrder;
	soundPacket.checksum = soundPacket.address ^ soundPacket.order;
}


void setup() 
{
  // put your setup code here, to run once:
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("Embedded C/Tog");

	if (packetType == 0)
	{
		baselineSetup();
	}
	else if (packetType == 1)
	{
		accessorySetup();
	}
	else if (packetType == 2)
	{
		idleSetup();
	}
	else if (packetType == 3)
	{
		soundSetup();
	}
	
	pinMode(OUTPIN, OUTPUT);
  	Serial.begin(9600);

	timer2_setup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
	lcd_key = read_LCD_buttons();
	buttonsPushed();
	delay(300);
}

void accessoryStuff()
{
	accessoryPacket.order ^=  0x08;
	accessoryPacket.checksum = accessoryPacket.address ^ accessoryPacket.order;
}


void sendPacket()
{
	do 
	{
		isRunAgain = false;

		if (testing && counter == 0)
		{
			Serial.println();
		}

		switch (state)
		{
			//if 0, send the preamble of 13 1's
			case 0: 
					//switch whichbit to send 1's
					whichBit = 1;

					// if 13 have been sent, switch whichbit to send a 0 (skillebit)
					if (counter == 13)
					{
						whichBit = 0;
					}
					// if 14 gets sent, you change state to go to the next part of the switch
					//And then we run the switch again.
					else if (counter == 14)
					{
						state = 1;
						isRunAgain = true;
						counter = 0;
            			bitMask = 0x80;
					}
					break;

			case 1: //address
					whichBit = ((bitMask & ((*pointerCurrentPacket).address))== 0 ? 0 : 1); 

					if (counter == 8)
					{
						whichBit = 0;
					}
					else if (counter == 9)
					{
						state = 2;
						isRunAgain = true;
						counter = 0;
            			bitMask = 0x80;
					}
         			break;

			case 2: //order
					whichBit = ((bitMask & ((*pointerCurrentPacket).order)) == 0 ? 0 : 1);

					if (counter == 8)
					{
						whichBit = 0;
					}
					else if (counter == 9)
					{
						state = 3;
						isRunAgain = true;
						counter = 0;
            			bitMask = 0x80;
					}
					break;

			case 3: //checksum
					whichBit = ((bitMask & ((*pointerCurrentPacket).checksum)) == 0 ? 0 : 1);

					if (counter == 8)
					{
						whichBit = 1;
					}
					else if (counter == 9)
					{
						state = 0;
						isRunAgain = true;
						counter = 0;
						if (packetType == 1)
						{
							accessoryStuff();
						}

           				bitMask = 0x80;
					}
					break;
		}
	}
	while (isRunAgain == true);

	//If it's a 0, send high high low low, to make a 0 bit
	if (whichBit == 0)
	{
		//If both prev and current are 0, make a 
		if (prevHighOrLow == 0 && highOrLow == 0)
		{
			digitalWrite(OUTPIN, HIGH);
			prevHighOrLow = 0;
			highOrLow = 1;
		}

 		else if (prevHighOrLow == 0 && highOrLow == 1)
		{
			digitalWrite(OUTPIN, HIGH);
			prevHighOrLow = 1;
			highOrLow = 1;   
		}

		else if (prevHighOrLow == 1 && highOrLow == 1)
		{
			digitalWrite(OUTPIN, LOW);
			prevHighOrLow = 1;
			highOrLow = 0;
		}

		else if (prevHighOrLow == 1 && highOrLow == 0)
		{
			digitalWrite(OUTPIN, LOW);
			prevHighOrLow = 0;
			highOrLow = 0;
			counter++;
    		bitMask >>= 1;

    		if (testing)
      		{
      			Serial.print("0");
      		}
		}
	}
	//If it's 1, send high low, to make a 1 bit
	else if (whichBit == 1)
	{
		//If it's 0, send high pulse (first part of 1 bit)
		if (highOrLow == 0)
		{
			digitalWrite(OUTPIN, HIGH);
			highOrLow = 1;
		}
		//If it's 1, send low pulse (second part of 1 bit)
		else if (highOrLow == 1)
		{
			digitalWrite(OUTPIN, LOW);
			highOrLow = 0;
			counter++;
      		bitMask >>= 1;

      		if (testing)
      		{
      			Serial.print("1");
      		}
		}
	}
}

ISR(TIMER2_COMPA_vect)
{
	sendPacket();
}

void timer2_setup() 
{
// Don't get interrupted while setting up the timer
	noInterrupts();
	////
	//// Set timer2 interrupt at 58 microseconds interval
	////
	TCCR2A = 0; // set entire TCCR2A register to 0
	TCCR2B = 0; // same for TCCR2B
	TCNT2  = 0; // initialize counter value to 0

	// 58 microseconds interval is 17,241 kHz
	// set compare match register for 17 kHz increments:
	// register = (Arduino clock speed) / ((desired interrupt frequency) * prescaler) - 1
	// register = (16*10^6) / (17.241 * 8) - 1 
	// register = 115
	// (must be <256 because timer2 is only 8 bit (timer1 is 16 bit))
	OCR2A = 115; 
	// = 116 ticks before reset (because 0 counts)
	// = 58 microseconds for each tick at 2 MHz

	// Turn on CTC mode
	TCCR2A |= (1 << WGM21);

	// Set CS21 bit for 8 prescaler (and not CS20 or CS22)
	TCCR2B |= (1 << CS21); // = 2 MHz = 0,5 microseconds/tick
	// (0 << CS20) || (0 << CS21) == 0

	// Enable timer compare interrupt
	TIMSK2 |= (1 << OCIE2A);

	interrupts(); // Reenable interrupts
}
