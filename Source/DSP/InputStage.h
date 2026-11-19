#pragma once
#include <JuceHeader.h>

class InputStage
{
public:
    InputStage() = default;
    ~InputStage() = default;

    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;
        currentGainLinear = targetGainLinear;
    }

    // Pilote le gain d'entrée depuis le potard (valeur de 0.0 à 10.0)
    void setInputGainFromSlider(float sliderValue)
    {
        float db = juce::jmap(sliderValue, 0.0f, 10.0f, -24.0f, 24.0f);
        targetGainLinear = juce::Decibels::decibelsToGain(db);
    }

    // Setter pour activer ou désactiver la distorsion/saturation dans l'étage d'entrée
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
                // Lissage ultra-rapide du gain pour éviter les artefacts de type "zipper noise" lors de la rotation du potard
                currentGainLinear = currentGainLinear * 0.999f + targetGainLinear * 0.001f;

                float x = channelData[i] * currentGainLinear;

                // --- SECTION DISTORSION / SATURATION CONDITIONNELLE ---
                if (saturationEnabled)
                {
                    x = applyNonLinearSaturation(x);
                }

                channelData[i] = x;
            }
        }
    }

private:
    double sr = 44100.0;
    float targetGainLinear = 1.0f;
    float currentGainLinear = 1.0f;
    bool saturationEnabled = true; // Activée par défaut

    // Algorithme de saturation asymétrique (génère des harmoniques paires et impaires)
    float applyNonLinearSaturation(float x)
    {
        // Petit biais d'asymétrie pour simuler le comportement analogique (transfo / lampe)
        float bias = 0.02f;
        float driven = x + bias;

        // Saturation douce par fonction hyperbolique (tanh) compensée du biais
        float saturated = std::tanh(driven) - std::tanh(bias);

        return saturated;
    }
};