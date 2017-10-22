//Playing around with interrupts

char OUTPTN = 5;
char whichBit = 0;
char prevHighOrLow = 0;
char highOrLow = 0;
char counter = 0;

unsigned char isRunAgain = false;
unsigned char bitMask = 128; 

char state 0;

void setup() 
{
  // put your setup code here, to run once:

}

void loop() 
{
  // put your main code here, to run repeatedly:

}

void preamblePacketWithSkille()
{
	do 
	{
		isRunAgain = false;

		switch (state)
		{
			case 0:
					whichBit = 1;

					if (counter == 13)
					{
						whichBit = 0
					}
					else if (counter == 14)
					{
						state = 1;
						isRunAgain = true;
					}
					break;

			case 1:
					unsigned char address = 36;
					whichBit = (bitMask & address == 0 ? 0 : 1);

					if (counter == 8)
					{
						whichBit = 0;
					}
					else if (counter == 9)
					{
						state = 2;
						isRunAgain = true;
					}

			case 2:
					unsigned char order = 100;
					whichBit = (bitMask & order == 0 ? 0 : 1);

					if (counter == 8)
					{
						whichBit = 0;
					}
					else if (counter == 9)
					{
						state = 3;
						isRunAgain = true;
					}
					break;

			case 3:
					unsigned char checksum address ^ order;
					whichBit = (bitMask & checksum == 0 ? 0 : 1);

					else if (counter == 8)
					{
						whichBit = 1;
					}
					else if (counter == 9)
					{
						state = 0;
						isRunAgain = true;
					}
					break;
		}

	}
	while (isRunAgain == true);


	if (whichBit == 0)
	{
		if (prevHighOrLow == 0 && highOrLow == 0)
		{
			digitalWrite(OUTPTN, HIGH);
			prevHighOrLow = 0;
			highOrLow = 1;
			}

	 		else if (prevHighOrLow == 0 && highOrLow == 1)
			{
				digitalWrite(OUTPTN, HIGH);
				prevHighOrLow = 1;
				highOrLow = 1;   
			}

			else if (prevHighOrLow == 1 && highOrLow == 1)
			{
				digitalWrite(OUTPTN, LOW);
				prevHighOrLow = 1;
				highOrLow = 0;
			}

			else if (prevHighOrLow == 1 && highOrLow == 0)
			{
				digitalWrite(OUTPTN, LOW);
				prevHighOrLow = 0;
				highOrLow = 0;
				counter++;
			}
	}
	else if (whichBit == 1)
	{
		if (highOrLow == 0)
			{
				digitalWrite(OUTPTN, HIGH);
				highOrLow = 1;
			}
			else if (highOrLow == 1)
			{
				digitalWrite(OUTPTN, LOW);
				highOrLow = 0;
				counter++;
			}
	}
}


ISR(TIMER2_COMPA_vect)
{
	write8zerosOrOnes();
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
