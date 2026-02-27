#include <Arduino.h>

extern "C" {
  #include "user_interface.h"
}

#define STEP_PIN 5    // GPIO5 (D1)
#define DIR_PIN  4    // GPIO4 (D2)

// 60 ms esatti con prescaler 16
#define TIMER_TICKS 300000

// Accesso diretto registri (più preciso di digitalWrite)
#define GPIO_SET   GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, (1 << STEP_PIN))
#define GPIO_CLEAR GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, (1 << STEP_PIN))

void ICACHE_RAM_ATTR onTimer()
{
  // Impulso HIGH ~3 µs garantiti
  GPIO_SET;

  // 3 µs @ 80 MHz ≈ 240 cicli
  for (volatile uint16_t i = 0; i < 240; i++) {
    __asm__ __volatile__("nop");
  }

  GPIO_CLEAR;

  // Ricarica one-shot
  timer1_write(TIMER_TICKS);
}

void setup()
{
  // Disabilita WiFi completamente (riduce jitter)
  wifi_set_opmode(NULL_MODE);
  wifi_station_disconnect();
  wifi_set_sleep_type(NONE_SLEEP_T);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH);   // direzione fissa

  // Configurazione Timer1
  timer1_isr_init();
  timer1_attachInterrupt(onTimer);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);

  timer1_write(TIMER_TICKS);
}

void loop()
{
  // Mantiene il sistema stabile lato WDT
  delay(1);
}

