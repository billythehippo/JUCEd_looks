#pragma once
#include <JuceHeader.h>

class LedMeter : public juce::Component, private juce::Timer
{
public:
    LedMeter(juce::Colour col = juce::Colour(0xFF0A0A0A), uint8_t numSeg = 10, bool bal = true)
    {
        backgroundColour = col;
        numSegments = numSeg;
        useBallistics = bal;
        startTimerHz(60);
    }
    ~LedMeter() override
    {
        stopTimer();
    }

    // Настройки индикатора
    juce::Colour backgroundColour;
    bool useBallistics; // Тот самый параметр (вкл/выкл конденсатор)
    int numSegments;

    void setLevel(float v)
    {
        if (useBallistics)
        {
            if (v > targetLevel) targetLevel = v; // Хватаем пик
        }
        else
        {
            currentLevel = v; // Мгновенно
            targetLevel = v;
        }
    }

private:
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;

    void timerCallback() override
    {
        if (useBallistics)
        {
            // Плавное падение "конденсатора"
            currentLevel = (targetLevel > currentLevel) ? targetLevel : currentLevel * 0.92f;
            targetLevel *= 0.7f;
        }
        else
        {
            currentLevel = targetLevel;
        }

        // Порог отсечки, чтобы гас самый нижний сегмент
        if (currentLevel < 0.01f) currentLevel = 0.0f;

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF050505));
        g.fillRoundedRectangle(b, 2.0f);

        const float gap = 2.5f;
        const float segH = (b.getHeight() - (numSegments + 1) * gap) / numSegments;
        const float segW = b.getWidth() - gap * 2.0f;

        for (int i = 0; i < numSegments; ++i)
        {
            int idx = numSegments - 1 - i;

            juce::Colour col;
            bool isOn = false;

            // 1. КРАСНЫЙ (всегда самый верхний)
            if (idx == numSegments - 1)
            {
                col = juce::Colours::red;
                isOn = currentLevel >= 2.0f;
            }
            // 2. ВТОРОЙ РЫЖИЙ (предпоследний)
            else if (idx == numSegments - 2)
            {
                col = juce::Colours::orange;
                isOn = currentLevel > 1.4f;
            }
            // 3. ПЕРВЫЙ РЫЖИЙ (третий с конца)
            else if (idx == numSegments - 3)
            {
                col = juce::Colours::orange;
                isOn = currentLevel >= 1.0f;
            }
            // 4. ЗЕЛЕНЫЕ (все остальные)
            else
            {
                col = juce::Colours::lawngreen;
                // Распределяем зеленые равномерно до 1.0
                float threshold = ((float)idx + 0.5f) / (float)(numSegments - 3);
                isOn = currentLevel > threshold;
            }

            juce::Rectangle<float> r (b.getX() + gap, b.getY() + gap + i * (segH + gap), segW, segH);

            if (isOn)
            {
                // Bloom
                juce::Point<float> center = r.getCentre();
                juce::ColourGradient bloom (col.withAlpha(0.4f), center.x, center.y,
                                            col.withAlpha(0.0f), center.x + r.getWidth(), center.y, true);
                g.setGradientFill(bloom);
                g.fillRoundedRectangle(r.expanded(2.0f), 1.0f);

                g.setColour(col.brighter(0.2f));
                g.fillRoundedRectangle(r, 1.0f);

                g.setColour(juce::Colours::white.withAlpha(0.4f));
                g.fillRect(r.getX(), r.getY(), r.getWidth(), 1.0f);
            }
            else
            {
                g.setColour(col.withAlpha(0.08f));
                g.fillRoundedRectangle(r, 1.0f);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedMeter)
};
