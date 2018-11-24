#ifndef LEDANIMATIONS_CPP
#define LEDANIMATIONS_CPP

#include "application.h"
#include "LEDAnimations.h"

CRGB leds[NUM_LEDS];

typedef void (LEDAnimations::*AnimationList)();

AnimationList animationsAudioReactive[] = {&LEDAnimations::waterfall, &LEDAnimations::equalizerBorderOnly};

AnimationList animationsRails[] = {&LEDAnimations::sinelon, &LEDAnimations::confetti, &LEDAnimations::juggle,
                                   &LEDAnimations::fillColor, &LEDAnimations::rainbow,
                                   &LEDAnimations::rainbowSlide,
                                   &LEDAnimations::waterfallRainbowBorder};

LEDAnimations::LEDAnimations(SpectrumEqualizerClient *eq) : AmbientBeatsLEDAnimations(eq) {
    animationCount = ARRAY_SIZE(animationsAudioReactive);
}

int LEDAnimations::runAnimation() {
    equalizer->readAudioFrequencies();
    if (audioReactiveOn) {
        (this->*animationsAudioReactive[animation])();
    } else {
        (this->*animationsRails[animation])();
    }
}

int LEDAnimations::toggleAudioReactive() {
    audioReactiveOn = !audioReactiveOn;
    animation = 0;

    if (audioReactiveOn) {
        animationCount = ARRAY_SIZE(animationsAudioReactive) - 1;
    } else {
        animationCount = ARRAY_SIZE(animationsRails) - 1;
    }

    return audioReactiveOn;
}

void LEDAnimations::clearAllLeds() {
    for (uint8_t j = 0; j < NUM_LEDS; j++) {
        leds[j] = 0;
    }
}

void LEDAnimations::fillColor() {
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
    uint16_t frequencyThreshold = clampSensitivity(sensitivity);

    fadeToBlackBy(leds, NUM_LEDS, 10);

    if (!audioReactiveOn || frequencyValue > frequencyThreshold) {
        leds[position] += CHSV(hue + random8(64), saturation, brightness);
    }
}

// a colored dot sweeping back and forth, with fading trails
void LEDAnimations::sinelon() {
    int frequencyValue = equalizer->frequenciesLeftChannel[frequencyMode[0]];
    uint16_t frequencyThreshold = clampSensitivity(sensitivity);

    fadeToBlackBy(leds, NUM_LEDS, 1);

    if(!audioReactiveOn || (frequencyValue > frequencyThreshold)) {
        int pos = beatsin16(13, 0, NUM_LEDS);
        leds[pos] += CHSV(hue, saturation, brightness);
    }
}

// eight colored dots, weaving in and out of sync with each other
void LEDAnimations::juggle() {
    int frequencyValue = equalizer->frequenciesLeftChannel[frequencyMode[0]];
    uint16_t frequencyThreshold = clampSensitivity(sensitivity);

    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    if (!audioReactiveOn || frequencyValue > frequencyThreshold) {
        for (int i = 0; i < 8; i++) {
            int currentLocation = beatsin16(i + 7, 0, NUM_LEDS);
            leds[currentLocation] |= CHSV(dothue, saturation, brightness);
            dothue += 32;
        }
    }
}

void LEDAnimations::waterfall() {
    int sensitivityValueMinThreshold = clampSensitivity(sensitivity);
    waterfallBorder(equalizer->frequenciesLeftChannel[frequencyMode[4]], sensitivityValueMinThreshold, brightness);
}

void LEDAnimations::waterfallBorder(int frequencyValue, int frequencyValueMinThreshold, int brightness) {
    if (!audioReactiveOn || frequencyValue > frequencyValueMinThreshold) {
        int mappedFrequencyValue = map(frequencyValue, frequencyValueMinThreshold, 4096, 0, 255);
        mappedFrequencyValue = (mappedFrequencyValue) % 255; //offsetting the base color...
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
    equalizerLeft(equalizer->frequenciesLeftChannel[frequencyMode[1]], clampSensitivity(sensitivity), true);
    equalizerRight(equalizer->frequenciesLeftChannel[frequencyMode[6]], clampSensitivity(sensitivity), true);
}

void LEDAnimations::equalizerRight(int frequencyValue, int sensitivityThreshold, bool direction) {
    if (frequencyValue > sensitivityThreshold) {
        int numberToLight = map(frequencyValue, sensitivityThreshold, 3500, 0, NUM_LEDS / 2 - 1);
        CRGB color = CHSV(map(frequencyValue, sensitivityThreshold, 4096, 0, 255), saturation, brightness);
        if (direction) {
            for (int i = NUM_LEDS / 2 + 1; i < NUM_LEDS / 2 + 1 + numberToLight; i++) {
                leds[i] = color;
            }
        }
    }
}

void LEDAnimations::equalizerLeft(int frequencyValue, int sensitivityThreshold, bool direction) {
    if (frequencyValue > sensitivityThreshold) {
        int numberToLight = map(frequencyValue, sensitivityThreshold, 3500, 0, NUM_LEDS / 2 - 1);
        CRGB color = CHSV(map(frequencyValue, sensitivityThreshold, 4096, 0, 255), saturation, brightness);
        if (direction) {
            for (int i = NUM_LEDS / 2; i > NUM_LEDS / 2 - numberToLight; i--) {
                leds[i] = color;
            }
        }
    }
}

#endif
