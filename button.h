class CustomButton : public juce::LookAndFeel_V4
{
public:
    CustomButton() //juce::Colours(black.brighter(0.1f))
    {
        baseColour = juce::Colours::black.brighter(0.1f);
    }

    juce::Colour baseColour;
    // 1. Рисуем фон кнопки
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        auto cornerSize = 4.0f;

        // Определяем состояние
        bool isOn = button.getToggleState();

        // Цвет металла (как на крутилках)
        //baseColour = juce::Colours::blue;
        if (shouldDrawButtonAsDown) baseColour = baseColour.darker (0.2f);
        else if (shouldDrawButtonAsHighlighted) baseColour = baseColour.brighter (0.1f);

        // Рисуем тело кнопки
        g.setColour (baseColour);
        g.fillRoundedRectangle (bounds, cornerSize);

        // Контур
        g.setColour (juce::Colours::black);
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);

        // --- ТОТ САМЫЙ ОРАНЖЕВЫЙ ИНДИКАТОР ---
        auto lightSlot = bounds.withHeight(2.0f).withSizeKeepingCentre(bounds.getWidth() * 0.8f, 2.0f).translated(0, 2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(lightSlot, 1.0f);

        if (isOn)
        {
            // Делаем полоску очень тонкой (2 пикселя) и чуть короче ширины кнопки
            auto indicatorArea = bounds.withHeight(2.0f).withSizeKeepingCentre(bounds.getWidth() * 0.8f, 2.0f).translated(0, 2.0f);

            // 1. Свечение (Glow) — пошире и прозрачнее
            g.setColour(juce::Colours::orange.withAlpha(0.3f));
            g.fillRoundedRectangle(indicatorArea.expanded(2.0f, 1.0f), 1.0f);

            // 2. Сама "лампочка" — яркая
            g.setColour(juce::Colours::orange);
            g.fillRoundedRectangle(indicatorArea, 1.0f);
        }
    }

    // 2. Рисуем текст кнопки
    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override
    {
        auto font = juce::FontOptions ("Consolas", 13.0f, juce::Font::plain);
        g.setFont (font);

        // Текст становится ярче, когда кнопка включена
        g.setColour (button.getToggleState() ? juce::Colours::white : juce::Colours::grey);

        auto yIndent = button.getToggleState() ? 2.0f : 0.0f; // Эффект нажатия
        g.drawText (button.getButtonText(), button.getLocalBounds().toFloat().translated(0.0f, yIndent), juce::Justification::centred, true);
    }
};
