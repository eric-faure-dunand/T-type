/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Registry.hpp
*/

#ifndef ECS_REGISTRY_HPP
    #define ECS_REGISTRY_HPP

    #include <cstddef>
    #include <cstdint>
    #include <memory>
    #include <stdexcept>
    #include <string>
    #include <typeindex>
    #include <typeinfo>
    #include <unordered_map>
    #include <utility>
    #include <vector>

    #include "Entity.hpp"
    #include "SparseArray.hpp"

namespace r_type::ecs {

namespace detail {

/** @brief Type-erased view of a component storage, so kill() can clean every type. */
class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
    virtual void erase(std::size_t index) noexcept = 0;
};

template <class T>
class ComponentStorage final : public IComponentStorage {
public:
    explicit ComponentStorage(std::size_t capacity) : components(capacity) {}
    void erase(std::size_t index) noexcept override { components.erase(index); }

    SparseArray<T> components;
};

}

/**
 * @brief Owns the entities and the component storages.
 *
 * Errors:
 *  - spawn() beyond kMaxEntities            -> std::length_error
 *  - add/remove/get/kill on a dead handle   -> std::invalid_argument
 *  - any access to an unregistered type     -> std::out_of_range
 *  - get() of an absent component           -> std::out_of_range
 *  - has/tryGet/alive never throw on a dead handle (they answer false / nullptr)
 *
 * Thread-safety: none. A Registry must only be accessed by the game-logic
 * thread. Other threads (network...) must communicate through queues.
 */
class Registry {
public:
    Registry() : generations_(kMaxEntities, 0), alive_(kMaxEntities, false)
    {
        freeList_.reserve(kMaxEntities);
    }

    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;

    /**
     * @brief Creates an entity. Freed slots are reused first (LIFO).
     * @throws std::length_error if kMaxEntities entities are alive.
     */
    Entity spawn()
    {
        std::uint32_t index = 0;

        if (!freeList_.empty()) {
            index = freeList_.back();
            freeList_.pop_back();
        } else if (nextIndex_ < kMaxEntities) {
            index = static_cast<std::uint32_t>(nextIndex_++);
        } else {
            throw std::length_error("Registry: maximum number of entities reached ("
                + std::to_string(kMaxEntities) + ")");
        }
        alive_[index] = true;
        ++aliveCount_;
        return Entity(index, generations_[index]);
    }

    /**
     * @brief Destroys an entity immediately: removes all its components,
     *        bumps the slot generation and recycles the slot.
     * @throws std::invalid_argument if the handle is dead, stale or null.
     */
    void kill(Entity e)
    {
        checkAlive(e);
        for (auto &entry : storages_)
            entry.second->erase(e.index());
        alive_[e.index()] = false;
        generations_[e.index()] = (generations_[e.index()] + 1u) & Entity::kGenerationMask;
        freeList_.push_back(e.index());
        --aliveCount_;
    }

    /** @brief True if the handle refers to a currently alive entity. */
    bool alive(Entity e) const noexcept
    {
        return !e.isNull()
            && e.index() < kMaxEntities
            && alive_[e.index()]
            && generations_[e.index()] == e.generation();
    }

    std::size_t aliveCount() const noexcept { return aliveCount_; }

    /** @brief Registers component type T. Must be done at startup. Idempotent. */
    template <class T>
    void registerComponent()
    {
        const std::type_index key(typeid(T));
        if (storages_.find(key) == storages_.end())
            storages_.emplace(key, std::make_unique<detail::ComponentStorage<T>>(kMaxEntities));
    }

    /**
     * @brief Attaches a T built from @p args to the entity. Replaces an existing T.
     * @throws std::invalid_argument dead handle; std::out_of_range unregistered T.
     * @return Reference to the component (valid until it is removed or replaced).
     */
    template <class T, class... Args>
    T &add(Entity e, Args &&...args)
    {
        checkAlive(e);
        return getComponents<T>().emplace(e.index(), std::forward<Args>(args)...);
    }

    /**
     * @brief Detaches T from the entity. No-op if the entity has no T.
     * @throws std::invalid_argument dead handle; std::out_of_range unregistered T.
     */
    template <class T>
    void remove(Entity e)
    {
        checkAlive(e);
        getComponents<T>().erase(e.index());
    }

    /** @brief True if the entity is alive and has a T. @throws std::out_of_range unregistered T. */
    template <class T>
    bool has(Entity e) const
    {
        return alive(e) && getComponents<T>().has(e.index());
    }

    /**
     * @brief Accesses the T of the entity.
     * @throws std::invalid_argument dead handle; std::out_of_range absent or unregistered T.
     */
    template <class T>
    T &get(Entity e)
    {
        checkAlive(e);
        return getComponents<T>()[e.index()];
    }

    template <class T>
    const T &get(Entity e) const
    {
        checkAlive(e);
        return getComponents<T>()[e.index()];
    }

    /** @brief Pointer to the T of the entity, or nullptr (absent / dead). */
    template <class T>
    T *tryGet(Entity e)
    {
        return alive(e) ? getComponents<T>().tryGet(e.index()) : nullptr;
    }

    template <class T>
    const T *tryGet(Entity e) const
    {
        return alive(e) ? getComponents<T>().tryGet(e.index()) : nullptr;
    }

    /**
     * @brief The whole storage of T, for systems that iterate on a component type.
     * @throws std::out_of_range if T was not registered.
     */
    template <class T>
    SparseArray<T> &getComponents()
    {
        return storageOf<T>().components;
    }

    template <class T>
    const SparseArray<T> &getComponents() const
    {
        return storageOf<T>().components;
    }

private:
    template <class T>
    detail::ComponentStorage<T> &storageOf() const
    {
        const auto it = storages_.find(std::type_index(typeid(T)));
        if (it == storages_.end())
            throw std::out_of_range(std::string("Registry: component type not registered: ")
                + typeid(T).name());
        return static_cast<detail::ComponentStorage<T> &>(*it->second);
    }

    void checkAlive(Entity e) const
    {
        if (!alive(e))
            throw std::invalid_argument("Registry: invalid or stale entity handle");
    }

    std::vector<std::uint32_t> generations_;
    std::vector<bool> alive_;
    std::vector<std::uint32_t> freeList_;
    std::size_t nextIndex_ = 0;
    std::size_t aliveCount_ = 0;
    std::unordered_map<std::type_index, std::unique_ptr<detail::IComponentStorage>> storages_;
};

}

#endif