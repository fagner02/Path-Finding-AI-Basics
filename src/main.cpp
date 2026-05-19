
#include <DrawObject.h>
#include <GL/gl.h>
#include <Input.h>
#include <Label.h>
#include <SFML/Graphics.hpp>
#include <TextBox.h>
#include <algorithm>
#include <chrono>
#include <codecvt>
#include <constants.h>
#include <costs.h>
#include <cstring>
#include <experiments.h>
#include <fstream>
#include <future>
#include <heuristics.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <path_finding.h>
#include <print.h>
#include <rectshape.h>
#include <set>
#include <stack>
#include <structures.h>
#include <thread>
#include <tuple>
#include <vector>

void draw(sf::Vector2u &size, sf::RenderWindow &window, float &containerSize,
          float &blockSize, block blocks[space_size][space_size],
          sf::Vector2f scale, sf::Vector2f translate) {
    window.clear(sf::Color(225, 225, 225));

    for (int y = 0; y < space_size; y++) {
        for (int x = 0; x < space_size; x++) {
            int yindex = space_size - y - 1;
            point pos = {(x * 50.0 + x * 10.0 + size.x / 2.0 -
                          containerSize / 2.0 + translate.x) *
                             scale.x,
                         (yindex * 50.0 + yindex * 10.0 + size.y / 2.0 -
                          containerSize / 2.0 + translate.y) *
                             scale.y};
            blocks[x][y].shape.setScale(scale);
            blocks[x][y].text.setScale(scale);
            blocks[x][y].shape.setPosition(pos.x, pos.y);
            auto t = blocks[x][y].shape.getLocalBounds();
            window.draw(blocks[x][y].shape);

            std::stringstream ss;
            ss << std::fixed << std::setprecision(0) << blocks[x][y].info.cost;
            blocks[x][y].text.setString(ss.str());

            auto textSize = blocks[x][y].text.getLocalBounds();
            blocks[x][y].text.setOrigin(textSize.getPosition());
            blocks[x][y].text.setPosition((pos.x + (t.width * scale.x) / 2.0 -
                                           (textSize.width * scale.x) / 2.0),
                                          (pos.y + (t.height * scale.y) / 2.0 -
                                           (textSize.height * scale.y) / 2.0));

            window.draw(blocks[x][y].text);
        }
    }
}

std::string processFileName(std::string file_name, std::string default_name,
                            std::string extension) {
    if (file_name == "") {
        file_name = default_name;
    }
    if (file_name.find(extension) == std::string::npos) {
        file_name += extension;
    }
    return file_name;
}

class Row : public DrawObject {
  public:
    std::vector<DrawObject *> childrenContainer;
    float gap = 5;

    Row(std::vector<DrawObject *> children) {
        childrenContainer = children;
        updateLayout();
    }

    void draw(sf::RenderTarget &target,
              sf::RenderStates states) const override {
        if (!visible)
            return;
        for (auto &child : childrenContainer) {
            child->draw(target, states);
        }
    }

    sf::Vector2f calculateSize() const override {
        float width = 0;
        float height = 0;
        for (auto &child : childrenContainer) {
            auto size = child->calculateSize();
            width += size.x;
            if (size.y > height) {
                height = size.y;
            }
        }
        width += gap * (childrenContainer.size() - 1);
        return sf::Vector2f(width, height);
    }

    void setPosition(sf::Vector2f position) override {
        this->position = position;
        updateLayout();
    }

    void updateLayout() {
        float height = 0;
        float x = 0;
        for (auto &child : childrenContainer) {
            child->setPosition(sf::Vector2f(position.x + x, position.y));
            auto childSize = child->calculateSize();
            x += childSize.x + gap;
            if (childSize.y > height) {
                height = childSize.y;
            }
        }
        for (auto &child : childrenContainer) {
            auto childSize = child->calculateSize();
            child->setSize(sf::Vector2f(childSize.x, height));
        }
        size = calculateSize();
    }

    void addChild(DrawObject *child) {
        childrenContainer.push_back(child);
        updateLayout();
    }
};

class CenterBox : public DrawObject {
  public:
    DrawObject *child;
    CenterBox(DrawObject *child) : child(child) { updateLayout(); }
    CenterBox() { child = nullptr; }
    void updateLayout() {
        auto childSize = child->calculateSize();
        auto childPos = sf::Vector2f(position.x + (size.x - childSize.x) / 2.0,
                                     position.y + (size.y - childSize.y) / 2.0);
        child->setPosition(childPos);
    }
    void draw(sf::RenderTarget &target,
              sf::RenderStates states) const override {
        if (visible)
            child->draw(target, states);
    }
    void setSize(sf::Vector2f size) {
        this->size = size;
        updateLayout();
    }
    sf::Vector2f calculateSize() const override {
        return child->calculateSize();
    }
    void setPosition(sf::Vector2f position) override {
        this->position = position;
        updateLayout();
    }
};
class InstructionText : public DrawObject {
  public:
    sf::Text text;
    InstructionText(std::wstring text, sf::Font &font) {
        this->text.setFont(font);
        this->text.setString(text);
        this->text.setCharacterSize(20);
        this->text.setOrigin(this->text.getLocalBounds().getPosition());
        this->text.setFillColor(sf::Color(0, 0, 0));
        this->text.setOutlineColor(sf::Color(255, 255, 255));
        this->text.setOutlineThickness(3);
    }
    void draw(sf::RenderTarget &target,
              sf::RenderStates states) const override {
        if (visible)
            target.draw(text, states);
    }
    sf::Vector2f calculateSize() const override {
        return text.getGlobalBounds().getSize();
    }
    void setPosition(sf::Vector2f position) override {
        text.setPosition(position);
    }
    void setText(std::wstring text) {
        this->text.setString(text);
        parent->childUpdated = true;
        parent->update();
    }
};
class ClickableLabel : public DrawObject {
  public:
    Label *label;
    sf::Color hoverColor = sf::Color(150, 150, 150);
    sf::Color pressedColor = sf::Color(200, 200, 200);
    sf::Color defaultColor = sf::Color(100, 100, 100);
    bool pressed = false;

    ClickableLabel(Label *label, std::function<void()> onClick)
        : label(label), _onClick(onClick) {}
    ClickableLabel(createLabelArgs args, std::function<void()> onClick)
        : _onClick(onClick) {
        this->label = new Label(args);
    }

    void draw(sf::RenderTarget &target,
              sf::RenderStates states) const override {
        if (!visible)
            return;
        label->draw(target, states);
    }

    sf::Vector2f calculateSize() const override {
        return label->calculateSize();
    }

    void setPosition(sf::Vector2f position) override {
        label->setPosition(position);
    }
    void setSize(sf::Vector2f size) override { label->setSize(size); }
    sf::FloatRect getBounds() override { return label->getBounds(); }

    void onClick() {
        label->box.setFillColor(defaultColor);
        pressed = false;
        _onClick();
    }

    void hover() { label->box.setFillColor(hoverColor); }
    void unhover() { label->box.setFillColor(defaultColor); }
    void press() {
        pressed = true;
        label->box.setFillColor(pressedColor);
    }
    void release() {
        pressed = false;
        label->box.setFillColor(defaultColor);
    }

  private:
    std::function<void()> _onClick;
};

class Clickable {
  public:
    std::vector<ClickableLabel *> objs = {};
    ClickableLabel *add(ClickableLabel *obj) {
        objs.push_back(obj);
        return obj;
    }
};

class Column : public DrawObject {
  public:
    float gap;
    std::vector<DrawObject *> childrenContainer;

    Column(std::vector<DrawObject *> children, float gap) : gap(gap) {
        childrenContainer = children;
        for (auto &child : children) {
            child->parent = this;
        }
        updateLayout();
    }
    void updateLayout() {
        float y = 0;
        sf::Vector2f pos(position);
        pos.y += margin;
        pos.x += margin;
        for (auto &child : childrenContainer) {
            child->parentUpdated = true;
            child->setPosition(sf::Vector2f(pos.x, pos.y + y));
            y += child->calculateSize().y + gap;
        }
        size = calculateSize();
    }

    void update() override {
        if (childUpdated) {
            childUpdated = false;
            updateLayout();
        }
    }

    void addChild(DrawObject *child) {
        childrenContainer.push_back(child);
        updateLayout();
    }
    void addChildren(std::vector<DrawObject *> children) {
        for (auto &child : children) {
            childrenContainer.push_back(child);
        }
        updateLayout();
    }

    void draw(sf::RenderTarget &target,
              sf::RenderStates states) const override {
        if (!visible)
            return;
        for (auto &child : childrenContainer) {
            child->draw(target, states);
        }
    }

    sf::Vector2f calculateSize() const override {
        float width = 0;
        float height = 0;
        for (auto &child : childrenContainer) {
            auto size = child->calculateSize();
            if (size.x > width) {
                width = size.x;
            }
            height += size.y + gap;
        }
        return sf::Vector2f(width, height);
    }
    void setPosition(sf::Vector2f position) override {
        this->position = position;
        updateLayout();
    }
};

int main() {
    std::cout << "Hello, World!\n";

    sf::ContextSettings settings;

    settings.antialiasingLevel = 5;
    sf::RenderWindow window(sf::VideoMode({1200, 1000}), "My window",
                            sf::Style::Default, settings);

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
            blocks[i][j] =
                block{sf::RoundedRectangleShape(sf::Vector2f(50, 50), 10, 20),
                      sf::Text(), visited_info{-1, -1, -1}};
            blocks[i][j].shape.setFillColor(sf::Color(100, 100, 100));
            blocks[i][j].text.setFont(font);
            blocks[i][j].text.setFillColor(sf::Color(255, 255, 255));
            blocks[i][j].text.setCharacterSize(18);
        }
    }

    bool focused = false;
    bool loading = false;
    bool selectingExperiment = false;
    bool selectingAlgorithm = false;
    bool shouldAnimate = false;
    int selectedAlgorithm = -1;
    int selectedExperiment = -1;

    Clickable clickable;
    Input inputs = {};

    inputs.size = size;

    sf::Cursor textCursor;
    textCursor.loadFromSystem(sf::Cursor::Text);
    sf::Cursor arrowCursor;
    arrowCursor.loadFromSystem(sf::Cursor::Arrow);

    auto checkLabel =
        new Label(createLabelArgs{.font = font, .pad = 10, .text = L"animar:"});
    ClickableLabel *checkButton = clickable.add(new ClickableLabel(
        new Label(createLabelArgs{.font = font, .pad = 10, .text = L" "}),
        [&]() {
            shouldAnimate = !shouldAnimate;
            checkButton->label->setText(shouldAnimate ? L"X" : L" ");
        }));

    auto instructionText = new InstructionText(L"", font);

    std::map<int, std::wstring> algorithms = {{0, L"Dijkstra"},
                                              {1, L"Bfs"},
                                              {2, L"Dfs"},
                                              {3, L"Greedy Search"},
                                              {4, L"A*"}};
    auto setInstruction = [&]() {
        std::wstring title;
        if (selectingAlgorithm) {
            title = L"Algoritmo " + algorithms[selectedAlgorithm];
        } else {
            title = L"Experimento " + std::to_wstring(selectedExperiment + 1);
        }
        instructionText->setText(
            title +
            L"\nDigite o nome dos arquivo ao lado.\nCaso não digite nada a "
            L"predefinição é\nInput.txt e out.csv");
    };

    auto centerBox = new CenterBox();

    auto selectAlgorithm = [&](int i) {
        centerBox->visible = true;
        checkButton->visible = true;
        checkLabel->visible = true;
        selectingAlgorithm = true;
        selectingExperiment = false;
        selectedAlgorithm = i;
        setInstruction();
    };
    auto selectExperiment = [&](int i) {
        centerBox->visible = true;
        checkButton->visible = false;
        checkLabel->visible = false;
        selectingAlgorithm = false;
        selectingExperiment = true;
        selectedExperiment = i;
        setInstruction();
    };

    auto inputFile = new TextBox(createLabelArgs{font, .pad = 10}, false,
                                 L"arquivo de entrada");
    auto outputFile = new TextBox(createLabelArgs{font, .pad = 10}, false,
                                  L"arquivo de saida");

    auto buttonColumn = Column(
        std::vector<DrawObject *>{
            clickable.add(new ClickableLabel(
                createLabelArgs{.font = font, .pad = 10, .text = L"Dijkstra"},
                [&]() { selectAlgorithm(0); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{.font = font, .pad = 10, .text = L"Bfs"},
                [&]() { selectAlgorithm(1); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{.font = font, .pad = 10, .text = L"Dfs"},
                [&]() { selectAlgorithm(2); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{.font = font, .pad = 10, .text = L"Greedy"},
                [&]() { selectAlgorithm(3); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{.font = font, .pad = 10, .text = L"A*"},
                [&]() { selectAlgorithm(4); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{
                    .font = font, .pad = 10, .text = L"Experimento 1"},
                [&]() { selectExperiment(0); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{
                    .font = font, .pad = 10, .text = L"Experimento 2"},
                [&]() { selectExperiment(1); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{
                    .font = font, .pad = 10, .text = L"Experimento 3"},
                [&]() { selectExperiment(2); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{
                    .font = font, .pad = 10, .text = L"Experimento 4"},
                [&]() { selectExperiment(3); })),
            clickable.add(new ClickableLabel(
                createLabelArgs{
                    .font = font, .pad = 10, .text = L"Experimento 5"},
                [&]() { selectExperiment(4); })),
            inputs.add(inputFile),
            inputs.add(outputFile),
        },
        5);
    buttonColumn.margin = 10;
    buttonColumn.updateLayout();

    Label toastElem(createLabelArgs{.font = font,
                                    .pad = 10,
                                    .text = L"",
                                    .boxColor = sf::Color(80, 80, 80),
                                    .outlineThickness = 0});
    toastElem.visible = false;
    std::wstring toastText = L"";

    auto setToastText = [&](std::wstring text) {
        std::thread([&toastText, text, &toastElem, &shouldDraw]() {
            toastElem.visible = true;
            toastText = text;
            shouldDraw = true;
            auto color = toastElem.box.getFillColor();
            auto textColor = toastElem.text.getFillColor();
            std::this_thread::sleep_for(std::chrono::seconds(5));
            int sub = 10;
            while (color.a > 0) {
                color.a = int(color.a) - sub < 0 ? 0 : color.a - sub;
                textColor.a =
                    int(textColor.a) - sub < 0 ? 0 : textColor.a - sub;
                toastElem.box.setFillColor(color);
                toastElem.text.setFillColor(textColor);
                shouldDraw = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            toastText = L"";
            toastElem.visible = false;
            color.a = 255;
            textColor.a = 255;
            toastElem.box.setFillColor(color);
            toastElem.text.setFillColor(textColor);
        }).detach();
    };

    auto row = new Row(std::vector<DrawObject *>{checkLabel, checkButton});

    centerBox->child = (new Column(
        std::vector<DrawObject *>{
            instructionText,
            clickable.add(new ClickableLabel(
                createLabelArgs{
                    .font = font, .pad = 10, .text = L"utilizar arquivo txt"},
                [&]() {
                    centerBox->visible = false;
                    std::vector<point> start_points;
                    std::vector<point> target_points;
                    std::vector<int> cost_ids;
                    std::vector<int> heuristic_ids;
                    std::vector<std::vector<int>> orders;
                    std::vector<std::set<point>> constraints;
                    using convert_type = std::codecvt_utf8<wchar_t>;
                    std::wstring_convert<convert_type, wchar_t> converter;
                    std::string file_name =
                        processFileName(converter.to_bytes(inputFile->value),
                                        "./input/input.txt", ".txt");
                    std::ifstream file(file_name);
                    if (file.fail()) {
                        setToastText(L"Arquivo de entrada nao encontrado");
                        return;
                    }
                    int count;
                    file >> count;

                    for (int i = 0; i < count; i++) {
                        start_points.push_back(point());
                        target_points.push_back(point());
                        cost_ids.push_back(0);
                        heuristic_ids.push_back(0);
                        orders.push_back(std::vector<int>());
                        constraints.push_back(std::set<point>());

                        file >> start_points[i].x >> start_points[i].y;
                        std::cout << start_points[i].x << " "
                                  << start_points[i].y << std::endl;
                        file >> target_points[i].x >> target_points[i].y;
                        file >> cost_ids[i];
                        file >> heuristic_ids[i];
                        for (int j = 0; j < 4; j++) {
                            int k;
                            file >> k;
                            orders[i].push_back(k);
                        }
                        int goal_count;
                        file >> goal_count;
                        for (int j = 0; j < goal_count; j++) {
                            point p;
                            file >> p.x >> p.y;
                            constraints[i].insert(p);
                        }
                    }
                    file.close();

                    std::string output_file =
                        processFileName(converter.to_bytes(outputFile->value),
                                        "./output/out.csv", ".csv");
                    if (selectingAlgorithm) {
                        selectingAlgorithm = false;
                        selectingExperiment = false;

                        std::thread([start_points, target_points, count,
                                     constraints, orders, heuristic_ids,
                                     output_file, &blocks, selectedAlgorithm,
                                     cost_ids, &shouldDraw, shouldAnimate,
                                     &setToastText]() {
                            std::stringstream ss;
                            for (int i = 0; i < count; i++) {
                                fill_blocks(blocks, constraints[i],
                                            start_points[i], target_points[i]);
                                switch (selectedAlgorithm) {
                                case 0: {
                                    ss << dijkstra(
                                              start_points[i], target_points[i],
                                              costs[cost_ids[i]], blocks,
                                              std::ref(shouldDraw), 0,
                                              constraints[i], shouldAnimate)
                                       << "\n";
                                    break;
                                }
                                case 1: {
                                    ss << bfs(start_points[i], target_points[i],
                                              costs[cost_ids[i]], blocks,
                                              std::ref(shouldDraw), 0,
                                              constraints[i], orders[i],
                                              shouldAnimate)
                                       << "\n";
                                    break;
                                }
                                case 2: {
                                    ss << dfs(start_points[i], target_points[i],
                                              costs[cost_ids[i]], 0,
                                              constraints[i], blocks,
                                              std::ref(shouldDraw), orders[i],
                                              shouldAnimate)
                                       << "\n";
                                    break;
                                }
                                case 3: {
                                    ss << greedy_search(
                                              start_points[i], target_points[i],
                                              costs[cost_ids[i]],
                                              heuristic_fns[heuristic_ids[i]],
                                              cost_ids[i], heuristic_ids[i],
                                              constraints[i], blocks,
                                              std::ref(shouldDraw),
                                              shouldAnimate)
                                       << "\n";
                                    break;
                                }
                                case 4: {
                                    ss << a_star(
                                              start_points[i], target_points[i],
                                              costs[cost_ids[i]],
                                              heuristic_fns[heuristic_ids[i]],
                                              cost_ids[i], heuristic_ids[i],
                                              constraints[i], blocks,
                                              std::ref(shouldDraw),
                                              shouldAnimate)
                                       << "\n";
                                    break;
                                }
                                default:
                                    break;
                                }
                            }

                            std::ofstream out(output_file);
                            out << ss.str();
                            out.close();
                            setToastText(L"Arquivo de saída gerado");
                        }).detach();
                    }
                    if (selectingExperiment) {
                        selectingAlgorithm = false;
                        selectingExperiment = false;
                        std::thread([start_points, target_points, orders,
                                     constraints, selectedExperiment,
                                     output_file, &setToastText]() {
                            switch (selectedExperiment) {
                            case 0: {
                                experiment1(start_points, target_points,
                                            setToastText, output_file);
                                break;
                            }
                            case 1: {
                                experiment2(start_points, target_points,
                                            setToastText, output_file);
                                break;
                            }
                            case 2: {
                                experiment3(start_points, target_points,
                                            setToastText, output_file);
                                break;
                            }
                            case 3: {
                                experiment4(start_points, target_points, orders,
                                            setToastText, output_file);
                                break;
                            }
                            case 4: {
                                experiment5(start_points, target_points, orders,
                                            constraints, setToastText,
                                            output_file);
                                break;
                            }
                            default:
                                break;
                                setToastText(L"Arquivo de saída gerado");
                            }
                        }).detach();
                    }
                })),
            clickable.add(new ClickableLabel(
                createLabelArgs{.font = font,
                                .pad = 10,
                                .text = L"utilizar entrada aleatória"},
                [&]() {
                    centerBox->visible = false;
                    using convert_type = std::codecvt_utf8<wchar_t>;
                    std::wstring_convert<convert_type, wchar_t> converter;
                    std::string output_file =
                        processFileName(converter.to_bytes(outputFile->value),
                                        "./output/out.csv", ".csv");
                    if (selectingAlgorithm) {
                        selectingAlgorithm = false;
                        selectingExperiment = false;
                        std::thread([&blocks, &shouldDraw, selectedAlgorithm,
                                     output_file, shouldAnimate,
                                     &setToastText]() {
                            srand((unsigned)time(NULL));
                            point start = {rand() % 31, rand() % 31};
                            point target = {rand() % 31, rand() % 31};
                            std::set<point> constraints = {};
                            while (constraints.size() < 4) {
                                constraints.insert({rand() % 31, rand() % 31});
                            }
                            int cost_id = rand() % 4;
                            int heuristic_id = rand() % 2;
                            std::ofstream save("./input/random_input.txt");
                            save << "1\n";
                            save << start.x << " " << start.y << "\n";
                            save << target.x << " " << target.y << "\n";
                            save << cost_id << "\n";
                            save << heuristic_id << "\n";
                            save << "0 1 2 3\n";
                            save << constraints.size() << "\n";
                            for (auto &c : constraints) {
                                save << c.x << " " << c.y << "\n";
                            }
                            fill_blocks(blocks, constraints, start, target);
                            std::ofstream out(output_file);
                            switch (selectedAlgorithm) {
                            case 0: {
                                out << dijkstra(start, target, costs[cost_id],
                                                blocks, std::ref(shouldDraw),
                                                cost_id, constraints,
                                                shouldAnimate)
                                    << "\n";
                                break;
                            }
                            case 1: {
                                out << bfs(start, target, costs[cost_id],
                                           blocks, std::ref(shouldDraw),
                                           cost_id, constraints, {0, 1, 2, 3},
                                           shouldAnimate)
                                    << "\n";
                                break;
                            }
                            case 2: {
                                out << dfs(start, target, costs[cost_id],
                                           cost_id, constraints, blocks,
                                           std::ref(shouldDraw), {0, 1, 2, 3},
                                           shouldAnimate)
                                    << "\n";
                                break;
                            }
                            case 3: {
                                out << greedy_search(
                                           start, target, costs[cost_id],
                                           heuristic_fns[heuristic_id], cost_id,
                                           heuristic_id, constraints, blocks,
                                           std::ref(shouldDraw), shouldAnimate)
                                    << "\n";
                                break;
                            }
                            case 4: {
                                out << a_star(
                                           start, target, costs[cost_id],
                                           heuristic_fns[heuristic_id], cost_id,
                                           heuristic_id, constraints, blocks,
                                           std::ref(shouldDraw), shouldAnimate)
                                    << "\n";
                                break;
                            }
                            default:
                                break;
                            }
                            out.close();
                            setToastText(L"Arquivo de saída gerado");
                        }).detach();
                    }
                    if (selectingExperiment) {
                        selectingAlgorithm = false;
                        selectingExperiment = false;
                        auto a1 = std::thread([&blocks, &shouldDraw,
                                               &selectedExperiment, &loading,
                                               output_file, &setToastText]() {
                            loading = true;
                            switch (selectedExperiment) {
                            case 0: {
                                experiment1(output_file);
                                break;
                            }
                            case 1: {
                                experiment2(output_file);
                                break;
                            }
                            case 2: {
                                experiment3(output_file);
                                break;
                            }
                            case 3: {
                                experiment4(output_file);
                                break;
                            }
                            case 4: {
                                experiment5(output_file);
                                break;
                            }
                            default:
                                break;
                            }
                            loading = false;
                            setToastText(L"Arquivo de saída gerado");
                        });
                        a1.detach();
                    }
                })),
            row},
        5));

    centerBox->setSize(sf::Vector2f(size));
    centerBox->visible = false;

    point start = {0, 0};
    point target = {0, 15};
    std::set<point> constraints = {point{0, 20}, point{2, 22}};

    sf::Vector2f scale = {0.5, 0.5};
    sf::Vector2f translate = {size.x * scale.x, size.y * scale.y};
    sf::Vector2f lastMousePos = {0, 0};

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
                translate = {size.x * scale.x, size.y * scale.y};
                centerBox->setSize(sf::Vector2f(size));
                inputs.size = size;
            }
            if (event.type == sf::Event::MouseMoved && !loading) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                for (auto &button : clickable.objs) {
                    if (button->getBounds().contains(mousePos.x, mousePos.y)) {
                        button->hover();
                    } else {
                        button->unhover();
                    }
                }
                inputs.MouseMove(mousePos);
                if (inputs.isHovering) {
                    window.setMouseCursor(textCursor);
                } else {
                    window.setMouseCursor(arrowCursor);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed && !loading) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (auto &button : clickable.objs) {
                        if (button->getBounds().contains(mousePos.x,
                                                         mousePos.y)) {
                            button->press();
                        }
                    }
                    inputs.MousePressed(mousePos);
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && !loading) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    inputs.MouseReleased(mousePos);
                    if (inputs.isHovering) {
                        window.setMouseCursor(textCursor);
                    }
                    for (auto &button : clickable.objs) {
                        if (button->pressed && button->getBounds().contains(
                                                   mousePos.x, mousePos.y)) {
                            button->onClick();
                        } else {
                            button->release();
                        }
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed && !loading) {
                if (inputs.inputFocused) {
                    inputs.KeyPressed(event.key);
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
            if (event.type == sf::Event::TextEntered) {
                inputs.TextEntered(event.text.unicode);
            }
            shouldDraw = true;
        }

        if (shouldDraw) {
            draw(size, window, containerSize, blockSize, blocks, scale,
                 translate);

            if (toastElem.visible) {
                toastElem.text.setString(toastText);
                auto textSize = toastElem.text.getLocalBounds();
                toastElem.text.setOrigin(textSize.left, textSize.top);
                toastElem.box.setSize(
                    sf::Vector2f(textSize.width + pad * 2.0,
                                 textSize.height - 5.0 + pad * 2.0));
                toastElem.box.setPosition(
                    size.x / 2.0 - toastElem.box.getSize().x / 2.0, 20);
                toastElem.text.setPosition(size.x / 2.0 - textSize.width / 2.0,
                                           20 + pad);
                window.draw(toastElem.box);
                window.draw(toastElem.text);
            }

            if (loading) {
                sf::Text loadingText;
                loadingText.setFont(font);
                loadingText.setString("Executando...");
                loadingText.setCharacterSize(35);
                loadingText.setFillColor(sf::Color(100, 100, 100));
                loadingText.setOutlineColor(sf::Color(255, 255, 255));
                loadingText.setOutlineThickness(5);
                auto textSize = loadingText.getGlobalBounds();
                loadingText.setPosition(size.x / 2.0 - textSize.width / 2.0,
                                        size.y / 2.0 - textSize.height / 2.0);
                window.draw(loadingText);
            }
            window.draw(buttonColumn);
            window.draw(*centerBox);
            for (auto &button : inputs.inputs) {
                window.draw(*button);
            }
            window.display();
            shouldDraw = false;
        }
    }
    return 0;
}
