#pragma once
#ifndef NEBEL_SIMULATION_WORLD_HPP
#define NEBEL_SIMULATION_WORLD_HPP

#include "../util.hpp"
#include "../panic.hpp"

#include "entity.hpp"
#include "logger.hpp"

#include <vector>
#include <algorithm>
#include <span>
#include <limits>

namespace simulation {

        class World {
                public:
                        World(size_t width, size_t height)
                                : m_width(width), m_height(height), m_grid(width * height, INVALID_INDEX)
                        {}

                        size_t width() const
                        {
                                return m_width;
                        }

                        size_t height() const
                        {
                                return m_height;
                        }

                        void clear()
                        {
                                m_entities.clear();
                                std::ranges::fill(m_grid, INVALID_INDEX);
                        }

                        std::span<const Entity> get_entities() const
                        {
                                return m_entities;
                        }

                        std::span<Entity> get_entities()
                        {
                                return m_entities;
                        }

                        bool collision(coord x, coord y) const
                        {
                                check_range(x, y);
                                return m_grid[index_of(x, y)] != INVALID_INDEX;
                        }

                        bool add_entity(Entity entity)
                        {
                                check_range(entity.x, entity.y);
                                const size_t cell = index_of(entity.x, entity.y);
                                if(m_grid[cell] != INVALID_INDEX)
                                        return false;
                                m_grid[cell] = m_entities.size();
                                m_entities.push_back(entity);
                                return true;
                        }

                        bool move_entity(coord x0, coord y0, coord x1, coord y1)
                        {
                                check_range(x0, y0);
                                check_range(x1, y1);

                                const size_t src_cell = index_of(x0, y0);
                                const size_t dst_cell = index_of(x1, y1);
                                const size_t entity_index = m_grid[src_cell];
                                if(entity_index == INVALID_INDEX || m_grid[dst_cell] != INVALID_INDEX)
                                        return false;

                                Entity& entity = m_entities[entity_index];
                                entity.x = x1;
                                entity.y = y1;

                                m_grid[src_cell] = INVALID_INDEX;
                                m_grid[dst_cell] = entity_index;
                                return true;
                        }

                        bool remove_entity(coord x, coord y)
                        {
                                check_range(x, y);

                                const size_t cell = index_of(x, y);
                                const size_t index = m_grid[cell];
                                if(index == INVALID_INDEX)
                                        return false;
                                remove_entity_index(index);
                                return true;
                        }

                        template <typename Pred>
                        void remove_filter(Pred pred)
                        {
                                size_t i = m_entities.size();
                                while(i-- > 0) {
                                        if(pred(m_entities[i])) {
                                                remove_entity_index(i);
                                        }
                                }
                        }

                        size_t get_iteration() const
                        {
                                return m_iteration;
                        }

                        void set_iteration(size_t iteration)
                        {
                                m_iteration = iteration;
                        }

                        size_t increment_iteration()
                        {
                                ++m_iteration;
                                return m_iteration;
                        }

                private:
                        static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

                        size_t              m_width;
                        size_t              m_height;
                        size_t              m_iteration = 0;
                        std::vector<Entity> m_entities;
                        std::vector<size_t> m_grid;

                        inline size_t index_of(coord x, coord y) const
                        {
                                return static_cast<size_t>(y) * m_width + static_cast<size_t>(x);
                        }

                        inline void check_range(coord x, coord y) const
                        {
                                if(x >= m_width || y >= m_height) [[unlikely]] {
                                        PANIC(simulation, "World: Invalid range check. w={} h={} x={} y={}", m_width, m_height, x, y);
                                }
                        }

                        void remove_entity_index(size_t index)
                        {
                                const size_t cell = index_of(m_entities[index].x, m_entities[index].y);
                                const size_t last_index = m_entities.size() - 1;
                                if(index != last_index) {
                                        Entity &moved = m_entities[last_index];
                                        m_entities[index] = moved;
                                        m_grid[index_of(moved.x, moved.y)] = index;
                                }
                                m_entities.pop_back();
                                m_grid[cell] = INVALID_INDEX;
                        }
        };

} // namespace simulation

#endif // NEBEL_SIMULATION_WORLD_HPP
