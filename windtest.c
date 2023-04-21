#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<math.h>

void lcd_init(void);
void lcd_moveto(unsigned char);
void lcd_stringout(char *);

void sci_init(void);
void sci_out(char);
void sci_outs(char *);

const float zeroWindAdjustment =  0.2;
#define MYUBRR 47   // Value for UBRR0 register
int main(void) {

    sci_init();                 // Initialize the SCI port
    lcd_init();                 // Initialize the LCD

    //lcd_moveto(0x00);
    int TMP_Therm_ADunits;  //temp termistor value from wind sensor
	float RV_Wind_ADunits;    //RV output from wind sensor 
	float RV_Wind_Volts;
	int TempCtimes100;
	float zeroWind_ADunits;
	float zeroWind_volts;
	float WindSpeed_MPH;
	int temperature_temp = 0;
	int wind_wind = 0;

	ADMUX |= (1 << REFS0 ); // Set reference to AVCC
	ADMUX &= ~(1 << REFS1 );
	ADMUX &= ~(1 << ADLAR ); // 10 bit mode
	ADCSRA |= (7 << ADPS0 ); // Set the prescalar to 128
	//ADMUX |= (1 << MUX0 ); // Set the channel to 3
	ADCSRA |= (1 << ADEN ); // Enable the ADC

	while(1){
		ADMUX |= (1 << MUX0 ); // Set the channel to 1
		ADCSRA |= (1 << ADSC ); // Start a conversion
      	while ( ADCSRA & (1 << ADSC )){ // wait for conversion complete
        	
    	}
    	TMP_Therm_ADunits = ADC;
    	ADMUX &= ~(1 << MUX0); 
	    //TMP_Therm_ADunits = analogRead(analogPinForTMP);
	    //RV_Wind_ADunits = analogRead(analogPinForRV);
	    ADMUX |= (2 << MUX0 ); // Set the channel to 2
	    ADCSRA |= (1 << ADSC ); // Start a conversion
      	while ( ADCSRA & (1 << ADSC )){ // wait for conversion complete
        	
    	}
    	RV_Wind_ADunits = ADC;
    	ADMUX &= ~(2 << MUX0); 

    	
	    RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);


	    // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
	    // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
	    TempCtimes100 = (0.005 *((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;  

	    zeroWind_ADunits = -0.0006*((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39

	    zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  

	    // This from a regression from data in the form of 
	    // Vraw = V0 + b * WindSpeed ^ c
	    // V0 is zero wind at a particular temperature
	    // The constants b and c were determined by some Excel wrangling with the solver.
	    
	   	if(RV_Wind_Volts > zeroWind_volts){
	   		WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) / 0.2300) , 2.7265); 
	   	}

	    //lcd_moveto(0x40);
	    //lcd_stringout("TMP volts: ");
	    //Serial.print(TMP_Therm_ADunits * 0.0048828125);
	    /*int speed = round(TMP_Therm_ADunits * 0.0048828125);
	    char str[3];
		sprintf(str, "%03d", speed);
	    lcd_stringout(str);*/

	   	char str[50];
	    /*lcd_moveto(0x40);
	    lcd_stringout("TempC: ");
	    int temp = round(TempCtimes100/100);
	    sprintf(str, "%03d", temp);
	    lcd_stringout(str);*/

	    lcd_moveto(0x00);
	    lcd_stringout("RV volts ");
	    sprintf(str,"%05.2f", (float)RV_Wind_Volts);
		//printf(str);
	    lcd_stringout(str);

	    lcd_moveto(0x40);
	    lcd_stringout("TempC*100 ");
	    sprintf(str, "%03d", TempCtimes100);
	    lcd_stringout(str);
	    
	    lcd_moveto(0x14);
	    lcd_stringout("ZeroWind volts ");
	    sprintf(str,"%05.2f", zeroWind_volts);
		//printf(str);
	    lcd_stringout(str);

	    lcd_moveto(0x54);
	    lcd_stringout("WindSpeed MPH ");
	    //int speed = (WindSpeed_MPH);
		//fprintf(str, "%e", speed);
		//dtostrf(WindSpeed_MPH, 6, 4, str); 
		sprintf(str,"%05.2f", (float)WindSpeed_MPH);
		//printf(str);
	    lcd_stringout(str);

	    _delay_ms(500);
	}

	return 0;
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



