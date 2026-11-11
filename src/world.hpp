#pragma once
#ifndef NEBEL_WORLD_HPP
#define NEBEL_WORLD_HPP

#include "util.hpp"
#include "panic.hpp"
#include "entity.hpp"

#include <vector>
#include <algorithm>
#include <span>
#include <limits>

class World {
public:
        World(size_t width, size_t height)
                : m_width(width), m_height(height)
        {}

        size_t width() const
        {
                return m_width;
        }

        size_t height() const
        {
                return m_height;
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
                return std::ranges::any_of(m_entities, [x, y](const Entity &entity) {
                        return entity.x == x && entity.y == y;
                });
        }

        bool add_entity(Entity entity)
        {
                check_range(entity.x, entity.y);
                if(collision(entity.x, entity.y))
                        return false;
                m_entities.push_back(entity);
                return true;
        }

        bool move_entity(coord x0, coord y0, coord x1, coord y1)
        {
                check_range(x0, y0);
                check_range(x1, y1);
                if(collision(x1, y1))
                        return false;

                auto entity = std::ranges::find_if(m_entities, [x0, y0](const Entity& entity) {
                        return entity.x == x0 && entity.y == y0;
                });

                if(entity == m_entities.end())
                        return false;

                entity->x = x1;
                entity->y = y1;
                return true;
        }

private:
        size_t              m_width;
        size_t              m_height;
        std::vector<Entity> m_entities;

        inline void check_range(coord x, coord y) const
        {
                if(x >= m_width || y >= m_height) [[unlikely]] {
                        PANIC("(World) Invalid range check. w={} h={} x={} y={}", m_width, m_height, x, y);
                }
        }
};

#endif // NEBEL_WORLD_HPP
