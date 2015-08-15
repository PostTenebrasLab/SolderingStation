#define VERSION "1.0"
#define INTRO

#define sclk 	      13		// Don't change
#define mosi 	      11		// Don't change
#define cs_tft	    10		//

#define dc       	  9
#define rst  	      12

#define STANDBYin   A4
#define POTI   	    A5
#define TEMPin 	    A7
#define PWMpin 	    3
#define BLpin		    5

#define CNTRL_GAIN  10
#define Kp          1
#define Ki          1
#define Kd          1

#define DELAY_MAIN_LOOP 	10
#define DELAY_MEASURE 		50
#define ADC_TO_TEMP_GAIN 	0.53     // Compared to the original Weller Station
#define AMBIENT_TEMP      25.0
#define STANDBY_TEMP		  175

#define OVER_SHOT 			  2
#define MAX_PWM_LOW			  180
#define MAX_PWM_HI			  210		//254
#define MAX_POTI			    400		//400C

#define PWM_DIV           1024	//default: 64   31250/64 = 2ms

int getTemperature();
void writeHeating(int tempSOLL, int tempVAL, int pwmVAL);
void doStandby();
uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
void setPwmFrequency(int pin, int divisor);
