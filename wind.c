
#define analogPinForRV    PC2   // change to pins you the analog pins are using
#define analogPinForTMP   PC1

// to calibrate your sensor, put a glass over it, but the sensor should not be
// touching the desktop surface however.
// adjust the zeroWindAdjustment until your sensor reads about zero with the glass over it. 

const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa.

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor 
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;

// ADC SETUP //
ADMUX |= (1 << REFS0 ); // Set reference to AVCC
ADMUX &= ~(1 << REFS1 );
ADMUX |= (1 << ADLAR ); // Left adjust the output
ADCSRA |= (7 << ADPS0 ); // Set the prescalar to 128
ADMUX |= (1 << MUX0 ); // Set the channel to 3
ADCSRA |= (1 << ADEN ); // Enable the ADC


void setup() {

  Serial.begin(9600);   // faster printing to get a bit better throughput on extended info
  // remember to change your serial monitor

  Serial.println("start");
  // put your setup code here, to run once:

  //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  //pinMode(A2, INPUT);        // GND pin      
  //pinMode(A3, INPUT);        // VCC pin
  //digitalWrite(A3, LOW);     // turn off pullups

}



void loop() {

  while (1) {
    ADCSRA |= (1 << ADSC ); // Start a conversion
    while ( ADCSRA & (1 << ADSC )); // wait for conversion complete
      temperature_temp = ADCH ; // Get converted value
  }

  if (millis() - lastMillis > 200){      // read every 200 ms - printing slows this down further
    
    //TMP_Therm_ADunits = analogRead(analogPinForTMP);
    TMP_Therm_ADunits = temperature_temp;
    //RV_Wind_ADunits = analogRead(analogPinForRV);
    //RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);

    // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
    // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
    TempCtimes100 = (0.005 *((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;  

    zeroWind_ADunits = -0.0006*((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39

    zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  

    // This from a regression from data in the form of 
    // Vraw = V0 + b * WindSpeed ^ c
    // V0 is zero wind at a particular temperature
    // The constants b and c were determined by some Excel wrangling with the solver.
    
   //WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) /.2300) , 2.7265);   
   
    Serial.print("  TMP volts ");
    Serial.print(TMP_Therm_ADunits * 0.0048828125);
    
    //Serial.print(" RV volts ");
    //Serial.print((float)RV_Wind_Volts);

    Serial.print("\t  TempC*100 ");
    Serial.print(TempCtimes100 );

    Serial.print("   ZeroWind volts ");
    Serial.print(zeroWind_volts);

    Serial.print("   WindSpeed MPH ");
    Serial.println((float)WindSpeed_MPH);
    lastMillis = millis();    
  } 

}
