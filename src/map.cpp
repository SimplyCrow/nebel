#include <spdlog/spdlog.h>

#include "map.hpp"
#include "panic.hpp"

namespace sim {

        Map::Map(size_t width, size_t height)
                : m_width(width), m_height(height), m_map(width * height, INVALID)
        {
                spdlog::info("(Map) Created w={} h={}", m_width, m_height);
        }

        size_t Map::width() const
        {
                return m_width;
        }

        size_t Map::height() const
        {
                return m_height;
        }

        size_t Map::get(size_t x, size_t y) const
        {
                if(x >= m_width || y >= m_height) {
                        PANIC("Tried to access invalid map index. x={} y={} w={} h={}", x, y, m_width, m_height);
                }
                return m_map[y * m_width + x];
        }

        size_t Map::get(glm::ivec2 pos) const
        {
                return get(pos.x, pos.y);
        }

        bool Map::contains(size_t x, size_t y) const
        {
                return get(x, y) != INVALID;
        }

        bool Map::contains(glm::ivec2 pos) const
        {
                return contains(pos.x, pos.y);
        }

        void Map::set(size_t x, size_t y, size_t value)
        {
                if(x >= m_width || y >= m_height) {
                        PANIC("Tried to access invalid map index. x={} y={} w={} h={}", x, y, m_width, m_height);
                }

                m_map[y * m_width + x] = value;
        }

        void Map::set(glm::ivec2 pos, size_t value)
        {
                set(pos.x, pos.y, value);
        }

        void Map::move(size_t x1, size_t y1, size_t x2, size_t y2)
        {
                if(x1 >= m_width || y1 >= m_height || x2 >= m_width || y2 >= m_height) {
                        PANIC("Tried to access invalid map index. x1={} y1={} x2={} y2={} w={} h={}", x1, y1, x2, y2, m_width, m_height);
                }

                m_map[y2 * m_width + x2] = m_map[y1 * m_width + x1];
                m_map[y1 * m_width + x1] = INVALID;
        }

        void Map::move(glm::ivec2 pos1, glm::ivec2 pos2)
        {
                move(pos1.x, pos1.y, pos2.x, pos2.y);
        }

} // namespace sim
