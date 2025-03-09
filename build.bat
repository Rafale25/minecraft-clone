@REM mkdir build;
@REM cd ./build && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release && cmake --build . -j  && cd ..

mkdir build;
cmake --build ./build -j

@REM --config Release
@REM cmake --build build -j --preset release
