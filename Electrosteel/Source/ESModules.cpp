/*
  ==============================================================================

    ESModules.cpp
    Created: 2 Jul 2021 3:06:27pm
    Author:  Matthew Wang

  ==============================================================================
*/

#include "ESModules.h"
#include "PluginEditor.h"

//==============================================================================
//==============================================================================

ESModule::ESModule(ESAudioProcessorEditor& editor, AudioProcessorValueTreeState& vts, AudioComponent& ac,
                   float relLeftMargin, float relDialWidth, float relDialSpacing,
                   float relTopMargin, float relDialHeight) :
editor(editor),
vts(vts),
ac(ac),
relLeftMargin(relLeftMargin),
relDialWidth(relDialWidth),
relDialSpacing(relDialSpacing),
relTopMargin(relTopMargin),
relDialHeight(relDialHeight),
outlineColour(Colours::transparentBlack)
{
    setInterceptsMouseClicks(false, true);
    
    String& name = ac.getName();
    StringArray& paramNames = ac.getParamNames();
    for (int i = 0; i < paramNames.size(); i++)
    {
        String paramName = name + " " + paramNames[i];
        String displayName = paramNames[i];
        dials.add(new ESDial(editor, paramName, displayName, false, true));
        addAndMakeVisible(dials[i]);
        sliderAttachments.add(new SliderAttachment(vts, paramName, dials[i]->getSlider()));
        dials[i]->getSlider().addListener(this);
        for (auto t : dials[i]->getTargets())
        {
            t->addListener(this);
            t->addMouseListener(this, true);
            t->updateRange();
            t->updateValue(false);
        }
    }
    
    if (ac.isToggleable())
    {
        addAndMakeVisible(enabledToggle);
        buttonAttachments.add(new ButtonAttachment(vts, name, enabledToggle));
    }
}

ESModule::~ESModule()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void ESModule::paint(Graphics &g)
{
    Rectangle<int> area = getLocalBounds();
    
    g.setColour(outlineColour);
    g.drawRect(area);
}

void ESModule::resized()
{
    Rectangle<int> area = getLocalBounds();
    
    float h = area.getHeight();
    
    for (int i = 0; i < ac.getParamNames().size(); ++i)
    {
        dials[i]->setBoundsRelative(relLeftMargin + (relDialWidth+relDialSpacing)*i, relTopMargin,
                                    relDialWidth, relDialHeight);
    }
    
    if (ac.isToggleable())
    {
        enabledToggle.setBounds(0, 0, h*0.2f, h*0.2f);
    }
}

void ESModule::setBounds (float x, float y, float w, float h)
{
    Rectangle<float> newBounds (x, y, w, h);
    setBounds(newBounds);
}

void ESModule::setBounds (Rectangle<float> newBounds)
{
    Component::setBounds(newBounds.toNearestInt());
}

ESDial* ESModule::getDial (int index)
{
    return dials[index];
}

//==============================================================================
//==============================================================================

OscModule::OscModule(ESAudioProcessorEditor& editor, AudioProcessorValueTreeState& vts,
                     AudioComponent& ac) :
ESModule(editor, vts, ac, 0.05f, 0.132f, 0.05f, 0.18f, 0.78f),
chooser("Select wavetable file or folder...",
              File::getSpecialLocation(File::userDocumentsDirectory))
{
    outlineColour = Colours::darkgrey;
    
    // Pitch slider should snap to ints
    getDial(OscPitch)->setRange(-24., 24., 1.);
    
    double pitch = getDial(OscPitch)->getSlider().getValue();
    double fine = getDial(OscFine)->getSlider().getValue()*0.01; // Fine is in cents for better precision
    pitchLabel.setText(String(pitch+fine, 3), dontSendNotification);
    pitchLabel.setLookAndFeel(&laf);
    pitchLabel.setEditable(true);
    pitchLabel.setJustificationType(Justification::centred);
    pitchLabel.setColour(Label::backgroundColourId, Colours::darkgrey.withBrightness(0.2f));
    pitchLabel.addListener(this);
    addAndMakeVisible(pitchLabel);
    
    RangedAudioParameter* set = vts.getParameter(ac.getName() + " ShapeSet");
    shapeCB.addItemList(oscShapeSetNames, 1);
    shapeCB.setSelectedItemIndex(set->convertFrom0to1(set->getValue()), dontSendNotification);
    if (shapeCB.getSelectedItemIndex() == shapeCB.getNumItems()-1)
    {
        Oscillator& osc = static_cast<Oscillator&>(ac);
        String text = osc.getWaveTableFile().getFileNameWithoutExtension();
        shapeCB.changeItemText(shapeCB.getItemId(shapeCB.getNumItems()-1), text);
        shapeCB.setText(text, dontSendNotification);
    }
    shapeCB.setLookAndFeel(&laf);
    shapeCB.addListener(this);
    addAndMakeVisible(shapeCB);
    comboBoxAttachments.add(new ComboBoxAttachment(vts, ac.getName() + " ShapeSet", shapeCB));
    
    sendSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    sendSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, false, 10, 10);
    addAndMakeVisible(sendSlider);
    sliderAttachments.add(new SliderAttachment(vts, ac.getName() + " FilterSend", sendSlider));
    
    f1Label.setText("F1", dontSendNotification);
    f1Label.setJustificationType(Justification::bottomRight);
    f1Label.setLookAndFeel(&laf);
    addAndMakeVisible(f1Label);
    
    f2Label.setText("F2", dontSendNotification);
    f2Label.setJustificationType(Justification::topRight);
    f2Label.setLookAndFeel(&laf);
    addAndMakeVisible(f2Label);
}

OscModule::~OscModule()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void OscModule::resized()
{
    ESModule::resized();
    
    for (int i = 1; i < ac.getParamNames().size(); ++i)
    {
        dials[i]->setBoundsRelative(relLeftMargin + (relDialWidth*(i+1))+(relDialSpacing*i),
                                    relTopMargin, relDialWidth, relDialHeight);
    }
    
    pitchLabel.setBoundsRelative(relLeftMargin+relDialWidth+0.5f*relDialSpacing,
                                 0.4f, relDialWidth, 0.2f);
    
    shapeCB.setBoundsRelative(relLeftMargin+3*relDialWidth+relDialSpacing, 0.01f,
                              relDialWidth+2*relDialSpacing, 0.16f);
    
    sendSlider.setBoundsRelative(0.94f, 0.f, 0.06f, 1.0f);
    
    f1Label.setBoundsRelative(0.9f, 0.05f, 0.06f, 0.15f);
    f2Label.setBoundsRelative(0.9f, 0.80f, 0.06f, 0.15f);
}

void OscModule::sliderValueChanged(Slider* slider)
{
    if (slider == &getDial(OscPitch)->getSlider() || slider == &getDial(OscFine)->getSlider() )
    {
        displayPitch();
    }
    else if (MappingTarget* mt = dynamic_cast<MappingTarget*>(slider))
    {
        displayPitchMapping(mt);
    }
}

void OscModule::labelTextChanged(Label* label)
{
    if (label == &pitchLabel)
    {
        auto value = pitchLabel.getText().getDoubleValue();
        int i = value;
        double f = value-i;
        getDial(OscPitch)->getSlider().setValue(i);
        getDial(OscFine)->getSlider().setValue(f*100.);
    }
}

void OscModule::comboBoxChanged(ComboBox *comboBox)
{
    if (comboBox == &shapeCB)
    {
        if (shapeCB.getSelectedItemIndex() == shapeCB.getNumItems()-1)
        {
            Oscillator& osc = static_cast<Oscillator&>(ac);
            osc.setLoadingTables(true);
            chooser.launchAsync (FileBrowserComponent::openMode |
                                 FileBrowserComponent::canSelectFiles |
                                 FileBrowserComponent::canSelectDirectories,
                                 [this] (const FileChooser& chooser)
                                 {
                String path = chooser.getResult().getFullPathName();
                Oscillator& osc = static_cast<Oscillator&>(ac);
                
                if (path.isEmpty())
                {
                    shapeCB.setSelectedItemIndex(0);
                    osc.setLoadingTables(false);
                    return;
                }
                
                File file(path);
            
                osc.setWaveTableFile(file);
                shapeCB.changeItemText(shapeCB.getItemId(shapeCB.getNumItems()-1),
                                       file.getFileNameWithoutExtension());
                shapeCB.setText(file.getFileNameWithoutExtension(), dontSendNotification);
                
                osc.clearWaveTables();
                osc.addWaveTables(file);
                osc.waveTablesChanged();
            });
        }
    }
}

void OscModule::mouseEnter(const MouseEvent& e)
{
    if (MappingTarget* mt = dynamic_cast<MappingTarget*>(e.originalComponent->getParentComponent()))
    {
        displayPitchMapping(mt);
    }
}

void OscModule::mouseExit(const MouseEvent& e)
{
    displayPitch();
}

void OscModule::displayPitch()
{
    auto pitch = getDial(OscPitch)->getSlider().getValue();
    auto fine = getDial(OscFine)->getSlider().getValue()*0.01;
    pitchLabel.setColour(Label::textColourId, Colours::gold.withBrightness(0.95f));
    pitchLabel.setText(String(pitch+fine, 3), dontSendNotification);
}

void OscModule::displayPitchMapping(MappingTarget* mt)
{
    if (!mt->isActive())
    {
        displayPitch();
        return;
    }
    auto value = mt->getValue();
    if (mt->getParentComponent() == getDial(OscPitch))
    {
        pitchLabel.setColour(Label::textColourId, mt->getColour());
        String text;
        if (mt->isBipolar()) text = String::charToString(0xb1);
        else text = (value >= 0 ? "+" : "-");
        text += String(fabs(value), 3);
        pitchLabel.setText(text, dontSendNotification);
    }
    else if (mt->getParentComponent() == getDial(OscFine))
    {
        pitchLabel.setColour(Label::textColourId, mt->getColour());
        String text;
        if (mt->isBipolar()) text = String::charToString(0xb1);
        else text = (value >= 0 ? "+" : "-");
        text += String(fabs(value*0.01), 3);
        pitchLabel.setText(text, dontSendNotification);
    }
}

//==============================================================================
//==============================================================================

FilterModule::FilterModule(ESAudioProcessorEditor& editor, AudioProcessorValueTreeState& vts,
                           AudioComponent& ac) :
ESModule(editor, vts, ac, 0.05f, 0.2f, 0.05f, 0.2f, 0.7f)
{
    outlineColour = Colours::darkgrey;
    
    double cutoff = getDial(FilterCutoff)->getSlider().getValue();
    cutoffLabel.setText(String(cutoff, 2), dontSendNotification);
    cutoffLabel.setLookAndFeel(&laf);
    cutoffLabel.setEditable(true);
    cutoffLabel.setJustificationType(Justification::centred);
    cutoffLabel.setColour(Label::backgroundColourId, Colours::darkgrey.withBrightness(0.2f));
    cutoffLabel.addListener(this);
    addAndMakeVisible(cutoffLabel);
    
    RangedAudioParameter* set = vts.getParameter(ac.getName() + " Type");
    typeCB.addItemList(filterTypeNames, 1);
    typeCB.setSelectedItemIndex(set->convertFrom0to1(set->getValue()));
    typeCB.setLookAndFeel(&laf);
    addAndMakeVisible(typeCB);
    comboBoxAttachments.add(new ComboBoxAttachment(vts, ac.getName() + " Type", typeCB));
}

FilterModule::~FilterModule()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void FilterModule::resized()
{
    ESModule::resized();
    
    for (int i = 1; i < ac.getParamNames().size(); ++i)
    {
        dials[i]->setBoundsRelative(relLeftMargin + (relDialWidth*(i+1))+(relDialSpacing*i),
                                    relTopMargin, relDialWidth, relDialHeight);
    }
    
    cutoffLabel.setBoundsRelative(relLeftMargin+relDialWidth+0.5f*relDialSpacing,
                                  0.42f, relDialWidth, 0.16f);
    
    typeCB.setBoundsRelative(relLeftMargin+relDialWidth, 0.01f,
                             relDialWidth+relDialSpacing, 0.16f);
}

void FilterModule::sliderValueChanged(Slider* slider)
{
    if (slider == &getDial(FilterCutoff)->getSlider())
    {
        displayCutoff();
    }
    else if (MappingTarget* mt = dynamic_cast<MappingTarget*>(slider))
    {
        displayCutoffMapping(mt);
    }
}

void FilterModule::labelTextChanged(Label* label)
{
    if (label == &cutoffLabel)
    {
        auto value = cutoffLabel.getText().getDoubleValue();
        getDial(FilterCutoff)->getSlider().setValue(value);
    }
}

void FilterModule::mouseEnter(const MouseEvent& e)
{
    if (MappingTarget* mt = dynamic_cast<MappingTarget*>(e.originalComponent->getParentComponent()))
    {
        displayCutoffMapping(mt);
    }
}

void FilterModule::mouseExit(const MouseEvent& e)
{
    displayCutoff();
}

void FilterModule::displayCutoff()
{
    double cutoff = getDial(FilterCutoff)->getSlider().getValue();
    cutoffLabel.setColour(Label::textColourId, Colours::gold.withBrightness(0.95f));
    cutoffLabel.setText(String(cutoff, 2), dontSendNotification);
}

void FilterModule::displayCutoffMapping(MappingTarget* mt)
{
    if (!mt->isActive()) displayCutoff();
    else if (mt->getParentComponent() == getDial(FilterCutoff))
    {
        auto value = mt->getValue();
        cutoffLabel.setColour(Label::textColourId, mt->getColour());
        String text;
        if (mt->isBipolar()) text = String::charToString(0xb1);
        else text = (value >= 0 ? "+" : "-");
        text += String(fabs(value), 2);
        cutoffLabel.setText(text, dontSendNotification);
    }
}

//==============================================================================
//==============================================================================

EnvModule::EnvModule(ESAudioProcessorEditor& editor, AudioProcessorValueTreeState& vts,
                     AudioComponent& ac) :
ESModule(editor, vts, ac, 0.04f, 0.13f, 0.0675f, 0.16f, 0.84f)
{
    velocityToggle.setButtonText("Scale to velocity");
    addAndMakeVisible(velocityToggle);
    buttonAttachments.add(new ButtonAttachment(vts, ac.getName() + " Velocity", velocityToggle));
}

EnvModule::~EnvModule()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void EnvModule::resized()
{
    ESModule::resized();
    
    velocityToggle.setBoundsRelative(relLeftMargin, 0.f, 2*relDialWidth+relDialSpacing, 0.16f);
}

//==============================================================================
//==============================================================================

LFOModule::LFOModule(ESAudioProcessorEditor& editor, AudioProcessorValueTreeState& vts,
                     AudioComponent& ac) :
ESModule(editor, vts, ac, 0.12f, 0.13f, 0.185f, 0.16f, 0.84f),
chooser("Select wavetable file or folder...",
        File::getSpecialLocation(File::userDocumentsDirectory))
{
    double rate = getDial(LowFreqRate)->getSlider().getValue();
    rateLabel.setText(String(rate, 2) + " Hz", dontSendNotification);
    rateLabel.setLookAndFeel(&laf);
    rateLabel.setEditable(true);
    rateLabel.setJustificationType(Justification::centred);
    rateLabel.setColour(Label::backgroundColourId, Colours::darkgrey.withBrightness(0.2f));
    rateLabel.addListener(this);
    addAndMakeVisible(rateLabel);
    
    RangedAudioParameter* set = vts.getParameter(ac.getName() + " ShapeSet");
    shapeCB.addItemList(oscShapeSetNames, 1);
    shapeCB.setSelectedItemIndex(set->convertFrom0to1(set->getValue()), dontSendNotification);
    if (shapeCB.getSelectedItemIndex() == shapeCB.getNumItems()-1)
    {
        LowFreqOscillator& osc = static_cast<LowFreqOscillator&>(ac);
        String text = osc.getWaveTableFile().getFileNameWithoutExtension();
        shapeCB.changeItemText(shapeCB.getItemId(shapeCB.getNumItems()-1), text);
        shapeCB.setText(text, dontSendNotification);
    }
    shapeCB.setLookAndFeel(&laf);
    shapeCB.addListener(this);
    addAndMakeVisible(shapeCB);
    comboBoxAttachments.add(new ComboBoxAttachment(vts, ac.getName() + " ShapeSet", shapeCB));
    
    syncToggle.setButtonText("Sync to note-on");
    addAndMakeVisible(syncToggle);
    buttonAttachments.add(new ButtonAttachment(vts, ac.getName() + " Sync", syncToggle));
}

LFOModule::~LFOModule()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void LFOModule::resized()
{
    ESModule::resized();
    
    rateLabel.setBoundsRelative(relLeftMargin-0.3*relDialSpacing, 0.f,
                                relDialWidth+0.6f*relDialSpacing, 0.16f);
    shapeCB.setBoundsRelative(relLeftMargin+relDialWidth+0.7f*relDialSpacing, 0.f,
                              relDialWidth+0.6f*relDialSpacing, 0.16f);
    syncToggle.setBoundsRelative(relLeftMargin+2*relDialWidth+1.7f*relDialSpacing, 0.f,
                                 relDialWidth+0.6f*relDialSpacing, 0.16f);
}

void LFOModule::sliderValueChanged(Slider* slider)
{
    if (slider == &getDial(LowFreqRate)->getSlider())
    {
        displayRate();
    }
    else if (MappingTarget* mt = dynamic_cast<MappingTarget*>(slider))
    {
        displayRateMapping(mt);
    }
}

void LFOModule::labelTextChanged(Label* label)
{
    if (label == &rateLabel)
    {
        auto value = rateLabel.getText().getDoubleValue();
        getDial(LowFreqRate)->getSlider().setValue(value);
    }
}

void LFOModule::comboBoxChanged(ComboBox *comboBox)
{
    if (comboBox == &shapeCB)
    {
        if (shapeCB.getSelectedItemIndex() == shapeCB.getNumItems()-1)
        {
            LowFreqOscillator& osc = static_cast<LowFreqOscillator&>(ac);
            osc.setLoadingTables(true);
            chooser.launchAsync (FileBrowserComponent::openMode |
                                 FileBrowserComponent::canSelectFiles |
                                 FileBrowserComponent::canSelectDirectories,
                                 [this] (const FileChooser& chooser)
                                 {
                String path = chooser.getResult().getFullPathName();
                LowFreqOscillator& osc = static_cast<LowFreqOscillator&>(ac);
                if (path.isEmpty())
                {
                    shapeCB.setSelectedItemIndex(0);
                    osc.setLoadingTables(false);
                    return;
                }
                
                File file(path);
                
                osc.setWaveTableFile(file);
                shapeCB.changeItemText(shapeCB.getItemId(shapeCB.getNumItems()-1),
                                       file.getFileNameWithoutExtension());
                shapeCB.setText(file.getFileNameWithoutExtension(), dontSendNotification);
                
                osc.clearWaveTables();
                osc.addWaveTables(file);
                osc.waveTablesChanged();
            });
        }
    }
}

void LFOModule::mouseEnter(const MouseEvent& e)
{
    if (MappingTarget* mt = dynamic_cast<MappingTarget*>(e.originalComponent->getParentComponent()))
    {
        displayRateMapping(mt);
    }
}

void LFOModule::mouseExit(const MouseEvent& e)
{
    displayRate();
}

void LFOModule::displayRate()
{
    double rate = getDial(LowFreqRate)->getSlider().getValue();
    rateLabel.setColour(Label::textColourId, Colours::gold.withBrightness(0.95f));
    rateLabel.setText(String(rate, 2) + " Hz", dontSendNotification);
}

void LFOModule::displayRateMapping(MappingTarget* mt)
{
    if (!mt->isActive()) displayRate();
    else if (mt->getParentComponent() == getDial(LowFreqRate))
    {
        auto value = mt->getValue();
        rateLabel.setColour(Label::textColourId, mt->getColour());
        String text;
        if (mt->isBipolar()) text = String::charToString(0xb1);
        else text = (value >= 0 ? "+" : "");
        text += String(fabs(value), 2) + " Hz";
        rateLabel.setText(text, dontSendNotification);
    }
}

//==============================================================================
//==============================================================================

OutputModule::OutputModule(ESAudioProcessorEditor& editor, AudioProcessorValueTreeState& vts,
                           AudioComponent& ac) :
ESModule(editor, vts, ac, 0.1f, 0.2f, 0.1f, 0.125f, 0.75f)
{
    outlineColour = Colours::darkgrey;
    
    masterDial = std::make_unique<ESDial>(editor, "Master", "Master", false, false);
    sliderAttachments.add(new SliderAttachment(vts, "Master", masterDial->getSlider()));
    addAndMakeVisible(masterDial.get());
}

OutputModule::~OutputModule()
{
    sliderAttachments.clear();
    buttonAttachments.clear();
    comboBoxAttachments.clear();
}

void OutputModule::resized()
{
    ESModule::resized();
    
    masterDial->setBoundsRelative(0.7f, relTopMargin, 0.2f, relDialHeight);
}
