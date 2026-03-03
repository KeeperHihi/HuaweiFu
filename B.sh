rm ./replay/*
g++ CppCode/main.cpp -o CppCode/main.exe
python3 run.py ./interactor data/sample.in CppCode/main.exe -r 20000 40000 50000 60000 80000
