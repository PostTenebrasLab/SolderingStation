//*******************************//
// Soldering Station
// Matthias Wagner
// www.k-pank.de/so
// Get.A.Soldering.Station@gmail.com
//*******************************//


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#define ST7735_GREY 0x3333
#include <SPI.h>


#include "SolderStation.h"
#include "iron.h"
#include "stationLOGO.h"


Adafruit_ST7735 tft = Adafruit_ST7735(cs_tft, dc, rst);  // Invoke custom library

float pwm = 0; //pwm Out Val 0.. 255
int target = 0;
int actual_temp = 25;
int soll_temp_tmp;
unsigned long last_time;
long sum = 0;
long dt = 0;
boolean standby_act = false;
int initial_pos;
boolean stopped = true;

void setup(void) {
	
	pinMode(BLpin, OUTPUT);
	digitalWrite(BLpin, LOW);
	
	pinMode(STANDBYin, INPUT_PULLUP);
	
	pinMode(PWMpin, OUTPUT);
	digitalWrite(PWMpin, LOW);
	setPwmFrequency(PWMpin, PWM_DIV);
	digitalWrite(PWMpin, LOW);
	
	tft.initR(INITR_BLACKTAB);
	SPI.setClockDivider(SPI_CLOCK_DIV4);  // 4MHz

	initial_pos = analogRead(POTI);
	
	tft.setRotation(0);	// 0 - Portrait, 1 - Lanscape
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(true);
	
	
	
	//Print station Logo
	tft.drawBitmap(2,1,stationLOGO1,124,47,ST7735_GREY);

	tft.drawBitmap(3,3,stationLOGO1,124,47,ST7735_YELLOW);
	tft.drawBitmap(3,3,stationLOGO2,124,47,Color565(254,147,52));
	tft.drawBitmap(3,3,stationLOGO3,124,47,Color565(255,78,0));
	
	//BAcklight on
	digitalWrite(BLpin, HIGH);
	last_time = micros();
	
	
#if defined(INTRO)
	
	delay(500);
	
	//Print Iron
  tft.fillScreen(ST7735_BLACK);
	tft.drawBitmap(0,0,ptl_logo_bits,128,160,ST7735_GREY);
//	tft.drawBitmap(15,50,iron,100,106,ST7735_GREY);
//	tft.drawBitmap(17,52,iron,100,106,ST7735_YELLOW);
	delay(500);
 
	tft.fillScreen(ST7735_BLACK);
	tft.setTextSize(2);
	tft.setTextColor(ST7735_GREY);
	tft.setCursor(70,130);
	tft.print(VERSION);
	
	tft.setTextSize(2);
	tft.setTextColor(ST7735_YELLOW);
	tft.setCursor(72,132);
	tft.print(VERSION);
	
	tft.setTextSize(1);
	tft.setTextColor(ST7735_GREY);
	tft.setCursor(103,0);
	tft.print("v");
	tft.print(VERSION);
	
	tft.setTextColor(ST7735_YELLOW);
	tft.setCursor(104,1);
	tft.print("v");
	tft.print(VERSION);
	
	delay(2500);
#endif
	
	
	tft.fillRect(0,47,128,125,ST7735_BLACK);
	tft.setTextColor(ST7735_WHITE);

	tft.setTextSize(1);
	tft.setCursor(1,84);
	tft.print("actual");
	
	tft.setTextSize(2);
	tft.setCursor(117,47);
	tft.print("o");
	
	tft.setTextSize(1);
	tft.setCursor(1,129);
	tft.print("target");
	
	tft.setTextSize(2);
	tft.setCursor(117,92);
	tft.print("o");
	
	tft.setCursor(80,144);
	tft.print("   %");
	
	tft.setTextSize(1);
	tft.setCursor(1,151);		//60
	tft.print("pwm");
	
	tft.setTextSize(2);

	writeHeating(target, actual_temp, pwm);

}

void loop() {

	int old_temp = actual_temp;
	actual_temp = getTemperature();

	if((abs(analogRead(POTI)-initial_pos) < 20) && stopped)
		return;

	stopped = false;
  
  target = map(analogRead(POTI), 1023, 0, AMBIENT_TEMP, MAX_POTI);
  
	if (!digitalRead(STANDBYin))
		doStandby();
	else
		tft.setTextColor(ST7735_WHITE);



	/* PWM */
	dt = micros() - last_time;
	/* proportional */
	pwm = Kp*(actual_temp-old_temp);
	/* integral */
	sum += (actual_temp-target)*dt;
	pwm += Ki*sum;
	/* derivativ */
	pwm += Kd*(actual_temp-old_temp)/dt;

	// TODO remove this when PWM work
	int diff = (soll_temp_tmp + OVER_SHOT)- actual_temp;
	pwm = diff*CNTRL_GAIN;
	
	int MAX_PWM;

	//Set max heating Power 
	MAX_PWM = actual_temp <= STANDBY_TEMP ? MAX_PWM_LOW : MAX_PWM_HI;

	//8 Bit Range
	pwm = pwm > MAX_PWM ? pwm = MAX_PWM : pwm < 0 ? pwm = 0 : pwm;
	
	//NOTfall sicherheit / Spitze nicht eingesteckt
	if (actual_temp > MAX_POTI + 50){
		pwm = 0;
		actual_temp = 0;
		initial_pos = analogRead(POTI);
		stopped = true;
	}
	
	
	analogWrite(PWMpin, pwm);

	writeHeating(target, actual_temp, pwm);
	
	delay(DELAY_MAIN_LOOP);		//wait for some time
}


int getTemperature()
{
  analogWrite(PWMpin, 0);		//switch off heater
  delay(DELAY_MEASURE);			//wait for some time (to get low pass filter in steady state)
  int adcValue = analogRead(TEMPin); // read the input on analog pin 7:
  Serial.print("ADC Value ");
  Serial.print(adcValue);
  analogWrite(PWMpin, pwm);	//switch heater back to last value
  return round(((float) adcValue)*ADC_TO_TEMP_GAIN+AMBIENT_TEMP); //apply linear conversion to actual temperature
}





void writeHeating(int tempSOLL, int tempVAL, int pwmVAL){
	static int d_tempSOLL = 2;		//Tiefpass für Anzeige (Poti zittern)

	static int tempSOLL_OLD = 	10;
	static int tempVAL_OLD	= 	10;
	static int pwmVAL_OLD	= 	10;
	//TFT Anzeige
	
	pwmVAL = map(pwmVAL, 0, 255, 0, 100);
	
	tft.setTextSize(5);
	if (tempVAL_OLD != tempVAL){
		tft.setCursor(30,57);
		tft.setTextColor(ST7735_BLACK);
		//tft.print(tempSOLL_OLD);
		//erste Stelle unterschiedlich
		if ((tempVAL_OLD/100) != (tempVAL/100)){
			tft.print(tempVAL_OLD/100);
		}
		else
			tft.print(" ");
		
		if ( ((tempVAL_OLD/10)%10) != ((tempVAL/10)%10) )
			tft.print((tempVAL_OLD/10)%10 );
		else
			tft.print(" ");
		
		if ( (tempVAL_OLD%10) != (tempVAL%10) )
			tft.print(tempVAL_OLD%10 );
		
		tft.setCursor(30,57);
		tft.setTextColor(ST7735_WHITE);
		
		if (tempVAL < 100)
			tft.print(" ");
		if (tempVAL <10)
			tft.print(" ");
		
		int tempDIV = round(float(tempSOLL - tempVAL)*8.5);
		tempDIV = tempDIV > 254 ? tempDIV = 254 : tempDIV < 0 ? tempDIV = 0 : tempDIV;
		tft.setTextColor(Color565(tempDIV, 255-tempDIV, 0));
		if (standby_act)
			tft.setTextColor(ST7735_CYAN);
		tft.print(tempVAL);
		
		tempVAL_OLD = tempVAL;
	}
	
	//if (tempSOLL_OLD != tempSOLL){
	if ((tempSOLL_OLD+d_tempSOLL < tempSOLL) || (tempSOLL_OLD-d_tempSOLL > tempSOLL)){
		tft.setCursor(30,102);
		tft.setTextColor(ST7735_BLACK);
		//tft.print(tempSOLL_OLD);
		//erste Stelle unterschiedlich
		if ((tempSOLL_OLD/100) != (tempSOLL/100)){
			tft.print(tempSOLL_OLD/100);
		}
		else
			tft.print(" ");
		
		if ( ((tempSOLL_OLD/10)%10) != ((tempSOLL/10)%10) )
			tft.print((tempSOLL_OLD/10)%10 );
		else
			tft.print(" ");
		
		if ( (tempSOLL_OLD%10) != (tempSOLL%10) )
			tft.print(tempSOLL_OLD%10 );
		
		//Neuen Wert in Weiß schreiben
		tft.setCursor(30,102);
		tft.setTextColor(ST7735_WHITE);
		if (tempSOLL < 100)
			tft.print(" ");
		if (tempSOLL <10)
			tft.print(" ");
		
		tft.print(tempSOLL);
		tempSOLL_OLD = tempSOLL;
		
	}
	
	
	tft.setTextSize(2);
	if (pwmVAL_OLD != pwmVAL){
		tft.setCursor(80,144);
		tft.setTextColor(ST7735_BLACK);
		//tft.print(tempSOLL_OLD);
		//erste stelle Unterscheidlich
		if ((pwmVAL_OLD/100) != (pwmVAL/100)){
			tft.print(pwmVAL_OLD/100);
		}
		else
			tft.print(" ");
		
		if ( ((pwmVAL_OLD/10)%10) != ((pwmVAL/10)%10) )
			tft.print((pwmVAL_OLD/10)%10 );
		else
			tft.print(" ");
		
		if ( (pwmVAL_OLD%10) != (pwmVAL%10) )
			tft.print(pwmVAL_OLD%10 );
		
		tft.setCursor(80,144);
		tft.setTextColor(ST7735_WHITE);
		if (pwmVAL < 100)
			tft.print(" ");
		if (pwmVAL <10)
			tft.print(" ");
		
		tft.print(pwmVAL);
		pwmVAL_OLD = pwmVAL;
		
	}
	
}

/*
 * turn the screen off and change target temp to STANDBY_TEMP
 */
void doStandby(){
  
	tft.setCursor(2,55);
	tft.setTextColor(ST7735_BLACK);
	tft.print("Standby");
//	target = STANDBY_TEMP;

	target = (!stopped && (target >= STANDBY_TEMP ))?  STANDBY_TEMP : target;

}


uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}



void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
