#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>

#include <cstdlib>
#include <ctime>

#include "renderer/renderer.hpp"
#include "simulation.hpp"
#include "world.hpp"

static int WIDTH  = 800;
static int HEIGHT = 600;

void unhandled_exception()
{
        spdlog::critical("(Main) Unexpected exception");
        try {
                std::rethrow_exception(std::current_exception());
        } catch(const std::exception& e) {
                spdlog::critical("(Main) what: {}", e.what());
        } catch(...) {
                spdlog::critical("(Main) what: unknown exception");
        }

        spdlog::default_logger()->flush();
        std::abort();
}

int main()
{
        std::set_terminate(unhandled_exception);

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        sim::Simulation simulation(100, 100);
        Renderer renderer(WIDTH, HEIGHT, 60, simulation);

        while(!renderer.should_close()) {
                simulation.iteration();
                renderer.iteration();
        }

        spdlog::info("(Main) Exited main");
}
