mkdir build;
cd ./build && cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release && cmake --build . && cd ..

# g++ src/main.cpp src/glad.c -I./include -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
