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
#include<math.h>

int hexchar2int(char);
int hex2int(char *);
int checksum_valid(char *);
int parse_comma_delimited_str(char *, char **, int);

int get_wind_direction(void);
double get_wind_speed(void);

void lcd_init(void);
void lcd_moveto(unsigned char);
void lcd_stringout(char *);

void sci_init(void);
char sci_in(void);
void sci_out(char);
void sci_outs(char *);


#define FOSC 7372800        // Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR 47   // Value for UBRR0 register

int main(void) {

    sci_init();                 // Initialize the SCI port
    
    lcd_init();                 // Initialize the LCD

    lcd_moveto(0x00);
    lcd_stringout("New Hole");        // Print string on line 1
    lcd_moveto(0x40);
    lcd_stringout("Input the number of");        // Print string on line 2
    lcd_moveto(0x14);
    lcd_stringout("yards: 000");        // Print string on line 3
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

    double TMP_Therm_ADunits;  //temp termistor value from wind sensor
    double RV_Wind_ADunits;    //RV output from wind sensor 
    double RV_Wind_Volts;
    int TempCtimes100;
    double zeroWind_ADunits;
    double zeroWind_volts;
    double WindSpeed_MPH;

    int i = 0x1B;
    int j = 0;
    long distance = 0;
    char currnum[] = "000";

    ADMUX |= (1 << REFS0 ); // Set reference to AVCC
    ADMUX &= ~(1 << REFS1 );
    ADMUX &= ~(1 << ADLAR ); // 10-Bit output
    ADCSRA |= (7 << ADPS0 ); // Set the prescalar to 128
    //ADMUX |= (5 << MUX0 ); // Set the channel to 5
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
                //distance = strol(currnum, NULL, 10);
                distance = (currnum[0]-'0')*100 + (currnum[1]-'0')*10 + (currnum[0]-'0');
                sci_out(0xFE);              // Clear the screen
                sci_out(0x51);
                break;
            }
        }
        lcd_moveto(0x00);
        lcd_stringout("New Hole");        // Print string on line 1
        lcd_moveto(0x40);
        lcd_stringout("Input the direction");        // Print string on line 2
        lcd_moveto(0x14);
        lcd_stringout("of the hole: N ");
        lcd_moveto(0x20);

        char direction[] = "N ";
        while(1){ //hole direction loop
            if(!button_select){ 
                sci_out(0xFE);              // Clear the screen
                sci_out(0x51);
                break;
            }
            if (strcmp(direction, "N ") == 0) {
                if(!button_up) {
                direction[1] = 'W';
                } else if (!button_down) {
                direction[1] = 'E';
                }
            } else if (strcmp(direction, "NE") == 0) {
                if(!button_up) {
                direction[1] = ' ';
                } else if (!button_down) {
                direction[0] = 'E';
                direction[1] = ' ';
                }
            } else if (strcmp(direction, "E ") == 0) {
                if(!button_up) {
                direction[0] = 'N';
                direction[1] = 'E';
                } else if (!button_down){
                direction[0] = 'S';
                direction[1] = 'E';
                }
            } else if (strcmp(direction, "SE") == 0) {
                if(!button_up) {
                direction[0] = 'E';
                direction[1] = ' ';
                } else if (!button_down){
                direction[0] = 'S';
                direction[1] = ' ';
                }
            } else if (strcmp(direction, "S ") == 0) {
                if(!button_up) {
                direction[1] = 'E';
                } else if (!button_down){
                direction[1] = 'W';
                }
            } else if (strcmp(direction, "SW") == 0) {
                if(!button_up) {
                direction[1] = ' '; 
                } else if (!button_down){
                direction[0] = 'W';
                direction[1] = ' ';
                }
            }else if (strcmp(direction, "W ") == 0) {
                if(!button_up) {
                direction[0] = 'W';
                direction[1] = ' ';
                } else if (!button_down){
                direction[0] = 'N';
                direction[1] = 'W';
                }
            } else {
                if(!button_up) { //NW
                direction[0] = 'W';
                direction[1] = ' ';
                } else if (!button_down){
                direction[1] = ' ';
                }
            }
            lcd_moveto(0x20);
            lcd_stringout(direction);
        }
        int winddir;
        double windspeed;
        char str[50];
        while(1) { //wind  test loop
            winddir = get_wind_direction();
            _delay_ms(250);
            if(winddir >= 0 && winddir <= 30){ //N direction
                direction[0] = 'N';
                direction[1] = ' ';
            }
            else if(winddir > 30 && winddir <= 75){ //NE direction
                direction[0] = 'N';
                direction[1] = 'E';
            }
            else if(winddir > 75 && winddir <= 105){ //E direction
                direction[0] = 'E';
                direction[1] = ' ';
            }
            else if(winddir > 105 && winddir <= 165){ //SE Direction
                direction[0] = 'S';
                direction[1] = 'E';
            }
            else if(winddir > 165 && winddir <= 195){ //S Direction
                direction[0] = 'S';
                direction[1] = ' ';
            }
            else if(winddir > 195 && winddir <= 255){ //SW Direction
                direction[0] = 'S';
                direction[1] = 'W';
            }
            else if(winddir > 255 && winddir <= 290){ //W Direction
                direction[0] = 'W';
                direction[1] = ' ';
            }
            else { //NW Direction
                direction[0] = 'N';
                direction[1] = 'W';
            }

            lcd_moveto(0x40);
            lcd_stringout("Wind Info: ");
            lcd_moveto(0x14);
            sprintf(str, "%04d", winddir);
            windspeed = get_wind_speed();
            sprintf(str, "%d mph", windspeed);
            lcd_stringout(str);
            lcd_moveto(0x);
            lcd_stringout(direction);
        }




        //the dream
        lcd_moveto(0x00);
        lcd_stringout("Wind info:00mph N ");        // Print string on line 1
        lcd_moveto(0x40);
        lcd_stringout("Yards left:000yds");        // Print string on line 2
        lcd_moveto(0x14);
        lcd_stringout("Like:000yds 0dg N ");        // Print string on line 3
        cd_moveto(0x54);
        lcd_stringout("Spd:00mph Const:000%");
    }

    return 0;   /* never reached */
}

double get_wind_speed(){
        double zeroWindAdjustment = 0.2;
        double TMP_Therm_ADunits;  //temp termistor value from wind sensor
        double RV_Wind_ADunits;    //RV output from wind sensor 
        double RV_Wind_Volts;
        int TempCtimes100;
        double zeroWind_ADunits;
        double zeroWind_volts;
        double WindSpeed_MPH;

        ADMUX |= (1 << MUX0 ); // Set the channel to 1
        ADCSRA |= (1 << ADSC ); // Start a conversion
        while ( ADCSRA & (1 << ADSC )){} // wait for conversion complete
        
        TMP_Therm_ADunits = ADC;
        ADMUX &= ~(1 << MUX0); 
        ADMUX |= (2 << MUX0 ); // Set the channel to 2
        ADCSRA |= (1 << ADSC ); // Start a conversion
        while ( ADCSRA & (1 << ADSC )){} // wait for conversion complete
        RV_Wind_ADunits = ADC;
        ADMUX &= ~(2 << MUX0); 

        
        RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);


        // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
        // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
        TempCtimes100 = (0.005 *((double)TMP_Therm_ADunits * (double)TMP_Therm_ADunits)) - (16.862 * (double)TMP_Therm_ADunits) + 9075.4;  

        zeroWind_ADunits = -0.0006*((double)TMP_Therm_ADunits * (double)TMP_Therm_ADunits) + 1.0727 * (double)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39

        zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  

        // This from a regression from data in the form of 
        // Vraw = V0 + b * WindSpeed ^ c
        // V0 is zero wind at a particular temperature
        // The constants b and c were determined by some Excel wrangling with the solver.
        
        if(RV_Wind_Volts > zeroWind_volts){
            WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) / 0.2300) , 2.7265); 
        }
        return WindSpeed_MPH;
}

int hexchar2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

int hex2int(char *c)
{
    int value;

    value = hexchar2int(c[0]);
    value = value << 4;
    value += hexchar2int(c[1]);

    return value;
}

int checksum_valid(char *string)
{
    char *checksum_str;
    int checksum;
    unsigned char calculated_checksum = 0;

    // Checksum is postcede by *
    checksum_str = strchr(string, '*');
    if (checksum_str != NULL){
        // Remove checksum from string
        *checksum_str = '\0';
        // Calculate checksum, starting after $ (i = 1)
        int i = 1;
        for (i = 1; i < strlen(string); i++) {
            calculated_checksum = calculated_checksum ^ string[i];
        }
        checksum = hex2int((char *)checksum_str+1);
        //printf("Checksum Str [%s], Checksum %02X, Calculated Checksum %02X\r\n",(char *)checksum_str+1, checksum, calculated_checksum);
        if (checksum == calculated_checksum) {
            //printf("Checksum OK");
            return 1;
        }
    } else {
        //printf("Error: Checksum missing or NULL NMEA message\r\n");
        return 0;
    }
    return 0;
}

int parse_comma_delimited_str(char *string, char **fields, int max_fields)
{
    int i = 0;
    fields[i++] = string;

    while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
        *string = '\0';
        fields[i++] = ++string;
    }

    return --i;
}

int get_wind_direction()
// read the wind direction sensor, return heading in degrees
{
    unsigned int adc;

    ADMUX |= (5 << MUX0 );
    //adc = averageAnalogRead(WDIR); // get the current reading from the sensor
    ADCSRA |= (1 << ADSC ); // Start a conversion
    while ( ADCSRA & (1 << ADSC )){ } // wait for conversion complete 
    adc = ADC;
    ADMUX &= ~(5 << MUX0);
    // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
    // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
    // Note that these are not in compass degree order! See Weather Meters datasheet for more information.
    
    if (adc < 70) return (113);
    if (adc < 103) return (90);
    if (adc < 194) return (135);
    if (adc < 297) return (180);
    if (adc < 471) return (45);
    if (adc < 641) return (225);
    if (adc < 797) return (0);
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
  sci_in - Read a byte from SCI port
*/
char sci_in ()
{
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
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
