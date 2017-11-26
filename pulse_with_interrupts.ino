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

unsigned char isRunAgain = false;

// 0x80 = 10000000
unsigned char bitMask = 0x80; 
unsigned char address = 0;
unsigned char order = 0;
unsigned char checksum  = 0x0;

char state = 0;
//-------------------------------------------------------------------------------------------------------------//
//0x24 = 36, 0x28 = 40
unsigned char trainAddress = 40;

// 0x40 = 01000000
unsigned char trainOrder = 0x40;
unsigned char trainDirection = 0x0;
unsigned char trainSpeed = 8;
unsigned char DCCspeed = 0;
unsigned char trainChecksum = 0x0;

//-------------------------------------------------------------------------------------------------------------//

unsigned char accessoryNumber = 101;
unsigned char accessoryAddress = 0;
unsigned char accessoryDirection = 1;
unsigned char accessoryOutput = 0;
unsigned char accessorySwitch = 0;
unsigned char accessoryOrder = 0;
unsigned char packetType = 0;

struct Packet
{
	address, 
	order, 
	order
	
};

struct Packet livepacket;
livepacket.address = 36;

struct Packet *pointerName;
pointerName = &livepacket;

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



void buttonsPushed()
{
	switch (lcd_key)
	{
		case btnUP: //Trainspeed goes up by 1
      		if (trainSpeed == 30)
			{
				trainSpeed = 30;
			}
			else
			{
				trainSpeed = trainSpeed + 1;
				creatingOrder();
			}
 
			noInterrupts();
      		lcd.setCursor(0,0);
		  	lcd.print("UP            ");
		  	lcd.setCursor(0, 1);
			lcd.print(trainSpeed);
			interrupts();
			Serial.println("UP");
			Serial.print(trainSpeed);
			break;

		case btnDOWN: //Trainspeed goes down by 1			
		  	if (trainSpeed == 0)
			{
				trainSpeed = 0;
			}
			else 
			{
				trainSpeed = trainSpeed - 1;
				creatingOrder();
			}
		
			noInterrupts();
		  	lcd.setCursor(0, 0);
			lcd.print("DOWN           ");
			lcd.setCursor(0, 1);
			lcd.print(trainSpeed);

			interrupts();

			Serial.println("DOWN");
			Serial.println(trainSpeed);
			break;

		case btnLEFT:


			noInterrupts();
			lcd.setCursor(0, 0);
			lcd.print("LEFT  ");
			interrupts();
			Serial.println("LEFT");
			break;

		case btnRIGHT:
			noInterrupts();
			lcd.setCursor(0, 0);
			lcd.print("RIGHT");
			interrupts();
			Serial.println("RIGHT");

			sendPacket();
			break;

		case btnSELECT: //Iterate through the different things to do
			noInterrupts();
			lcd.setCursor(0, 0);
			lcd.print("SPEED          ");
			interrupts();
			Serial.println("SPEED");
			break;

		case btnNONE:
			break;
	}
}


 void creatingOrder()
 {
 	DCCspeed = trainSpeed;

 	 	//if the direction is set to 1, then
	if (trainDirection == 1)
	{
		trainOrder |= 0x20;
	}
	
	//
	if (1 & DCCspeed == 1)
	{
		DCCspeed |= 0x20;
		//lcd.setCursor(1, 8);
		//lcd.print(trainSpeed);
	}
	
	DCCspeed >>= 1;
	trainOrder |= DCCspeed;
	//lcd.setCursor(1, 9);
	//lcd.print(trainSpeed);

	//Calculates the accessory address
	//Starts at zero, then you divide the accessoryNumber with 4. and plus it one. You set your accessoryAddress to that.
	//That turns into 00010111
	accessoryAddress = (accessoryNumber/4) + 1;
	//After you calculate accessoryAdress, you compare it to 0x80. 
	//Meaning a bitwise OR operation...

	// AccessoryAdress - 00010111
	//       0x80      - 01010000
	//       result      01010111
	accessoryAddress |= 0x80;

	accessoryOutput = (accessoryNumber % 4) - 1;
	if (accessoryOutput < 0) 
	{
		accessoryOutput = 3;
	}

	// 0xF8 = 11111000
	//So we put the accessory order to be like the one above since it was 0
	accessoryOrder = accessoryOrder | 0xF8;

	//Then we first bitshift accessoryoutput 
	accessoryOrder |= (accessoryOutput << 1);

	accessoryOrder |= accessoryDirection;


	if (packetType == 0)
	{
		address = trainAddress;
		order = trainOrder;
	}
	else if (packetType == 1)
	{
		address = accessoryAddress;
		order = accessoryOrder;
	}
	checksum = address ^ order;
 }

void setup() 
{
  // put your setup code here, to run once:
	//We set the lcd screen to how many rows and columns
	lcd.begin(16, 2);
	//We put the cursor at the top lefr corner
	lcd.setCursor(0, 0);

	//we print out on the lcd some text
	lcd.print("Embedded C/Tog");

	creatingOrder();
	
	pinMode(OUTPIN, OUTPUT);
  	Serial.begin(9600);


	lcd.setCursor(1, 1);
	lcd.print(trainSpeed);
	Serial.print(trainSpeed);

	timer2_setup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
	lcd_key = read_LCD_buttons();
	//buttonsPushed();
	delay(300);
}

void accessoryStuff()
{
	accessoryOrder ^=  0x08;
	order = accessoryOrder;

	checksum = address ^ order;
}


void sendPacket()
{
	do 
	{
		isRunAgain = false;

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
					whichBit = ((bitMask & pointerName -> address == 0 ? 0 : 1); 

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
					whichBit = ((bitMask & order) == 0 ? 0 : 1);

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
					whichBit = ((bitMask & checksum) == 0 ? 0 : 1);

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

        		Serial.print("0");
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

      		Serial.print("1");
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
