#pragma once
#include <JuceHeader.h>

class LedMeterCircle : public juce::Component, private juce::Timer
{
public:
    LedMeterCircle(juce::Colour col = juce::Colour(0xFF0A0A0A), uint8_t numSeg = 10, bool bal = true)
    {
        backgroundColour = col;
        numSegments = numSeg;
        useBallistics = bal;
        startTimerHz(60);
    }

    ~LedMeterCircle() override
    {
        stopTimer();
    }

    void setLevel(float v)
    {
        if (useBallistics)
        {
            if (v > targetLevel) targetLevel = v;
        }
        else
        {
            currentLevel = v;
            targetLevel = v;
        }
    }

    juce::Colour backgroundColour;
    bool useBallistics;
    int numSegments;

private:
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;

    void timerCallback() override
    {
        if (useBallistics)
        {
            currentLevel = (targetLevel > currentLevel) ? targetLevel : currentLevel * 0.92f;
            targetLevel *= 0.7f;
        }
        else
        {
            currentLevel = targetLevel;
        }
        if (currentLevel < 0.01f) currentLevel = 0.0f;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        // Корпус с легким закруглением
        g.setColour(backgroundColour);
        g.fillRoundedRectangle(b, 4.0f);

        // Вычисляем диаметр кружка исходя из высоты
        const float gap = 4.0f;
        const float diameter = (b.getHeight() - (numSegments + 1) * gap) / (float)numSegments;
        const float width = b.getWidth();
        if (roundf(width) < roundf(diameter)) this->setSize(diameter + 2 * gap, getHeight());
        const float centerX = b.getCentreX();

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

            // Центрируем кружок по горизонтали
            juce::Rectangle<float> r (centerX - diameter * 0.5f, gap + i * (diameter + gap), diameter, diameter);

            if (isOn)
            {
                // 1. Мягкое свечение вокруг (Bloom)
                juce::ColourGradient bloom (col.withAlpha(0.35f), r.getCentreX(), r.getCentreY(),
                                            col.withAlpha(0.0f), r.getCentreX() + diameter, r.getCentreY(), true);
                g.setGradientFill(bloom);
                g.fillEllipse(r.expanded(diameter * 0.5f));

                // 2. Тело линзы (эффект объема)
                // Смещаем центр градиента чуть вверх, чтобы имитировать блик сверху
                juce::ColourGradient lens (col.brighter(0.8f), r.getCentreX(), r.getY() + diameter * 0.2f,
                                           col.darker(0.3f), r.getCentreX(), r.getBottom(), true);
                g.setGradientFill(lens);
                g.fillEllipse(r);

                // 3. Маленький "стеклянный" блик в самом верху
                g.setColour(juce::Colours::white.withAlpha(0.4f));
                g.fillEllipse(r.getCentreX() - diameter * 0.15f, r.getY() + diameter * 0.15f, diameter * 0.3f, diameter * 0.2f);
            }
            else
            {
                // Выключенный диод - глубокая матовая ямка
                g.setColour(juce::Colours::black.withAlpha(0.6f));
                g.fillEllipse(r);
                g.setColour(col.withAlpha(0.15f));
                g.drawEllipse(r, 1.0f);
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedMeterCircle)
};
