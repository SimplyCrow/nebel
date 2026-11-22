#pragma once
#ifndef NEBEL_SIMULATION_WORLD_BUFFER_HPP
#define NEBEL_SIMULATION_WORLD_BUFFER_HPP

#include <utility>
#include <atomic>
#include <array>
#include <cstddef>
#include <memory>

#include "world.hpp"

namespace simulation {

        class WorldBuffer {
        public:
                using SharedPtr = std::shared_ptr<simulation::WorldBuffer>;
                static constexpr size_t InvalidIndex = static_cast<size_t>(-1);

                explicit WorldBuffer(const World& initial)
                        : m_worlds{initial, initial, initial} {}

                WorldBuffer(const WorldBuffer&) = delete;
                WorldBuffer& operator=(const WorldBuffer&) = delete;

                std::pair<const World*, World*> new_simulation_step()
                {
                        const size_t render_idx = m_render_idx.load(std::memory_order_acquire);
                        const size_t published_idx = m_published_idx.load(std::memory_order_acquire);

                        const size_t protected_render_idx =
                                (published_idx == InvalidIndex) ? m_last_published_idx : render_idx;

                        size_t read_idx = (published_idx != InvalidIndex) ? published_idx : m_last_published_idx;

                        size_t write_idx = 0;
                        while(write_idx == protected_render_idx || write_idx == read_idx) {
                                ++write_idx;
                        }

                        m_worlds[write_idx].clear();
                        m_sim_read_idx = read_idx;
                        m_sim_write_idx = write_idx;
                        return {&m_worlds[read_idx], &m_worlds[write_idx]};
                }

                void publish_simulation_step()
                {
                        m_last_published_idx = m_sim_write_idx;
                        m_published_idx.store(m_sim_write_idx, std::memory_order_release);
                }

                const World& acquire_render_world()
                {
                        const size_t newest = m_published_idx.exchange(InvalidIndex, std::memory_order_acq_rel);
                        if(newest != InvalidIndex) {
                                m_current_render_idx = newest;
                                m_render_idx.store(newest, std::memory_order_release);
                        }

                        return m_worlds[m_current_render_idx];
                }

        private:
                std::array<World, 3> m_worlds;
                std::atomic<size_t> m_published_idx{InvalidIndex};
                std::atomic<size_t> m_render_idx{0};

                size_t m_current_render_idx = 0;

                size_t m_last_published_idx = 0;
                size_t m_sim_read_idx = 0;
                size_t m_sim_write_idx = 1;
        };

}; // namespace simulation

#endif // NEBEL_SIMULATION_WORLD_BUFFER_HPP
