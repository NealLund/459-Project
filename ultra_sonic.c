/*
 * Ultrasonic sensor HC-05 interfacing with AVR ATmega16
 * http://www.electronicwings.com
 */ 

#define F_CPU 7372800UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void lcd_init(void);
void lcd_moveto(unsigned char);
void lcd_stringout(char *);

void sci_init(void);
void sci_out(char);
void sci_outs(char *);

#define MYUBRR 47   // Value for UBRR0 register

#define  Trigger_pin	PD5	/* Trigger pin */
#define  Trigger_pin2	PD7	/* Trigger pin */

int TimerOverflow = 0;

int flagVariable = 0;

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;	/* Increment Timer Overflow count */
}

int main(void)
{
	clock_t start_t, end_t;

	sci_init();                 // Initialize the SCI port
    
    lcd_init();                 // Initialize the LCD

	char string[10];
	long count;
	double distance;
	double distance_2;
	double speed;
	
	//DDRD = 0x05;		/* Make trigger pin as output */
	//PORTD = 0xFF;		/* Turn on Pull-up */
	
	lcd_moveto(0x00);
	lcd_stringout("Ultrasonic");
	
	sei();			/* Enable global interrupt */
	TIMSK1 = (1 << TOIE1);	/* Enable Timer1 overflow interrupts */
	TCCR1A = 0;		/* Set all bit to zero Normal operation */
	DDRD &= ~(1<<DDD6);						// Set Pin D6 as input to read Echo
	PORTD |= (1<<PORTD6);					// Enable pull up on D6
	PORTD &= ~(1<<PD5);						// Init D5 as low (trigger)

	PORTD &= ~(1<<PD7); //Init D7 as low (trigger)

	while(1)
	{	

		if(flagVariable==0){
			/* Give 10us trigger pulse on trig. pin to HC-SR04 */
			PORTD |= (1 << Trigger_pin2);
			_delay_us(10);
			PORTD &= (~(1 << Trigger_pin2));
			
			TCNT1 = 0;	/* Clear Timer counter */
			TCCR1B = 0x41;	/* Capture on rising edge, No prescaler*/
			TIFR1 = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
			TIFR1 = 1<<TOV1;	/* Clear Timer Overflow flag */

			/*Calculate width of Echo by Input Capture (ICP) */
			
			while ((TIFR1 & (1 << ICF1)) == 0);/* Wait for rising edge */
			TCNT1 = 0;	/* Clear Timer counter */
			TCCR1B = 0x01;	/* Capture on falling edge, No prescaler */
			TIFR1 = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
			TIFR1 = 1<<TOV1;	/* Clear Timer Overflow flag */
			TimerOverflow = 0;/* Clear Timer overflow count */

			while ((TIFR1 & (1 << ICF1)) == 0);/* Wait for falling edge */
			count = ICR1 + (65535 * TimerOverflow);	/* Take count */
			/* 8MHz Timer freq, sound speed =343 m/s */
			distance = (double)count / 466.47;

			//Set flag
			if(distance<20){
				flagVariable = 1;
			}

			dtostrf(distance, 2, 2, string);/* distance to string */
			strcat(string, " cm   ");	/* Concat unit i.e.cm */
			lcd_moveto(0x40);
			lcd_stringout("Dist = ");
			lcd_moveto(0x14);
			lcd_stringout(string);	/* Print distance */
			_delay_ms(200);
		}
		
		
		if(flagVariable == 1){
			/* Give 10us trigger pulse on trig. pin to HC-SR04 */
			PORTD |= (1 << Trigger_pin);
			_delay_us(10);
			PORTD &= (~(1 << Trigger_pin));
			
			TCNT1 = 0;	/* Clear Timer counter */
			TCCR1B = 0x41;	/* Capture on rising edge, No prescaler*/
			TIFR1 = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
			TIFR1 = 1<<TOV1;	/* Clear Timer Overflow flag */

			/*Calculate width of Echo by Input Capture (ICP) */
			
			while ((TIFR1 & (1 << ICF1)) == 0);/* Wait for rising edge */
			TCNT1 = 0;	/* Clear Timer counter */
			TCCR1B = 0x01;	/* Capture on falling edge, No prescaler */
			TIFR1 = 1<<ICF1;	/* Clear ICP flag (Input Capture flag) */
			TIFR1 = 1<<TOV1;	/* Clear Timer Overflow flag */
			TimerOverflow = 0;/* Clear Timer overflow count */

			while ((TIFR1 & (1 << ICF1)) == 0);/* Wait for falling edge */
			count = ICR1 + (65535 * TimerOverflow);	/* Take count */
			/* 8MHz Timer freq, sound speed =343 m/s */
			distance_2 = (double)count / 466.47;

			//Set flag
			if(distance_2<20){
				flagVariable = 0;
			}

			dtostrf(distance_2, 2, 2, string);/* distance to string */
			strcat(string, " cm   ");	/* Concat unit i.e.cm */
			lcd_moveto(0x40);
			lcd_stringout("Dist_2 = ");
			lcd_moveto(0x14);
			lcd_stringout(string);	/* Print distance */
			_delay_ms(200);
		}


		//speed = ((15)/(end_t-start_t))/100;
	}
}

/*
  lcd_init - Initialize the LCD
*/
void lcd_init()
{
    _delay_ms(250);             // Wait 500msec for the LCD to start up
    _delay_ms(250);
    sci_out(0xFE);              // Clear the screen
    sci_out(0x51);

    sci_out(0xFE);              // Set contrast to 40
    sci_out(0x52);
    sci_out(40);

    sci_out(0xFE);              // Set backlight to 15
    sci_out(0x53);
    sci_out(15);

    sci_out(0xFE);              // Turn on cursor underline
    sci_out(0x47);

}

/*
  moveto - Move the cursor to the position given by the argument.
  Row 1 Column 0 is 0x00
  Row 2 Column 0 is 0x40
  Row 3 Column 0 is 0x14
  Row 4 column 0 is 0x54
*/
void lcd_moveto(unsigned char pos)
{
    sci_out(0xFE);              // Set the cursor position
    sci_out(0x45);
    sci_out(pos);
}


/*
  lcd_stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void lcd_stringout(char *str)
{
    sci_outs(str);              // Output the string
}

/* ----------------------------------------------------------------------- */

/*
  sci_init - Initialize the SCI port
*/
void sci_init(void) {
    UBRR0 = MYUBRR;          // Set baud rate
    UCSR0B |= (1 << TXEN0);  // Turn on transmitter
    UCSR0C = (3 << UCSZ00);  // Set for asynchronous operation, no parity, 
                             // one stop bit, 8 data bits
}

/*
  sci_out - Output a byte to SCI port
*/
void sci_out(char ch)
{
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

/*
  sci_outs - Print the contents of the character string "s" out the SCI
  port. The string must be terminated by a zero byte.
*/
void sci_outs(char *s)
{
    char ch;

    while ((ch = *s++) != '\0')
        sci_out(ch);
}
