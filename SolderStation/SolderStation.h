
#define VERSION "1.1"

//#define INTRO
#define INVERT_POT
//#define DEBUG

#define sclk        13		// Don't change
#define mosi        11		// Don't change
#define cs_tft      10		//

#define dc       	9
#define rst  	    12

#define STANDBYin   A4
#define POT   	    A5
#define TEMPin 	    A7
#define PWMpin 	    3
#define BLpin		5

#define PWM_CTE     0.17    // PWM ~ 0.17 * target -> maintain temp
#define Kp          3.0     // old 1.4
#define Ki          0.00001     // 
#define Kd          2.0     // 

#define DELAY_MEASURE 		50
#define ADC_TO_TEMP_GAIN 	0.53     // Compared to the original Weller Station
#define AMBIENT_TEMP        25
#define STANDBY_TEMP		175
#define MAX_IDLE_TIME       600000  // max idle time (msec) before automatically power the solder off

#define MAX_POT 		    400		//400C
#define PWM_MAX             230     // 0..255
#define PWM_DIV             1024	//default: 64   31250/64 = 2ms

int getTemperature();
void writeHeating(int tempSOLL, int tempVAL, int pwmVAL);
void doStandby();
uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
void setPwmFrequency(int pin, int divisor);
