//Playing around with interrupts

char OUTPIN = 5;
char whichBit = 0;
char prevHighOrLow = 0;
char highOrLow = 0;
char counter = 0;

unsigned char isRunAgain = false;
unsigned char bitMask = 0x80; 
unsigned char address = 0;
unsigned char order = 0;
unsigned char checksum  = 0x0;

char state = 0;
//-------------------------------------------------------------------------------------------------------------//
//0x24 = 36, 0x28 = 40
unsigned char trainAddress = 40;
unsigned char trainOrder = 0x40;
unsigned char trainDirection = 0x0;
unsigned char trainSpeed = 8;
unsigned char trainChecksum = 0x0;

//-------------------------------------------------------------------------------------------------------------//

unsigned char accessoryNumber = 101;
unsigned char accessoryAddress = 0;
unsigned char accessoryDirection = 1;
unsigned char accessoryOutput = 0;
unsigned char accessorySwitch = 0;
unsigned char accessoryOrder = 0;
unsigned char packetType = 0;

void setup() 
{
  // put your setup code here, to run once:

	timer2_setup();
	pinMode(OUTPIN, OUTPUT);
  Serial.begin(9600);


	if (trainDirection == 1)
	{
		trainOrder |= 0x20;
	}
	
	if (1 & trainSpeed == 1)
	{
		trainSpeed |= 0x20;
	}

	trainSpeed >>= 1;
	trainOrder |= trainSpeed;


	accessoryAddress = (accessoryNumber/4) + 1;
	accessoryAddress |= 0x80;

	accessoryOutput = (accessoryNumber % 4) - 1;
	if (accessoryOutput < 0) 
	{
		accessoryOutput = 3;
	}

	accessoryOrder = accessoryOrder | 0xF8;

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

 Serial.println(address);
 Serial.println(order);
 Serial.println(checksum);
}

void loop() 
{
  // put your main code here, to run repeatedly:

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
			case 0: //preamble
					whichBit = 1;

					if (counter == 13)
					{
						whichBit = 0;
					}
					else if (counter == 14)
					{
						state = 1;
						isRunAgain = true;
						counter = 0;
            bitMask = 0x80;
					}
					break;

			case 1: //address
					whichBit = ((bitMask & address) == 0 ? 0 : 1);

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


	if (whichBit == 0)
	{
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
			}
	}
	else if (whichBit == 1)
	{
		if (highOrLow == 0)
		{
			digitalWrite(OUTPIN, HIGH);
			highOrLow = 1;
		}
		else if (highOrLow == 1)
		{
			digitalWrite(OUTPIN, LOW);
			highOrLow = 0;
			counter++;
      bitMask >>= 1;
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
