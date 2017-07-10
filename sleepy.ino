#include <Arduino.h>
#include <avr/power.h> // To disable/enable peripherals
#include <avr/sleep.h> // For sleep functions/macros
#include <avr/interrupt.h> // For interrupt commands
#include <avr/wdt.h> // For Watchdog Timer functions 

#include <SD.h>                      // need to include the SD library
#define SD_ChipSelectPin 4  //using digital pin 4 on arduino nano 328, can use other pins
#define SPEAKER_PIN 8
#include <TMRpcm.h>           //  also need to include this library...
#include <SPI.h>

volatile int toggle = 0;
volatile int timeout = 0;
volatile long the_time = 0;
volatile int track_number = 1;
String track_name;
char buf[]={"0001.wav"};

TMRpcm tmrpcm;

void WDT_init(void){
  /* Disable interrupts */
  noInterrupts();
  /* Reset watchdog timer */
  wdt_reset();
  /* Start timed sequence for changing WDE and prescaler bits.
  * Setting WDCE will allow updates for 4 clock cycles
  */
  WDTCSR = (1<<WDCE)|(1<<WDE);
  /* Set watchdog timeout prescaler value */
  WDTCSR = (1<<WDP3)|(1<<WDP0);
  /* Set Watchdog Timer Configuration to interrupt mode */
  WDTCSR |= (1<<WDIE);
  /* Enable global interrupts */
  interrupts();
}

ISR(WDT_vect){
  timeout = 1;
}

void interruptRoutine(){
  toggle = 1;
}

void setup(){
  /* Disable digital input buffer on ADC pins */
//  DIDR0 = (1 << ADC5D) | (1 << ADC4D) | (1 << ADC3D) | (1 << ADC2D) | (1 << ADC1D) | (1 << ADC0D);
  /* Disable digital input buffer on Analog comparator pins */
//  DIDR1 |= (1 << AIN1D) | (1 << AIN0D);
  /* Disable Analog Comparator interrupt */
//  ACSR &= ~(1 << ACIE);
  /* Disable Analog Comparator */
//  ACSR |= (1 << ACD);
  /* Disable unused peripherals to save power */
  /* Disable ADC (ADC must be disabled before shutdown) */
//  ADCSRA &= ~(1 << ADEN);
  /* Shut down the ADC */
//  power_adc_disable();
  /* Disable SPI */
  //power_spi_disable();
  /* Disable TWI */
//  power_twi_disable();
  /* Disable the USART 0 module */
//  power_usart0_disable();
  /* Disable the Timer 0 module */
//  power_timer0_disable();
  /* Disable the Timer 1 module */
//  power_timer1_disable();
  /* Disable the Timer 2 module */
  /* Here we first remove the clock source to shutdown Timer2 in case it is in
  * asynchronous mode. Then we disable it in the Power Reduction Register.
  */
//  TCCR2B &= ~ ((1 << CS22) | (1 << CS21) | (1 << CS20));
//  power_timer2_disable();
  /* Set all unused digital and analog pins to inputs with pull-up resistors
  * Note: Putting pull-up resistors on the analog pins and the two Analog
  * Comparator pins will not do much since the pins are disconnected.
  * It is a good practice to always do this to unused pins anyway.
  */
  for (int pin = 0; pin < 20; pin++){
    if (pin == LED_BUILTIN || pin == SPEAKER_PIN || pin == SD_ChipSelectPin){
      pinMode(pin,OUTPUT);
      digitalWrite(pin,LOW);
    }else{
//      pinMode(pin, INPUT_PULLUP);
    }
  }
  /* Set sleep mode */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  /* Setup the interrupt on pin 2 and attach an interrupt handler */
  attachInterrupt(0, interruptRoutine, FALLING);
  /* Setup Watchdog Timer */
  WDT_init();

  /* Music stuff */
  Serial.begin(9600);
  Serial.println("This works"); 
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not
  }
  Serial.println("This totally works yoh!"); 
  tmrpcm.play("0010.wav"); //the sound file "music" will play each time the arduino powers up, or is reset
  while(tmrpcm.isPlaying()){
      delay(100);
    }
  /* Go to sleep */
  sleep_mode();
}

void loop(){
//  Serial.println("alive");
  if (toggle){
    /* Toggle LED */
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    Serial.print("Playing sound: ");
    Serial.println(track_name);
    /* Clear interrupt flag */
    toggle = 0;
  }
  if (timeout){
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    timeout = 0;
    track_number++;
    the_time += 8;
//    Serial.println("TIMEOUT");
    String padding;
    if(track_number < 10){
      padding = "000";
    }else if(track_number < 100){
      padding = "00";
    }else if(track_number < 1000){
      padding = "0";
    }
    track_name = String(padding) + String(track_number) + ".wav";
    Serial.print("Changing soundclip to: ");
    Serial.println(track_name);
    track_name.toCharArray(buf, 9);
    tmrpcm.play(buf);
    while(tmrpcm.isPlaying()){
      delay(100);
    }
  }
  delay(1000);
  /* Go back to sleep */
  sleep_mode();
}

