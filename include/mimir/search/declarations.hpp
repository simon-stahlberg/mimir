#ifndef MIMIR_SEARCH_DECLARATIONS_HPP_
#define MIMIR_SEARCH_DECLARATIONS_HPP_

#include "config.hpp"


namespace mimir {

template<Config C>
class StateImpl;
template<Config C>
using State = const StateImpl<C>*;

/* State */


/* SearchNode */
template<typename T>
class View;
template<Config C>
class SearchNode;
template<Config C>
class View<SearchNode<C>>;

}


#endif