class CustomFader : public juce::LookAndFeel_V4
{
public:
    CustomFader(juce::Colour mainColour = juce::Colours::orange)
    : indicatorColour(mainColour)
    {
        // setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.3f));
        // setColour(juce::Label::textColourId, juce::Colours::white);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        bool isVertical = style == juce::Slider::LinearVertical;
        auto bounds = juce::Rectangle<float>(x, y, width, height);

        const bool isHover = slider.isMouseOverOrDragging();
        auto activeColour = isHover ? indicatorColour.brighter(0.5f) : indicatorColour;

        // --- 1. КАНАВКА (TRACK) ---
        auto trackWidth = isVertical ? 4.0f : bounds.getHeight() * 0.1f;
        auto trackRect = bounds.withSizeKeepingCentre(isVertical ? trackWidth : bounds.getWidth(),
                                                      isVertical ? bounds.getHeight() : trackWidth);
        // --- 1.1 РИСУЕМ НАСЕЧКИ (TICKS) ---
        if (showTicks)
        {
            g.setColour(juce::Colours::white.withAlpha(0.2f));

            // 1. Берем те же границы, что мы задавали в getSliderLayout
            // Если текстовое поле внизу, JUCE уже вычел его из height.
            // Нам нужно рисовать строго между реальными точками остановки ручки.

            if (showTicks)
            {
                g.setColour (juce::Colours::white.withAlpha (0.2f));
                //  Считаем шаг между насечками
                float step = (float)height / (float)(numTicks - 1);

                float centerX = (float)x + (float)width * 0.5f;
                float gap = (float)width * 0.3f; // Динамический зазор от центра
                float len = 5.0f;

                for (int i = 0; i < numTicks; ++i)
                {
                    //  Позиция каждой черточки
                    float tickY = (float)kheigth / 2 + (step * (float)i);

                    // Рисуем (центрируем по X)
                    g.drawLine (centerX - gap - len, tickY, centerX - gap, tickY, 1.0f);
                    g.drawLine (centerX + gap, tickY, centerX + gap + len, tickY, 1.0f);
                }
            }
        }

        // Внутренняя тень канавки
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(trackRect, 2.0f);

        // Металлический отблеск по краям
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawRoundedRectangle(trackRect.expanded(0.5f), 2.0f, 1.0f);

        // --- 2. РУЧКА (THUMB / CAP) ---
        float thumbW = isVertical ? kwidth * 0.7f : kheigth;
        float thumbH = isVertical ? kheigth : bounds.getHeight() * 0.7f;

        auto thumbRect = juce::Rectangle<float>(thumbW, thumbH)
        .withCentre(isVertical ? juce::Point<float>(trackRect.getCentreX(), sliderPos)
        : juce::Point<float>(sliderPos, trackRect.getCentreY()));

        // Тело ручки: сложный градиент (шлифованный алюминий)
        if (gradientEnabled)
        {
            juce::ColourGradient grad(juce::Colours::grey.brighter(0.2f), thumbRect.getX(), thumbRect.getY(),
                                      juce::Colours::black.brighter(0.1f),
                                      isVertical ? thumbRect.getX() : thumbRect.getRight(),
                                      isVertical ? thumbRect.getBottom() : thumbRect.getY(), false);
            g.setGradientFill(grad);
        }
        else g.setColour (juce::Colours::darkgrey.darker (0.5f));

        g.fillRoundedRectangle(thumbRect, 3.0f);

        // --- 3. ДЕТАЛИЗАЦИЯ (БОГАТСТВО) ---
        // Центральная световая полоса
        g.setColour(indicatorColour);
        auto indicatorLine = isVertical ? thumbRect.withSizeKeepingCentre(thumbW, 2.0f)
        : thumbRect.withSizeKeepingCentre(2.0f, thumbH);
        g.fillRect(indicatorLine);

        if (isHover)
        {
            g.setColour (activeColour.withAlpha (0.4f));
            g.fillRoundedRectangle (indicatorLine.expanded (2.0f), 1.0f);
        }

        g.setColour (activeColour);
        g.fillRect (indicatorLine);

        // Тонкие насечки на ручке для "хвата"
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        for (float i = -2; i <= 2; i += 2) {
            if (isVertical)
                g.drawHorizontalLine(thumbRect.getCentreY() + i * 4, thumbRect.getX() + 2, thumbRect.getRight() - 2);
            else
                g.drawVerticalLine(thumbRect.getCentreX() + i * 4, thumbRect.getY() + 2, thumbRect.getBottom() - 2);
        }

        // Финальный контур
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.drawRoundedRectangle(thumbRect, 3.0f, 1.0f);
    }

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

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        auto layout = LookAndFeel_V4::getSliderLayout(slider);
        auto bounds = slider.getLocalBounds();

        if (slider.getTextBoxPosition() != juce::Slider::NoTextBox)
        {
            int textBoxHeight = 20; // Высота окошка с цифрами

            // Для вертикального фейдера: текст снизу, фейдер сверху
            layout.textBoxBounds = bounds.removeFromBottom(textBoxHeight);

            // ОЧЕНЬ ВАЖНО: уменьшаем зону хода фейдера (sliderBounds),
            // чтобы ручка в крайнем нижнем положении не наезжала на текст
            layout.sliderBounds = bounds.reduced(0, 15);
        }
        else
        {
            layout.textBoxBounds = {};
            layout.sliderBounds = bounds.reduced(0, 15); // Запас под половину ручки
        }

        return layout;
    }

    float kheigth = 30.0f;
    float kwidth = 25.0f; //bounds.getWidth()
    bool gradientEnabled = false;
    bool showTicks = true;
    int numTicks = 20;
    juce::Colour indicatorColour = juce::Colours::orange;

};
