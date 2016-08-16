/*
 * DMSProject_2015.c
 *
 * Created: 08/03/2015 02:08:27
 *  Author: Kuzan
 */ 


#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#define LCD_DATA PORTD   	// In my case PORTB is the PORT from which I send data to my LCD
#define Control_PORT PORTB		// In my case PORTC is the PORT from which I set the RS , R/W and En
#define En PORTB2		// Enable signal
#define RW PORTB1		// Read/Write signal
#define RS PORTB0		// Register Select signal
#define state_closed 0	//curtains state
#define state_open 1	//crrtains state
#define outside_bright 1	// state
#define outside_dark 0	// state
#define auto_mode 1
#define overide_mode 0

#define outside_threshold 53
static int state;
static int mode;
static int outside;
static uint16_t brightness;
void LCD_cmd(unsigned char cmd)
{
	LCD_DATA=cmd;
	Control_PORT =(0<<RS)|(0<<RW)|(1<<En);	// RS and RW as LOW and EN as HIGH
	_delay_ms(1);
	Control_PORT =(0<<RS)|(0<<RW)|(0<<En);	// RS, RW , LOW and EN as LOW
	_delay_ms(1);
	return;
}


void LCD_write(unsigned char data)
{
	LCD_DATA= data;
	Control_PORT = (1<<RS)|(0<<RW)|(1<<En);	// RW as LOW and RS, EN as HIGH
	_delay_ms(1);
	Control_PORT = (1<<RS)|(0<<RW)|(0<<En);	// EN and RW as LOW and RS HIGH
	_delay_ms(1);			// delay to get things executed
	return ;
}

void move_forward(int time){
	int seconds=0;
	TCNT1 = 0;
	PORTB |= 1 << PINB6;
	while(seconds<time){//one second passes
		if (TCNT1 > 15625)
		{
			TCNT1 = 0;
			seconds++;
		}	
	}
	PORTB &= 0 << PINB6;
	
}


void move_back(int time){
	int seconds=0;
	TCNT1 = 0;
	PORTB |= 1 << PINB7;
	while(seconds<time){
		if (TCNT1 > 15625)//one second passes
		{
			TCNT1 = 0;
			seconds++;
		}
	}
	PORTB &= 0 << PINB7;
}

void LCDWriteString(const char *msg)
{
		LCD_cmd(0x01); // clear LCD
		_delay_ms(1);
		
		LCD_cmd(0x0E); // cursor ON
		_delay_ms(1);
		
		LCD_cmd(0x80); // ---8 go to first line and --0 is for 0th position
		_delay_ms(1);
	int i=0;
	while (msg[i]!='\0'){
		LCD_write(msg[i]);
		i++;
	}
}

void init_LCD(){
		LCD_cmd(0x38); // initialization of 16X2 LCD in 8bit mode
		_delay_ms(1);
		
		LCD_cmd(0x01); // clear LCD
		_delay_ms(1);
		
		LCD_cmd(0x0E); // cursor ON
		_delay_ms(1);
		
		LCD_cmd(0x80); // ---8 go to first line and --0 is for 0th position
		_delay_ms(1);	
}
int main(void)
{
	DDRB=0xFF;//set pin 0 and 1 of port B to be output for the motor
	DDRD=0xFF;//set port D to be outputs for LDR
	DDRC=0x00;//set port C to be input

	TCCR1B |= 1<<CS10 | 1<<CS11;//set prescaler of internal clock to 64
	PORTB |=1<<PORTB3;
	state=state_closed;
	outside=outside_bright;
	mode=auto_mode;
	_delay_ms(200);//pause 2 seconds
	
	ADCSRA=0x8F;
	ADMUX=0x20;
	sei();
		
	ADCSRA|=(1<<ADSC);
	
	init_LCD();
	//move_back(5);//close curtains
	LCDWriteString("Welcome");
	_delay_ms(2000);
	while(1)
	{
		if(bit_is_set(PINC,1)){
			mode=overide_mode;
		}
		else{
			mode=auto_mode;
		}
		
		
		//TODO:: Please write your application code
		if(outside==outside_dark && state==state_open && mode==auto_mode){
			LCDWriteString("Dark Oustside");
			_delay_ms(1000);
			LCDWriteString("Lights On");
			PORTB |=1<<PORTB3;
			_delay_ms(500);
			LCDWriteString("Curtains Closing ...");
			move_back(5);//close curtains
			state=state_closed;
			LCDWriteString("Curtains Closed");
			_delay_ms(1000);
			LCDWriteString("Dark Outside");
			
					LCD_cmd(0x01); // clear LCD
					LCDWriteString("Level: ");
					LCD_write(brightness);
					_delay_ms(1);
					_delay_ms(1000);
		}
		else if(outside==outside_bright && state==state_closed && mode==auto_mode){
			LCDWriteString("Bright Oustside");
			_delay_ms(1000);
			LCDWriteString("Lights Off");
			PORTB &=0<<PORTB3;
			_delay_ms(500);
			LCDWriteString("Curtains opening ...");
			move_forward(5);//open curtains
			state=state_open;
			LCDWriteString("Curtains Open");
			_delay_ms(1000);
			LCDWriteString("Dark Bright");
			
					LCD_cmd(0x01); // clear LCD
					LCDWriteString("Level: ");
					LCD_write(brightness);
					_delay_ms(1);
					_delay_ms(1000);
		}
		else if(mode==overide_mode){
			LCDWriteString("Override Mode");
			_delay_ms(1000);
		}
	}
}

ISR(ADC_vect){
	
	uint16_t x= (ADCL>>6);
	uint16_t y=(ADCH<<2);
	x=x|y;
	brightness=(x/102.4)+48;
	if(brightness>outside_threshold){
		outside=outside_dark;
	}
	else{
		outside=outside_bright;
	}
	
		
	ADCSRA |= 1<<ADSC;
	
}