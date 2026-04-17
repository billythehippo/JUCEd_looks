#pragma once
#include <JuceHeader.h>

class CustomDial : public juce::LookAndFeel_V4
{
public:
    CustomDial()
    {
        // Настраиваем глобальные цвета для всех слайдеров, использующих этот стиль
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.3f));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider& slider) override
    {
        // --- 1. ЛОГИКА СОСТОЯНИЯ И ПОДСВЕТКИ ---
        const bool isHover = slider.isMouseOverOrDragging();
        // Если навели, делаем цвет ярче, если нет — используем стандартный оранжевый
        auto activeIndicatorColour = isHover ? indicatorColour.brighter(0.5f) : indicatorColour;

        // Фикс размытия
        auto centreX = (float)x + (float)width  * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;

        auto bounds = juce::Rectangle<int>(x, y, width, height).reduced(5);
        auto radius = (float)juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        radius = std::floor(radius);

        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // --- 2. РИСУЕМ НАСЕЧКИ (TICKS) ---
        if (showTicks) // Используем твой флаг
        {
            for (int i = 0; i < numTicks; ++i)
            {
                auto angle = rotaryStartAngle + i * (rotaryEndAngle - rotaryStartAngle) / (numTicks - 1);

                juce::Path tick;
                float tickLen = 3.0f;
                float innerRadius = radius + radOffset;

                tick.startNewSubPath(centreX, centreY - innerRadius);
                tick.lineTo(centreX, centreY - innerRadius - tickLen);

                g.setColour(juce::Colours::white.withAlpha(0.3f));
                g.strokePath(tick, juce::PathStrokeType(1.5f), juce::AffineTransform::rotation(angle, centreX, centreY));
            }
        }

        // --- 3. ТЕЛО КРУТИЛКИ ---
        auto dialRadius = radius - radOffset;
        auto dialBounds = juce::Rectangle<float>(centreX - dialRadius, centreY - dialRadius, dialRadius * 2, dialRadius * 2);

        if (gradientEnabled)
        {
            juce::ColourGradient grad(juce::Colours::grey.darker(0.1f), centreX, dialBounds.getY(),
                                      juce::Colours::black, centreX, dialBounds.getBottom(), false);
            g.setGradientFill(grad);
        }
        else g.setColour(juce::Colours::darkgrey.darker (0.5f));

        g.fillEllipse(dialBounds);

        // Ободки
        g.setColour(juce::Colours::black);
        g.drawEllipse(dialBounds, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawEllipse(dialBounds.reduced(1.0f), 1.0f);

        // --- 4. ИНДИКАТОР ---
        juce::Path p;
        // Прямоугольник индикатора
        p.addRoundedRectangle(-1.5f, -dialRadius, 3.0f, dialRadius * 0.45f, 1.0f);

        // Рисуем мягкое свечение под индикатором при наведении
        if (isHover)
        {
            g.setColour(activeIndicatorColour.withAlpha(0.3f));
            // Немного раздуваем область света для мягкости
            g.fillPath(p, juce::AffineTransform::rotation(toAngle)
            .scaled(1.2f, 1.1f, 0.0f, -dialRadius * 0.5f)
            .translated(centreX, centreY));
        }

        // Рисуем сам индикатор
        g.setColour(activeIndicatorColour);
        g.fillPath(p, juce::AffineTransform::rotation(toAngle).translated(centreX, centreY));
    }

                          // Кастомизация текстового поля
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        if (!label.isBeingEdited())
        {
            g.fillAll(label.findColour(juce::Label::backgroundColourId));
            g.setColour(label.findColour(juce::Label::textColourId));
            g.setFont(juce::FontOptions("Consolas", 13.0f, juce::Font::plain));

            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            g.drawText(label.getText(), textArea, juce::Justification::centred, true);

            // Тонкое подчеркивание
            g.setColour(juce::Colours::orange.withAlpha(0.4f));
            g.fillRect(label.getLocalBounds().removeFromBottom(1));
        }
    }

    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds();


        if (slider.getTextBoxPosition() == juce::Slider::NoTextBox)
        {
            // Вся область слайдера отдается под саму крутилку
            layout.sliderBounds = bounds;
            layout.textBoxBounds = {}; // Пустой прямоугольник
        }
        else
        {
            // Стандартное поведение: отрезаем снизу 20-30 пикселей под текст
            int textBoxHeight = 20;
            layout.sliderBounds = bounds.withTrimmedBottom (textBoxHeight);
            layout.textBoxBounds = bounds.removeFromBottom (textBoxHeight);
        }

        return layout;
    }

    juce::Colour indicatorColour = juce::Colours::orange; // Основной цвет
    int numTicks = 11;             // Количество насечек
    float radOffset = 2.0f;        // Отступ для тела крутилки
    bool gradientEnabled = false;  // Вкл/выкл градиент
    bool showTicks = true;         // Вкл/выкл насечки
};
