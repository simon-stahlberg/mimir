#if !defined(PLANNERS_OPEN_LIST_BASE_HPP_)
#define PLANNERS_OPEN_LIST_BASE_HPP_

#include <memory>

namespace planners
{
    template<typename T>
    class OpenListBase
    {
      public:
        virtual ~OpenListBase() = default;

        virtual void insert(const T& item, double priority) = 0;

        virtual T pop() = 0;

        virtual std::size_t size() = 0;
    };

    using OpenList = std::shared_ptr<OpenListBase<int32_t>>;
}  // namespace planners

#endif  // PLANNERS_OPEN_LIST_BASE_HPP_
