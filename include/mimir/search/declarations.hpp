#ifndef MIMIR_SEARCH_DECLARATIONS_HPP_
#define MIMIR_SEARCH_DECLARATIONS_HPP_

#include "config.hpp"


namespace mimir {

template<typename T>
class View;

/* State */
template<Config C>
class State;
template<Config C>
class View<State<C>>;

/* SearchNode */
template<Config C>
class SearchNode;
template<Config C>
class View<SearchNode<C>>;

}


#endif