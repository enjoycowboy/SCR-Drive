#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>

volatile int  Rglob=0;
volatile int  Sglob=0;
volatile int  Tglob=0;
volatile int cutoff = 0;
volatile int angulo = 0;

void setup(){
  
  Serial.begin(115200); 
  
	DDRB  = 0x00; //R INPUT PINB5 DIGITAL PIN 13 / VERDE
	DDRB  |= (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4);
	PORTB |= (1 << PORTB5);
  
	DDRC = 0x00; //S INPUT PINC0 ANALOG INPUT 0 / VERMELHO
	PORTC |= (1 << PORTC0);
  
	DDRD = 0x00; //T INPUT PIND7 DIGITAL PIN 7 / AMARELO
	PORTD |= (1 << PORTD7);

  //OUTROS PINOS COMO OUTPUT

  //interr de pino
	EICRA = 0x05;
	PCICR |= (1 << PCIE2) | (1 << PCIE1) | (1 << PCIE0);
	PCMSK2 |= (1 << PCINT23);
	PCMSK1 |= (1 << PCINT8);
	PCMSK0 |= (1 << PCINT0);
  
 /*
	3 timers ajustados  pra contar até 1024 cada um 
	em cada interr. Em cada interr, o valor é comparado a leitura do ADC
	lógica de calculo dos timers:
	em 0.02 segundos (1/50) ele precisa contar até 1024.
	portanto cada tick vai durar 0,000019531
	o que implica em f = 51200Hz.
  
 */

// interr timer 0 FASE R

	TCCR0A = 0;
	TCCR0B = 0;
	TCNT0 = 0;

	OCR0A = 38;// 51282.05128205128 Hz (16000000/((38+1)*8))
	TCCR0A |= (1 << WGM01);// CTC
	TIMSK0 |= (1 << OCIE0A);// Output Compare Match A Interrupt Enable

// interr timer 1 FASE S 

	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;

	OCR1A = 311;// 51282.05128205128 Hz (16000000/((311+1)*1))
	TCCR1B |= (1 << WGM12);// CTC
	TIMSK1 |= (1 << OCIE1A);// Output Compare Match A Interrupt Enable

// interr timer 2 FASE T

	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2 = 0;
	OCR2A = 38;// 51282.05128205128 Hz (16000000/((38+1)*8))
	TCCR2A |= (1 << WGM21);// CTC
	TIMSK2 |= (1 << OCIE2A);// Output Compare Match A Interrupt Enable
 
  sei();
}

void loop(){
  
	cutoff = analogRead(A5);
	angulo = cutoff * 0.1578125;
	
  
  /*
		esse trecho vai ser comentado fora com o gerador de sinais pq tipo ele vai
		chavear o opto a cada 1024 ticks ou menos

   */

	if (Rglob >= 1000){
		Rglob = 0;
		PORTB = (0 << PB2);
	}

	if (Sglob >= 1000){
		Sglob = 0;
		PORTB = (0 << PB3);
	}

	if (Tglob >= 1000){
		Tglob = 0;
		PORTB = (0 << PB4);
	}

	/*if (Rtrig || Strig || Ttrig)
	{
		Serial.println ("------------------------ OPTO TRIG -------------------------")
	}*/

	Serial.println("");
	Serial.print("ADC = ");
	Serial.print(cutoff);
  
	Serial.print("\t CORTE = ");
	Serial.print(angulo);
	Serial.print("\t R = ");
	Serial.print(Rglob);
	Serial.print("\t S = ");
	Serial.print(Sglob);
	Serial.print("\t T = ");
	Serial.print(Tglob);
  
  // compara o angulo atual com o angulo de corte e dispara o MOC
	
	if (Rglob >= cutoff){
		PORTB |= (1 << PB2); 
	}

	if (Sglob >= cutoff) {
		PORTB |= (1 << PB3); 
	}

	if (Tglob >= cutoff) {
		PORTB |= (1 << PB4); 
	}

 
}


// IRQs do tipo PCINT representam as interrupções do optoacoplador.
// inclusive precisa resetar o timer e disparar da primeira vez

ISR(PCINT0_vect){ //INTERR fase R.
	
	TCNT0 = 0x00;
	TCCR0B |= (1 << CS01);// Prescaler 8
	PORTB = (0 << PB2);
	Rglob = 0;
}

ISR(PCINT1_vect){ //INTERR fase S
	
	TCCR1B |= (1 << CS10);// Prescaler 1
	TCNT1 = 0x00;
	Sglob = 0;  
	PORTB = (0 << PB3);
}

ISR(PCINT2_vect){ //INTERR fase T. A parte de desativar o sinal do moc talvez precise ficar inclusa
	
	TCCR2B |= (1 << CS21);// Prescaler 8
	TCNT2 = 0x00;
	Tglob = 0;
	PORTB = (0 << PB4);
}

ISR(TIMER0_COMPA_vect){
	Rglob++;
  	
}

ISR(TIMER1_COMPA_vect){
 	Sglob++;
	
}

ISR(TIMER2_COMPA_vect){
	Tglob++;
  	
}