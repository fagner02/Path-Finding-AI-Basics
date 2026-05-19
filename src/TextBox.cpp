#include <Input.h>
#include <TextBox.h>

TextBox::TextBox(createLabelArgs args, bool isMultiline, std::wstring tooltip)
    : isMultiline(isMultiline) {
    label = new Label(args);
    padding = args.pad;
    cursorLine = sf::RectangleShape(sf::Vector2f(2.0f, 20.0f));
    scrollThumb = sf::RoundedRectangleShape(sf::Vector2f(10.0f, 10.0f), 5, 20);
    value = args.text;
    this->tooltip = tooltip;

    if (value.size() == 0) {
        label->text.setFillColor(sf::Color(150, 150, 150));
        label->setText(tooltip);
    } else {
        label->setText(value);
    }

    label->setSize(label->text.getGlobalBounds().getSize() +
                   sf::Vector2f(padding * 2, padding * 2));
    setThumbValues(true);
}

void TextBox::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    if (!visible)
        return;
    target.draw(label->box, states);

    auto textBox = label->text.getGlobalBounds();

    auto containerBox = label->box.getGlobalBounds();
    containerBox.height -= padding * 2.0f;
    containerBox.width -= padding * 2.0f;
    float sub = getScrollSub();

    glEnable(GL_SCISSOR_TEST);
    glScissor(containerBox.left + padding,
              inputs->size.y - containerBox.top - containerBox.height - padding,
              containerBox.width, containerBox.height);

    if (inputs->selecting && hasFocus) {
        if (inputs->selectedChars.y != -1) {
            for (int k = inputs->selectedChars.x; k < inputs->selectedChars.y;
                 k++) {
                drawCharSelection(target, states, k);
            }
            for (int k = inputs->selectedChars.x - 1;
                 k > inputs->selectedChars.y - 2; k--) {
                drawCharSelection(target, states, k);
            }
        }
    }

    target.draw(label->text, states);

    if (hasFocus) {
        target.draw(cursorLine, states);
    }
    glDisable(GL_SCISSOR_TEST);
    target.draw(scrollThumb, states);
}
sf::FloatRect TextBox::getBounds() { return label->getBounds(); }
sf::Vector2f TextBox::calculateSize() const { return label->calculateSize(); }
void TextBox::setPosition(sf::Vector2f position) {
    label->setPosition(position);
    setThumbValues();
}
void TextBox::setSize(sf::Vector2f size) {
    label->setSize(size);
    setThumbValues();
}
void TextBox::setText(std::wstring text) {
    if (text.size() == 0) {
        label->setText(tooltip);
        label->text.setFillColor(sf::Color(150, 150, 150));
    } else {
        label->setText(text);
        label->text.setFillColor(sf::Color(255, 255, 255));
    }
}
void TextBox::drawCharSelection(sf::RenderTarget &target,
                                sf::RenderStates states, int k) const {
    auto pos = label->text.findCharacterPos(k);
    auto glyph = label->text.getFont()->getGlyph(
        value[k], label->text.getCharacterSize(), false);
    sf::RectangleShape rect(sf::Vector2f(glyph.bounds.width + 2, 20));
    rect.setFillColor(sf::Color(100, 100, 200));
    rect.setPosition(pos.x, pos.y);
    target.draw(rect, states);
}

float TextBox::getScrollSub() const {
    auto scrollThumbBox = scrollThumb.getGlobalBounds();
    auto containerBox = label->box.getGlobalBounds();
    auto textBox = label->text.getGlobalBounds();
    float diff =
        (containerBox.height - (2.0f * padding) - scrollThumbBox.height);
    if (diff == 0) {
        return 0;
    }

    float offset = textBox.height - (containerBox.height - 3.0f * padding);
    if (offset < 0)
        offset = 0;
    float sub =
        offset * (scrollThumbBox.top - containerBox.top - padding) / diff;

    return sub;
}

void TextBox::setThumbPos(float ypos) {
    float lower = label->box.getGlobalBounds().top + label->padding;
    float upper = label->box.getGlobalBounds().top +
                  label->box.getGlobalBounds().height - padding -
                  scrollThumb.getGlobalBounds().height;

    if (ypos < lower) {
        ypos = lower;
    }
    if (ypos > upper) {
        ypos = upper;
    }
    scrollThumb.setPosition(scrollThumb.getPosition().x, ypos);
}

void TextBox::deleteSelected(sf::Vector2i selectedChars) {
    if (selectedChars.y > selectedChars.x) {
        value.erase(value.begin() + selectedChars.x,
                    value.begin() + selectedChars.y);
        cursor = selectedChars.x;
    } else {
        value.erase(value.begin() + selectedChars.y - 1,
                    value.begin() + selectedChars.x);
        cursor = selectedChars.y - 1;
        if (selectedChars.y - 1 > value.size()) {
            cursor = value.size();
        }
    }
}

void TextBox::setThumbValues(bool set_pos) {
    if (parent != nullptr && !parentUpdated) {
        parent->childUpdated = true;
        parent->update();
    }
    parentUpdated = false;

    auto containerBox = label->box.getGlobalBounds();
    auto textBox = label->text.getGlobalBounds();
    containerBox.width -= padding * 2.0f;
    containerBox.height -= padding * 2.0f;
    float ratio = (textBox.height + 5.0f) / containerBox.height;
    if (ratio < 1) {
        ratio = 1;
    }
    float height = containerBox.height / ratio;
    if (std::isnan(height) || height < 10.0f)
        height = 10.0f;

    scrollThumb.setSize(sf::Vector2f(10.0f, height));
    auto scrollThumbBox = scrollThumb.getLocalBounds();
    scrollThumb.setPosition(containerBox.left + containerBox.width -
                                scrollThumbBox.width + label->padding,
                            containerBox.top + label->padding);
    if (set_pos) {
        scrollThumb.setPosition(containerBox.left + containerBox.width -
                                    scrollThumbBox.width + label->padding,
                                containerBox.top + padding);
    }
}

std::wstring TextBox::getSelect(sf::Vector2i selectedChars) {
    if (selectedChars.x > selectedChars.y) {
        return value.substr(selectedChars.y - 1,
                            selectedChars.x - selectedChars.y + 1);
    } else {
        return value.substr(selectedChars.x, selectedChars.y - selectedChars.x);
    }
}
