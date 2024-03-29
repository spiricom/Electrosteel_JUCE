/*
 ==============================================================================
 
 ESComponents.h
 Created: 19 Feb 2021 12:42:05pm
 Author:  Matthew Wang
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ESLookAndFeel.h"

class ESAudioProcessorEditor;

//==============================================================================

class MappingSource : public Component
{
public:
    MappingSource(ESAudioProcessorEditor& editor, MappingSourceModel& model,
                  const String &displayName);
    ~MappingSource() override;
    
    Colour getColour() { return model.colour; }
    
    MappingSourceModel& getModel() { return model; }
    
    void resized() override;
    
    Label label;
    DrawableButton button;
    
private:
    ESAudioProcessor& processor;
    MappingSourceModel& model;

    std::unique_ptr<Drawable> image;
    
    ESLookAndFeel laf;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingSource)
};

//==============================================================================

class MappingTarget : public Slider,
                      public DragAndDropTarget
{
public:
    
    MappingTarget(ESAudioProcessorEditor& editor, MappingTargetModel& model);
    ~MappingTarget() override;
    
    String getTextFromValue(double value) override { return text; }
    Colour getColour()
    {
        if (model.currentSource == nullptr) return Colours::transparentBlack;
        else return model.currentSource->colour;
    }
    Colour getScalarColour()
    {
        if (model.currentScalarSource == nullptr) return Colours::transparentBlack;
        else return model.currentScalarSource->colour;
    }
    
    MappingTargetModel& getModel() { return model; }
    bool isBipolar() { return model.bipolar; }
    bool isActive() { return sliderEnabled; }
    
    bool isInterestedInDragSource(const SourceDetails &dragSourceDetails) override;
    void itemDropped(const SourceDetails &dragSourceDetails) override;
    
    void paint(Graphics&) override;
    void resized() override;
    
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    
    void update(bool directChange, bool sendListenerNotif);

    void setText(String s);
    void setTextColour(Colour colour);
    
    void setMapping(MappingSource* source, float end);
    void setMappingRange(float end, bool directChange, bool sendListenerNotif);
    void setMappingScalar(MappingSource* source);
    void removeMapping();
    void removeScalar();
    
    Label* getValueLabel();
    String getScalarString();
    
    static void menuCallback(int result, MappingTarget* target);
    
private:
    ESAudioProcessor& processor;
    MappingTargetModel& model;
    
    String text;
    bool sliderEnabled;
    double overflowValue;
    double lastProportionalValue;
    double lastProportionalParentValue;
    
    ESLookAndFeel laf;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingTarget)
};

//==============================================================================

class ESDial : public Component,
               public Slider::Listener
{
public:
    
    ESDial(ESAudioProcessorEditor& editor, const String& paramName, const String& displayName, bool isSource, bool isTarget);
    ~ESDial() override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged(Slider* slider) override;
    
    void mouseDown(const MouseEvent& event) override;
    
    void setRange(double newMin, double newMax, double newInt);
    
    void setText (const String& newText, NotificationType notification);
    void setFont (const Font& newFont);
    
    MappingTarget* getTarget(int index);
    OwnedArray<MappingTarget>& getTargets();
    MappingSource* getSource();
    
    Slider& getSlider() { return slider; }
    Label& getLabel() { return label; }
    
private:
    
    Slider slider;
    OwnedArray<MappingTarget> t;
    std::unique_ptr<MappingSource> s;
    Label label;
    double lastSliderValue = DBL_MAX;
    
    ESLookAndFeel laf;
    
    static const int numTargets = 3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ESDial)
};

//==============================================================================

class ESTabbedComponent : public TabbedComponent
{
public:
    using TabbedComponent::TabbedComponent;
    
    void currentTabChanged (int newCurrentTabIndex, const String &newCurrentTabName) override
    {
        TabbedButtonBar& bar = getTabbedButtonBar();
        for (int i = 0; i < bar.getNumTabs(); ++i)
        {
            bar.getTabButton(i)->setAlpha(i == getCurrentTabIndex() ? 1.0f : 0.7f);
        }
    }
};

//==============================================================================
class CopedentTable : public Component,
                      public TableListBoxModel,
                      public ComboBox::Listener
{
public:
    CopedentTable(ESAudioProcessor& p, AudioProcessorValueTreeState& vts) :
    processor(p),
    copedentArray(processor.copedentArray),
    fundamental(processor.copedentFundamental),
    fundamentalField(*this),
    numberField(*this),
    nameField(*this),
    exportChooser("Export copedent to .xml...",
                  File::getSpecialLocation(File::userDocumentsDirectory),
                  "*.xml"),
    importChooser("Import copedent .xml...",
                  File::getSpecialLocation(File::userDocumentsDirectory),
                  "*.xml")
    {
        for (int i = 0; i < CopedentColumnNil; ++i)
        {
            columnList.add(cCopedentColumnNames[i]);
        }
        
        stringTable.setModel (this);
        stringTable.setLookAndFeel(&laf);
        stringTable.setColour (ListBox::outlineColourId, Colours::grey);
        stringTable.setOutlineThickness (1);
        
        leftTable.setModel (this);
        leftTable.setLookAndFeel(&laf);
        leftTable.setColour (ListBox::outlineColourId, Colours::grey);
        leftTable.setOutlineThickness (1);
        
        pedalTable.setModel (this);
        pedalTable.setLookAndFeel(&laf);
        pedalTable.setColour (ListBox::outlineColourId, Colours::grey);
        pedalTable.setOutlineThickness (1);
        
        rightTable.setModel (this);
        rightTable.setLookAndFeel(&laf);
        rightTable.setColour (ListBox::outlineColourId, Colours::grey);
        rightTable.setOutlineThickness (1);
        
        int i = 0;
        int columnId = 1;
        TableHeaderComponent::ColumnPropertyFlags flags =
        TableHeaderComponent::ColumnPropertyFlags::notResizableOrSortable;
        stringTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        
        leftTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        leftTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        leftTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        
        pedalTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        pedalTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        pedalTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        pedalTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        pedalTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        
        rightTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);
        rightTable.getHeader().addColumn(columnList[i++], columnId++, 20, 1, 1000, flags);

        addAndMakeVisible (stringTable);
        addAndMakeVisible (leftTable);
        addAndMakeVisible (pedalTable);
        addAndMakeVisible (rightTable);
        
        fundamentalField.setRowAndColumn(0, 0);
        fundamentalField.setLookAndFeel(&laf);
        addAndMakeVisible (fundamentalField);
        
        fundamentalLabel.setText("Fundamental", dontSendNotification);
        fundamentalLabel.setJustificationType(Justification::centred);
        fundamentalLabel.setLookAndFeel(&laf);
        addAndMakeVisible (fundamentalLabel);
        
        exportButton.setButtonText("Export .xml");
        exportButton.setLookAndFeel(&laf);
        exportButton.onClick = [this] { exportXml(); };
        addAndMakeVisible(exportButton);
        
        importButton.setButtonText("Import .xml");
        importButton.setLookAndFeel(&laf);
        importButton.onClick = [this] { importXml(); };
        addAndMakeVisible(importButton);
        
        numberLabel.setText("#", dontSendNotification);
        numberLabel.setJustificationType(Justification::centred);
        numberLabel.setLookAndFeel(&laf);
        addAndMakeVisible (numberLabel);
        
        numberField.setRowAndColumn(0, -1);
        numberField.setLookAndFeel(&laf);
        addAndMakeVisible (numberField);
        
        nameLabel.setText("Name", dontSendNotification);
        nameLabel.setJustificationType(Justification::centred);
        nameLabel.setLookAndFeel(&laf);
        addAndMakeVisible (nameLabel);
        
        nameField.setRowAndColumn(0, -2);
        nameField.setLookAndFeel(&laf);
        addAndMakeVisible (nameField);
        
        sendOutButton.setButtonText("Send copedent via MIDI");
        sendOutButton.setLookAndFeel(&laf);
        sendOutButton.onClick = [this] { processor.sendCopedentMidiMessage(); };
        addAndMakeVisible(sendOutButton);
    }
    
    ~CopedentTable()
    {
        stringTable.setLookAndFeel(nullptr);
        leftTable.setLookAndFeel(nullptr);
        pedalTable.setLookAndFeel(nullptr);
        rightTable.setLookAndFeel(nullptr);
        fundamentalField.setLookAndFeel(nullptr);
        fundamentalLabel.setLookAndFeel(nullptr);
        exportButton.setLookAndFeel(nullptr);
        importButton.setLookAndFeel(nullptr);
        numberField.setLookAndFeel(nullptr);
        nameField.setLookAndFeel(nullptr);
        numberLabel.setLookAndFeel(nullptr);
        nameLabel.setLookAndFeel(nullptr);
        sendOutButton.setLookAndFeel(nullptr);
    }
    
    //==============================================================================
    // this loads the embedded database XML file into memory
    void exportXml()
    {
        exportChooser.launchAsync (FileBrowserComponent::saveMode |
                                   FileBrowserComponent::canSelectFiles |
                                   FileBrowserComponent::warnAboutOverwriting,
                                   [this] (const FileChooser& chooser)
                                   {
            String path = chooser.getResult().getFullPathName();
            if (path.isEmpty()) return;
            
            File file(path);
            
            ValueTree copedentVT("Copedent");
        
            for (int c = 0; c < numColumns; ++c)
            {
                String name = String(cCopedentColumnNames[c]);
                ValueTree child(name);
                for (int r = 0; r < numRows; ++r)
                {
                    child.setProperty("r" + String(r), copedentArray[c][r], nullptr);
                }
                copedentVT.addChild(child, -1, nullptr);
            }
            
            copedentVT.setProperty("Fundamental", fundamental, nullptr);
            
            std::unique_ptr<XmlElement> xml = copedentVT.createXml();
            
            xml->writeTo(file, XmlElement::TextFormat());
        });
    }
    
    void importXml()
    {
        importChooser.launchAsync (FileBrowserComponent::openMode |
                                   FileBrowserComponent::canSelectFiles,
                                   [this] (const FileChooser& chooser)
                                   {
            String path = chooser.getResult().getFullPathName();
            if (path.isEmpty()) return;
            File file(path);
            importXml(XmlDocument::parse(file).get());
        });
    }
    
    void importXml(XmlElement* xml)
    {
        for (int c = 0; c < numColumns; ++c)
        {
            String name = String(cCopedentColumnNames[c]);
            XmlElement* columnXml = xml->getChildByName(name);
            if (columnXml == nullptr) continue;
            for (int r = 0; r < numRows; ++r)
            {
                float value = columnXml->getDoubleAttribute("r" + String(r));
                copedentArray.getReference(c).set(r, value);
            }
        }
        fundamental = xml->getDoubleAttribute("Fundamental");
        resized();
    }
    
    String getTextFromData (const int columnNumber, const int rowNumber, bool asDestination) const
    {
        double value;
        
        if (columnNumber == -1) return String(processor.copedentNumber);
        else if (columnNumber == -2) return processor.copedentName;
        
        if (columnNumber == 0)
            value = fundamental;
        else
            value = copedentArray[columnNumber-1][rowNumber];
        
        if (columnNumber > 1 && value == 0.0) return String();
        
        // Not really important since tuning shouldn't matter much at precision this high,
        // but this will stop any potential ugly numbers from displaying
        value = round( value * 1000. ) / 1000.;
        
        String text = String();
        bool displayAsDestination = columnNumber <= 1 || asDestination;
        if (displayAsDestination)
        {
            if (columnNumber > 1)
            {
//                if (value > 0.0f) for (int i = 0; i < value; ++i) text += String("+");
//                else for (int i = 0; i < -value; ++i) text += String("-");
                value += copedentArray[0][rowNumber];
                value = round( value * 1000. ) / 1000.;
            }
            int n = round(value);
            double f = value - n;
            if (f == -0.5)
            {
                n -= 1;
                f += 1.;
            }
            
            bool useSharps = true;//(n % 12 != 3) && (n % 12 != 10);
            
            text += MidiMessage::getMidiNoteName(n, useSharps, true, 4);
            if (f > 0.0) text += "+" + String(f);
            else if (f < 0.0) text += String(f);
        }
        else
        {
            if (value > 0.0) text += "+";
            text += String(value);
        }
        return text;
    }
    
    void setDataFromText (const int columnNumber, const int rowNumber, const String& newText)
    {
        String text = newText.toUpperCase().removeCharacters(" ");
        
        if (columnNumber == -1)
        {
            int n = text.getIntValue();
            processor.copedentNumber = n < 0 ? 0 : n;
            return;
        }
        else if (columnNumber == -2)
        {
            processor.copedentName = text;
            return;
        }
        
        double value;
        if (text.isEmpty()) value = 0.0f;
        else if (!text.containsAnyOf("CDEFGAB"))
        {
            value = text.getDoubleValue();
            
            if (text.contains("/"))
            {
                int whereIsDivide = text.indexOf("/");
                String denominator = text.substring(whereIsDivide+1);
                value = value / denominator.getDoubleValue();
                float h = mtof(fundamental);
                value = ftom(value * h);
            }

            // Value isn't an offset
            if (!text.startsWith("+") && !text.startsWith("-"))
            {
                // Change value to an offset if not string column
                if (columnNumber > 1)
                {
                    value -= copedentArray[0][rowNumber];
                }
            }
        }
        else if (!text.containsOnly("0123456789CDEFGAB#+-.,"))
        {
            return;
        }
        else
        {
            int i = text.indexOfAnyOf("CDEFGAB");
            
            // Start by getting the basic pitch class number
            if (text[i] == 'C') value = 0.;
            else if (text[i] == 'D') value = 2.;
            else if (text[i] == 'E') value = 4.;
            else if (text[i] == 'F') value = 5.;
            else if (text[i] == 'G') value = 7.;
            else if (text[i] == 'A') value = 9.;
            else if (text[i] == 'B') value = 11.;
            i++;
            
            // Adjust for sharps and flats
            for (; i < text.length(); i++)
            {
                if (text[i] == '#') value++;
                else if (text[i] == 'B') value--;
                else break;
            }
            
            int fineIndex = text.indexOfAnyOf("+-", i);
            if (fineIndex >= 0) value += text.substring(fineIndex).getDoubleValue();
            else fineIndex = text.length();
                
            // Check for octave
            String withoutFine = text.substring(0, fineIndex);
            int octave = withoutFine.getTrailingIntValue();
            
            // Use it if we have it
            if (withoutFine.endsWith(String(octave)))
            {
                value += octave * 12 + 12;
            }
            else // Otherwise use the current fundamental octave
            {
                value += int(copedentArray[0][rowNumber] / 12) * 12;
                
                // Get the value as an offset from the fundamental
                double offset = value - copedentArray[0][rowNumber];
                
                // Make sure the offset is in the right direction if specified
                // and if not specified, minimize the offset size
                if (text.startsWith("-"))
                {
                    if (offset > 0.) value -= 12;
                }
                else if (text.startsWith("+"))
                {
                    if (offset < 0.) value += 12;
                }
                else if (offset > 6.) value -= 12;
                else if (offset < -6.) value += 12;
            }
            
            // Change value to an offset if not string column
            if (columnNumber > 1)
            {
                value -= copedentArray[0][rowNumber];
            }
        }
        if (columnNumber == 0)
        {
            fundamental = value;
        }
        else
            copedentArray.getReference(columnNumber-1).set(rowNumber, value);
        if (columnNumber == 1) resized();
    }
    
    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows() override
    {
        return numRows;
    }
    
    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        if (rowNumber % 2) g.fillAll (Colours::darkgrey.withBrightness(0.1f));
        else g.fillAll (Colours::darkgrey.withBrightness(0.1f));
    }
    
    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool /*rowIsSelected*/) override
    {
        g.setColour (getLookAndFeel().findColour (ListBox::textColourId));
        g.setFont (Font(14.));
        
        for (int c = 0; c < numColumns; ++c)
        {
            for (int r = 0; r < numRows; ++r)
            {
                g.drawText (getTextFromData(c, r, true), 2, 0, width - 4, height,
                            Justification::centredLeft, true);
            }
        }
        
        g.setColour (getLookAndFeel().findColour (ListBox::backgroundColourId));
        g.fillRect (width - 1, 0, 1, height);
    }
    
    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                        Component* existingComponentToUpdate) override
    {
        // The other columns are editable text columns, for which we use the custom Label component
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);
        
        // same as above...
        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent (*this);
        
        textLabel->setRowAndColumn (rowNumber, columnId);
        return textLabel;
    }
    
    void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override
    {
        // Display behavior setting
    }
    
    //==============================================================================
    void resized() override
    {
        Rectangle<int> area = getLocalBounds();
        
        int h = area.getHeight();
        int n = (numColumns*2)+3;
        int w = area.getWidth() / n;
        int r = area.getWidth() - (w*n) - 2;
        
        Rectangle<int> bottomArea = area.removeFromBottom(h*0.15);
        bottomArea.removeFromTop(h*0.03);
        Rectangle<int> upperBottomArea = bottomArea.removeFromTop(h*0.06);
        
        fundamentalLabel.setBounds(upperBottomArea.removeFromLeft(w*4).reduced(0.f, h*0.01f));
        fundamentalField.setBounds(bottomArea.removeFromLeft(w*4));
        
        upperBottomArea.removeFromRight(2);
        bottomArea.removeFromRight(2);
        
        exportButton.setBounds(upperBottomArea.removeFromRight(w*4));
        importButton.setBounds(bottomArea.removeFromRight(w*4));
        
        upperBottomArea.removeFromRight(w*4.5);
        bottomArea.removeFromRight(w*4.5);
        
        nameField.setBounds(upperBottomArea.removeFromRight(w*4.2));
        nameLabel.setBounds(upperBottomArea.removeFromRight(w*1.5).reduced(0.f, h*0.01f));
        upperBottomArea.removeFromRight(w*0.5);
        numberField.setBounds(upperBottomArea.removeFromRight(w*1.8));
        numberLabel.setBounds(upperBottomArea.removeFromRight(w*0.7).reduced(0.f, h*0.01f));
        
        sendOutButton.setBounds(bottomArea.removeFromRight(w*8));

        stringTable.setBounds(area.removeFromLeft(w*2+r));
        area.removeFromLeft(w);
        leftTable.setBounds(area.removeFromLeft(w*6));
        area.removeFromLeft(w);
        pedalTable.setBounds(area.removeFromLeft(w*10));
        area.removeFromLeft(w);
        rightTable.setBounds(area.removeFromLeft(w*4));
        
        int columnId = 1;
        stringTable.getHeader().setColumnWidth(columnId++, w*2+r-2);
        
        leftTable.getHeader().setColumnWidth(columnId++, w*2);
        leftTable.getHeader().setColumnWidth(columnId++, w*2-1);
        leftTable.getHeader().setColumnWidth(columnId++, w*2-1);
        
        pedalTable.getHeader().setColumnWidth(columnId++, w*2);
        pedalTable.getHeader().setColumnWidth(columnId++, w*2);
        pedalTable.getHeader().setColumnWidth(columnId++, w*2);
        pedalTable.getHeader().setColumnWidth(columnId++, w*2-1);
        pedalTable.getHeader().setColumnWidth(columnId++, w*2-1);
        
        rightTable.getHeader().setColumnWidth(columnId++, w*2-1);
        rightTable.getHeader().setColumnWidth(columnId++, w*2-1);
        
        h = stringTable.getHeight() / (numRows+1);
        r = stringTable.getHeight() - (h*(numRows+1)) - 2;
        stringTable.setHeaderHeight(h+r);
        stringTable.setRowHeight(h);
        leftTable.setHeaderHeight(h+r);
        leftTable.setRowHeight(h);
        pedalTable.setHeaderHeight(h+r);
        pedalTable.setRowHeight(h);
        rightTable.setHeaderHeight(h+r);
        rightTable.setRowHeight(h);
    }
    
    //==============================================================================
    // This is a custom Label component, which we use for the table's editable text columns.
    class EditableTextCustomComponent : public Label
    {
    public:
        EditableTextCustomComponent (CopedentTable& td)  : owner (td)
        {
            // double click to edit the label text; single click handled below
            setEditable (true, true, false);
            setJustificationType(Justification::centred);
            setColour(Label::backgroundColourId, Colours::darkgrey.withBrightness(0.1f));
        }
        
        void mouseDown (const MouseEvent& event) override
        {
            //            // single click on the label should simply select the row
            //            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);
            //
            Label::mouseDown (event);
        }
        
        void textWasEdited() override
        {
            owner.setDataFromText (columnId, row, getText());
            setText (owner.getTextFromData(columnId, row, true), dontSendNotification);
        }
        
        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getTextFromData(columnId, row, true), dontSendNotification);
        }
        
        void paint (Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            if (! dynamic_cast<LookAndFeel_V4*> (&lf))
                lf.setColour (textColourId, Colours::black);
            
            Label::paint (g);
            g.setColour(Colours::lightgrey);
            
            if (columnId > 0)
            {
                g.fillRect(0, getHeight()-1, getWidth(), 1);
                if (columnId != 1 && columnId != 4 &&
                    columnId != 9 && columnId != 11)
                    g.fillRect(getWidth()-1, 0, 1, getHeight());
            }
            
            if (TextEditor* editor = getCurrentTextEditor())
                editor->setJustification(Justification::centredLeft);
        }
        
    private:
        CopedentTable& owner;
        int row, columnId;
        Colour textColour;
    };
    
private:
    
    ESAudioProcessor& processor;
    
    static const int numColumns = CopedentColumnNil;
    static const int numRows = NUM_STRINGS;        // Number of strings
    
    StringArray columnList;
    Array<Array<float>>& copedentArray;
    float& fundamental;
    
    TableListBox stringTable;
    TableListBox leftTable;
    TableListBox pedalTable;
    TableListBox rightTable;
    
    TextButton exportButton;
    TextButton importButton;
    
    EditableTextCustomComponent fundamentalField;
    Label fundamentalLabel;
    EditableTextCustomComponent numberField;
    Label numberLabel;
    EditableTextCustomComponent nameField;
    Label nameLabel;
    TextButton sendOutButton;
    
    FileChooser exportChooser;
    FileChooser importChooser;
    
    ESLookAndFeel laf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CopedentTable)
};
