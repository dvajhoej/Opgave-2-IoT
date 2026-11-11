
/**
 * @file main.c
 * @author dvajhoej
 * @date 2026-11-11
 * @brief Timer1 interrupt og kommunikation med UART0 og UART1
 */

#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/atomic.h>

// Variabler der bruges af timer interrupt og main
volatile uint8_t timerFlag = 0;
volatile uint32_t seconds = 0;

// Opsætning af Timer1
void timer1_init(void) {
  // Nulstiller timerens indstillinger og tæller
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // Timeren skal give interrupt hvert sekund
  OCR1A = 62499;

  // CTC mode, så timeren starter forfra
  TCCR1B |= (1 << WGM12);

  // Sætter prescaler til 256
  TCCR1B |= (1 << CS12);

  // Aktiverer interrupt for Timer1
  TIMSK1 |= (1 << OCIE1A);
}

// Opsætning af UART0
void uart0_init(void) {
  // Indstiller baudrate til 9600 ved 16 MHz
  UCSR0A = 0;
  UBRR0H = 0;
  UBRR0L = 103;

  // Aktiverer modtagelse, afsendelse og modtage-interrupt
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

  // 8 databits, ingen paritet og 1 stopbit
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Opsætning af UART1
void uart1_init(void) {
  // Indstiller baudrate til 9600 ved 16 MHz
  UCSR1A = 0;
  UBRR1H = 0;
  UBRR1L = 103;

  // Aktiverer afsendelse på UART1
  UCSR1B = (1 << TXEN1);

  // 8 databits, ingen paritet og 1 stopbit
  UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

// Sender et tegn på UART0
void uart0_send(char c) {
  // Venter til UART0 er klar til at sende
  while (!(UCSR0A & (1 << UDRE0))) {
  }

  UDR0 = c;
}

// Sender et tegn på UART1
void uart1_send(char c) {
  // Venter til UART1 er klar til at sende
  while (!(UCSR1A & (1 << UDRE1))) {
  }

  UDR1 = c;
}

// Sender en tekst på UART1
void uart1_send_text(const char *text) {
  // Sender et tegn ad gangen indtil teksten slutter
  while (*text != '\0') {
    uart1_send(*text);
    text++;
  }
}

// Bliver kørt hver gang Timer1 giver interrupt
ISR(TIMER1_COMPA_vect) {
  // Skifter LED mellem tændt og slukket
  PORTB ^= (1 << PB0);

  // Tæller et sekund op
  seconds++;

  // Fortæller main at der skal sendes en besked
  timerFlag = 1;
}

// Bliver kørt når UART0 modtager et tegn
ISR(USART0_RX_vect) {
  // Gemmer det modtagne tegn
  char c = UDR0;

  // Hvis det er et stort bogstav, laves det til et lille
  if (c >= 'A' && c <= 'Z') {
    c += ('a' - 'A');
  }
  // Hvis det er et lille bogstav, laves det til et stort
  else if (c >= 'a' && c <= 'z') {
    c -= ('a' - 'A');
  }

  // Sender det konverterede tegn på begge UART-porte
  uart0_send(c);
  uart1_send(c);
}

int main(void) {
  // Sætter PB0 som output til LED'en
  DDRB |= (1 << PB0);

  // Starter opsætningen af timer og UART
  timer1_init();
  uart0_init();
  uart1_init();

  // Aktiverer interrupts
  sei();

  // Buffer til teksten med sekundtælleren
  char buffer[50];

  // Programmet kører hele tiden
  while (1) {
    uint8_t sendSeconds = 0;
    uint32_t currentSeconds = 0;

    // Læser flag og tæller sikkert uden interrupts
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      if (timerFlag) {
        timerFlag = 0;
        currentSeconds = seconds;
        sendSeconds = 1;
      }
    }

    if (sendSeconds) {
      // Laver sekundtælleren om til en tekst
      snprintf(buffer, sizeof(buffer), "Sekunder efter opstart: %lu\r\n",
               (unsigned long)currentSeconds);

      // Sender teksten på UART1
      uart1_send_text(buffer);
    }
  }

  return 0;
}
