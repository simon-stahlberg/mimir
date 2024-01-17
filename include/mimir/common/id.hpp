#ifndef MIMIR_COMMON_ID_HPP_
#define MIMIR_COMMON_ID_HPP_


namespace mimir {

/// @brief An identifier for an object of type T.
///        Objects of type T will follow a persistent indexing scheme from 0,1,...
///        We provide spezialized containers allowing to store information per ID.
/// @tparam T 
template<typename T>
class ID {
private:
    int m_id;

public:
    ID(int id) : m_id(id) { }
};

}  // namespace mimir

#endif  // MIMIR_COMMON_ID_HPP_