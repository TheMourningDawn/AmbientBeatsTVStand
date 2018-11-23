#ifndef LEDANIMATIONS_CPP
#define LEDANIMATIONS_CPP

#include "application.h"
#include "LEDAnimations.h"

CRGB leds[NUM_LEDS];

SpectrumEqualizerClient *equalizer;

uint16_t globalSensitivity = 500;
uint8_t frequencyMode[7] = {0, 1, 2, 3, 4, 5, 6};

typedef void (LEDAnimations::*AnimationList)();

bool musicReactive = true;
int animationCount = 0;

AnimationList animationsMusicReactive[] = {&LEDAnimations::waterfall, &LEDAnimations::equalizerBorderOnly};

// WARNING: I tried to name this animations, and the particle compiler kept timing out. wtf, eh?
AnimationList animationsRails[] = {&LEDAnimations::sinelon, &LEDAnimations::confetti, &LEDAnimations::juggle,
                                   &LEDAnimations::fillSolid, &LEDAnimations::rainbow, &LEDAnimations::rainbowSlide,
                                   &LEDAnimations::waterfallRainbowBorder};


LEDAnimations::LEDAnimations() : equalizer(new SpectrumEqualizerClient()) {
    animationCount = ARRAY_SIZE(animationsMusicReactive);
}

LEDAnimations::LEDAnimations(SpectrumEqualizerClient *eq) : equalizer(eq) {
    animationCount = ARRAY_SIZE(animationsMusicReactive);
}

int LEDAnimations::runCurrentAnimation() {
    equalizer->readAudioFrequencies();
    if(musicReactive) {
        (this->*animationsMusicReactive[currentAnimation])();
    } else {
        (this->*animationsRails[currentAnimation])();
    }
}

int LEDAnimations::nextAnimation() {
    currentAnimation++;

    currentAnimation = wrapToRange(currentAnimation, 0, animationCount - 1);

    clearAllLeds();
    return currentAnimation;
}

int LEDAnimations::previousAnimation() {
    currentAnimation--;
    currentAnimation = wrapToRange(currentAnimation, 0, animationCount - 1);
    clearAllLeds();
    return currentAnimation;
}

int LEDAnimations::setAnimation(int animationNumber) {
    currentAnimation = wrapToRange(animationNumber, 0, animationCount - 1);
    clearAllLeds();
    return currentAnimation;
}

int LEDAnimations::getCurrentAnimation() {
    return currentAnimation;
}

void LEDAnimations::setCurrentBrightness(int newBrightness) {
    brightness = newBrightness;
}

void LEDAnimations::setCurrentSaturation(int newSaturation) {
    saturation = newSaturation;
}

int LEDAnimations::nextFrequencyMode() {
    int wrapEnd = frequencyMode[6];
    for (int i = 6; i > 0; i--) {
        frequencyMode[i] = frequencyMode[i - 1];
    }
    frequencyMode[0] = wrapEnd;

    return wrapEnd;
}

int LEDAnimations::previousFrequencyMode() {
    int wrapBegining = frequencyMode[0];
    for (int i = 0; i < 6; i++) {
        frequencyMode[i] = frequencyMode[i + 1];
    }
    frequencyMode[6] = wrapBegining;

    return wrapBegining;
}

bool LEDAnimations::toggleAudioReactive() {
    musicReactive = !musicReactive;
    currentAnimation = 0;

    if (musicReactive) {
        animationCount = ARRAY_SIZE(animationsMusicReactive);
    } else {
        animationCount = ARRAY_SIZE(animationsRails);
    }

    return musicReactive;
}

int LEDAnimations::clampToRange(int numberToClamp, int lowerBound, int upperBound) {
    if (numberToClamp > upperBound) {
        return upperBound;
    } else if (numberToClamp < lowerBound) {
        return lowerBound;
    }
    return numberToClamp;
}

int LEDAnimations::clampSensitivity(int sensitivity) {
    return clampToRange(sensitivity, 0, 4096);
}

int LEDAnimations::wrapToRange(int numberToWrap, int lowerBound, int upperBound) {
    if (numberToWrap > upperBound) {
        return lowerBound;
    } else if (numberToWrap < lowerBound) {
        return upperBound;
    }
    return numberToWrap;
}

void LEDAnimations::clearAllLeds() {
  for(uint8_t j=0;j<NUM_LEDS;j++) {
      leds[j] = 0;
  }
}

void LEDAnimations::fillSolid() {
  fill_solid(leds, NUM_LEDS, hue);
}

void LEDAnimations::rainbow() {
    fill_rainbow(leds, NUM_LEDS, hue);
}

void LEDAnimations::rainbowSlide() {
   fill_rainbow(leds, NUM_LEDS, hue);
   hue++;
}

// random colored speckles that blink in and fade smoothly
void LEDAnimations::confetti() {
    uint8_t position = random16(NUM_LEDS);
    int frequencyValue = equalizer->frequenciesLeftChannel[frequencyMode[0]];
    uint16_t frequencyThreshold = clampSensitivity(globalSensitivity + 600);

    fadeToBlackBy(leds, NUM_LEDS, 10);

    if(!musicReactive || frequencyValue > frequencyThreshold) {
      leds[position] += CHSV(hue + random8(64), saturation, brightness);
    }
}

// a colored dot sweeping back and forth, with fading trails
void LEDAnimations::sinelon() {
    int frequencyValue = equalizer->frequenciesLeftChannel[frequencyMode[0]];
    uint16_t frequencyThreshold = clampSensitivity(globalSensitivity + 600);

    fadeToBlackBy(leds, NUM_LEDS, 1);

//    if(!musicReactive || (frequencyValue > frequencyThreshold)) {
      int pos = beatsin16(13, 0, NUM_LEDS);
      leds[pos] += CHSV(hue, saturation, brightness);
//    }
}

// eight colored dots, weaving in and out of sync with each other
void LEDAnimations::juggle() {
    int frequencyValue = equalizer->frequenciesLeftChannel[frequencyMode[0]];
    uint16_t frequencyThreshold = clampSensitivity(globalSensitivity + 600);

    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    if(!musicReactive || frequencyValue > frequencyThreshold) {
        for (int i = 0; i < 8; i++) {
            int currentLocation = beatsin16(i + 7, 0, NUM_LEDS);
            leds[currentLocation] |= CHSV(dothue, saturation, brightness);
            dothue += 32;
        }
    }
}

void LEDAnimations::waterfall() {
    int sensitivityValueMinThreshold = clampSensitivity(globalSensitivity + 700);
    waterfallBorder(equalizer->frequenciesLeftChannel[frequencyMode[4]], sensitivityValueMinThreshold, brightness);
}

void LEDAnimations::waterfallBorder(int frequencyValue, int frequencyValueMinThreshold, int brightness) {
    if(!musicReactive || frequencyValue > frequencyValueMinThreshold) {
        int mappedFrequencyValue = map(frequencyValue, frequencyValueMinThreshold, 4096, 0, 255);
        mappedFrequencyValue = (mappedFrequencyValue + 120) % 255; //offsetting the base color...
        leds[NUM_LEDS / 2] = CHSV(mappedFrequencyValue, saturation, brightness);
    } else {
        leds[NUM_LEDS / 2] = CRGB(0, 0, 0);
    }
    // When doing a memmove, don't move outside of the bounds of the array!!!
    memmove(&leds[0], &leds[1], (NUM_LEDS / 2) * sizeof(CRGB));
    memmove(&leds[NUM_LEDS / 2 + 1], &leds[NUM_LEDS / 2], (NUM_LEDS / 2) * sizeof(CRGB));
}

void LEDAnimations::waterfallBorderRemote() {
    leds[NUM_LEDS / 2] = CHSV(hue, saturation, brightness);
    memmove(&leds[0], &leds[1], (NUM_LEDS / 2) * sizeof(CRGB));
    memmove(&leds[NUM_LEDS / 2 + 1], &leds[NUM_LEDS / 2], (NUM_LEDS / 2) * sizeof(CRGB));
}

uint8_t hueCounter = 0;
void LEDAnimations::waterfallRainbowBorder() {
    leds[NUM_LEDS / 2] = CHSV(hueCounter, saturation, brightness);
    memmove(&leds[0], &leds[1], (NUM_LEDS / 2) * sizeof(CRGB));
    memmove(&leds[NUM_LEDS / 2 + 1], &leds[NUM_LEDS / 2], (NUM_LEDS / 2) * sizeof(CRGB));
    hueCounter++;
}

void LEDAnimations::equalizerBorderOnly() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  equalizerLeft(equalizer->frequenciesLeftChannel[frequencyMode[1]], clampSensitivity(globalSensitivity + 400), true);
  equalizerRight(equalizer->frequenciesLeftChannel[frequencyMode[6]], clampSensitivity(globalSensitivity + 400), true);
}

void LEDAnimations::equalizerRight(int frequencyValue, int sensitivityThreshold, bool direction) {
    if (frequencyValue > sensitivityThreshold) {
        int numberToLight = map(frequencyValue, sensitivityThreshold, 3500, 0, NUM_LEDS/2-1);
        CRGB color = CHSV(map(frequencyValue, sensitivityThreshold, 4096, 0, 255), saturation, brightness);
        if (direction) {
            for(int i = NUM_LEDS/2+1;i<NUM_LEDS/2+1+numberToLight;i++) {
                leds[i] = color;
            }
        }
    }
}

void LEDAnimations::equalizerLeft(int frequencyValue, int sensitivityThreshold, bool direction) {
    if (frequencyValue > sensitivityThreshold) {
        int numberToLight = map(frequencyValue, sensitivityThreshold, 3500, 0, NUM_LEDS/2-1);
        CRGB color = CHSV(map(frequencyValue, sensitivityThreshold, 4096, 0, 255), saturation, brightness);
        if (direction) {
            for(int i = NUM_LEDS/2;i>NUM_LEDS/2-numberToLight;i--) {
                leds[i] = color;
            }
        }
    }
}

#endif
