#pragma once
#ifndef NEBEL_MAP_HPP

#include <glm/glm.hpp>
#include <cstdint>

namespace sim
{
        class Map {
                public:
                        static constexpr size_t INVALID = SIZE_MAX;

                        Map(size_t width = 50, size_t height = 50);

                        size_t width() const;
                        size_t height() const;
                        size_t get(size_t x, size_t y) const;
                        size_t get(glm::ivec2 pos) const;
                        bool contains(size_t x, size_t y) const;
                        bool contains(glm::ivec2 pos) const;
                        void set(size_t x, size_t y, size_t value);
                        void set(glm::ivec2 pos, size_t value);
                        void move(size_t x1, size_t y1, size_t x2, size_t y2);
                        void move(glm::ivec2 pos1, glm::ivec2 pos2);

                private:
                        size_t m_width;
                        size_t m_height;
                        std::vector<size_t> m_map;
        };
}; // namespace simy

#endif // NEBEL_MAP_HPP
