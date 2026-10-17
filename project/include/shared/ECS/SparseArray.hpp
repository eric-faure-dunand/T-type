/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SparseArray.hpp
*/

#ifndef ECS_SPARSE_ARRAY_HPP
    #define ECS_SPARSE_ARRAY_HPP

    #include <cstddef>
    #include <optional>
    #include <stdexcept>
    #include <string>
    #include <utility>
    #include <vector>

namespace r_type::ecs {

/**
 * @brief Storage of one component type, indexed by entity slot index.
 *
 * Each slot is a std::optional<T>: an empty slot means "this entity has no
 * such component". The array is allocated once, with a fixed capacity, and
 * never reallocates: references returned by emplace(), operator[] and tryGet()
 * stay valid until the component is erased or replaced.
 *
 * Not thread-safe.
 *
 * @tparam T Component type. Any type is accepted: empty tags, move-only types...
 */
template <class T>
class SparseArray {
public:
    using value_type = std::optional<T>;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    /** @brief Allocates @p capacity empty slots. */
    explicit SparseArray(std::size_t capacity) : slots_(capacity) {}

    /**
     * @brief Constructs a component in slot @p i, replacing any existing one.
     * @throws std::out_of_range if @p i >= size().
     * @return A reference to the new component.
     */
    template <class... Args>
    T &emplace(std::size_t i, Args &&...args)
    {
        checkIndex(i);
        const bool had = slots_[i].has_value();
        try {
            slots_[i].emplace(std::forward<Args>(args)...);
        } catch (...) {
            if (had)
                --count_;
            throw;
        }
        if (!had)
            ++count_;
        return *slots_[i];
    }

    /** @brief Removes the component of slot @p i. No-op if empty or out of range. */
    void erase(std::size_t i) noexcept
    {
        if (i < slots_.size() && slots_[i].has_value()) {
            slots_[i].reset();
            --count_;
        }
    }

    /** @brief True if slot @p i holds a component (false if out of range). */
    bool has(std::size_t i) const noexcept { return i < slots_.size() && slots_[i].has_value(); }

    /** @brief Accesses the component of slot @p i. @throws std::out_of_range if absent. */
    T &operator[](std::size_t i)
    {
        if (!has(i))
            throw std::out_of_range("SparseArray: no component in slot " + std::to_string(i));
        return *slots_[i];
    }

    const T &operator[](std::size_t i) const
    {
        if (!has(i))
            throw std::out_of_range("SparseArray: no component in slot " + std::to_string(i));
        return *slots_[i];
    }

    /** @brief Pointer to the component of slot @p i, or nullptr if absent. */
    T *tryGet(std::size_t i) noexcept { return has(i) ? &*slots_[i] : nullptr; }
    const T *tryGet(std::size_t i) const noexcept { return has(i) ? &*slots_[i] : nullptr; }

    /** @brief Capacity (number of slots), NOT the number of components. */
    std::size_t size() const noexcept { return slots_.size(); }

    /** @brief Number of slots currently holding a component. */
    std::size_t count() const noexcept { return count_; }

    /**
     * @name Raw iteration over all slots (empty ones included)
     * Dereferencing gives a std::optional<T>: check it before use.
     * @{
     */
    iterator begin() noexcept { return slots_.begin(); }
    iterator end() noexcept { return slots_.end(); }
    const_iterator begin() const noexcept { return slots_.begin(); }
    const_iterator end() const noexcept { return slots_.end(); }
    /** @} */

private:
    void checkIndex(std::size_t i) const
    {
        if (i >= slots_.size())
            throw std::out_of_range("SparseArray: index " + std::to_string(i) + " out of range");
    }

    std::vector<value_type> slots_;
    std::size_t count_ = 0;
};

}

#endif