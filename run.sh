# ./build/Minecraft_Clone "127.0.0.1"
./build/Minecraft_Clone "162.19.137.231"
# sudo perf record -g ./build/Minecraft_Clone "162.19.137.231"
# gdb ./build/Minecraft_Clone
# valgrind --leak-check=full \
#          --show-leak-kinds=all \
#          --track-origins=yes \
#          --verbose \
#          --log-file=valgrind-out.txt \
#          ./build/Minecraft_Clone "162.19.137.231"

# sysctl kernel.perf_event_paranoid=3
