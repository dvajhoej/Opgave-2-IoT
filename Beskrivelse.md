# Softwarebeskrivelse – Timer1, UART0 og UART1

## Formål

I denne opgave har jeg bygget videre på mit program fra opgave 1 til en ATmega328PB Xplained Mini.

Programmet skal få en LED til at blinke hvert sekund, tælle hvor mange sekunder der er gået siden opstart og kunne kommunikere med en PC gennem både UART0 og UART1.

## Programmets opbygning

Programmet starter i `main()`, hvor PB0 bliver sat som output, da det er denne pin, LED'en er tilsluttet.

Derefter kaldes tre funktioner:

- `timer1_init()` – indstiller timeren, som styrer LED'en.
- `uart0_init()` – indstiller kommunikationen på UART0.
- `uart1_init()` – indstiller kommunikationen på UART1.

Med `sei()` aktiveres interrupts, og programmet går derefter ind i en uendelig `while(1)`-løkke.

I denne opgave bruges løkken også til at kontrollere et flag fra timeren.

## Timer1 – LED og sekundtæller

Timer1 bruges til at få LED'en til at blinke og holde styr på, hvor mange sekunder der er gået.

Timeren er sat op med en prescaler på 256 og en compare-værdi på 62499. Det giver et interval på cirka 1000 ms ved en clockfrekvens på 16 MHz.

Når timeren når denne værdi, bliver der udløst et interrupt.

I interruptfunktionen bruges:

`PORTB ^= (1 << PB0);`

Denne linje skifter PB0 mellem HIGH og LOW, så LED'en skiftevis tænder og slukker.

Samtidig bliver variablen `seconds` øget med 1, og `timerFlag` bliver sat til 1.

I `main()` kontrolleres flaget. Når det er sat, bliver det nulstillet, og sekundtælleren bliver sendt ud på UART1.

Eksempel:

`Sekunder efter opstart: 1201`

`Sekunder efter opstart: 1202`

## UART0 – Kommunikation

UART0 bruges til kommunikationen mellem ATmega328PB og PC'en.

Jeg bruger Hercules til at sende og modtage tekst gennem COM-porten. Baudraten er indstillet til 9600.

Når der modtages et tegn, bliver der udløst et UART-interrupt.

Programmet læser tegnet og undersøger, om det er et stort eller lille bogstav. Store bogstaver bliver lavet om til små, og små bogstaver bliver lavet om til store.

Det gøres ved at lægge 32 til eller trække 32 fra tegnets ASCII-værdi.

Tal og specialtegn bliver ikke ændret.

I modsætning til opgave 1 bliver det konverterede tegn nu sendt ud på både UART0 og UART1.

## UART1 – Kommunikation

UART1 er den ekstra serielle forbindelse, som er blevet tilføjet i denne opgave.

Den er forbundet til PC'en gennem en CP2102 USB-til-seriel adapter og er ligesom UART0 indstillet til 9600 baud.

UART1 bruges til at sende de konverterede tegn fra UART0 og til at vise, hvor mange sekunder programmet har kørt.

Funktionen `uart1_send()` sender et enkelt tegn, mens `uart1_send_text()` bruges til at sende en hel tekst.

## Interrupts og flag

Jeg bruger stadig to interrupts i programmet.

Det ene bliver udløst af Timer1 hvert sekund, og det andet bliver udløst, når UART0 modtager et tegn.

Det nye er, at timer-interruptet også sætter et flag.

Flaget fortæller `main()`, at der skal sendes en besked på UART1. På den måde bliver selve teksten sendt fra hovedprogrammet i stedet for direkte fra timer-interruptet.

Variablerne `timerFlag` og `seconds` er erklæret med `volatile`, fordi de bruges både i interruptfunktionen og i hovedprogrammet.

## Test

Jeg har testet programmet på mit ATmega328PB Xplained Mini-board med Hercules og en CP2102-adapter.

LED'en skifter mellem tændt og slukket hvert sekund, og sekundtælleren bliver vist på UART1.

Jeg har også testet kommunikationen ved at sende tekst gennem UART0 og kontrollere, at de konverterede tegn bliver sendt tilbage på både UART0 og UART1.

Eksempel:

**Sendt på UART0:** `Hello123!`

**Modtaget på UART0 og UART1:** `hELLO123!`

Testen viser, at bogstaverne bliver ændret korrekt, mens tal og specialtegn forbliver uændrede. Samtidig fortsætter LED'en med at blinke, og sekundtælleren bliver opdateret.

## Konklusion

Jeg har fået programmet til at fungere med begge UART-porte, Timer1 og en sekundtæller.

Opgaven har givet mig en bedre forståelse af, hvordan interrupts, flag og hovedprogrammet kan arbejde sammen, og hvordan man kan kommunikere gennem flere UART-porte på samme mikrocontroller.
