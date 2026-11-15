#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <random>

#include "simulation.hpp"
#include "panic.hpp"

namespace sim {

        int get_random(int min, int max)
        {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<int> distrib(min, max);
                return distrib(gen);
        }

        Simulation::Simulation(size_t width, size_t height)
                : m_world(width, height)
        {
                spdlog::info("(Simulation) Created with w={} h={}", width, height);
                for(size_t i = 0; i < 100 * 100 * 0.25f; ++i) {
                        Entity entity(get_random(0, width - 1), get_random(0, height - 1));

                        while(m_world.collision(entity.x, entity.y)) {
                                entity.x = get_random(0, width - 1);
                                entity.y = get_random(0, height - 1);
                        }

                        m_world.add_entity(entity);
                        spdlog::info("(Simulation) Entity(x={}, y={})", entity.x, entity.y);
                }
        }

        const World &Simulation::get_world() const
        {
                return m_world;
        }

        void Simulation::iteration()
        {
                //spdlog::info("(Simulation) iteration");
                for(Entity &entity : m_world.get_entities()) {
                        signed_coord off_x = get_random(-1, 1);
                        signed_coord off_y = get_random(-1, 1);
                        coord new_x = static_cast<coord>(std::clamp(
                                static_cast<signed_coord>(entity.x) + off_x,
                                static_cast<signed_coord>(0),
                                static_cast<signed_coord>(m_world.width()  - 1)
                        ));
                        coord new_y = static_cast<signed_coord>(std::clamp(
                                static_cast<signed_coord>(entity.y) + off_y,
                                static_cast<signed_coord>(0),
                                static_cast<signed_coord>(m_world.height() - 1)
                        ));
                        m_world.move_entity(entity.x, entity.y, new_x, new_y);
                }
        }
} // namespace sim
