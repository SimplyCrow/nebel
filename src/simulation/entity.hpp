#pragma once
#ifndef NEBEL_SIMULATION_ENTITY_HPP
#define NEBEL_SIMULATION_ENTITY_HPP

#include "../util.hpp"
#include "neural/neural.hpp"

namespace simulation {
        class Entity {
                public:
                        coord x = 0;
                        coord y = 0;
                        neural::Neural neural{{2, 10, 10, 2}, true};
        };

} // namespace simulation

#endif // NEBEL_SIMULATION_ENTITY_HPP
