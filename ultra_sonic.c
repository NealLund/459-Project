//Ultra Sonic Sensor 1
// Echo Pin: PD6
// Trigger Pin: PD5

//Ultra Sonic Sensor 2
// Echo Pin: PB0
// Trigger Pin: PD7


int duration, distance; // Add types 'duration' and 'distance'.
char cm[] = "cm";

void loop(){
	while(1){
		echoPin = (PIND & (1 << PD6));
	    triggerPin = PD5;

		digitalWrite (triggerPin, HIGH);
		delay(50);
		digitalWrite (triggerPin, LOW);
		duration=pulseIn(echoPin,HIGH);
		distance=(duration/2)/29.1;
		delay(50);
		lcd_moveto(0x30);
        lcd_stringout(ditance);
        lcd_moveto(0x1B)
        lcd_stringout(cm);
	}
}
