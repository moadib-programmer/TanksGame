const int sensorPin = 34; // Analog pin
int sensorValue = 0;

// Thresholds (you can adjust these by experimenting)
const int HIT_THRESHOLD = 800;          // Minimum value to detect touch
const int MEDIUM_IMPACT_THRESHOLD = 1500; // Above this = medium impact
const int HARD_IMPACT_THRESHOLD = 2500;   // Above this = hard impact

void setup() 
{
  Serial.begin(115200);
}

void loop() {
  sensorValue = analogRead(sensorPin);

  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  if (sensorValue >= HIT_THRESHOLD) {
    if (sensorValue >= HARD_IMPACT_THRESHOLD) {
      Serial.println("💥 HARD Impact Detected");
    }
    else if (sensorValue >= MEDIUM_IMPACT_THRESHOLD) {
      Serial.println("⚠️ Medium Impact Detected");
    }
    else {
      Serial.println("📌 Light Touch Detected");
    }
  } else {
    Serial.println("No touch");
  }

  delay(200); // Sampling delay
}
