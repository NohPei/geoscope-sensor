int analog_in;

void setup() {
  pinMode(A0, INPUT);
  Serial.begin(9600);
  // put your setup code here, to run once:

}

void loop() {
  analog_in = analogRead(A0);
  Serial.println(analog_in);
  // put your main code here, to run repeatedly:

}
