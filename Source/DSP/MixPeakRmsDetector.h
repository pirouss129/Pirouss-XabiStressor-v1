/*  ==============================================================================

    MixPeakRmsDetector.h
    Created: 5 Nov 2026 7:55:45pm
    Author:  rousseau-p

  ==============================================================================*/

#pragma once

#include <JuceHeader.h>

class MixPeakRmsDetector
{
public:
    MixPeakRmsDetector() = default;
    ~MixPeakRmsDetector() = default;

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
        currentLeftLevel = 0.0f;
        currentRightLevel = 0.0f;
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

    // Setter pour le mix Peak / RMS (de 0.0 à 10.0, par défaut 0.0 = full peak)
    void setPeakRmsMix(float mixVal)
    {
        // On normalise entre 0.0 (full peak) et 1.0 (full rms)
        peakRmsMix = juce::jlimit(0.0f, 1.0f, mixVal / 10.0f);
    }

    // Traitement du buffer de sidechain et retour du niveau combiné (linéaire - max des deux pour le compresseur)
    float processAndGetLevel(const juce::AudioBuffer<float>& buffer, float currentMixParam)
    {
        // Met à jour le mix si passé à la volée depuis le processBlock
        setPeakRmsMix(currentMixParam);

        if (buffer.getNumChannels() < 2)
            return 0.0f;

        const auto* left = buffer.getReadPointer(0);
        const auto* right = buffer.getReadPointer(1);
        int numSamples = buffer.getNumSamples();

        float combinedLevel = 0.0f;

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

            // Valeurs RMS instantanées (racine carrée)
            float leftRms = std::sqrt(std::max(0.0f, leftRmsVal));
            float rightRms = std::sqrt(std::max(0.0f, rightRmsVal));

            // Mix Peak/RMS pour chaque canal
            currentLeftLevel = ((1.0f - peakRmsMix) * leftPeakVal) + (peakRmsMix * leftRms);
            currentRightLevel = ((1.0f - peakRmsMix) * rightPeakVal) + (peakRmsMix * rightRms);

            // On prend le max des deux pour le niveau combiné du compresseur
            combinedLevel = std::max(currentLeftLevel, currentRightLevel);
        }

        return combinedLevel;
    }

    // Getters pour récupérer les niveaux indépendants gauche et droite (pour l'UI / bargraphs)
    float getLeftLevel() const { return currentLeftLevel; }
    float getRightLevel() const { return currentRightLevel; }

private:
    double sr = 44100.0;
    float attackTimeMs = 10.0f;
    float releaseTimeMs = 100.0f;
    float rmsWindowMs = 300.0f;
    float peakRmsMix = 0.0f; // 0.0 = Full Peak par défaut

    float peakAttackCoef = 0.0f;
    float peakReleaseCoef = 0.0f;
    float rmsCoef = 0.0f;

    float leftPeakVal = 0.0f;
    float rightPeakVal = 0.0f;
    float leftRmsVal = 0.0f;
    float rightRmsVal = 0.0f;

    float currentLeftLevel = 0.0f;
    float currentRightLevel = 0.0f;

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