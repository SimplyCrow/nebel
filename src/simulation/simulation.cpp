#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <random>

#include "../panic.hpp"

#include "simulation.hpp"
#include "world_buffer.hpp"

#include "logger.hpp"

namespace simulation {

        int get_random(int min, int max)
        {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<int> distrib(min, max);
                return distrib(gen);
        }

        Simulation::Simulation(Settings settings, WorldBuffer::SharedPtr world_buffer)
                : m_world_buffer(world_buffer)
        {
                m_period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1.0) / settings.iterations_per_second);
                m_report_start = std::chrono::steady_clock::now();
                const auto &[_, world] = m_world_buffer->new_simulation_step();
                for(size_t i = 0; i < settings.entities; ++i) {
                        Entity entity(get_random(0, settings.width - 1), get_random(0, settings.height - 1));

                        while(world.collision(entity.x, entity.y)) {
                                entity.x = get_random(0, settings.width - 1);
                                entity.y = get_random(0, settings.height - 1);
                        }

                        world.add_entity(entity);
                }
                m_world_buffer->publish_simulation_step();
                info("Created with w={} h={}", settings.width, settings.height);
        }

        void Simulation::iteration()
        {
                const auto start = std::chrono::steady_clock::now();
                ++m_ticks_since_last_report;

                const auto &[input, output] = m_world_buffer->new_simulation_step();
                output = input;
                output.increment_iteration();
                for(Entity &entity : output.get_entities()) {
                        signed_coord off_x = get_random(-1, 1);
                        signed_coord off_y = get_random(-1, 1);
                        coord new_x = static_cast<coord>(std::clamp(
                                static_cast<signed_coord>(entity.x) + off_x,
                                static_cast<signed_coord>(0),
                                static_cast<signed_coord>(output.width()  - 1)
                        ));
                        coord new_y = static_cast<signed_coord>(std::clamp(
                                static_cast<signed_coord>(entity.y) + off_y,
                                static_cast<signed_coord>(0),
                                static_cast<signed_coord>(output.height() - 1)
                        ));
                        output.move_entity(entity.x, entity.y, new_x, new_y);
                }
                m_world_buffer->publish_simulation_step();

                const auto deadline = start + m_period;
                if(std::chrono::steady_clock::now() < deadline) {
                        std::this_thread::sleep_until(deadline);
                }

                const auto now = std::chrono::steady_clock::now();
                const auto elapsed = now - m_report_start;
                if(elapsed >= std::chrono::seconds(5)) {
                        const double elapsed_sec = std::chrono::duration<double>(elapsed).count();
                        const double ticks_per_second = static_cast<double>(m_ticks_since_last_report) / elapsed_sec;
                        info("t/s: {}", ticks_per_second);
                        m_ticks_since_last_report = 0;
                        m_report_start = now;
                }
        }
} // namespace simulation
