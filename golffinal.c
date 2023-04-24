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

int hexchar2int(char);
int hex2int(char *);
int checksum_valid(char *);
int parse_comma_delimited_str(char *, char **, int);

int get_wind_direction(void);

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

    int i = 0x1B;
    int j = 0;
    long distance_yds = 0;
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
                distance_yds = (currnum[0]-'0')*100 + (currnum[1]-'0')*10 + (currnum[0]-'0');
                _delay_ms(250);
                break;
            }
        }
        sci_out(0xFE);              // Clear the screen
        sci_out(0x51);
        lcd_moveto(0x00);
        lcd_stringout("New Hole");        // Print string on line 1
        lcd_moveto(0x40);
        lcd_stringout("Input the direction");        // Print string on line 2
        lcd_moveto(0x14);
        lcd_stringout("of the hole: N ");
        lcd_moveto(0x21);

        char direction[] = "N ";
        int dir = 0;
        while(1){ //hole direction loop
            button_up = (PIND & (1 << PD2));
            button_down = (PINC & (1 << PC3));
            button_left = (PIND & (1 << PD4));
            button_right = (PINC & (1 << PC4));
            button_select = (PIND & (1 << PD3));
            if(!button_select){ 
                break;
            }
            else if (strcmp(direction, "N ") == 0) {
                if(!button_up) {
                    direction[1] = 'W';
                    dir = 135;
                    _delay_ms(250);
                } else if (!button_down) {
                    direction[1] = 'E';
                    dir = 225;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "NE") == 0) {
                if(!button_up) {
                    direction[1] = ' ';
                    dir = 180;
                    _delay_ms(250);
                } else if (!button_down) {
                    direction[0] = 'E';
                    direction[1] = ' ';
                    dir = 270;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "E ") == 0) {
                if(!button_up) {
                    direction[0] = 'N';
                    direction[1] = 'E';
                    dir = 225;
                    _delay_ms(250);
                } else if (!button_down){
                    direction[0] = 'S';
                    direction[1] = 'E';
                    dir = 315;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "SE") == 0) {
                if(!button_up) {
                    direction[0] = 'E';
                    direction[1] = ' ';
                    dir = 270;
                    _delay_ms(250);
                } else if (!button_down){
                    direction[0] = 'S';
                    direction[1] = ' ';
                    dir = 0;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "S ") == 0) {
                if(!button_up) {
                    direction[1] = 'E';
                    dir = 315;
                    _delay_ms(250);
                } else if (!button_down){
                    direction[1] = 'W';
                    dir = 45;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "SW") == 0) {
                if(!button_up) {
                    direction[1] = ' ';
                    dir = 0;
                    _delay_ms(250); 
                } else if (!button_down){
                    direction[0] = 'W';
                    direction[1] = ' ';
                    dir = 90;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "W ") == 0) {
                if(!button_up) {
                    direction[0] = 'S';
                    direction[1] = 'W';
                    dir = 45;
                    _delay_ms(250);
                } else if (!button_down){
                    direction[0] = 'N';
                    direction[1] = 'W';
                    dir = 135;
                    _delay_ms(250);
                }
            } else if (strcmp(direction, "NW") == 0) {
                if(!button_up) {
                    direction[0] = 'W';
                    direction[1] = ' ';
                    dir = 90;
                    _delay_ms(250);
                } else if (!button_down){
                    direction[1] = ' ';
                    dir = 180;
                    _delay_ms(250);
                }
            }
            lcd_moveto(0x21);
            lcd_stringout(direction);
            _delay_ms(100);
        }
        char *field[20];
        while(1){ //get inital gps fix
            int b = 0;
            char buffer[255];
            
            lcd_moveto(0x00);
            char c = sci_in();
            if(c == '$'){
                buffer[b] = c;
                b++;
                while (c != '\n'){
                    c = sci_in();
                    buffer[b]=c;
                    b++;
                }
                buffer[b-1] = '\0';
                if (checksum_valid(buffer)) {
                    if ((strncmp(buffer, "$GP", 3) == 0) | (strncmp(buffer, "$GN", 3) == 0)) {
                        if (strncmp(&buffer[3], "GGA", 3) == 0) {
                            i = parse_comma_delimited_str(buffer, field, 20);
                            break;
                        }
                    }
                }
            }
        }
        sci_out(0xFE);              // Clear the screen
        sci_out(0x51);
        sci_out(0xFE);              // Turn off cursor underline
        sci_out(0x48);
        double currlat = (field[2][0]-'0')*10 + (field[2][1]-'0')+ (field[2][2]-'0')*0.1 + (field[2][3]-'0')*0.01 + (field[2][5]-'0')*0.001
                            + (field[2][6]-'0')*0.0001 + (field[2][7]-'0')*0.00001 + (field[2][8]-'0')*0.000001;
        double currlong = (field[4][0]-'0')*100 + (field[4][1]-'0')*10 + (field[4][2]-'0') + (field[4][3]-'0')*0.1 + (field[4][4]-'0')*0.01 
                            + (field[4][6]-'0')*0.001 + (field[4][7]-'0')*0.0001 + (field[4][8]-'0')*0.00001 + (field[4][9]-'0')*0.000001;
        double center = cos(currlong);
        double holelat;
        double holelong;
        double distance = distance_yds*3;
        double distance_side = (double)distance/sqrt(2);
        if (strcmp(direction, "N ") == 0) {
            holelat = currlat + ((double)distance/364310);
            holelong = currlong;
        } else if (strcmp(direction, "NE") == 0) {
            holelat = currlat + ((double)distance_side/364310);
            holelong = currlong - ((double)distance_side/(364310*center));
        } else if (strcmp(direction, "E ") == 0) {
            holelat = currlat;
            holelong = currlong - ((double)distance/(364310*center));
        } else if (strcmp(direction, "SE") == 0) {
            holelat = currlat - ((double)distance_side/364310);
            holelong = currlong - ((double)distance_side/(364310*center));
        } else if (strcmp(direction, "S ") == 0) {
            holelat = currlat - ((double)distance/364310);
            holelong = currlong;
        } else if (strcmp(direction, "SW") == 0) {
            holelat = currlat - ((double)distance_side/364310);
            holelong = currlong + ((double)distance_side/(364310*center));
        }else if (strcmp(direction, "W ") == 0) {
            holelat = currlat;
            holelong = currlong + ((double)distance/(364310*center));
        } else { //NW
            holelat = currlat + ((double)distance_side/364310);
            holelong = currlong + ((double)distance_side/(364310*center));
        }
        int winddir;
        char str[50];
        double x_left;
        double y_left;
        double dist_left;
        lcd_moveto(0x00);
        lcd_stringout("Wind info:00mph N ");
        lcd_moveto(0x40);
        lcd_stringout("Yards left:");        // Print string on line 2
        lcd_stringout(currnum);
        lcd_stringout("yds");
        lcd_moveto(0x14);
        lcd_stringout("Like:");        // Print string on line 2
        lcd_stringout(currnum);
        lcd_stringout("yds 0dg  ");

        int TMP_Therm_ADunits;  //temp termistor value from wind sensor
        double RV_Wind_ADunits;    //RV output from wind sensor 
        double RV_Wind_Volts;
        double zeroWind_ADunits;
        double zeroWind_volts;
        double WindSpeed_MPH;
        double zeroWindAdjustment = 0.2;
        int windspeed;

        int projected_yds_left = distance_yds;

        int hitdeg = 0;
        char hitdir = ' ';

        while(1){ //main loop
            button_select = (PIND & (1 << PD3));
            if(!button_select){ 
                break;
            }
            int b = 0;
            char buffer[255];
            
            lcd_moveto(0x00);
            //get gps
            int printdist;
            char c = sci_in();
            if(c == '$'){
                buffer[b] = c;
                b++;
                while (c != '\n'){
                    c = sci_in();
                    buffer[b]=c;
                    b++;
                }
                buffer[b-1] = '\0';
                if (checksum_valid(buffer)) {
                    if ((strncmp(buffer, "$GP", 3) == 0) | (strncmp(buffer, "$GN", 3) == 0)) {
                        if (strncmp(&buffer[3], "GGA", 3) == 0) {
                            i = parse_comma_delimited_str(buffer, field, 20);
                            currlat = (field[2][0]-'0')*10 + (field[2][1]-'0')+ (field[2][2]-'0')*0.1 + (field[2][3]-'0')*0.01 + (field[2][5]-'0')*0.001
                                + (field[2][6]-'0')*0.0001 + (field[2][7]-'0')*0.00001 + (field[2][8]-'0')*0.000001;
                            currlong = (field[4][0]-'0')*100 + (field[4][1]-'0')*10 + (field[4][2]-'0') + (field[4][3]-'0')*0.1 + (field[4][4]-'0')*0.01 
                                + (field[4][6]-'0')*0.001 + (field[4][7]-'0')*0.0001 + (field[4][8]-'0')*0.00001 + (field[4][9]-'0')*0.000001;
                            x_left = fabs(currlong-holelong)*364310*center;
                            y_left = fabs(currlat-holelat)*364310;
                            dist_left = sqrt(x_left*x_left + y_left*y_left);
                            printdist = round(dist_left/3);
                            lcd_moveto(0x4B);
                            sprintf(str, "%03d", printdist);
                            lcd_stringout(str);        // Print string on line 2
                        }
                    }
                }
            }

            //get wind speed
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
            zeroWind_ADunits = -0.0006*((double)TMP_Therm_ADunits * (double)TMP_Therm_ADunits) + 1.0727 * (double)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39
            zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  
            
            if(RV_Wind_Volts > zeroWind_volts){
                WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) / 0.2300) , 2.7265); 
            }
            windspeed = round(WindSpeed_MPH);

            winddir = get_wind_direction();
            _delay_ms(250);
            switch(winddir){
                case 180:
                    direction[0] = 'N';
                    direction[1] = ' ';
                    break;
                case 225:
                    direction[0] = 'N';
                    direction[1] = 'E';
                    break;
                case 270:
                    direction[0] = 'E';
                    direction[1] = ' ';
                    break;
                case 315:
                    direction[0] = 'S';
                    direction[1] = 'E';
                    break;
                case 0:
                    direction[0] = 'S';
                    direction[1] = ' ';
                    break;
                case 45:
                    direction[0] = 'S';
                    direction[1] = 'W';
                    break;
                case 90:
                    direction[0] = 'W';
                    direction[1] = ' ';
                    break;
                case 135:
                    direction[0] = 'N';
                    direction[1] = 'W';
                    break;
                default:
                    direction[0] = '-';
                    direction[1] = '1';
            }

            lcd_moveto(0x00);
            lcd_stringout("Wind info:");
            sprintf(str, "%02dmph ", windspeed);
            lcd_stringout(str);
            lcd_stringout(direction);

            int wind_i = windspeed;
            hitdeg = 0;
            if((dir - winddir)==0){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    projected_yds_left = printdist*0.1 + printdist;
                }
                hitdeg = 0;
                hitdir = ' ';
            }
            else if(abs(dir - winddir) == 45){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    projected_yds_left = printdist*0.05 + printdist;
                    hitdeg += 4;
                }
                hitdir = 'R';
            }
            else if(abs(dir - winddir) == 315){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    projected_yds_left = printdist*0.05 + printdist;
                    hitdeg += 4;
                }
                hitdir = 'L';
            }
            else if(abs(dir - winddir) == 90){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    hitdeg += 8;
                }
                hitdir = 'R';
            }
            else if(abs(dir - winddir) == 270){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    hitdeg += 8;
                }
                hitdir = 'L';
            }
            else if(abs(dir - winddir) == 180){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    projected_yds_left = printdist - printdist*0.05;
                }
                hitdir = ' ';
            }
            else if(abs(dir - winddir) == 135){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    projected_yds_left = printdist - printdist*0.025;
                    hitdeg += 4;
                }
                hitdir = 'R';
            }
            else if(abs(dir - winddir) == 225){
                projected_yds_left = printdist;
                while(wind_i >= 5){
                    wind_i = wind_i - 5;
                    projected_yds_left = printdist - printdist*0.025;
                    hitdeg += 4;
                }
                hitdir = 'L';
            }
            else {
                projected_yds_left = printdist;
                hitdeg = 0;
                hitdir = ' ';
            }
            lcd_moveto(0x19);
            sprintf(str, "%03dyds ", projected_yds_left);
            lcd_stringout(str);
            sprintf(str, "%02ddg ", hitdeg);
            lcd_stringout(str);
            sci_out(hitdir);

        }
        /*while(1) { //wind vane test loop
            winddir = get_wind_direction();
            lcd_moveto(0x40);
            lcd_stringout("Wind Direction (Deg) ");
            lcd_moveto(0x14);
            sprintf(str, "%04d", winddir);
            lcd_stringout(str);
            _delay_ms(250);
        }*/

        //the dream
        /*lcd_moveto(0x00);
        lcd_stringout("Wind info:00mph N ");        // Print string on line 1
        lcd_moveto(0x40);
        lcd_stringout("Yards left:000yds");        // Print string on line 2
        lcd_moveto(0x14);
        lcd_stringout("Like:000yds 0dg N ");        // Print string on line 3
        lcd_moveto(0x54);
        lcd_stringout("Spd:00mph Cnst:000%");*/
    }

    return 0;   /* never reached */
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
    ADMUX |= (5 << MUX0 ); // Set the channel to 5
    ADCSRA |= (1 << ADSC ); // Start a conversion
    while ( ADCSRA & (1 << ADSC )){ } // wait for conversion complete 
    adc = ADC;
    ADMUX &= ~(5 << MUX0); //Clear channel selection

    // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
    // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
    // Note that these are not in compass degree order! See Weather Meters datasheet for more information.
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
    UCSR0B |= (1 << RXEN0);  // Turn on receiver
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
