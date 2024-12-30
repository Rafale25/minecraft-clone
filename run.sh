envParams='__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia DRI_PRIME=1'
ipAddress='162.19.137.231'
renderDistance='16'
programPath='./build/Minecraft_Clone'

bash -c "$envParams $programPath $ipAddress $renderDistance"

# sudo perf record -g ./build/Minecraft_Clone "162.19.137.231"
# gdb ./build/Minecraft_Clone
# valgrind --leak-check=full \
#          --show-leak-kinds=all \
#          --track-origins=yes \
#          --verbose \
#          --log-file=valgrind-out.txt \
#          ./build/Minecraft_Clone "162.19.137.231"

# sysctl kernel.perf_event_paranoid=3
