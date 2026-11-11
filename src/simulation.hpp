#pragma once
#ifndef NEBEL_SIMULATION_HPP
#define NEBEL_SIMULATION_HPP

#include <vector>
#include <glm/glm.hpp>

#include "world.hpp"

namespace sim
{
        int get_random(int min, int max);

        class Simulation {
                public:
                        Simulation(size_t width, size_t height);
                        void iteration();
                        const World &get_world() const;
                private:
                        World m_world;
        };

}; // namespace sim

#endif // NEBEL_SIMULATION_HPP
