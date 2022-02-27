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

#define MAX_POOL_SIZE 4

template <typename T>
class pooled_unique_ptr
{
   private:
    T* _ptr;
    size_t _ptr_pool_index; // position in the pool
    static inline bool _initialized = false;
    static inline size_t _pool_pos;                             // pointer to the next free position in the pool
    static inline char _pool_buffer[sizeof(T) * MAX_POOL_SIZE]; // TODO: check the type of the buffer
    static inline std::bitset<MAX_POOL_SIZE> _pool_slots;
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

        printf("current mmap (before allocation): %s\n", _pool_slots.to_string().c_str());
        auto pos = find_open_slot();
        printf("allocated slot: %zu\n", pos);
        _ptr = new (_pool_buffer + sizeof(T) * pos) T(std::forward<Args>(args)...);
        _ptr_pool_index = pos;
        _pool_slots.set(pos);
        printf("current mmap (after allocation): %s\n", _pool_slots.to_string().c_str());
    }

    ~pooled_unique_ptr()
    {
        printf("current mmap (before deallocation): %s \t deallocating slot %ld \n", _pool_slots.to_string().c_str(), _ptr_pool_index);
        if (_ptr != nullptr) {
            _pool_slots.set(_ptr_pool_index, false);
            _pool_open_slots.insert(_ptr_pool_index);
        }
        printf("current mmap (after deallocation): %s\n", _pool_slots.to_string().c_str());
        for (const auto& slot : _pool_open_slots) {
            printf("slot %zu is open\n", slot);
        }
    }

    // copy constructor - disabled
    pooled_unique_ptr(const pooled_unique_ptr&) = delete;

    // move constructor
    pooled_unique_ptr(pooled_unique_ptr&& other)
    {
        _ptr = std::move(other._ptr);
        _ptr_pool_index = other._ptr_pool_index;
        other._ptr = nullptr;
    };

    // copy assignment - disabled
    pooled_unique_ptr& operator=(const pooled_unique_ptr&) = delete;

    // move assignment
    pooled_unique_ptr& operator=(pooled_unique_ptr&& other)
    {
        _ptr = std::move(other._ptr);
        _ptr_pool_index = other._ptr_pool_index;
        other._ptr = nullptr;
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

    static void initialize_pool()
    {
        _pool_pos = 0;
        _pool_slots.reset();
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
