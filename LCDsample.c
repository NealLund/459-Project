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

void lcd_init(void);
void lcd_moveto(unsigned char);
void lcd_stringout(char *);

void sci_init(void);
void sci_out(char);
void sci_outs(char *);

char str1[] = "New Hole";
char str2[] = "Input the number of";
char str3[] = "yards: 000";

char str4[] = "Wind info: "
char str5[] = "Yards left: "
char str6[] = "Like: "
/*
Like calculations:

add 1 yard for every 1mph of headwind
e.g. target is 100yd & you're facing South, wind is blowing North at 10mph, shoot for 110yds

subract 0.5 yard for every 1mph of tailwind
e.g. target is 100yd & you're facing North, wind is blowing North at 10mph, shoot for 95yds
*/
char str7[] = " Consistency: "



#define FOSC 7372800        // Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR 47   // Value for UBRR0 register

int main(void) {

    sci_init();                 // Initialize the SCI port
    
    lcd_init();                 // Initialize the LCD

    /lcd_moveto(0x00);
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
            } /*else if(button_select){
                char dist[3];
                strncopy(dist, str3+7, 3);
                distance = atoi(dist);
                break;
            }*/
        }
        /*while(1) { //main information loop

        }*/
    }

    return 0;   /* never reached */
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
