
#include <OneWire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306();

int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 5


//Temperature chip i/o
OneWire ds(DS18S20_Pin); // on digital pin 5

#define VBATPIN A9
   
void setup() {  

  //small delay needed to get the display to behave with the power switch
  delay(200);  
  Serial.begin(9600);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  // initialize with the I2C addr 0x3C (for the 128x32)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  // init done
  display.setTextSize(1);
  display.setTextColor(WHITE,BLACK);
  // Clear the buffer.
  display.clearDisplay();
  display.display();

}



//adjust this to your desired reference temperature
float reftemp = 18.0;
//varies from species to species- x intercept of the linear temp vs velocity relationship
float tzero = 7.5;  
//optional offset value to calibrate the temperature sensor
float offset = -0.1;

//resetting our counters to zero for a new experiment
float previous_time = millis();
float cumulative_temp = 0;
float cumulative_effective_temp = 0;
float current_temp = getTemp();


void loop() {

float new_temp = getTemp();
if (new_temp != -1000)     //ignores random errors from the ds18b20...
{
 current_temp = new_temp;
}


//measures temperature for this interval
current_temp = current_temp + offset; 
//gets the amount of time since sketch began running
float current_time = millis(); 
//calculates time since the last loop
float interval_length = current_time - previous_time; 
//calculates the effective temperature for this interval
float effective_temp = current_temp - tzero; 
//now we add the effective degrees for this interval to the cumulative effective degrees
cumulative_effective_temp = cumulative_effective_temp + (effective_temp * interval_length);
//new we can calculate the temperature adjusted developmental time (in minutes)
float adjusted_time = cumulative_effective_temp/((reftemp-tzero)*60000);

//This will calulate the total degrees in this interval, add it to the cumulative degrees,
//and give us an average temperature for the experiment
cumulative_temp = cumulative_temp + (current_temp * interval_length);
float average_temp = (cumulative_temp / current_time);

//finally we need to set the "previous time" for the next loop to the be current time in this loop
previous_time = current_time;

//measuring the voltage in the LiPo battery
float measuredvbat = analogRead(VBATPIN);
measuredvbat *= 2;    // we divided by 2, so multiply back
measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
measuredvbat /= 1024; // convert to voltage

//Displays the current T-zero
display.setCursor(85,0);
display.print("Tz");
display.setCursor(103,0);
display.print(tzero,2);

//Displays the reference temp of choice
display.setCursor(85,8);
display.print("Ref");
display.setCursor(103,8);
display.print(reftemp,1);

//Displays the offset for our temperature probe (if necessary)
display.setCursor(85,16);
display.print("Off");
display.setCursor(103,16);
display.print(offset,1);

//Displays battery voltage
display.setCursor(85,24);
display.print("Vbat");
display.setCursor(109,24);
display.print(measuredvbat,1);

//Displays the current temperature
display.setCursor(0, 0);
display.print("Temp");
display.setCursor(43,0);
display.print(current_temp,1);

//Displays the average temperature for the experiment
display.setCursor(0,8);
display.print("AveTemp");
display.setCursor(43,8);
display.print(average_temp,1);

//Displays the temperature adjusted developmental time
display.setCursor(0,24);
display.print("AdjTime");
display.setCursor(43,24);
display.print(adjusted_time,1);

//Displays the actual time elapsed during the experiment (in minutes)
display.setCursor(0,16);
display.print("TruTime");
display.setCursor(43,16);
display.print(current_time/60000,1);

display.display();
delay(1000); 
 
}


// code for bildr.org for reading temperatures from the DS18B20
  float getTemp(){
 //returns the temperature from one DS18S20 in DEG Celsius

 byte data[12];
 byte addr[8];

 if ( !ds.search(addr)) {
   //no more sensors on chain, reset search
   ds.reset_search();
   return -1000;
 }

 if ( OneWire::crc8( addr, 7) != addr[7]) {
   Serial.println("CRC is not valid!");
   return -1000;
 }

 if ( addr[0] != 0x10 && addr[0] != 0x28) {
   Serial.print("Device is not recognized");
   return -1000;
 }

 ds.reset();
 ds.select(addr);
 ds.write(0x44,1); // start conversion, with parasite power on at the end

 byte present = ds.reset();
 ds.select(addr);  
 ds.write(0xBE); // Read Scratchpad

 
 for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
 }
 
 ds.reset_search();
 
 byte MSB = data[1];
 byte LSB = data[0];

 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float TemperatureSum = tempRead / 16;
 
 return TemperatureSum;
 
}
