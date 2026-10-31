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
                : m_map(width, height)
        {
                spdlog::info("(Simulation) Created with w={} h={}", width, height);
                for(size_t i = 0; i < 25; ++i) {
                        Node node(glm::linearRand(glm::ivec2(0, 0), glm::ivec2(width - 1, height - 1)));

                        while(m_map.contains(node.position)) {
                                node.position = glm::linearRand(glm::ivec2(0, 0), glm::ivec2(width - 1, height - 1));
                        }

                        spdlog::info("(Simulation) Node(x={}, y={})", node.position.x, node.position.y);

                        m_map.set(node.position.x, node.position.y, i);
                        m_nodes.push_back(node);
                }
        }

        const Map &Simulation::get_map() const
        {
                return m_map;
        }

        const Node &Simulation::get_node(size_t index) const
        {
                if(index >= m_nodes.size()) {
                        PANIC("(Simulation) Tried to get node out of range. index={}", index);
                }
                return m_nodes[index];
        }

        void Simulation::iteration()
        {
                //spdlog::info("(Simulation) iteration");
                for(auto &node : m_nodes) {
                        glm::ivec2 offset = glm::linearRand(glm::ivec2(-1, -1), glm::ivec2(1, 1));
                        glm::ivec2 new_position = node.position + offset;
                        new_position = glm::clamp(new_position, glm::ivec2(0, 0), glm::ivec2(m_map.width() - 1, m_map.height() - 1));
                        if(node.position == new_position)
                                continue;
                        if(m_map.contains(new_position))
                                continue;
                        m_map.move(node.position, new_position);
                        node.position = new_position;
                }
        }
} // namespace sim
