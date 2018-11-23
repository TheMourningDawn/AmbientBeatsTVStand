#ifndef LEDANIMATIONS_H
#define LEDANIMATIONS_H

#include "application.h"
#include "SpectrumEqualizerClient.h"
#include "FastLED.h"

FASTLED_USING_NAMESPACE;

#define NUM_LEDS 152

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

class LEDAnimations
{
  private:
    SpectrumEqualizerClient *equalizer;
  public:
    CRGB leds[NUM_LEDS];

    bool poweredOn = true;

    bool musicReactive = true;
    int animationCount;
    int globalSensitivity = 0;
    int currentAnimation = 0;
    int hue = 120;
    int saturation = 255;
    int brightness = 255;

    LEDAnimations();
    LEDAnimations(SpectrumEqualizerClient* eq);

    void setCurrentBrightness(int brightness);
    void setCurrentSaturation(int saturation);

    int getCurrentAnimation();
    int runCurrentAnimation();
    int nextAnimation();
    int previousAnimation();
    int setAnimation(int animationNumber);
    int nextFrequencyMode();
    int previousFrequencyMode();

    bool toggleAudioReactive();

    int clampToRange(int numberToClamp, int lowerBound, int upperBound);
    int clampSensitivity(int sensitivity);
    int wrapToRange(int numberToWrap, int lowerBound, int upperBound);

    void randomSilon();

    void clearAllLeds();
    void fillSolid();
    void rainbow();
    void rainbowSlide();
    void confetti();
    void sinelon();
    void bpm();
    void juggle();

    void waterfall();
    void waterfallBorder(int frequencyValue, int sensitivityValueMinThreshold, int brightness);
    void waterfallBorderRemote();
    void waterfallRainbowBorder();

    void equalizerBorderOnly();
    void equalizerLeft(int frequencyValue, int sensitivityThreshold, bool direction);
    void equalizerRight(int frequencyValue, int sensitivityThreshold, bool direction);

    bool getMusicReactive();
    void setMusicReactive(bool newMusicReactiveValue);
};

#endif
