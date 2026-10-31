#pragma once
#ifndef NEBEL_SIMULATION_HPP
#define NEBEL_SIMULATION_HPP

#include <vector>
#include <glm/glm.hpp>

#include "map.hpp"

namespace sim
{
        class Node {
        public:
                glm::ivec2 position;
        };

        int get_random(int min, int max);

        class Simulation {
                public:
                        Simulation(size_t width, size_t height);
                        void iteration();
                        const Map &get_map() const;
                        const Node &get_node(size_t index) const;
                private:
                        Map m_map;
                        std::vector<Node> m_nodes;
        };

}; // namespace sim

#endif // NEBEL_SIMULATION_HPP
