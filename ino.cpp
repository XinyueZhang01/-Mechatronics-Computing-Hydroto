#include <Arduino.h>
#include <PdArduino.h>

// Pin definitions
// Piezo disc to sense the rain
const int kInput1Pin = 0;
const int kInput2Pin = 1;
const int kInput3Pin = 2;
// Button for change the pitch
const int kButton1Pin = 0;
const int kButton2Pin = 1;
const int kButton3Pin = 2;
const int kButton4Pin = 13;
const int kButton5Pin = 14;
// Motor for beating the instruments
const int kMotor1Pin1 = 3;
const int kMotor1Pin2 = 4;  // Motor for beating the triangle
const int kMotor2Pin1 = 8;
const int kMotor2Pin2 = 5;  // Motor for beating the hand drum
const int kMotor3Pin1 = 9;
const int kMotor3Pin2 = 11;  // Motor for beating the kalimba
// Servo motor for controlling the keys to hit
const int kServoMotor4Pin = 12;  // Servo motor for spinning the beater of the kalimba
const int kServoMotor5Pin = 6;  // Servo motor for spinning the hand drum

// State variables
// Time measurement
static float currentTime = 0.0;
static float previousTime = 0.0;
// Change the beat
static int delayBeat = 0;
// Change the time between pitches played within one based on the delayBeat
static int delayPlay = 0;
// Angle of the kalimba beater
static float centralAngle = 0.164; // Central position of the kalimba. Decide the range of keys to play
static float angleToAdd = 0.0;  // Decide which key to play
// Decide if the project is truned on or off
static int startProject = 0;

void setup()
{
	pinMode(kInput1Pin, INPUT);
	pinMode(kInput2Pin, INPUT);
	pinMode(kInput3Pin, INPUT);
	
	pinMode(kButton1Pin, INPUT);
	pinMode(kButton2Pin, INPUT);
	pinMode(kButton3Pin, INPUT);
	pinMode(kButton4Pin, INPUT);
	pinMode(kButton5Pin, INPUT);
	
	pinMode(kMotor1Pin1, OUTPUT);
	pinMode(kMotor1Pin2, OUTPUT);
	pinMode(kMotor2Pin1, OUTPUT);
	pinMode(kMotor2Pin2, OUTPUT);
	pinMode(kMotor3Pin1, OUTPUT);
	pinMode(kMotor3Pin2, OUTPUT);
	pinMode(kServoMotor4Pin, OUTPUT);
	pinMode(kServoMotor5Pin, OUTPUT);
}

/**
 * Spin the motor forward and backward to hit the instruments (the beater is connected to the motor)
 * @function
 * @param {int} Motor pin 1.
 * @param {int} Motor pin 2.
 * @param {int} Duration time of motor spinning (both forward and backward).
 * @param {float} Speed of the motor when spin back.
 */
void controlMotor(int motorPin1, int motorPin2, int duration, float spinBack)
{
	// Set time
	int startTime = millis();
	// Spin the motor at high speed for the a time
	while (millis() - startTime < duration) {
		digitalWrite(motorPin1, LOW);
		pwmWrite(motorPin2, 1.0); // Set motor speed to maximum
	}
	// Turn off the motor
	digitalWrite(motorPin2, LOW);
	// Reset the time
	int delayTime = millis();
	// Spin back
	while (millis() - delayTime < duration - 30) {
		digitalWrite(motorPin2, LOW);
		pwmWrite(motorPin1, spinBack);
	}
	// Turn off the motor
	digitalWrite(motorPin1, LOW);
}

/**
 * Spin the servo motor, so a certain angle to choose a certain key of the kalimba
 * Call the controlMotor function to hit the key selected
 * @function
 * @param {float} Angle to add (decide which key to play).
 * @param {float} The central angle of the beater (decide range of keys to play).
 */
void spinMotor(float angleAdded, float angleCentral)
{
	// Servo motor spins
	pwmWrite(kServoMotor4Pin, angleAdded);
	// Delay for 0.2 seconds
	delay(200);
	controlMotor(kMotor3Pin2, kMotor3Pin1, 130, 0.70);
	// Delay for 0.3 seconds
	delay(300);
	// Servo Motor spins back to its central angle
	pwmWrite(kServoMotor4Pin, angleCentral);
	// Reset time measurement, so the if loop will only be activated once
}

void loop()
{
	// Change Kalimba and hand drum pitch region
	float button1Value = digitalRead(kButton1Pin);
	float button2Value = digitalRead(kButton2Pin);
	float button3Value = digitalRead(kButton3Pin);
	// Button one is clicked
	if(button1Value == 0){
		// Set central point on Kalimba (range: F4 - E3)
		centralAngle = 0.164;
		// Spin the hand drum so one certain key will be played
		pwmWrite(kServoMotor5Pin, 0.10);
	} 
	// Button two is clicked
	else if(button2Value == 0){
		// Set central point on Kalimba (range: G5 - F4)
		centralAngle = 0.144;
		// Spin the hand drum so one certain key will be played
		pwmWrite(kServoMotor5Pin, 0.20);
	} 
	// Button three is clicked
	else if (button3Value == 0){
		// Set central point on Kalimba (range: D2 - E3)
		centralAngle = 0.124;
		// Spin the hand drum so one certain key will be played
		pwmWrite(kServoMotor5Pin, 0.40);
	}
	
	
	// Project on & off
	float button4Value = digitalRead(kButton4Pin);
	float button5Value = digitalRead(kButton5Pin);
	// Button four clicked
	if(button4Value == 0){
		// Device turned on
		startProject = 1;
	} 
	// Button five clicked
	else if(button5Value == 0){
		// Device turned off
		startProject = 0;
	}
	

	// Read the signal input
	// float pitch1Value = analogRead(kInput1Pin);
	// float pitch2Value = analogRead(kInput2Pin);
	// float pitch3Value = analogRead(kInput3Pin);
	float pitch1Value = 0.7;
	float pitch2Value = 0.9;
	float pitch3Value = 0.8;
	// Map the range of reading to 0 - 1.0
	float frequencyRead1 = map(pitch1Value, 0.30, 0.833, 0.0, 1.0);
	float frequencyRead2 = map(pitch2Value, 0.30, 0.833, 0.0, 1.0);
	float frequencyRead3 = map(pitch3Value, 0.30, 0.833, 0.0, 1.0);
	
	//Set the duration time of each beat
	// Heavy rain: high frequency
	if(frequencyRead1 < 0.2 || frequencyRead2 < 0.2 || frequencyRead3 < 0.2){
		delayBeat = 600;
		delayPlay = 0;
	} 
	// Light rain: medium frequency
	else if (0.2 <= frequencyRead1 < 0.95 || 0.2 <= frequencyRead2 < 0.95 || 0.2 <= frequencyRead3 < 0.95){
		delayBeat = 1950;
		delayPlay = 650;
	} 
	// No rain: low frequency
	else{
		delayBeat = 3300;
		delayPlay = 1300;
	}
	
	
	switch(startProject){
		// Device turned on
		case 1:
			currentTime = millis(); 
			if(currentTime - previousTime >= 200 + delayBeat && currentTime - previousTime < 3000.0 + delayBeat){
				// Compare the number of raindrops and play different instruments based on that
				// Play the triangle
				if (pitch1Value >= pitch2Value && pitch2Value >= pitch3Value){
					// Call controlMotor function to beat the triangle
					controlMotor(kMotor1Pin1, kMotor1Pin2, 100, 0.72);
				} 
				// Play the hand drum
				else if (pitch1Value >= pitch3Value && pitch3Value > pitch2Value){
					//  Call controlMotor function to beat the hand drum
					controlMotor(kMotor2Pin1, kMotor2Pin2, 130, 0.70);
				} 
				// Play the kalimba (2 keys)
				else if (pitch2Value > pitch1Value && pitch1Value > pitch3Value){
					// Kalimba picth 1
					angleToAdd = centralAngle + 0.012;
					spinMotor(angleToAdd, centralAngle);
					// delay for a while
					delay(1000 + delayPlay);
					// Kalimba picth 2
					angleToAdd = centralAngle + 0.004;
					spinMotor(angleToAdd, centralAngle);
				} 
				// Play the kalimba (2 keys)
				else if (pitch2Value >= pitch3Value && pitch3Value > pitch1Value){
					// Kalimba picth 2
					angleToAdd = centralAngle + 0.004;
					spinMotor(angleToAdd, centralAngle);
					// delay for a while
					delay(1000 + delayPlay);
					// Kalimba picth 1
					angleToAdd = centralAngle + 0.012;
					spinMotor(angleToAdd, centralAngle);
				}
				// Play the kalimba (3 keys)
				else if (pitch3Value > pitch1Value && pitch1Value > pitch2Value){
					// Kalimba pitch 2
					angleToAdd = centralAngle + 0.004;
					spinMotor(angleToAdd, centralAngle);
					// delay for a while
					delay(300 + delayPlay);
					// Kalimba pitch 3
					angleToAdd = centralAngle - 0.004;
					spinMotor(angleToAdd, centralAngle);
					// delay for a while
					delay(300 + delayPlay);
					// Kalimba pitch 4
					angleToAdd = centralAngle - 0.012;
					spinMotor(angleToAdd, centralAngle);
				}
				// Play the kalimba (3 keys)
				else if (pitch3Value > pitch2Value && pitch2Value > pitch1Value){
					// Kalimba picth 4
					angleToAdd = centralAngle - 0.012;
					spinMotor(angleToAdd, centralAngle);
					// delay for a while
					delay(300 + delayPlay);
					// Kalimba picth 3
					angleToAdd = centralAngle - 0.004;
					spinMotor(angleToAdd, centralAngle);
					// delay for a while
					delay(300 + delayPlay);
					// Kalimba picth 2
					angleToAdd = centralAngle + 0.004;
					spinMotor(angleToAdd, centralAngle);
				}
				
				// Update the time measurement
				previousTime = millis();
			}
			break;
			
		// Device turned off
		case 0:
			// Reupdate the time measurement
			previousTime = millis();
			break;
	}
}
