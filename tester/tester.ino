void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Value of switches (4,5,6,7) is ");
  Serial.print(digitalRead(9));
  Serial.print(digitalRead(10));
  Serial.print(digitalRead(11));
  Serial.print(digitalRead(12));
  Serial.println();
    digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);              // wait for a second
    digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);              // wait for a second
    digitalWrite(5, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);              // wait for a second
      digitalWrite(4, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);              // wait for a second

    
  delay(500);              // wait for a second
  digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(5, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(4, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second

}
