//Playing around with interrupts

char pin_stateness = 0;
char OUTPTN = 5;
char previousness = 0;
char whichBit = 0;
char countToEight = 0;
char previousBit = 0;
char prevHighOrLow = 0;
char highOrLow = 0;
char counter = 0;

char state 0;
unsigned char whichByteToSend = 36;
unsigned char bitMask = 128; 

unsigned char bunchOfOnes = 0xff;
unsigned char bunchOfZeros = 0x00;

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
	switch (state)
	{
		case 0: whichBit = 1;

				if (counter == 13)
				{
					whichBit = 0

					state = 1;
				}
				break;

		case 1:
		case 3: 
				whichBit = 1;

				if (counter == 8 && state == 1)
				{
					whichBit = 0;
					state = 2;
				}
				else if (counter == 8 && state == 3)
				{
					whichBit = 1;
					state = 0;
				}
				break;

		case 2:
			whichBit = 0;

			if (counter == 8)
			{
				whichBit = 0;
				state = 3;
			}
			break;
	}

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
