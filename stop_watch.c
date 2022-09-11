/*
 * stop_watch.c
 *
 *  Created on: Sep 11, 2022
 *      Author: moham
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
typedef struct {
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
}stop_time;
stop_time time={0,0,0};

void increment(stop_time* time){
	time -> seconds++;
	if (time -> seconds == 60) {
		time -> seconds = 0;
		time -> minutes++;
	}
	if (time -> minutes == 60) {
		time -> minutes = 0;
		time -> hours++;
	}
	if (time -> hours == 100){
		time -> hours = 0;
	}

}
void display(stop_time*time){
	PORTA = (PORTA & 0xC0) | 0b00000001;
	PORTC = (PORTC & 0xF0) | (time->seconds%10); //refresh lower digit of seconds
	_delay_ms(2);
	PORTA = (PORTA & 0xC0) | 0b00000010;
	PORTC = (PORTC & 0xF0) | (time-> seconds/10); //refresh higher digit of seconds
	_delay_ms(2);
	PORTA = (PORTA & 0xC0) | 0b00000100;
	PORTC = (PORTC & 0xF0) | (time-> minutes%10); //refresh lower digit of minutes
	_delay_ms(2);
	PORTA = (PORTA & 0xC0) | 0b00001000;
	PORTC = (PORTC & 0xF0) | (time-> minutes/10); //refresh higher digit of minutes
	_delay_ms(2);
	PORTA = (PORTA & 0xC0) | 0b00010000;
	PORTC = (PORTC & 0xF0) | (time-> hours%10); //refresh lower digit of hours
	_delay_ms(2);
	PORTA = (PORTA & 0xC0) | 0b00100000;
	PORTC = (PORTC & 0xF0) | (time-> hours/10); //refresh higher digit of hours
	_delay_ms(2);
}

void initialization(void){
	DDRC|=(1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);  // Set the first 4-pins in PORTC as output pins.
	DDRD&=0b11110011;				  //Set the pins (2 , 3 ) in PORTC as input pins for external interrupts ( IN0 ,IN1 )
	DDRA|=(1<<PA0)|(1<<PA1)|(1<<PA2)|(1<<PA3)|(1<<PA4)|(1<<PA5); // Set the first 6-pins in PORTA as output pins.
	DDRB|=(1<<PB2);  //Set the pins (2 , 3 ) in PORTB as input pin for external interrupt (IN2)
	PORTD|=(1<<PD2); //Activate Internal Pull-Up Resistor
	PORTB|=(1<<PB2); //Activate Internal Pull-Up Resistor
	SREG=(1<<7); //  Enable the global interrupts;

							//The rising edge of INT1 and falling edge of INT0
	MCUCR|=(1<<ISC11)|(1<<ISC10)|(1<<ISC01);
	MCUCR&=~(1<<ISC00);
	MCUCSR&=~(1<<ISC2);   // the falling edge of INT2
	GICR|=0b11100000; //  External Interrupt Request (0 , 1 , 2) Enable

	/* frequency of timer 1 =(F_cpu/pre-scalar)=(1 MHZ /64 )=15625
	 the time of the clock=(1/15625)
	 so to achieve 1 second -> we will make the compare value =15625
	 */

	TCCR1B|=(1<<WGM12)|(1 << CS11) | (1 << CS10); // Set clock pre-scalar to 64 and timer1 for CTC mode
	TIMSK|=(1<<OCIE1A);
	OCR1A=15625;
	TCNT1 = 0;
}
ISR(TIMER1_COMPA_vect){
	increment(&time);
}
ISR(INT0_vect){  // for reset mode
	time.seconds = 0;
	time.minutes = 0;
	time.hours = 0;
	TCNT1 = 0;
}
ISR(INT1_vect){  // for pause mode
	TCCR1B&=0b11111000;					// No clock source (Timer/Counter stopped).
}
ISR(INT2_vect){ 	// for resume mode
	TCCR1B|=(1<<WGM12)|(1 << CS11) | (1 << CS10); // Set clock pre-scalar to 64
}
int main(void){
	initialization();
	while(1){
		display(&time);
	}
}

