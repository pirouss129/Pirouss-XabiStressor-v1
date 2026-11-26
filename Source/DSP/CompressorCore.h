#pragma once
#include <JuceHeader.h>

class CompressorCore
{
public:
    CompressorCore() = default;
    ~CompressorCore() = default;

    void prepare(double sampleRate)
    {
        sr = sampleRate;
        currentGrDb = 0.0f;
    }

    void reset()
    {
        currentGrDb = 0.0f;
    }

    // --- SETTERS DES PARAMÈTRES (liés à l'APVTS) ---
    void setThreshold(float threshDb) { thresholdDb = threshDb; }

    // Conversion des sliders 0-10 en millisecondes (ajustable ici !)
    void setAttackFromSlider(float attackSliderVal)
    {
        // 0-10 map vers 0.1ms à 30ms (courbe exponentielle pour plus de précision au début)
        attackMs = juce::jmap(attackSliderVal, 0.0f, 10.0f, 0.1f, 30.0f);
        updateEnvelopeCoefficients();
    }

    void setReleaseFromSlider(float releaseSliderVal)
    {
        // 0-10 map vers 50ms à 3000ms (3 secondes)
        releaseMs = juce::jmap(releaseSliderVal, 0.0f, 10.0f, 50.0f, 3000.0f);
        updateEnvelopeCoefficients();
    }

    void setRatioMode(int modeIndex)
    {
        currentRatioIndex = juce::jlimit(0, 7, modeIndex);
    }

    // --- TRAITEMENT AUDIO DU COMPRESSEUR ---
    // Prend le niveau de sidechain (linéaire) et applique la compression sur le buffer audio principal
    void process(juce::AudioBuffer<float>& buffer, float sidechainLevel)
    {
        if (buffer.getNumChannels() < 2) return;

        float ratio = ratioValues[currentRatioIndex];
        float kneeWidth = kneeWidthValues[currentRatioIndex];

        auto* leftData = buffer.getWritePointer(0);
        auto* rightData = buffer.getWritePointer(1);
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            // 1. Niveau instantané de l'échantillon (ou du sidechain local) en dB
            float sampleIn = std::max(std::abs(leftData[i]), std::abs(rightData[i]));
            float inputDb = juce::Decibels::gainToDecibels(sampleIn, -96.0f);

            // 2. Calcul de la cible de réduction de gain
            float targetGrDb = 0.0f;
            float overshoot = inputDb - thresholdDb;

            if (currentRatioIndex == 7) // Sgal Mood (Nuke)
            {
                if (inputDb > thresholdDb)
                    targetGrDb = overshoot * 0.9f;
            }
            else if (overshoot > -(kneeWidth * 0.5f) && kneeWidth > 0.0f)
            {
                if (overshoot < (kneeWidth * 0.5f))
                {
                    float x = overshoot + (kneeWidth * 0.5f);
                    float slope = (1.0f - (1.0f / ratio));
                    targetGrDb = (x * x) / (2.0f * kneeWidth) * slope;
                }
                else
                {
                    float slope = 1.0f - (1.0f / ratio);
                    targetGrDb = overshoot * slope;
                }
            }
            else if (overshoot >= (kneeWidth * 0.5f))
            {
                float slope = 1.0f - (1.0f / ratio);
                targetGrDb = overshoot * slope;
            }

            // 3. Suiveur d'enveloppe par échantillon (Attaque vs Relâchement)
            if (targetGrDb > currentGrDb)
                currentGrDb = attackCoef * (currentGrDb - targetGrDb) + targetGrDb;
            else
                currentGrDb = releaseCoef * (currentGrDb - targetGrDb) + targetGrDb;

            currentGrDb = juce::jmax(0.0f, currentGrDb);

            // 4. Application du gain
            float gainFactor = juce::Decibels::decibelsToGain(-currentGrDb);
            leftData[i] *= gainFactor;
            rightData[i] *= gainFactor;
        }
    }

    // Getter pour afficher la réduction de gain dans ton futur grMeter !
    float getCurrentGrDb() const { return currentGrDb; }

private:
    double sr = 44100.0;
    float thresholdDb = -18.0f; // Défaut demandé
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    int currentRatioIndex = 3; // Par défaut sur "là OK" (4:1)

    float attackCoef = 0.0f;
    float releaseCoef = 0.0f;
    float currentGrDb = 0.0f;

    // --- VARIABLES DE CONFIGURATION DES RATIOS & KNEES (Tweakables à souhait !) ---
    // Indices : 0: LOL 1, 1: LOL 1.5, 2: LOL 2, 3: Why Not, 4: là OK, 5: Héhé, 6: Opto, 7: Sgal Mood
    const float ratioValues[8] = { 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 10.0f, 20.0f };
    const float kneeWidthValues[8] = { 0.0f, 6.0f, 5.0f, 4.0f, 2.0f, 2.0f,  8.0f,  0.0f }; // Opto a un gros knee doux, Nuke aucun

    void updateEnvelopeCoefficients()
    {
        attackCoef = (attackMs > 0.0f) ? std::exp(-1.0f / (0.001f * attackMs * sr)) : 0.0f;
        releaseCoef = (releaseMs > 0.0f) ? std::exp(-1.0f / (0.001f * releaseMs * sr)) : 0.0f;
    }
};