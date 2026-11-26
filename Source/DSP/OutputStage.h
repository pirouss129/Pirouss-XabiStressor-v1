#pragma once

#include <JuceHeader.h>

class OutputStage
{
public:
    OutputStage() = default;
    ~OutputStage() = default;

    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;
        currentGainLinear = targetGainLinear;
    }

    // Pilote le gain de sortie depuis le potard (valeur de 0.0 à 10.0, avec un gain unity au milieu)
    void setOutputGainFromSlider(float sliderValue)
    {
        // Même logique de mappage que l'Input : 
        // 0 = -24 dB, 5.0 = 0 dB (unity), 10.0 = +24 dB
        float db = juce::jmap(sliderValue, 0.0f, 10.0f, -24.0f, 24.0f);
        targetGainLinear = juce::Decibels::decibelsToGain(db);
    }

    // Traitement du buffer audio de sortie
    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                // Lissage ultra-rapide pour éviter les clics lors de la manipulation du potard
                currentGainLinear = currentGainLinear * 0.999f + targetGainLinear * 0.001f;

                float x = channelData[i] * currentGainLinear;

                // --- CLIPPER DE SORTIE DE SÉCURITÉ ---
                // Évite de saturer brutalement le bus master du DAW au-delà de 0 dBFS
                x = applyOutputClipper(x);

                channelData[i] = x;
            }
        }
    }

private:
    double sr = 44100.0;
    float targetGainLinear = 1.0f;
    float currentGainLinear = 1.0f;

    // Clipper / Limiteur de peaks suprême
    float applyOutputClipper(float x)
    {
        // Option 1 : Soft-clipper transparent via tanh pour arrondir les angles si on dépasse 0 dB (1.0f)
        // Un soft-clip subtil sur les sommets préserve un peu de chaleur sans craquer
        /*
        if (x > 1.0f)
            return 1.0f - std::exp(-(x - 1.0f));
        else if (x < -1.0f)
            return -1.0f + std::exp((x + 1.0f));

        return x;
        */

		// Option 2 : Hard-clipper strict (moins musical mais plus sûr)
		return juce::jlimit(-0.9f, 0.9f, x);
    }
};