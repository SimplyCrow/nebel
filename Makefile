LIBS := sdl3 opengl spdlog libconfig++ freetype2

IMGUI_DIR    := ./vendor/imgui
IMGUI_CFLAGS := -I$(IMGUI_DIR)
IMGUI_SRCS   := $(IMGUI_DIR)/imgui.cpp \
		$(IMGUI_DIR)/imgui_draw.cpp \
		$(IMGUI_DIR)/imgui_tables.cpp \
		$(IMGUI_DIR)/imgui_widgets.cpp \
		$(IMGUI_DIR)/imgui_demo.cpp \
		$(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp \
		$(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

LIBS_CFLAGS := $(IMGUI_CFLAGS)
LIBS_SRCS := $(IMGUI_SRCS)

CXX := g++
OPTIMIZATION := -O3
DEBUGFLAGS := -ggdb
CXXFLAGS := -std=gnu++23 -Wall -Wextra -MP -MMD -I./vendor/glad2 $(shell pkgconf --shared --cflags $(LIBS)) $(LIBS_CFLAGS) $(OPTIMIZATION) $(DEBUGFLAGS)
LDFLAGS := $(shell pkgconf --shared --libs $(LIBS)) -lstdc++exp -lm

SRCS := $(LIBS_SRCS) src/main.cpp \
	src/renderer/renderer.cpp src/glad.cpp src/renderer/shader.cpp \
        src/simulation/simulation.cpp src/config/config.cpp \
	src/simulation/neural/neural.cpp
OBJS := $(patsubst %.cpp, %.o, $(SRCS))
DEPS := $(patsubst %.cpp, %.d, $(SRCS))

.PHONY: all
all: nebel

nebel: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: run
run: nebel
	./nebel

.PHONY: clean
clean:
	rm -rf ./src/*.o
	rm -rf ./src/*.d
	rm -rf ./src/**/*.o
	rm -rf ./src/**/*.d
	rm -rf ./vendor/imgui/*.o
	rm -rf ./vendor/imgui/*.d
	rm -rf ./vendor/imgui/**/*.o
	rm -rf ./vendor/imgui/**/*.d
	rm -f  nebel

-include $(DEPS)
