#pragma once
#ifndef NEBEL_SIMULATION_SIMULATION_HPP
#define NEBEL_SIMULATION_SIMULATION_HPP

#include <vector>
#include <chrono>
#include <cstdint>
#include <glm/glm.hpp>

#include "world_buffer.hpp"
#include "settings.hpp"

namespace simulation
{
        int get_random(int min, int max);

        class Simulation {
                public:
                        Simulation(Settings settings, WorldBuffer::SharedPtr world_buffer);
                        void iteration();
                        ~Simulation()
                        {
                                info("Exited");
                        }
                private:
                        WorldBuffer::SharedPtr m_world_buffer;
                        std::chrono::steady_clock::duration m_period;
                        std::chrono::time_point<std::chrono::steady_clock> m_report_start;
                        uint64_t m_ticks_since_last_report = 0;

                        uint64_t m_iterations_per_cycle = 300;
                        uint64_t m_current_iteration = 0;

                        const World* m_input;
                        World* m_output;

                        void start_simulation_step();

        };

}; // namespace simulation

#endif // NEBEL_SIMULATION_SIMULATION_HPP
