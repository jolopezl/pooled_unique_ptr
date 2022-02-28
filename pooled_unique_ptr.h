/**
 * @file pooled_unique_ptr.h
 * @author Jorge Lopez
 * @brief A quick and dirty implementation of a pooled unique pointer.
 * @version 0.1
 * @date 2022-02-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <bitset>
#include <map>
#include <utility>
#include <stdexcept>
#include <unordered_set>

// maximum number of objects in the pool (small for testing and debugging)
// used 4 for testing and debugging
#define MAX_POOL_SIZE 4

template <typename T>
class pooled_unique_ptr
{
   private:
    T* _ptr;
    size_t _ptr_pool_index; // position in the pool
    static inline bool _initialized = false;
    static inline size_t _pool_pos; // pointer to the next free position in the pool
    // static inline char _pool_buffer[sizeof(T) * MAX_POOL_SIZE]; // TODO: replace with something more C++ friendly (done!)
    static inline std::aligned_storage_t<sizeof(T), alignof(T)> _pool_buffer[MAX_POOL_SIZE];
    static inline std::unordered_set<size_t> _pool_open_slots; // keep track of O(1) average access/insert/delete
    size_t find_open_slot();

   public:
    pooled_unique_ptr() {}

    template <typename... Args>
    pooled_unique_ptr(Args&&... args)
    {
        // _ptr = new T(std::forward<Args>(args)...);
        if (!_initialized) {
            initialize_pool();
        }
        auto pos = find_open_slot();
        // _ptr = new (_pool_buffer + sizeof(T) * pos) T(std::forward<Args>(args)...);
        _ptr = new (&_pool_buffer[pos]) T(std::forward<Args>(args)...);
        _ptr_pool_index = pos;
    }

    ~pooled_unique_ptr()
    {
        if (_ptr != nullptr) {
            _pool_open_slots.insert(_ptr_pool_index);
        }
        // the following made the compilation much slower
        std::destroy_at(std::launder(reinterpret_cast<T*>(_ptr)));
    }

    // copy constructor - disabled
    pooled_unique_ptr(const pooled_unique_ptr&) = delete;

    // move constructor
    pooled_unique_ptr(pooled_unique_ptr&& other)
    {
        _ptr = std::move(other._ptr);
        _ptr_pool_index = other._ptr_pool_index;
        // other._ptr = nullptr;
        std::destroy_at(std::launder(reinterpret_cast<T*>(other._ptr)));
    };

    // copy assignment - disabled
    pooled_unique_ptr& operator=(const pooled_unique_ptr&) = delete;

    // move assignment
    pooled_unique_ptr& operator=(pooled_unique_ptr&& other)
    {
        _ptr = std::move(other._ptr);
        _ptr_pool_index = other._ptr_pool_index;
        // other._ptr = nullptr;
        std::destroy_at(std::launder(reinterpret_cast<T*>(other._ptr)));
        return *this;
    }

    // overloaded arrow operator
    T* operator->()
    {
        return _ptr;
    }

    // overloaded dereference operator
    T& operator*()
    {
        return *_ptr;
    }

    T* data()
    {
        return _ptr;
    }

    static void initialize_pool()
    {
        _pool_pos = 0;
        _initialized = true;
    }
};

template <typename T>
size_t pooled_unique_ptr<T>::find_open_slot()
{
    /**
     * @brief Find an open slot in the pool.
     * First uses all slots in order 0..MAX_POOL_SIZE-1
     * Once all slots are used, we rely on the _pool_open_slots set
     * @throws std::runtime_error indicates that the pool is full if there are no open slots
     * @return The index of the open slot.
     */
    size_t pos = _pool_pos;
    if (_pool_pos < MAX_POOL_SIZE) {
        pos = this->_pool_pos;
        ++this->_pool_pos;
    } else {
        if (this->_pool_open_slots.empty()) {
            throw std::runtime_error("ERROR: Memory pool is full");
        } else {
            pos = *(this->_pool_open_slots.begin());
            this->_pool_open_slots.erase(pos);
        }
    }
    return pos;
}
