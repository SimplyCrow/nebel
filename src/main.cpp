#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <libconfig.h++>
#include <glm/glm.hpp>

#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <memory>
#include <stop_token>
#include <map>
#include <ctime>
#include <string>
#include <filesystem>

#include "logger.hpp"
#include "simulation/world_buffer.hpp"
#include "config/config.hpp"
#include "renderer/renderer.hpp"
#include "simulation/simulation.hpp"

void unhandled_exception()
{
        logger::critical("(Main) Unexpected exception");
        try {
                std::rethrow_exception(std::current_exception());
        } catch(const std::exception& e) {
                logger::critical("(Main) what: {}", e.what());
        } catch(...) {
                logger::critical("(Main) what: unknown exception");
        }

        logger::get_logger()->flush();
        std::abort();
}

void simulation_thread(std::stop_token st, simulation::Settings settings, simulation::WorldBuffer::SharedPtr world_buffer)
{
        simulation::Simulation simulation(settings, world_buffer);
        while(!st.stop_requested()) {
                simulation.iteration();
        }
}

void render_thread(renderer::Settings settings, simulation::WorldBuffer::SharedPtr world_buffer)
{
        renderer::Renderer renderer(settings);
        while(!renderer.should_close()) {
                renderer.iteration(world_buffer);
        }
}

int main()
{
        auto named_default_logger = spdlog::default_logger()->clone("Main");
        spdlog::set_default_logger(named_default_logger);
        spdlog::set_pattern("[%H:%M:%S.%f] [%^%l%$] (%n) %v");

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        std::set_terminate(unhandled_exception);


        config::Config config("./res/config.cfg");
        auto simulation_config = config.get_config<simulation::Settings>();
        auto renderer_config = config.get_config<renderer::Settings>();

        simulation::World world(simulation_config.width, simulation_config.height);
        auto world_buffer = std::make_shared<simulation::WorldBuffer>(world);
        std::jthread thread_simulation(simulation_thread, simulation_config, world_buffer);
        render_thread(renderer_config, world_buffer);

        logger::info("Exited main");
}
