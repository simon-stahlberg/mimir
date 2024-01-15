#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATOR_HPP_

#include "../common/config.hpp"
#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir::search
{

template<typename Derived>
class SuccessorGeneratorBase {
private:
    SuccessorGeneratorBase() = default;
    friend Derived;

public:
    
};


template<typename Configuration>
class SuccessorGenerator : public SuccessorGeneratorBase<SuccessorGenerator<Configuration>> {
    // Implement configuration independent functionality. 
};


/// @brief Concrete implementation of a grounded successor generator.
template<>
class SuccessorGenerator<Grounded> : public SuccessorGeneratorBase<SuccessorGenerator<Grounded>> {
    // Implement configuration specific functionality. 
};


/// @brief Concrete implementation of a lifted successor generator.
template<>
class SuccessorGenerator<Lifted> : public SuccessorGeneratorBase<SuccessorGenerator<Lifted>> {
    // Implement configuration specific functionality. 
};


}  // namespace mimir::search

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATOR_HPP_
