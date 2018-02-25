#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>

volatile int  Rglob=0;
volatile int  Sglob=0;
volatile int  Tglob=0;
volatile bool Rcnt=true;
volatile bool Scnt=true;
volatile bool Tcnt=true;
volatile int angle_cutoff = 0;

void setup(){
	cli();
	Serial.begin(9600); 
	DDRB = 0xDF; //R INPUT PINB5 DIGITAL PIN 13 / VERDE
	DDRC = 0xFE; //S INPUT PINC0 ANALOG INPUT 0 / VERMELHO
	DDRD = 0x7F; //T INPUT PIND7 DIGITAL PIN 7 / AMARELO

	//OUTROS PINOS COMO OUTPUT
	
	/*
	3 timers ajustados pra 100hz pra contar até 1024 cada um 
	em cada interr. Em cada interr, o valor é comparado a leitura do ADC
	
	*/

	//TIMR0. fase R
	TCCR0A = 0;
	TCCR0B = 0;
	TCNT0 = 0;    // 100.16025641025641 Hz (16000000/((155+1)*1024))
	OCR0A = 155;  // CTC
	TCCR0A |= (1 << WGM01);  // Prescaler 1024
	TCCR0B |= (1 << CS02) | (1 << CS00);  // Output Compare Match A Interrupt Enable
	TIMSK0 |= (1 << OCIE0A);

	//TIMR1. fase S
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = 624;              // 100 Hz (16000000/((624+1)*256))
	TCCR1B |= (1 << WGM12);   // CTC
	TCCR1B |= (1 << CS12);    // Prescaler 256
	TIMSK1 |= (1 << OCIE1A);  // Output Compare Match A Interrupt Enable

	//TIMR2. fase T
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2 = 0;
	OCR2A = 155;    // 100.16025641025641 Hz (16000000/((155+1)*1024))  
	TCCR2A |= (1 << WGM21);// CTC  
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);// Prescaler 1024  
	TIMSK2 |= (1 << OCIE2A);// Output Compare Match A Interrupt Enable
 

}

void loop(){
	
	int cutoff = analogRead(A5);
	
	/*
	 esse trecho vai ser comentado fora com o gerador de sinais pq tipo ele vai
	chavear o opto a cada 1024 ticks ou menos
	 */

	if (Rglob >= 1024)
	{
		Rglob = 0;
	}

	if (Sglob >= 1024)
	{
		Sglob = 0;
	}

	if (Tglob >= 1024)
	{
		Tglob = 0;
	}

	Serial.println("Angulo atual, %d" &cutoff);
	Serial.print("\t R = %d", &Rglob);
	Serial.print("\t S = %d", &Sglob);
	Serial.print("\t T = %d", &Tglob);

	// compara o angulo atual com o angulo de corte e dispara o MOC
	if (Rglob == cutoff)
	{
		PORTB |= (1 << PB2); 
	}

	if (Sglob == cutoff)
	{
		PORTB |= (1 << PB3); 
	}

	if (Tglob == cutoff)
	{
		PORTB |= (1 << PB4); 
	}


}


// IRQs do tipo PCINT representam as interrupções do optoacoplador.
// inclusive precisa resetar o timer
ISR(PCINT0_vect){ //INTERR fase R.

	Serial.println("FASE R TRIGADA")
	TCNT0 = 0;
	PORTB |= (0 << PB2);
}

ISR(PCINT1_vect){ //INTERR fase S

	Serial.println("FASE S TRIGADA")
	TCNT1 = 0;
	PORTB |= (0 << PB3);
}

ISR(PCINT2_vect){ //INTERR fase T. A parte de desativar o sinal do moc talvez precise ficar inclusa

	Serial.println("FASE T TRIGADA")
	TCNT2 = 0;
	PORTB |= (0 << PB4);
}

ISR(TIMER0_COMPA_vect){

	Rcnt = ~Rcnt;
	
	if (Scnt == true)
	{
		++Rglob;
	}

}

ISR(TIMER1_COMPA_vect){

	Scnt = ~Scnt;
	
	if (Scnt == true)
	{
		++Sglob;
	}

}

ISR(TIMER2_COMPA_vect){

	Tcnt = ~Tcnt;
	if (Tcnt == true)
	{
		++Tglob;
	}


}