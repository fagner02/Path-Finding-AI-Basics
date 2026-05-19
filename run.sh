rm main.exe
g++ -std=c++17 -pthread -I./include \
src/main.cpp \
src/structures.cpp \
src/print.cpp \
src/costs.cpp \
src/path_finding.cpp \
src/heuristics.cpp \
src/experiments.cpp \
src/rectshape.cpp \
src/point.cpp \
src/goal_test.cpp \
src/node.cpp \
src/Input.cpp \
src/TextBox.cpp \
-o main \
-lsfml-graphics \
-lsfml-window \
-lsfml-system \
-lGL
./main