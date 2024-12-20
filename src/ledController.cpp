#include "ledController.h"

// NeoPixel configuration
#define NUMPIXELS 24
#define MAXCOLOURS 6

String colourBlindModeLED = "";

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Reference colour (default: white)
int refR = 255, refG = 255, refB = 255;

uint32_t Wheel(byte WheelPos) {
    if (WheelPos < 85) {
        return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

void error() {
	for(int i = 0; i < NUMPIXELS; i++) {
		pixels.setPixelColor(i, pixels.Color(255, 0, 0));
	}
	pixels.show();
	delay(200);
	for(int i = 0; i < NUMPIXELS; i++) {
		pixels.setPixelColor(i, pixels.Color(0, 0, 0));
	}
	pixels.show();
	delay(200);
	for(int i = 0; i < NUMPIXELS; i++) {
		pixels.setPixelColor(i, pixels.Color(255, 0, 0));
	}
	pixels.show();
	delay(200);
	for(int i = 0; i < NUMPIXELS; i++) {
		pixels.setPixelColor(i, pixels.Color(0, 0, 0));
	}
	pixels.show();
}

void changeGameMode() {
	for (int i = 0; i < NUMPIXELS; i++) {
        pixels.clear();
        
        for (int j = 0; j < 6; j++) {
            int ledIndex = (i + j) % NUMPIXELS;
            pixels.setPixelColor(ledIndex, Wheel((ledIndex * 256 / NUMPIXELS + i * 5) & 255));
        }

        pixels.show();
        delay(30);
    }
}

void turnOffLEDs() {
	for(int i = 0; i < NUMPIXELS; i++) {
		pixels.setPixelColor(i, pixels.Color(0, 0, 0));
	}
	pixels.show();
}

void updateRingColours() {
	for(int i = 0; i < NUMPIXELS; i++) {
		pixels.setPixelColor(i, pixels.Color(refR, refG, refB));
	}
	pixels.show();
}

void setEasyColour() {
	refR = random(0, 3)*(255/2);
	refG = random(0, 3)*(255/2);
	refB = random(0, 3)*(255/2);
	if (refR+refG+refB == 0) {
		refR = 255;
	} else if (refR+refG+refB > 700) {
		refG = 0;
	}

	updateRingColours();
}

void setHardColour() {
	refR = random(0, 5)*(255/4);
	refG = random(0, 5)*(255/4);
	refB = random(0, 5)*(255/4);
	if (refR+refG+refB == 0) {
		refR = 255;
	} else if (refR+refG+refB > 700) {
		refG = 0;
	}

	updateRingColours();
}

// Calculate similarity percentage between reference and input colour
int calculateSimilarity(int r, int g, int b) {

	// Euclidian first
	float diffR = pow(refR - r, 2);
	float diffG = pow(refG - g, 2);
	float diffB = pow(refB - b, 2);
	float euclidian = sqrt(diffR + diffG + diffB);
	float eSimilarity = 100.0 - ((euclidian * 100.0) / 441.67);

	// Hue similarity based on ratios
	// tr for target ratio, gr for guess ratio
	float trR = refR/static_cast<float>(refR+refG+refB);
	float trG = refG/static_cast<float>(refR+refG+refB);
	float trB = refB/static_cast<float>(refR+refG+refB);

	float grR = r/static_cast<float>(r+g+b);
	float grG = g/static_cast<float>(r+g+b);
	float grB = b/static_cast<float>(r+g+b);

	float rDiffR = abs(trR-grR);
	float rDiffG = abs(trG-grG);
	float rDiffB = abs(trB-grB);

	float hSimilarity = 100.0 - ((rDiffR+rDiffG+rDiffB)*100.0/3.00);

	float similarity = 60*eSimilarity/100.00 + 40*hSimilarity/100.00;

	return static_cast<int>(similarity);
}

int adjustAndCalculateSimilarity(int r, int g, int b) {
	if (colourBlindModeLED == "NONE") {
		return calculateSimilarity(r, g, b);
	} else if (colourBlindModeLED == "PROTANOPIA") {
		int adjustedR = refR > 100 ? (r * 2) : r+50; // Reduces the impact of red
		return calculateSimilarity(adjustedR, g, b);
	} else if (colourBlindModeLED == "DEUTERANOPIA") {
		int adjustedG = refG > 100 ? (g * 2) : g+50; // Reduces the impact of green
		return calculateSimilarity(r, adjustedG, b);
	} else if (colourBlindModeLED == "TRITANOPIA") {
		int adjustedB = refB > 100 ? (b * 2) : b+50; // Reduces the impact of blue
		return calculateSimilarity(r, g, adjustedB);
	} else {
		return calculateSimilarity(r, g, b);
	}
}

// Get colour based on similarity percentage
uint32_t getColourBasedOnAccuracy(int accuracy) {
	if (accuracy > 80) return pixels.Color(0, 100, 0);   // Deep Green
	if (accuracy > 75) return pixels.Color(0, 128, 0);   // Green
	if (accuracy > 70) return pixels.Color(50, 205, 50); // Lime Green
	if (accuracy > 60) return pixels.Color(255, 255, 0); // Yellow
	if (accuracy > 55) return pixels.Color(255, 215, 0); // Gold
	if (accuracy > 45) return pixels.Color(255, 165, 0); // Orange
	if (accuracy > 40) return pixels.Color(255, 69, 0);  // Orange Red
	return pixels.Color(255, 0, 0); // Dark Red
}

// Display colour based on accuracy
void displayBasedOnAccuracy(int accuracy) {
	turnOffLEDs();
	uint32_t colour = getColourBasedOnAccuracy(accuracy);
	for (int i = 0; i < NUMPIXELS; i++) {
		if ((i*100)/24 <= accuracy) {
			pixels.setPixelColor(i, colour);
			pixels.show();
			delay(40);
		}
	}
	
}

// Display dynamic colour-moving effect
void displayDynamicStandby() {
	for (long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 512) {
		for (int i = 0; i < NUMPIXELS; i++) {
			int pixelHue = firstPixelHue + (i * 65536L / NUMPIXELS);
			pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
		}
		pixels.show();
		delay(20); // Adjust speed
	}
}

void loading() {
	turnOffLEDs();
	for (int i = 0; i < NUMPIXELS; i++) {
		for (int j = 0; j < NUMPIXELS; j++) {
			if (i == j) {
				pixels.setPixelColor(j, pixels.Color(200, 200, 200));	
			} else {
				pixels.setPixelColor(j, pixels.Color(0, 0, 0));
			}
		}
		pixels.show();
		delay(30);
	}
}

uint32_t foundColours[MAXCOLOURS] = {
	pixels.Color(0, 0, 0),
	pixels.Color(0, 0, 0),
	pixels.Color(0, 0, 0),
	pixels.Color(0, 0, 0),
	pixels.Color(0, 0, 0),
	pixels.Color(0, 0, 0)
};

int numColoursFound = 0;
float pulseValue = 0;
bool pulseUp = true;

void resetColourFinder() {
	numColoursFound = 0;
}
bool addColour(int r, int g, int b) {
	foundColours[numColoursFound++] = pixels.Color(r, g, b);
	if (numColoursFound == MAXCOLOURS) {
		return 1;
	} else {
		return 0;
	}
}

String getFoundColours() {
	String rgbString = "";
  
	for (int i = 0; i < numColoursFound; i++) {
		uint32_t colour = foundColours[i];

		// Extract red, green, and blue components from the 32-bit color value
		uint8_t r = (colour >> 16) & 0xFF;
		uint8_t g = (colour >> 8) & 0xFF;
		uint8_t b = colour & 0xFF;

		// Append the RGB values to the string
		rgbString += "(" + String(r) + "," + String(g) + "," + String(b) + ")";

		if (i < numColoursFound-1) {
			rgbString += ", ";
		}
	}

	return rgbString;
}

void waitingGlow() {
	if (pulseUp) {
		pulseValue += 0.001;
		if (pulseValue >= 1.0) {
			pulseValue = 1.0;
			pulseUp = false;
		}
	} else {
		pulseValue -= 0.001;
		if (pulseValue <= 0.4) {
			pulseValue = 0.4;
			pulseUp = true;
		}
	}

	uint8_t brightness = (uint8_t)(pulseValue * 255);

	if (numColoursFound == 0) {
		// Pulse white
		for (int i = 0; i < NUMPIXELS; i++) {
			pixels.setPixelColor(i, pixels.Color(brightness, brightness, brightness));
		}
	} else {
		for (int i=0; i<numColoursFound; i++) { // Iterate through the number of colours found
			for (int j = round(i * (NUMPIXELS / numColoursFound)); j < round((i+1) * (NUMPIXELS / numColoursFound)); j++) {
				pixels.setPixelColor(j, foundColours[i]);
			}
		}
	}

	pixels.show();
}

void closeLidLights() {
	refR = 255;
	refG = 255;
	refB = 255;

	updateRingColours();
	delay(500);

	refR = 0;
	refG = 0;
	refB = 0;

	updateRingColours();
	delay(500);
}

void setupLED() {
	pixels.begin(); // Initialize NeoPixel
	pinMode(LED_PIN, OUTPUT);
	displayDynamicStandby();
	colourBlindModeLED = getColourBlindMode();
}

int compareColour(int r, int g, int b) {
	int accuracy = adjustAndCalculateSimilarity(r, g, b);
	displayBasedOnAccuracy(accuracy);
	return accuracy;
}