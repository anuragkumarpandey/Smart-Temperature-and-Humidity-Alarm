#include <Wire.h>
#include <LiquidCrystal_I2C.h>  
#include <DHT.h>

// Pins
#define DHT_PIN 2           // DHT11 → Digital Pin 2
#define DHT_TYPE DHT11      // DHT11 sensor
#define BUZZER_PIN 8        
#define LED_GREEN 4         // Your green LED [file:427]
#define LED_RED 5           // Your red LED [file:427]

// I2C LCD 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address, columns, rows
DHT dht(DHT_PIN, DHT_TYPE);

const float TEMP_NORMAL_HIGH = 25.0;
const float TEMP_ALARM = 30.0;

void setup() {
  Serial.begin(9600);
  Serial.println("I2C LCD + DHT11 + LEDs");
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  dht.begin();
  
  // Initialize I2C LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Temp Alarm");
  lcd.setCursor(0, 1);
  lcd.print("I2C DHT11 Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT11 ERROR - Check wiring");
    lcd.clear();
    lcd.print("DHT11 ERROR!");
    delay(2000);
    return;
  }
  
  Serial.print("Temp: "); Serial.print(temp, 1);
  Serial.print("C Humidity: "); Serial.println(hum, 1);
  
  updateDisplay(temp, hum);
  updateLEDs(temp);
  checkAlarm(temp);
  delay(2000);
}

void updateDisplay(float temp, float hum) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: "); lcd.print(temp, 1); lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: "); lcd.print(hum, 1); lcd.print("%");
}

void updateLEDs(float temp) {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  if (temp <= 25.0) digitalWrite(LED_GREEN, HIGH);  // Green
  else digitalWrite(LED_RED, HIGH);                 // Red
}

void checkAlarm(float temp) {
  if (temp >= 30.0) {
    digitalWrite(BUZZER_PIN, HIGH); delay(500);
    digitalWrite(BUZZER_PIN, LOW); delay(1000);
  }
}
