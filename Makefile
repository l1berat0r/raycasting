FILES=raycasting.c map.c
BUILD_DIR=build

wasm:
	emcc -s SAFE_HEAP=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ALLOW_MEMORY_GROWTH=1 -o \$(BUILD_DIR)/raycasting_wasm.js --preload-file resources --use-preload-plugins \$(FILES)

main:
	gcc $(FILES) -g -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lSDL2_image -lm -o $(BUILD_DIR)/raycasting
