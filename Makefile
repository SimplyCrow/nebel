CXX := g++
CXXFLAGS := -std=gnu++23 -Wall -Wextra -MP -MMD -I./vendor/glad2
LDFLAGS := $(shell pkgconf --libs sdl3) $(shell pkgconf --libs opengl) $(shell pkgconf --libs spdlog) -lm

SRCS := src/main.cpp src/renderer/renderer.cpp \
        src/glad.cpp src/renderer/shader.cpp \
        src/simulation.cpp
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
	rm -f  nebel

-include $(DEPS)
