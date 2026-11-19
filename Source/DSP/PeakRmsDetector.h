#pragma once

#include <JuceHeader.h>

class PeakRmsDetector
{
public:
    PeakRmsDetector() = default;
    ~PeakRmsDetector() = default;

    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;
        updatePeakCoefficients();
        updateRmsCoefficients();
        reset();
    }

    void reset()
    {
        leftPeakVal = 0.0f;
        rightPeakVal = 0.0f;
        leftRmsVal = 0.0f;
        rightRmsVal = 0.0f;
    }

    // Setters pour les paramètres temporels (en millisecondes)
    void setPeakAttack(float attackMs)
    {
        attackTimeMs = attackMs;
        updatePeakCoefficients();
    }

    void setPeakRelease(float releaseMs)
    {
        releaseTimeMs = releaseMs;
        updatePeakCoefficients();
    }

    void setRmsWindow(float windowMs)
    {
        rmsWindowMs = windowMs;
        updateRmsCoefficients();
    }

    // Traitement par bloc stéréo
    void process(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2) return;

        const auto* left = buffer.getReadPointer(0);
        const auto* right = buffer.getReadPointer(1);
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float lIn = std::abs(left[i]);
            float rIn = std::abs(right[i]);

            // Suiveur de Peak (attaque / relâchement dissymétrique)
            leftPeakVal = (lIn > leftPeakVal)
                ? (peakAttackCoef * (leftPeakVal - lIn) + lIn)
                : (peakReleaseCoef * (leftPeakVal - lIn) + lIn);

            rightPeakVal = (rIn > rightPeakVal)
                ? (peakAttackCoef * (rightPeakVal - rIn) + rIn)
                : (peakReleaseCoef * (rightPeakVal - rIn) + rIn);

            // Suiveur RMS (intégration du carré du signal)
            float lSq = lIn * lIn;
            float rSq = rIn * rIn;
            leftRmsVal = rmsCoef * (leftRmsVal - lSq) + lSq;
            rightRmsVal = rmsCoef * (rightRmsVal - rSq) + rSq;
        }
    }

    // Getters renvoyant directement les niveaux en dBFS (bornés à -60 dB minimum)
    float getLeftPeakDb() const { return juce::Decibels::gainToDecibels(leftPeakVal, -60.0f); }
    float getRightPeakDb() const { return juce::Decibels::gainToDecibels(rightPeakVal, -60.0f); }

    float getLeftRmsDb() const { return juce::Decibels::gainToDecibels(std::sqrt(std::max(0.0f, leftRmsVal)), -60.0f); }
    float getRightRmsDb() const { return juce::Decibels::gainToDecibels(std::sqrt(std::max(0.0f, rightRmsVal)), -60.0f); }

private:
    double sr = 44100.0;
    float attackTimeMs = 10.0f;
    float releaseTimeMs = 100.0f;
    float rmsWindowMs = 300.0f;

    float peakAttackCoef = 0.0f;
    float peakReleaseCoef = 0.0f;
    float rmsCoef = 0.0f;

    float leftPeakVal = 0.0f;
    float rightPeakVal = 0.0f;
    float leftRmsVal = 0.0f;
    float rightRmsVal = 0.0f;

    void updatePeakCoefficients()
    {
        peakAttackCoef = (attackTimeMs > 0.0f) ? std::exp(-1.0f / (0.001f * attackTimeMs * sr)) : 0.0f;
        peakReleaseCoef = (releaseTimeMs > 0.0f) ? std::exp(-1.0f / (0.001f * releaseTimeMs * sr)) : 0.0f;
    }

    void updateRmsCoefficients()
    {
        rmsCoef = (rmsWindowMs > 0.0f) ? std::exp(-1.0f / (0.001f * rmsWindowMs * sr)) : 0.0f;
    }
};