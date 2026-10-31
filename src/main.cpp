#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>

#include <cstdlib>
#include <ctime>

#include "renderer/renderer.hpp"
#include "simulation.hpp"

static int WIDTH  = 800;
static int HEIGHT = 600;

int main()
{
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        sim::Simulation simulation(100, 100);
        Renderer renderer(WIDTH, HEIGHT, 60, simulation);

        while(!renderer.should_close()) {
                simulation.iteration();
                renderer.iteration();
        }

        spdlog::info("(Main) Exited main");
}
