all:
	g++ -std=c++20 main.cpp -o main -I"dep/include"  -L"dep/lib" -lsfml-graphics -lsfml-audio -lsfml-network -lsfml-window -lsfml-system 
clean:
	rm -f *.exe