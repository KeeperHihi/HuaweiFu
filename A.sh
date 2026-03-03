rm ./replay/*
g++ CppCode/main.cpp -o CppCode/main.exe
# g++ -D_GLIBCXX_DEBUG -o CppCode/main.exe CppCode/main.cpp
# g++ -fsanitize=address -g -o CppCode/main.exe CppCode/main.cpp
# g++ -fsanitize=undefined -g -o CppCode/main.exe CppCode/main.cpp
python3 run.py ./interactor data/sample_official.in CppCode/main.exe -r 20000 40000 50000 60000 80000
