#pragma once
#ifndef NEBEL_SIMULATION_NEURAL_NEURAL_HPP
#define NEBEL_SIMULATION_NEURAL_NEURAL_HPP

#include <vector>
#include <span>
#include <random>
#include <algorithm>
#include <cmath>

#include "logger.hpp"
#include "../../panic.hpp"

namespace simulation::neural
{

        class Layer {
        public:
                Layer(size_t input_count, size_t node_count, bool sigmoid = false)
                        : m_sigmoid(sigmoid), m_input_count(input_count), m_node_count(node_count),
                          m_nodes(node_count, 0), m_biases(node_count, 0), m_weights(input_count * node_count, 0)
                {}

                std::span<const float> calculate(std::span<const float> input)
                {
                        if(input.size() != m_input_count) {
                                PANIC(simulation::neural, "Invalid input provided to layer (exp: {} got: {})", m_input_count, input.size());
                        }

                        for(size_t i = 0; i < m_node_count; ++i) {
                                float z = m_biases[i];
                                for(size_t j = 0; j < m_input_count; ++j) {
                                        z += m_weights[weight_index(j, i)] * input[j];
                                }
                                m_nodes[i] = activation(z);
                        }

                        return m_nodes;
                }

                constexpr float activation(float x) const
                {
                        if(m_sigmoid)
                                return sigmoid(x);
                        return tanh(x);
                }

                constexpr float tanh(float x) const
                {
                        return std::tanh(x);
                }

                constexpr float sigmoid(float x) const
                {
                        return 1.0f / (1.0f + std::exp(-x));
                }

                inline void set_sigmoid(bool sigmoid)
                {
                        m_sigmoid = sigmoid;
                }

                void randomize()
                {
                        static thread_local std::mt19937 generator(std::random_device{}());

                        float fan_in = static_cast<float>(m_input_count);
                        float fan_out = static_cast<float>(m_node_count);

                        float limit = std::sqrt(6.0f / (fan_in + fan_out));
                        std::uniform_real_distribution<float> weight_distribution(-limit, limit);
                        std::uniform_real_distribution<float> bias_distribution(-0.3f, 0.3f);

                        for(float &weight : m_weights) {
                                weight = weight_distribution(generator);
                        }

                        for(float &bias : m_biases) {
                                bias = bias_distribution(generator);
                        }
                }

                void debug() const
                {
                        info("- i: {} n: {}", m_input_count, m_node_count);
                        for(float bias : m_biases) {
                                info("  - b: {}", bias);
                        }
                        for(float weight : m_weights) {
                                info("  - w: {}", weight);
                        }
                }

        private:
                bool m_sigmoid;
                size_t m_input_count;
                size_t m_node_count;
                std::vector<float> m_nodes;
                std::vector<float> m_biases;
                std::vector<float> m_weights;

                inline size_t weight_index(size_t input, size_t node) const
                {
                        return node * m_input_count + input;
                }
        };

        class Neural {
        public:
                Neural(std::span<const size_t> configuration, bool should_randomize)
                {
                        if(configuration.size() < 2) {
                                PANIC(simulation::neural, "Tried to create network with less then two layers");
                        }
                        m_input_count = configuration[0];
                        size_t last_input_size = m_input_count;
                        for(size_t i = 1; i < configuration.size(); ++i) {
                                m_layers.emplace_back(last_input_size, configuration[i]);
                                if(i == configuration.size() - 1) {
                                        m_layers[i].set_sigmoid(false);
                                        m_outputs.resize(configuration[i]);
                                }
                                last_input_size = configuration[i];
                        }

                        if(should_randomize)
                                randomize();
                }

                Neural(std::initializer_list<size_t> configuration, bool should_randomize)
                        : Neural(std::span<const size_t>(configuration.begin(), configuration.end()), should_randomize)
                {}

                Neural(const Neural &) = default;
                Neural& operator=(const Neural &) = default;
                Neural(Neural &&) noexcept = default;
                Neural& operator=(Neural &&) noexcept = default;

                void randomize()
                {
                        for(Layer &layer : m_layers) {
                                layer.randomize();
                        }
                }

                void debug() const
                {
                        info("Debug neural:");
                        for(const Layer &layer : m_layers) {
                                layer.debug();
                        }
                }

                std::span<const char> calculate(std::span<const float> input)
                {
                        if(input.size() != m_input_count) {
                                PANIC(simulation::neural, "Invalid input provided to input of neural network (exp: {} got: {})", m_input_count, input.size());
                        }
                        std::span<const float> last_layer = input;
                        for(auto &layer : m_layers) {
                                last_layer = layer.calculate(last_layer);
                        }

                        return calculate_fire(last_layer);
                }

                std::span<const char> calculate(std::initializer_list<float> input)
                {
                        return calculate(std::span<const float>(input.begin(), input.end()));
                }

        private:
                size_t m_input_count = 0;
                std::vector<Layer> m_layers;
                std::vector<char> m_outputs;
                float m_fire_threshold = 0.0f;

                std::span<const char> calculate_fire(std::span<const float> last_layer)
                {
                        for(size_t i = 0; i < last_layer.size(); ++i) {
                                m_outputs[i] = last_layer[i] > m_fire_threshold;
                        }
                        return m_outputs;
                }
        };

} // namespace simulation::neural

#endif // NEBEL_SIMULATION_NEURAL_NEURAL_HPP
