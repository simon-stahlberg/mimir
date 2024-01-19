#ifndef MIMIR_SEARCH_DECLARATIONS_HPP_
#define MIMIR_SEARCH_DECLARATIONS_HPP_

#include "config.hpp"


namespace mimir {

template<Config C> 
class StateImpl;
template<Config C>
using State = const StateImpl<C>*;

template<Config C>
using StatePtr = const StateImpl<C>*;
template<Config C> 
using ConstStatePtr = const StateImpl<C>*;

template<Config C>
class SearchNodeImpl;
template<Config C>
using SearchNode = SearchNodeImpl<C>*;

template<Config C>
using SearchNodePtr = SearchNodeImpl<C>*;
template<Config C>
using ConstSearchNodePtr = const SearchNodeImpl<C>*;

}


#endif 