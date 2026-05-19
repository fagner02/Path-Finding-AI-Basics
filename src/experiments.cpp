#include <experiments.h>

// experimento 1 - busca em largura vs Busca em profundidade vs Busca de custo
// uniforme(Dijkstra) entrada aleatória
void experiment1(std::string output_file) {
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo log.csv\n";
        return;
    }
    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";
    std::ofstream out("./input/experiment1_random_input.txt");
    out << "50\n";
    srand((unsigned)time(NULL));

    for (int i = 0; i < 50; i++) {
        point start = {rand() % 31, rand() % 31};
        point target = {rand() % 31, rand() % 31};

        for (int j = 0; j < 4; j++) {
            out << start.x << " " << start.y << "\n";
            out << target.x << " " << target.y << "\n";
            out << j << "\n";
            out << "-1\n";
            out << "0 1 2 3\n";
            out << "0\n";
            auto cost = costs[j];
            fill_blocks(blocks);
            file << dijkstra(start, target, cost, blocks, std::ref(shouldDraw),
                             j, {})
                 << "\n";
            fill_blocks(blocks);
            file << bfs(start, target, cost, blocks, std::ref(shouldDraw), j,
                        {})
                 << "\n";
            fill_blocks(blocks);
            file << dfs(start, target, cost, j, {}, blocks,
                        std::ref(shouldDraw))
                 << "\n";
        }
    }
    out.close();
    file.close();
}

// experimento 1 - busca em largura vs Busca em profundidade vs Busca de custo
// uniforme(Dijkstra) entrada do arquivo .txt
void experiment1(std::vector<point> start_points,
                 std::vector<point> target_points,
                 std::function<void(std::wstring)> toastText,
                 std::string output_file) {
    if (start_points.size() != 50 || target_points.size() != 50) {
        toastText(L"A entrada do experimento 1 deve ter 50 instâncias\n");
        return;
    }
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        toastText(L"Erro ao abrir o arquivo log.csv\n");
        return;
    }
    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    for (int i = 0; i < 50; i++) {
        point start = start_points[i];
        point target = target_points[i];

        for (int j = 0; j < 4; j++) {
            auto cost = costs[j];
            fill_blocks(blocks);
            file << dijkstra(start, target, cost, blocks, std::ref(shouldDraw),
                             j, {})
                 << "\n";
            fill_blocks(blocks);
            file << bfs(start, target, cost, blocks, std::ref(shouldDraw), j,
                        {})
                 << "\n";
            fill_blocks(blocks);
            file << dfs(start, target, cost, j, {}, blocks,
                        std::ref(shouldDraw))
                 << "\n";
        }
    }
    file.close();
}

// experimento 2 - busca de custo uniforme(Dijkstra) vs A*
// entrada aleatória
void experiment2(std::string output_file) {
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo log.csv\n";
        return;
    }
    std::ofstream out("./input/experiment2_random_input.txt");
    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    srand((unsigned)time(NULL));

    out << "50\n";
    for (int i = 0; i < 50; i++) {
        point start = {rand() % 31, rand() % 31};
        point target = {rand() % 31, rand() % 31};

        for (int j = 0; j < 4; j++) {
            out << start.x << " " << start.y << "\n";
            out << target.x << " " << target.y << "\n";
            out << j << "\n";
            out << "-1\n";
            out << "0 1 2 3\n";
            out << "0\n";
            auto cost = costs[j];
            fill_blocks(blocks);
            file << dijkstra(start, target, cost, blocks, std::ref(shouldDraw),
                             j, {})
                 << "\n";
            for (int k = 0; k < 2; k++) {
                fill_blocks(blocks);
                file << a_star(start, target, cost, heuristic_fns[k], j, k, {},
                               blocks, std::ref(shouldDraw))
                     << "\n";
            }
        }
    }
    out.close();
    file.close();
}

// experimento 2 - busca de custo uniforme(Dijkstra) vs A*
// entrada do arquivo .txt
void experiment2(std::vector<point> start_points,
                 std::vector<point> target_points,
                 std::function<void(std::wstring)> toastText,
                 std::string output_file) {
    if (start_points.size() != 50 || target_points.size() != 50) {
        toastText(L"A entrada do experimento 2 deve ter 50 instâncias\n");
        return;
    }
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        toastText(L"Erro ao abrir o arquivo log.csv\n");
        return;
    }
    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    for (int i = 0; i < 50; i++) {
        point start = start_points[i];
        point target = target_points[i];

        for (int j = 0; j < 4; j++) {
            auto cost = costs[j];
            fill_blocks(blocks);
            file << dijkstra(start, target, cost, blocks, std::ref(shouldDraw),
                             j, {})
                 << "\n";
            for (int k = 0; k < 2; k++) {
                fill_blocks(blocks);
                file << a_star(start, target, cost, heuristic_fns[k], j, k, {},
                               blocks, std::ref(shouldDraw))
                     << "\n";
            }
        }
    }
    file.close();
}

// experimento 3 - busca gulosa vs A*
// entrada aleatória
void experiment3(std::string output_file) {
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo log.csv\n";
        return;
    }

    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    srand((unsigned)time(NULL));

    std::ofstream out("./input/experiment3_random_input.txt");
    out << "50\n";

    for (int i = 0; i < 50; i++) {
        point start = {rand() % 31, rand() % 31};
        point target = {rand() % 31, rand() % 31};

        for (int j = 0; j < 4; j++) {
            auto cost = costs[j];

            for (int k = 0; k < 2; k++) {
                out << start.x << " " << start.y << "\n";
                out << target.x << " " << target.y << "\n";
                out << j << "\n";
                out << k << "\n";
                out << "0 1 2 3\n";
                out << "0\n";
                fill_blocks(blocks);
                file << greedy_search(start, target, cost, heuristic_fns[k], j,
                                      k, {}, blocks, std::ref(shouldDraw))
                     << "\n";
                fill_blocks(blocks);
                file << a_star(start, target, cost, heuristic_fns[k], j, k, {},
                               blocks, std::ref(shouldDraw))
                     << "\n";
            }
        }
    }
    out.close();
    file.close();
}

// experimento 3 - busca gulosa vs A*
// entrada do arquivo .txt
void experiment3(std::vector<point> start_points,
                 std::vector<point> target_points,
                 std::function<void(std::wstring)> toastText,
                 std::string output_file) {
    if (start_points.size() != 50 || target_points.size() != 50) {
        toastText(L"A entrada do experimento 3 deve ter 50 instâncias\n");
        return;
    }
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        toastText(L"Erro ao abrir o arquivo log.csv\n");
        return;
    }

    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    srand((unsigned)time(NULL));

    for (int i = 0; i < 50; i++) {
        point start = start_points[i];
        point target = start_points[i];

        for (int j = 0; j < 4; j++) {
            auto cost = costs[j];

            for (int k = 0; k < 2; k++) {
                fill_blocks(blocks);
                file << greedy_search(start, target, cost, heuristic_fns[k], j,
                                      k, {}, blocks, std::ref(shouldDraw))
                     << "\n";
                fill_blocks(blocks);
                file << a_star(start, target, cost, heuristic_fns[k], j, k, {},
                               blocks, std::ref(shouldDraw))
                     << "\n";
            }
        }
    }
    file.close();
}

// experimento 4 - busca em largura vs Busca em profundidade vs Busca em
// profundidade com randomização da vizinhança entrada do arquivo aleatória
void experiment4(std::string output_file) {
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo log.csv\n";
        return;
    }

    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    srand((unsigned)time(NULL));

    std::ofstream out("./input/experiment4_random_input.txt");
    out << "20\n";
    for (int i = 0; i < 20; i++) {
        point start = {rand() % 31, rand() % 31};
        point target = {rand() % 31, rand() % 31};

        std::vector<int> order = {0, 1, 2, 3};
        std::random_shuffle(order.begin(), order.end());
        for (int j = 0; j < 4; j++) {
            out << start.x << " " << start.y << "\n";
            out << target.x << " " << target.y << "\n";
            out << j << "\n";
            out << "-1\n";
            for (int k = 0; k < 4; k++) {
                out << order[k] << (k == 3 ? "" : " ");
            }
            out << "\n";
            out << "0\n";
            auto cost = costs[j];

            fill_blocks(blocks);
            file << bfs(start, target, cost, blocks, std::ref(shouldDraw), j,
                        {}, order)
                 << "\n";
            fill_blocks(blocks);
            file << dfs(start, target, cost, j, {}, blocks,
                        std::ref(shouldDraw), order)
                 << "\n";
        }
    }
    out.close();
    file.close();
}

// experimento 4 - busca em largura vs Busca em profundidade vs Busca em
// profundidade com randomização da vizinhança entrada do arquivo .txt
void experiment4(std::vector<point> start_points,
                 std::vector<point> target_points,
                 std::vector<std::vector<int>> orders,
                 std::function<void(std::wstring)> toastText,
                 std::string output_file) {
    if (start_points.size() != 20 || target_points.size() != 20 ||
        orders.size() != 20) {
        toastText(L"A entrada do experimento 4 deve ter 20 instâncias\n");
        return;
    }
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        toastText(L"Erro ao abrir o arquivo log.csv\n");
        return;
    }

    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    for (int i = 0; i < 20; i++) {
        point start = start_points[i];
        point target = target_points[i];

        std::vector<int> order = orders[i];
        for (int j = 0; j < 4; j++) {
            auto cost = costs[j];

            fill_blocks(blocks);
            file << bfs(start, target, cost, blocks, std::ref(shouldDraw), j,
                        {}, order)
                 << "\n";
            fill_blocks(blocks);
            file << dfs(start, target, cost, j, {}, blocks,
                        std::ref(shouldDraw), order)
                 << "\n";
        }
    }
    file.close();
}

// experimento 5 - caminho mínimo com uma parada a mais
// entrada do arquivo aleatória
void experiment5(std::string output_file) {
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo log.csv\n";
        return;
    }

    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    srand((unsigned)time(NULL));

    std::ofstream out("./input/experiment5_random_input.txt");
    out << "25\n";

    for (int i = 0; i < 25; i++) {
        point start = {rand() % 31, rand() % 31};
        point target = {rand() % 31, rand() % 31};

        std::vector<int> order = {0, 1, 2, 3};

        std::set<point> constraints = {};
        while (constraints.size() < 4) {
            constraints.insert({rand() % 31, rand() % 31});
        }

        for (int j = 0; j < 4; j++) {

            auto cost = costs[j];
            for (int k = 0; k < 2; k++) {
                out << start.x << " " << start.y << "\n";
                out << target.x << " " << target.y << "\n";
                out << j << "\n";
                out << k << "\n";
                for (int l = 0; l < 4; l++) {
                    out << order[l] << (l == 3 ? "" : " ");
                }
                out << "\n";
                out << constraints.size() << "\n";
                for (auto &c : constraints) {
                    out << " " << c.x << " " << c.y << "\n";
                }
                fill_blocks(blocks);
                file << a_star(start, target, cost, heuristic_fns[k], j, k,
                               constraints, blocks, std::ref(shouldDraw))
                     << "\n";
            }
        }
    }
    out.close();
    file.close();
}

// experimento 5 - caminho mínimo com uma parada a mais
// entrada do arquivo .txt
void experiment5(std::vector<point> start_points,
                 std::vector<point> target_points,
                 std::vector<std::vector<int>> orders,
                 std::vector<std::set<point>> constraints,
                 std::function<void(std::wstring)> toastText,
                 std::string output_file) {
    if (start_points.size() != 25 || target_points.size() != 25 ||
        orders.size() != 25 || constraints.size() != 25) {
        toastText(L"A entrada do experimento 5 deve ter 25 instâncias\n");
        return;
    }
    block blocks[space_size][space_size];
    bool shouldDraw = false;
    fill_blocks(blocks);
    std::ofstream file(output_file);
    if (!file.is_open()) {
        toastText(L"Erro ao abrir o arquivo log.csv\n");
        return;
    }

    file << "algorithm,visited_qty,generated_qty,path_size,path,start,target,"
            "cost_id,heuristic_id,cost,order,pharmacies\n";

    for (int i = 0; i < 25; i++) {
        point start = start_points[i];
        point target = target_points[i];

        std::vector<int> order = orders[i];
        std::set<point> constraint = constraints[i];

        for (int j = 0; j < 4; j++) {
            auto cost = costs[j];
            for (int k = 0; k < 2; k++) {
                fill_blocks(blocks);
                file << a_star(start, target, cost, heuristic_fns[k], j, k,
                               constraint, blocks, std::ref(shouldDraw))
                     << "\n";
            }
        }
    }
    file.close();
}
