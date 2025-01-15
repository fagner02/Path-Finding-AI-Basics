#include <constants.h>
#include <costs.h>
#include <print.h>
#include <rectshape.h>
#include <structures.h>
#include <heuristics.h>
#include <path_finding.h>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <stack>
#include <thread>
#include <tuple>
#include <vector>
#include <fstream>
#include <experiments.h>
#include <json/json.h>
#include <GL/gl.h>

void draw(
    sf::Vector2u& size,
    sf::RenderWindow& window,
    float& containerSize,
    float& blockSize,
    block blocks[space_size][space_size],
    sf::Vector2f scale,
    sf::Vector2f translate
) {
    window.clear(sf::Color(225, 225, 225));

    for (int y = 0; y < space_size; y++) {
        for (int x = 0; x < space_size; x++) {
            int yindex = space_size - y - 1;
            point pos = {
                (x * 50.0 + x * 10.0 + size.x / 2.0 - containerSize / 2.0 + translate.x) * scale.x,
               (yindex * 50.0 + yindex * 10.0 + size.y / 2.0 - containerSize / 2.0 + translate.y) * scale.y
            };
            blocks[x][y].shape.setScale(scale);
            blocks[x][y].text.setScale(scale);
            blocks[x][y].shape.setPosition(pos.x, pos.y);
            auto t = blocks[x][y].shape.getLocalBounds();
            window.draw(blocks[x][y].shape);

            std::stringstream ss;
            ss << std::fixed << std::setprecision(0) << blocks[x][y].info.cost;
            blocks[x][y].text.setString(ss.str());

            auto textSize = blocks[x][y].text.getLocalBounds();

            blocks[x][y].text.setPosition(
                (pos.x + (t.width * scale.x) / 2.0 - (textSize.width * scale.x) / 2.0),
                (pos.y + (t.height * scale.y) / 2.0 - (textSize.height * scale.y) / 2.0)
            );

            window.draw(blocks[x][y].text);
        }
    }
}

auto create_label(
    sf::Font& font,
    sf::Vector2f& pos,
    float pad,
    std::string text
) {

    label _label = {
        sf::Text(),
        sf::RoundedRectangleShape()
    };
    _label.text = (sf::Text());
    _label.text.setFont(font);
    _label.text.setString(text);
    _label.text.setCharacterSize(18);
    auto textSize = _label.text.getGlobalBounds();
    textSize.height *= 1;

    auto boxSize = sf::Vector2f(textSize.width + pad * 2, textSize.height - 5 + pad * 2);

    _label.box = sf::RoundedRectangleShape(boxSize, 10, 20);
    _label.box.setFillColor(sf::Color(100, 100, 100));
    _label.box.setPosition(pos);
    _label.box.setOutlineColor(sf::Color(255, 255, 255));
    _label.box.setOutlineThickness(2);

    _label.text.setPosition(sf::Vector2f(pos.x + boxSize.x / 2 - textSize.width / 2, pos.y + boxSize.y / 2.0 - textSize.height + 5));
    pos.y += boxSize.y + 5;
    return _label;
}

void draw_tree(node root) {
    // sf::CircleShape circle(10);
    // circle.setFillColor(sf::Color(100, 100, 100));
    // circle.setPosition(100, 100);
    // window.draw(circle);
    // for (int i = 0; i < root.children.size(); i++) {
    //     draw_tree(root.children[i]);
    // }
}

auto create_textbox(
    sf::Font& font,
    sf::Vector2f& pos,
    float pad,
    sf::Vector2f size
) {

    label _label = {
        sf::Text(),
        sf::RoundedRectangleShape()
    };
    _label.text = (sf::Text());
    _label.text.setFont(font);
    _label.text.setCharacterSize(18);
    auto textSize = _label.text.getGlobalBounds();
    textSize.height *= 1;

    auto boxSize = sf::Vector2f(textSize.width + pad * 2, textSize.height - 5 + pad * 2);

    _label.box = sf::RoundedRectangleShape(size, 10, 20);
    _label.box.setFillColor(sf::Color(100, 100, 100));
    _label.box.setPosition(pos);
    _label.box.setOutlineColor(sf::Color(255, 255, 255));
    _label.box.setOutlineThickness(2);

    _label.text.setPosition(sf::Vector2f(pos.x + pad - textSize.width / 2, pos.y + pad - textSize.height + 5));
    pos.y += boxSize.y + 5;
    return _label;
}

struct text_input {
    label _label;
    std::string value;
    int cursor = 0;
    sf::RectangleShape cursorLine = sf::RectangleShape(sf::Vector2f(1, 20));
    bool hasFocus = false;
};


int main() {
    std::cout << "Hello, World!\n";

    sf::ContextSettings settings;

    settings.antialiasingLevel = 5;
    sf::RenderWindow window(sf::VideoMode({ 1200, 1000 }), "My window", sf::Style::Default, settings);

    window.setPosition(sf::Vector2i(0, 0));
    auto font = sf::Font();
    if (!font.loadFromFile("./assets/Roboto-Regular.ttf")) {
        std::cout << "Error loading font\n";
    } else {
        std::cout << "Font loaded\n";
    }

    sf::Vector2u size = window.getSize();
    float blockSize = 50;
    float pad = 10;
    float containerSize = (pad * space_size + blockSize * space_size);

    bool shouldDraw = true;

    block blocks[space_size][space_size];

    for (int i = 0; i < space_size; i++) {
        for (int j = 0; j < space_size; j++) {
            blocks[i][j] = block{
                sf::RoundedRectangleShape(sf::Vector2f(50, 50), 10, 20),
                sf::Text(),
                visited_info{-1, -1, -1}
            };
            blocks[i][j].shape.setFillColor(sf::Color(100, 100, 100));
            blocks[i][j].text.setFont(font);
            blocks[i][j].text.setFillColor(sf::Color(255, 255, 255));
            blocks[i][j].text.setCharacterSize(18);
        }
    }

    // Json::Value root;
    // std::ifstream file("./e.json");
    // file >> root;

    // std::cout << "initial: [\n";
    // for (int i = 0; i < root["initial"].size(); i++) {
    //     std::cout << "[" << root["initial"][i][0] << ", " << root["initial"][i][1] << "],\n";
    // }
    // std::cout << "]\n";

    // return 0;


    // experiment1();
    // experiment2();
    // experiment3();
    // experiment4();
    // experiment5();

    bool focused = false;

    sf::Cursor textCursor;
    textCursor.loadFromSystem(sf::Cursor::Text);
    sf::Cursor arrowCursor;
    arrowCursor.loadFromSystem(sf::Cursor::Arrow);

    std::vector<label> texts;
    sf::Vector2f pos(10, 10);
    texts.push_back(create_label(font, pos, pad, "Dijkstra"));
    texts.push_back(create_label(font, pos, pad, "Bfs"));
    texts.push_back(create_label(font, pos, pad, "Dfs"));
    texts.push_back(create_label(font, pos, pad, "Greedy"));
    texts.push_back(create_label(font, pos, pad, "A*"));

    std::vector<text_input> inputs = {
        {create_textbox(font, pos, pad, { 200, 50 }), "a\nb"}
    };

    point start = { 0, 0 };
    point target = { 0, 30 };
    std::vector<button> buttons = {
        {&texts[0], [&]() {
            auto a1 = std::thread([&]() {
                fill_blocks(blocks);
                std::cout << dijkstra(start, target, cost_all10, blocks, std::ref(shouldDraw), 0, true) << "\n";
            });
            a1.detach();
         }},
        {&texts[1],[&]() {
            auto a1 = std::thread([&]() {
                fill_blocks(blocks);
                std::cout << bfs(start, target, cost_all10, blocks, std::ref(shouldDraw), 0, {0,1,2,3}, true) << "\n";
            });
            a1.detach();
        } },
        {&texts[2],[&]() {
            auto a1 = std::thread([&]() {
                fill_blocks(blocks);
                std::cout << dfs(start, target, cost_all10, blocks, std::ref(shouldDraw), 0, { 0,1,2,3 }, true) << "\n";
            });
            a1.detach();
        }},
        {&texts[3],[&]() {
            auto a1 = std::thread([&]() {
                fill_blocks(blocks);
                std::cout << greedy_search(start, target, cost_all10, heuristic1, blocks, std::ref(shouldDraw), 0, 1, true) << "\n";
            });
            a1.detach();
        }},
        {&texts[4],[&]() {
            auto a1 = std::thread([&]() {
                fill_blocks(blocks);
                std::cout << a_star(start, target, cost_all10, heuristic1, blocks, std::ref(shouldDraw), 0, 1, true) << "\n";
            });
            a1.detach();
        }}
    };
    sf::Vector2f scale = { 0.5,0.5 };
    sf::Vector2f translate = { size.x * scale.x,size.y * scale.y };

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                std::terminate();
            }
            if (event.type == sf::Event::Resized) {
                size = window.getSize();
                sf::View view(sf::FloatRect(0, 0, size.x, size.y));
                window.setView(view);
                translate = { size.x * scale.x, size.y * scale.y };
            }
            if (event.type == sf::Event::TextEntered) {
                for (int i = 0; i < inputs.size(); i++) {
                    if (inputs[i].hasFocus) {
                        if (event.text.unicode == 13) {
                            inputs[i].value.insert(inputs[i].value.begin() + inputs[i].cursor, '\n');
                            inputs[i].cursor++;
                        } else if (event.text.unicode == 8) {
                            if (inputs[i].cursor > 0) {
                                inputs[i].value.erase(inputs[i].value.begin() + inputs[i].cursor - 1);
                                inputs[i].cursor--;
                            }
                        } else {
                            inputs[i].value.insert(inputs[i].value.begin() + inputs[i].cursor, static_cast<char>(event.text.unicode));
                            inputs[i].cursor++;
                        }
                        inputs[i]._label.text.setString(inputs[i].value);
                    }
                }
            }
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                for (int i = 0; i < buttons.size(); i++) {
                    if (buttons[i]._label->box.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        buttons[i]._label->box.setFillColor(sf::Color(150, 150, 150));
                    } else {
                        buttons[i]._label->box.setFillColor(sf::Color(100, 100, 100));
                    }
                }
                for (int i = 0; i < inputs.size(); i++) {
                    if (inputs[i]._label.box.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        window.setMouseCursor(textCursor);
                    } else {
                        window.setMouseCursor(arrowCursor);
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (int i = 0; i < buttons.size(); i++) {
                        if (buttons[i]._label->box.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            buttons[i].pressed = true;
                            buttons[i]._label->box.setFillColor(sf::Color(200, 200, 200));
                        }
                    }
                    for (int i = 0; i < inputs.size(); i++) {
                        if (inputs[i]._label.box.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            // inputs[i]._label->box.setFillColor(sf::Color(130, 130, 130));
                            inputs[i].hasFocus = true;
                            focused = true;
                            for (int j = 0; j < inputs[i].value.size(); j++) {
                                auto pos = inputs[i]._label.text.findCharacterPos(j);
                                if (pos.x < mousePos.x) {
                                    inputs[i].cursor = j + 1;
                                }
                            }
                        } else {
                            inputs[i]._label.box.setFillColor(sf::Color(100, 100, 100));
                            inputs[i].hasFocus = false;
                            focused = false;
                        }
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (int i = 0; i < buttons.size(); i++) {
                        if (buttons[i].pressed && buttons[i]._label->box.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                            buttons[i]._label->box.setFillColor(sf::Color(100, 100, 100));
                            buttons[i].fn();
                            buttons[i].pressed = false;
                        } else {
                            buttons[i]._label->box.setFillColor(sf::Color(100, 100, 100));
                            buttons[i].pressed = false;
                        }
                    }
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (focused) {
                    if (event.key.code == sf::Keyboard::Left) {
                        for (int i = 0; i < inputs.size(); i++) {
                            if (inputs[i].hasFocus) {
                                if (inputs[i].cursor > 0) {
                                    inputs[i].cursor--;
                                }
                            }
                        }
                    }
                    if (event.key.code == sf::Keyboard::Right) {
                        for (int i = 0; i < inputs.size(); i++) {
                            if (inputs[i].hasFocus) {
                                if (inputs[i].cursor < inputs[i].value.size()) {
                                    inputs[i].cursor++;
                                }
                            }
                        }
                    }
                } else {
                    if (event.key.code == sf::Keyboard::Equal) {
                        scale.x += 0.01;
                        scale.y += 0.01;
                    }
                    if (event.key.code == sf::Keyboard::Dash) {
                        scale.x -= 0.01;
                        scale.y -= 0.01;
                    }
                    if (event.key.code == sf::Keyboard::Up) {
                        translate.y -= 50;
                    }
                    if (event.key.code == sf::Keyboard::Down) {
                        translate.y += 50;
                    }
                    if (event.key.code == sf::Keyboard::Left) {
                        translate.x -= 50;
                    }
                    if (event.key.code == sf::Keyboard::Right) {
                        translate.x += 50;
                    }
                }
            }

            shouldDraw = true;
        }

        if (shouldDraw) {
            draw(size, window, containerSize, blockSize, blocks, scale, translate);

            for (int i = 0; i < texts.size();i++) {
                window.draw(texts[i].box);
                window.draw(texts[i].text);
                glDisable(GL_SCISSOR_TEST);
            }
            for (int i = 0; i < inputs.size(); i++) {
                window.draw(inputs[i]._label.box);
                auto elemPos = inputs[i]._label.box.getGlobalBounds();
                glEnable(GL_SCISSOR_TEST);
                glScissor(elemPos.left + pad, size.y - elemPos.top - elemPos.height + pad, elemPos.width - pad, elemPos.height - pad);
                window.draw(inputs[i]._label.text);
                glDisable(GL_SCISSOR_TEST);
                if (inputs[i].hasFocus) {
                    auto charSize = inputs[i]._label.text.findCharacterPos(inputs[i].cursor);
                    auto b = inputs[i]._label.text.getCharacterSize();

                    auto a = inputs[i]._label.text.getGlobalBounds();
                    inputs[i].cursorLine.setPosition(charSize.x, charSize.y);
                    inputs[i]._label.box.setPosition(inputs[i]._label.text.getPosition().x, a.top - floor((a.height) / (charSize.y - a.top)) * b);
                    window.draw(inputs[i].cursorLine);
                }
            }
            window.display();
            shouldDraw = false;
        }
    }
    return 0;
}
