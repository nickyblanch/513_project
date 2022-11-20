// ECE 513
// Author: Nicolas Blanchard

int led = D7;  // The on-board LED

void setup() {
  pinMode(led, OUTPUT);
}

void loop() {
  void loop() {
  // Get some data
  String data = "test string";
  // Trigger the integration
  Particle.publish("Temp", data, PRIVATE);
  // Wait 60 seconds
  delay(60000);
}             // Wait for 30 seconds
}