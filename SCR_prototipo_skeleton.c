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
volatile bool Rtrig, Strig, Ttrig = false;

void setup(){
  
  Serial.begin(9600); 
  
	DDRB  = 0x00; //R INPUT PINB5 DIGITAL PIN 13 / VERDE
	DDRB  = (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4);
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
	PCMSK0 |= (1 << PCINT5);
  
  /*
  3 timers ajustados pra 100hz pra contar até 1024 cada um 
  em cada interr. Em cada interr, o valor é comparado a leitura do ADC
  
  */

  //TIMR0. fase R
	TCCR0A = 0;
	TCCR0B = 0;
	TCNT0 = 0;    // 100.16025641025641 Hz (16000000/((155+1)*1024))
	OCR0A = 155;  
	TCCR0A |= (1 << WGM01); // CTC
	TCCR0A = 0x00;// afeta o disparo do timer, eu só quero disparar na interrupção, portanto
	TIMSK0 |= (1 << OCIE0A);// Output Compare Match A Interrupt Enable

  //TIMR1. fase S
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = 624;              // 100 Hz (16000000/((624+1)*256))
	CCR1B |= (1 << WGM12);   // CTC. Prescaler 256, setado na interr
	TIMSK1 |= (1 << OCIE1A);  // Output Compare Match A Interrupt Enable

  //TIMR2. fase T
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2 = 0;
	OCR2A = 155;    // 100.16025641025641 Hz (16000000/((155+1)*1024))  
	TCCR2A |= (1 << WGM21);// CTC  . Prescaler 1024  interr
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

	if (Rglob >= 1023){
		Rglob = 0;
		PORTB |= (0 << PB2);
	}

	if (Sglob >= 1023){
		Sglob = 0;
		PORTB |= (1 << PB3);
	}

	if (Tglob >= 1023){
		Tglob = 0;
		PORTB |= (1 << PB4);
	}

	Serial.println("");
	Serial.print("POT = ");
	Serial.print(cutoff);
  
	Serial.print("\t Angulo de corte = ");
	Serial.print(angulo);
	Serial.print("\t R = ");
	Serial.print(TCNT0);
	Serial.print("\t S = ");
	Serial.print(TCNT1);
	Serial.print("\t T = ");
	Serial.print(TCNT2);
  
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
// inclusive precisa resetar o timer
ISR(PCINT0_vect){ //INTERR fase R.
	Rtrig = true;	
	TCNT0 = 0x00;
	TCCR0B |= (1 << CS02) | (1 << CS00);  //prescaler
	PORTB |= (0 << PB2);
	Rglob = 0;
}

ISR(PCINT1_vect){ //INTERR fase S
	Rtrig = true;	
	TCCR1B |= (1 << CS12);    // Prescaler 256
	TCNT1 = 0x00;
	Sglob = 0;  
	PORTB |= (0 << PB3);
}

ISR(PCINT2_vect){ //INTERR fase T. A parte de desativar o sinal do moc talvez precise ficar inclusa
	Rtrig = true;	
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
	TCNT2 = 0x00;
	Tglob = 0;
}

ISR(TIMER0_COMPA_vect){
	Rglob++;
  	Rtrig = false;
}

ISR(TIMER1_COMPA_vect){
 	Sglob++;
	Strig = false;
}

ISR(TIMER2_COMPA_vect){
	Tglob++;
  	Ttrig = false;
}