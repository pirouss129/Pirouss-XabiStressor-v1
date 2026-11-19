#pragma once

#include <JuceHeader.h>

class WetDryMix
{
public:
    WetDryMix() = default;
    ~WetDryMix() = default;

    void prepare(double sampleRate, int maxSamplesPerBlock)
    {
        // On prépare un buffer tampon pour stocker le signal "Dry" (sec) d'origine
        dryBuffer.setSize(2, maxSamplesPerBlock); // Gère jusqu'à 2 canaux (stéréo)
        currentMix = targetMix;
    }

    // Pilote le mix depuis le potard (0.0 = 100% Dry, 10.0 = 100% Wet)
    void setMixFromSlider(float sliderValue)
    {
        // Mappage de 0-10 vers un pourcentage de 0.0 à 1.0 (0% à 100% wet)
        targetMix = juce::jmap(sliderValue, 0.0f, 10.0f, 0.0f, 1.0f);
    }

    // Étape 1 : Sauvegarde du signal d'entrée non modifié (Dry)
    void captureDry(const juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        dryBuffer.setSize(numChannels, numSamples, false, false, true);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }
    }

    // Étape 2 : Mélange du signal traité (Wet, qui est dans le buffer) avec le Dry sauvegardé
    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* wetData = buffer.getWritePointer(ch);
            const auto* dryData = dryBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                // Lissage du mix pour éviter les clics lors de la rotation du potard
                currentMix = currentMix * 0.999f + targetMix * 0.001f;

                float dry = dryData[i];
                float wet = wetData[i];

                // Formule de mix parallèle linéaire : Dry * (1 - mix) + Wet * mix
                wetData[i] = dry * (1.0f - currentMix) + wet * currentMix;
            }
        }
    }

private:
    juce::AudioBuffer<float> dryBuffer;
    float targetMix = 1.0f; // Par défaut à 100% wet (ou 50/50 selon ton choix, 1.0 = full wet)
    float currentMix = 1.0f;
};