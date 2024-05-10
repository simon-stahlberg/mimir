#ifndef MIMIR_SEARCH_OPENLISTS_TAGS_HPP_
#define MIMIR_SEARCH_OPENLISTS_TAGS_HPP_

#include <concepts>

namespace mimir
{
/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct OpenListTag
{
};

template<class DerivedTag>
concept IsOpenListTag = std::derived_from<DerivedTag, OpenListTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsOpenListTag O>
struct OpenListDispatcher
{
};

template<typename T>
struct is_openlist_dispatcher : std::false_type
{
};

template<IsOpenListTag O>
struct is_openlist_dispatcher<OpenListDispatcher<O>> : std::true_type
{
};

template<typename T>
concept IsOpenListDispatcher = is_openlist_dispatcher<T>::value;

}

#endif