#pragma once
#include <JuceHeader.h>

class HarmonicSaturation
{
public:
    HarmonicSaturation() = default;
    ~HarmonicSaturation() = default;

    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;
        currentDistOdd = targetDistOdd;
        currentDistEven = targetDistEven;
    }

    // Pilote la distorsion impaire (0.0 à 10.0)
    void setDistOddFromSlider(float sliderValue)
    {
        targetDistOdd = juce::jmap(sliderValue, 0.0f, 10.0f, 0.0f, 4.0f);
    }

    // Pilote la distorsion paire (0.0 à 10.0)
    void setDistEvenFromSlider(float sliderValue)
    {
        targetDistEven = juce::jmap(sliderValue, 0.0f, 10.0f, 0.0f, 4.0f);
    }

    // Setter pour activer ou désactiver la saturation harmonique
    void setSaturationEnabled(bool enabled)
    {
        saturationEnabled = enabled;
    }

    // Traitement du buffer audio
    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                // Lissage pour éviter les zips
                currentDistOdd = currentDistOdd * 0.999f + targetDistOdd * 0.001f;
                currentDistEven = currentDistEven * 0.999f + targetDistEven * 0.001f;

                float x = channelData[i];

                // Application de la saturation harmonique uniquement si elle est activée
                if (saturationEnabled)
                {
                    x = applyHarmonicSaturation(x, currentDistOdd, currentDistEven);
                }

                channelData[i] = x;
            }
        }
    }

private:
    double sr = 44100.0;

    float targetDistOdd = 0.0f;
    float currentDistOdd = 0.0f;

    float targetDistEven = 0.0f;
    float currentDistEven = 0.0f;

    bool saturationEnabled = false; // Désactivée par défaut

    float applyHarmonicSaturation(float x, float oddDrive, float evenDrive)
    {
        // 1. Harmoniques paires (Even) via un terme quadratique signé (x * |x|)
        float evenComponent = x * std::abs(x);
        float mixed = x + evenDrive * evenComponent;

        // 2. Harmoniques impaires (Odd) et saturation globale via tanh
        float driven = mixed * (1.0f + oddDrive);

        return std::tanh(driven);
    }
};