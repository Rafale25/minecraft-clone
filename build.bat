mkdir build;
cd ./build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release && cmake --build . -j  && cd ..
