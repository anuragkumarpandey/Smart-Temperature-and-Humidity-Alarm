#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Pins
#define DHT_PIN 7          // DHT11 -> Digital Pin 7
#define DHT_TYPE DHT11     // DHT11 sensor
#define BUZZER 8
#define LED_GREEN 4        // green LED (normal Arduino code)
#define LED_RED 5          // red LED (ASM controlled)
#define INTERRUPT_PIN 3    // Interrupt demo: touch D3 to GND

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);

const float TEMP_NORMAL_HIGH = 25.0;
const float TEMP_ALARM = 30.0;

// Interrupt flag
volatile bool interruptTriggered = false;
bool alarmMuted = false;

// ASM Functions - Red LED control (D5 = PORTD PD5)
void redLedOn_ASM() {
  asm volatile (
    "sbi %0, %1"
    :
    : "I" (_SFR_IO_ADDR(PORTD)), "I" (PD5)
  );
}

void redLedOff_ASM() {
  asm volatile (
    "cbi %0, %1"
    :
    : "I" (_SFR_IO_ADDR(PORTD)), "I" (PD5)
  );
}

// Interrupt Service Routine (ISR)
void interruptISR() {
  interruptTriggered = true;
}

void updateDisplay(float temp, float hum);
void updateLEDs(float temp);
void checkAlarm(float temp);

void setup() {
  Serial.begin(9600);
  Serial.println("=== Smart Temperature Alarm ===");
  Serial.println("Features:");
  Serial.println("- DHT11 sensor + I2C LCD");
  Serial.println("- Red LED = AVR Assembly (sbi/cbi)");
  Serial.println("- Interrupt on D3 (touch to GND)");

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);

  // Enable external interrupt on pin 3
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interruptISR, FALLING);

  dht.begin();

  // LCD startup
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Temp Alarm");
  lcd.setCursor(0, 1);
  lcd.print("ASM + IRQ Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Check for interrupt first (priority!)
  if (interruptTriggered) {
    interruptTriggered = false;
    alarmMuted = !alarmMuted;
    Serial.print("INTERRUPT! Alarm ");
    Serial.println(alarmMuted ? "MUTED" : "ACTIVE");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INTERRUPT!");
    lcd.setCursor(0, 1);
    lcd.print(alarmMuted ? "ALARM MUTED" : "ALARM ON");
    delay(1500);
    lcd.clear();
  }

  // Read sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT11 ERROR - Check wiring");
    lcd.clear();
    lcd.print("DHT11 ERROR!");
    delay(2000);
    return;
  }

  // Serial output
  Serial.print("Temp: ");
  Serial.print(temp, 1);
  Serial.print("C Hum: ");
  Serial.println(hum, 1);

  // Update display
  updateDisplay(temp, hum);

  // Update LEDs (ASM for red LED)
  updateLEDs(temp);

  // Check alarm
  checkAlarm(temp);

  delay(2000);
}

void updateDisplay(float temp, float hum) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(hum, 1);
  lcd.print("%");
}

void updateLEDs(float temp) {
  // Green LED - normal Arduino functions
  digitalWrite(LED_GREEN, LOW);

  // Red LED - AVR Assembly
  redLedOff_ASM();

  if (temp <= TEMP_NORMAL_HIGH) {
    digitalWrite(LED_GREEN, HIGH);   // Green ON
  } else {
    redLedOn_ASM();                  // Red ON
  }
}

void checkAlarm(float temp) {
  if (temp >= TEMP_ALARM && !alarmMuted) {
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);
    delay(1000);
  } else {
    digitalWrite(BUZZER, LOW);
  }
}
