/*************************************************************
*       at328-6.c - Demonstrate interface to a serial LCD display
*
*       This program will print a message on an LCD display
*       using a serial interface.  It is designed to work with a
*       Matrix Orbital LK204-25 using an RS-232 interface.
*
* Revision History
* Date     Author      Description
* 11/07/07 A. Weber    First Release
* 02/26/08 A. Weber    Code cleanups
* 03/03/08 A. Weber    More code cleanups
* 04/22/08 A. Weber    Added "one" variable to make warning go away
* 04/11/11 A. Weber    Adapted for ATmega168
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_wind_direction(void);
void lcd_init(void);
void lcd_moveto(unsigned char);
void lcd_stringout(char *);

void sci_init(void);
void sci_out(char);
void sci_outs(char *);

char str1[] = "New Hole";
char str2[] = "Input the number of";
char str3[] = "yards: 000";


#define FOSC 7372800        // Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR 47   // Value for UBRR0 register

int main(void) {

    sci_init();                 // Initialize the SCI port
    
    lcd_init();                 // Initialize the LCD

    lcd_moveto(0x00);
    lcd_stringout(str1);        // Print string on line 1
    lcd_moveto(0x40);
    lcd_stringout(str2);        // Print string on line 2
    lcd_moveto(0x14);
    lcd_stringout(str3);        // Print string on line 3
    lcd_moveto(0x1B);           //move cursor to first 0
   
    /* NEED TO INITIALIZE
    turning on all the output ports
    reading button_up, button_down, button_left, button_right, button_select
    reading ultrasonic_1 and ultrasonic_2
    reading gps
    reading windvane_volts
    reading windspeed_volts
    */
    unsigned char button_up;
    unsigned char button_down;
    unsigned char button_left;
    unsigned char button_right;
    unsigned char button_select;

    int i = 0x1B;
    int j = 0;
    int distance = 0;
    char currnum[] = "000";

    ADMUX |= (1 << REFS0 ); // Set reference to AVCC
    ADMUX &= ~(1 << REFS1 );
    ADMUX &= ~(1 << ADLAR ); // Left adjust the output
    ADCSRA |= (7 << ADPS0 ); // Set the prescalar to 128
    ADMUX |= (5 << MUX0 ); // Set the channel to 5
    ADCSRA |= (1 << ADEN ); // Enable the ADC

    while (1) {                 // Loop forever
        while (1) {   //initial menu loop
            button_up = (PIND & (1 << PD2));
            button_down = (PINC & (1 << PC3));
            button_left = (PIND & (1 << PD4));
            button_right = (PINC & (1 << PC4));
            button_select = (PIND & (1 << PD3));
            
            if(!button_up){
                if(currnum[j] < 0x39){ currnum[j] = currnum[j]+1; }
                else {currnum[j] = 0x30;}
                lcd_moveto(0x1B);
                lcd_stringout(currnum);
                lcd_moveto(i);
                _delay_ms(250);
            } else if(!button_down) {
                if(currnum[j] > 0x30){currnum[j]=currnum[j]-1;}
                else {currnum[j] = 0x39;}
                lcd_moveto(0x1B);
                lcd_stringout(currnum);
                lcd_moveto(i);
                _delay_ms(250);
            } else if(!button_right){
                if (i < 0x1D ){ 
                    i++;
                    j++;
                } else {
                    i = 0x1B;
                    j = 0;
                }
                lcd_moveto(i); 
                _delay_ms(250);
            } else if(!button_left){
                if (i > 0x1B ){ 
                    i--;
                    j--;
                } else {
                    i = 0x1D;
                    j = 2;
                }
                lcd_moveto(i);
                _delay_ms(250);
            } else if(!button_select){
                sci_out(0xFE);              // Clear the screen
                sci_out(0x51);
                break;
            }
        }
        int winddir;
        char str[50];
        while(1) { //wind vane test loop
            winddir = get_wind_direction();
            lcd_moveto(0x40);
            lcd_stringout("Wind Direction (Deg) ");
            lcd_moveto(0x14);
            sprintf(str, "%04d", winddir);
            lcd_stringout(str);
            _delay_ms(250);
        }
    }

    return 0;   /* never reached */
}

int get_wind_direction()
// read the wind direction sensor, return heading in degrees
{
    unsigned int adc;


    //adc = averageAnalogRead(WDIR); // get the current reading from the sensor
    ADCSRA |= (1 << ADSC ); // Start a conversion
    while ( ADCSRA & (1 << ADSC )){ } // wait for conversion complete 
    adc = ADC;

    // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
\
    // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
    // Note that these are not in compass degree order! See Weather Meters datasheet for more information.
    //return adc;
    //if (adc < 70) return (90);
    //if (adc < 89) return (90);
    if (adc < 103) return (90);

    //if (adc < 132) return (135);
    if (adc < 194) return (135);

    //if (adc < 249) return (180);
    if (adc < 297) return (180);

    //if (adc < 411) return (45);
    if (adc < 471) return (45);

    //if (adc < 606) return (225);
    if (adc < 641) return (225);

    //if (adc < 708) return (0);
    if (adc < 797) return (0);

    //if (adc < 833) return (315);
    if (adc < 897) return (315);

    if (adc < 957) return (270);

    return (-1); // error, disconnected?*/
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
