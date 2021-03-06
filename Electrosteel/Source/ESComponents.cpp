/*
  ==============================================================================

    ESComponents.cpp
    Created: 19 Feb 2021 12:42:05pm
    Author:  Matthew Wang

  ==============================================================================
*/

#include "ESComponents.h"

ESButton::ESButton (const String& t, Colour n, Colour o, Colour d)
: Button (t),
normalColour   (n), overColour   (o), downColour   (d),
normalColourOn (n), overColourOn (o), downColourOn (d),
useOnColours(false),
maintainShapeProportions (false),
outlineWidth (0.0f)
{
}

ESButton::~ESButton() {}

void ESButton::setColours (Colour newNormalColour, Colour newOverColour, Colour newDownColour)
{
    normalColour = newNormalColour;
    overColour   = newOverColour;
    downColour   = newDownColour;
}

void ESButton::setOnColours (Colour newNormalColourOn, Colour newOverColourOn, Colour newDownColourOn)
{
    normalColourOn = newNormalColourOn;
    overColourOn   = newOverColourOn;
    downColourOn   = newDownColourOn;
}

void ESButton::shouldUseOnColours (bool shouldUse)
{
    useOnColours = shouldUse;
}

void ESButton::setOutline (Colour newOutlineColour, const float newOutlineWidth)
{
    outlineColour = newOutlineColour;
    outlineWidth = newOutlineWidth;
}

void ESButton::setBorderSize (BorderSize<int> newBorder)
{
    border = newBorder;
}

void ESButton::setBounds (float x, float y, float w, float h)
{
    Rectangle<float> bounds (x, y, w, h);
    Component::setBounds(bounds.toNearestInt());
}

void ESButton::setBounds (Rectangle<float> newBounds)
{
    Component::setBounds(newBounds.toNearestInt());
}

void ESButton::setShape (const Path& newShape,
                            const bool resizeNowToFitThisShape,
                            const bool maintainShapeProportions_,
                            const bool hasShadow)
{
    shape = newShape;
    maintainShapeProportions = maintainShapeProportions_;
    
    shadow.setShadowProperties (DropShadow (Colours::black.withAlpha (0.5f), 3, Point<int>()));
    setComponentEffect (hasShadow ? &shadow : nullptr);
    
    if (resizeNowToFitThisShape)
    {
        auto newBounds = shape.getBounds();
        
        if (hasShadow)
            newBounds = newBounds.expanded (4.0f);
        
        shape.applyTransform (AffineTransform::translation (-newBounds.getX(),
                                                            -newBounds.getY()));
        
        setSize (1 + (int) (newBounds.getWidth()  + outlineWidth) + border.getLeftAndRight(),
                 1 + (int) (newBounds.getHeight() + outlineWidth) + border.getTopAndBottom());
    }
    
    repaint();
}

void ESButton::paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    g.setGradientFill(ColourGradient(Colour(25, 25, 25), juce::Point<float>(-getX(),-getY()), Colour(10, 10, 10), juce::Point<float>(-getX(), getParentHeight()-getY()), false));
    g.fillRect(getLocalBounds());
    
    if (! isEnabled())
    {
        shouldDrawButtonAsHighlighted = false;
        shouldDrawButtonAsDown = false;
    }
    
    auto r = border.subtractedFrom (getLocalBounds())
    .toFloat()
    .reduced (outlineWidth * 0.5f);

    if (shouldDrawButtonAsDown)
    {
        const float sizeReductionWhenPressed = 0.04f;
        
        r = r.reduced (sizeReductionWhenPressed * r.getWidth(),
                       sizeReductionWhenPressed * r.getHeight());
    }
    
    auto trans = shape.getTransformToScaleToFit (r, maintainShapeProportions);
    
    if      (shouldDrawButtonAsDown)        g.setColour (getToggleState() && useOnColours ? downColourOn   : downColour);
    else if (shouldDrawButtonAsHighlighted) g.setColour (getToggleState() && useOnColours ? overColourOn   : overColour);
    else                                    g.setColour (getToggleState() && useOnColours ? normalColourOn : normalColour);
    
    g.fillPath (shape, trans);
    
    if (outlineWidth > 0.0f)
    {
        g.setColour (outlineColour);
        g.strokePath (shape, PathStrokeType (outlineWidth), trans);
    }
}



ESLight::ESLight(const String& name, Colour normalColour, Colour onColour) :
Component(name),
normalColour(normalColour),
onColour(onColour),
isOn(false),
brightness(1.0f),
lightSize(5.0f)
{
    setPaintingIsUnclipped(true);
}

ESLight::~ESLight()
{
}

void ESLight::setBounds (float x, float y, float d)
{
    Rectangle<float> newBounds (x, y, d, d);
    setBounds(newBounds);
}

void ESLight::setBounds (Rectangle<float> newBounds)
{
    lightSize = newBounds.getWidth() * 0.25f;
    Component::setBounds(newBounds.expanded(lightSize, lightSize).toNearestInt());
}

void ESLight::setState (bool state)
{
    if (state == isOn) return;
    isOn = state;
    repaint();
}

void ESLight::setBrightness (float newBrightness)
{
    if (newBrightness == brightness) return;
    brightness = newBrightness;

    repaint();
}

void ESLight::paint (Graphics &g)
{
    g.setGradientFill(ColourGradient(Colour(25, 25, 25), juce::Point<float>(-getX(),-getY()), Colour(10, 10, 10), juce::Point<float>(-getX(), getParentHeight()-getY()), false));
    g.fillRect(getLocalBounds());
    
    Rectangle<float> area = getLocalBounds().toFloat();
    Rectangle<float> innerArea = area.reduced(lightSize, lightSize);
    g.setColour(normalColour.interpolatedWith(onColour, isOn ? (brightness * 0.5f) : 0.0f));
    g.fillEllipse(innerArea);
    
    if (isOn)
    {
        float r = area.getWidth() * 0.5f * (1.0f - brightness);
        g.setGradientFill(ColourGradient(onColour, innerArea.getCentre(), onColour.withAlpha(0.0f),
                                         juce::Point<float>(area.getCentreX(), area.getY() + r),
                                         true));
        g.fillEllipse(area);
    }
}
