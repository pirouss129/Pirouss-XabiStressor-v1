/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// --- CONSTRUCTEUR / DESTRUCTEUR ---
//==============================================================================
PiroussXabiStressorv1AudioProcessor::PiroussXabiStressorv1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
		 .withInput("Sidechain", juce::AudioChannelSet::stereo(), false) // <-- Ajout du bus Sidechain désactivé par défaut
                     #endif
                       ),
#endif
	// --- Initialisation de l'APVTS avec les paramètres ---
	apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
	inputStageParam = apvts.getRawParameterValue("INPUT_STAGE");
	detectorFilterParam = apvts.getRawParameterValue("DETECTOR_FILTER");
	bypassParam = apvts.getRawParameterValue("BYPASS");
	listenScParam = apvts.getRawParameterValue("LISTEN_SC");
	driveInputParam = apvts.getRawParameterValue("DRIVE_INPUT");
	driveOutputParam = apvts.getRawParameterValue("DRIVE_OUTPUT");
	detectorInputParam = apvts.getRawParameterValue("DETECTOR_INPUT");
	mixPeakRmsParam = apvts.getRawParameterValue("MIX_PEAK_RMS");
    	attackParam = apvts.getRawParameterValue("ATTACK");
		releaseParam = apvts.getRawParameterValue("RELEASE");
		thresholdParam = apvts.getRawParameterValue("THRESHOLD");
		ratioParam = apvts.getRawParameterValue("RATIO");
		compBypassParam = apvts.getRawParameterValue("COMP_BYPASS");
	oddSaturationParam = apvts.getRawParameterValue("DIST_ODD");
	evenSaturationParam = apvts.getRawParameterValue("DIST_EVEN");
	outputStageParam = apvts.getRawParameterValue("OUTPUT_STAGE");
    wetDryMixParam = apvts.getRawParameterValue("WET_DRY_MIX");
}

PiroussXabiStressorv1AudioProcessor::~PiroussXabiStressorv1AudioProcessor()
{
}

//==============================================================================
const juce::String PiroussXabiStressorv1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PiroussXabiStressorv1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PiroussXabiStressorv1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PiroussXabiStressorv1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PiroussXabiStressorv1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PiroussXabiStressorv1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PiroussXabiStressorv1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void PiroussXabiStressorv1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PiroussXabiStressorv1AudioProcessor::getProgramName (int index)
{
    return {};
}

void PiroussXabiStressorv1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// --- PREPARE TO PLAYS ---
//==============================================================================
void PiroussXabiStressorv1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	inputMeterDetector.prepare(sampleRate, samplesPerBlock);

	inputStage.prepare(sampleRate, samplesPerBlock);

    sideChainFilter.prepare({ sampleRate,
                              static_cast<juce::uint32>(samplesPerBlock),
                              static_cast<juce::uint32>(getTotalNumInputChannels()) });

	mixPeakRmsDetector.prepare(sampleRate, samplesPerBlock);

	harmonicSaturation.prepare(sampleRate, samplesPerBlock);
    
	outputStage.prepare(sampleRate, samplesPerBlock);

	wetDryMix.prepare(sampleRate, samplesPerBlock);

	outputMeterDetector.prepare(sampleRate, samplesPerBlock);
}

void PiroussXabiStressorv1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PiroussXabiStressorv1AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // 1. Le bus de sortie principal doit être en mono ou stéréo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // 2. L'entrée principale doit correspondre à la sortie
#if ! JucePlugin_IsSynth
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
#endif

    // 3. Gestion du bus d'entrée secondaire (Sidechain, index 1) s'il existe
    if (layouts.inputBuses.size() > 1)
    {
        auto sidechainLayout = layouts.inputBuses[1];
        if (!sidechainLayout.isDisabled()
            && sidechainLayout != juce::AudioChannelSet::mono()
            && sidechainLayout != juce::AudioChannelSet::stereo())
            return false;
    }

    return true;
#endif
}
#endif

//==============================================================================
// --- PROCESS BLOCK ---
//==============================================================================
void PiroussXabiStressorv1AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Nettoyage des flux inutilisés
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // --- GESTION DU BYPASS GLOBAL ---
    if (*bypassParam > 0.5f)
    {
        inputMeterDetector.process(buffer);
        outputMeterDetector.process(buffer);
        return;
    }

    // 1. Étage d'entrée & Vumètre d'input
    inputStage.setInputGainFromSlider(*inputStageParam);
    // On met à jour l'état de la saturation dans l'étage d'entrée selon l'APVTS
    inputStage.setSaturationEnabled(*driveInputParam > 0.5f);
    inputStage.process(buffer);
    inputMeterDetector.process(buffer);

    // --- CAPTURE DU DRY (avant les traitements/sat/comp) ---
    wetDryMix.captureDry(buffer);

    // --- SÉLECTION DE LA SOURCE DU DÉTECTEUR (Detector Input) ---
    // 0 = Main Input, 1 = Ext SC, 2 = Comp Output (Feedback)
    int detectorChoice = static_cast<int>(*detectorInputParam);
    juce::AudioBuffer<float> sidechainBuffer;

    if (detectorChoice == 1)
    {
        // --- 1. EXTERNAL SIDECHAIN ---
        auto* sidechainBus = getBus(true, 1);
        if (sidechainBus != nullptr && sidechainBus->isEnabled())
        {
            sidechainBuffer = getBusBuffer(buffer, true, 1);
        }
        else
        {
            sidechainBuffer.makeCopyOf(buffer); // Fallback si le bus externe n'est pas actif
        }
    }
    else if (detectorChoice == 2)
    {
        // --- 2. COMP OUTPUT (FEEDBACK) ---
        // On récupère le signal après le passage du compresseur (du bloc précédent ou de l'état actuel)
        sidechainBuffer.makeCopyOf(buffer);
    }
    else
    {
        // --- 0. MAIN INPUT (Interne classique) ---
        sidechainBuffer.makeCopyOf(buffer);
    }

    // --- FILTRAGE DU SIDECHAIN ---
    sideChainFilter.setMode(static_cast<int>(*detectorFilterParam));
    sideChainFilter.process(sidechainBuffer);

    // --- DÉTECTION MIX PEAK/RMS SUR LE SIDECHAIN ---
    float controlLevel = mixPeakRmsDetector.processAndGetLevel(sidechainBuffer, *mixPeakRmsParam);

    // --- GESTION DU BOUTON "LISTEN SC" ---
    if (*listenScParam > 0.5f)
    {
        for (int ch = 0; ch < totalNumInputChannels; ++ch)
            buffer.copyFrom(ch, 0, sidechainBuffer, ch, 0, buffer.getNumSamples());

        outputMeterDetector.process(buffer);
        return;
    }

    // --- CŒUR DU COMPRESSEUR ---
    if (*compBypassParam > 0.5f)
    {
        compressorCore.setThreshold(*thresholdParam);
        compressorCore.setAttackFromSlider(*attackParam);
        compressorCore.setReleaseFromSlider(*releaseParam);
        compressorCore.setRatioMode(static_cast<int>(*ratioParam));

        // On applique la compression sur le buffer principal avec le niveau calculé
        compressorCore.process(buffer, controlLevel);
    }

    // --- SATURATION HARMONIQUE (Odd / Even) ---
    harmonicSaturation.setSaturationEnabled(*driveOutputParam > 0.5f);
    harmonicSaturation.setDistOddFromSlider(*oddSaturationParam);
    harmonicSaturation.setDistEvenFromSlider(*evenSaturationParam);
    harmonicSaturation.process(buffer);

    // --- ÉTAGE DE SORTIE ---
    outputStage.setOutputGainFromSlider(*outputStageParam);
    outputStage.process(buffer);

    // --- MIX WET / DRY ---
    wetDryMix.setMixFromSlider(*wetDryMixParam);
    wetDryMix.process(buffer);

    // --- VUMÈTRE DE SORTIE ---
    outputMeterDetector.process(buffer);
}

//==============================================================================
bool PiroussXabiStressorv1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PiroussXabiStressorv1AudioProcessor::createEditor()
{
    return new PiroussXabiStressorv1AudioProcessorEditor (*this);
}

//==============================================================================
// --- GET & SET STATE ---
//==============================================================================
void PiroussXabiStressorv1AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Optionnel : sauvegarde de la taille de la fenêtre si ton UI est redimensionnable
    if (auto* editor = dynamic_cast<PiroussXabiStressorv1AudioProcessorEditor*>(getActiveEditor()))
    {
        apvts.state.setProperty("editorWidth", editor->getWidth(), nullptr);
        apvts.state.setProperty("editorHeight", editor->getHeight(), nullptr);
    }

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PiroussXabiStressorv1AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// --- APVTS - PARAMETERS LAYOUT ---
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PiroussXabiStressorv1AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

	// --- INPUT STAGE ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("INPUT_STAGE", "Input Stage", 0.0f, 10.0f, 5.0f));
	// --- BYPASS ---
	layout.add(std::make_unique<juce::AudioParameterBool>("BYPASS", "Bypass", false));

	// --- DRIVE INPUT ---
	layout.add(std::make_unique<juce::AudioParameterBool>("DRIVE_INPUT", "Drive Input", true));
	// --- DRIVE OUTPUT ---
	layout.add(std::make_unique<juce::AudioParameterBool>("DRIVE_OUTPUT", "Drive Output", true));

	// --- DETECTOR INPUT ---
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		"DETECTOR_INPUT",
		"Detector Input",
		juce::StringArray{ "Main Input", "Ext SC", "FeedBack"},
		0 // Index par défaut ("Main Input")
	));

    // --- DETECTOR FILTER ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "DETECTOR_FILTER",
        "Detector Filter",
        juce::StringArray{ "Flat", "HP", "HP & Boost1", "HP & Boost2" },
        0 // Index par défaut ("Flat")
    ));

    // --- LISTEN SIDECHAIN ---
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "LISTEN_SC",
        "Listen SideChain",
        false
    ));

	// --- PEAK / RMS ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "MIX_PEAK_RMS",
        "Mix Peak / RMS",
        0.0f,
        10.0f,
		0.0f // Valeur par défaut à 0 full peak
    ));
	// --- ATTACK ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", 0.0f, 10.0f, 0.0f));
	// --- RELEASE ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", 0.0f, 10.0f, 0.0f));
	// --- THRESHOLD ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("THRESHOLD", "Threshold", -40.0f, 0.0f, -20.0f));
	// --- RATIO ---
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "RATIO", "Ratio Mode",
        juce::StringArray{ "LOL 1", "LOL 2", "Why Not", "OK", "Haaaaa", "Opto", "Sgal Mood" },
        0 // Index par défaut (1:1 => LOL 1)
    ));
	// --- COMP BYPASS ---
	layout.add(std::make_unique<juce::AudioParameterBool>("COMP_BYPASS", "Comp Bypass", true));
	// --- ODD SATURATION ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("DIST_ODD", "Dist Odd", 0.0f, 10.0f, 0.0f));
	// --- EVEN SATURATION ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("DIST_EVEN", "Dist Even", 0.0f, 10.0f, 0.0f));
	// --- OUTPUT STAGE ---
	layout.add(std::make_unique<juce::AudioParameterFloat>("OUTPUT_STAGE", "Output Stage", 0.0f, 10.0f, 5.0f));
    // --- WET/DRY MIX ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("WET_DRY_MIX", "Wet Dry Mix", 0.0f, 10.0f, 10.0f));
	return layout;
	
}

void PiroussXabiStressorv1AudioProcessor::setOptoMode()
{
    // 1. Ratio sur "Opto" (Index 5)
    if (auto* ratioParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("RATIO")))
    {
        // On calcule la valeur normalisée (entre 0.0 et 1.0) correspondant à l'index 5
        float choiceNormalized = ratioParam->convertTo0to1(static_cast<float>(5));
        ratioParam->setValueNotifyingHost(choiceNormalized);
    }

    // 2. SideChain Detector sur "FeedBack" (Index 2)
    if (auto* scParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("DETECTOR_INPUT")))
    {
        float choiceNormalized = scParam->convertTo0to1(static_cast<float>(2));
        scParam->setValueNotifyingHost(choiceNormalized);
    }

    // 3. Attack au max
    if (auto* atkParam = apvts.getParameter("ATTACK"))
        atkParam->setValueNotifyingHost(1.0f);

    // 4. Release au max
    if (auto* relParam = apvts.getParameter("RELEASE"))
        relParam->setValueNotifyingHost(1.0f);

	// 5. Detector full RMS 
    if (auto* mixParam = apvts.getParameter("MIX_PEAK_RMS"))
        mixParam->setValueNotifyingHost(1.0f);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PiroussXabiStressorv1AudioProcessor();
}
