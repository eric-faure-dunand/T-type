/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Registry.hpp
*/

#ifndef ECS_REGISTRY_HPP
    #define ECS_REGISTRY_HPP

    #include <algorithm>
    #include <cstddef>
    #include <cstdint>
    #include <memory>
    #include <stdexcept>
    #include <string>
    #include <typeindex>
    #include <type_traits>
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

template <class... Components>
class View;

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
        return makeEntity(index, generations_[index]);
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
            entry.second->erase(entityIndex(e));
        alive_[entityIndex(e)] = false;
        generations_[entityIndex(e)] = (generations_[entityIndex(e)] + 1u) & kGenerationMask;
        freeList_.push_back(entityIndex(e));
        --aliveCount_;
    }

    /** @brief True if the handle refers to a currently alive entity. */
    bool alive(Entity e) const noexcept
    {
        return e != Entity::Null
            && entityIndex(e) < kMaxEntities
            && alive_[entityIndex(e)]
            && generations_[entityIndex(e)] == entityGeneration(e);
    }

    std::size_t aliveCount() const noexcept { return aliveCount_; }

    /**
     * @brief Handle of the alive entity living in slot @p index, or Entity::Null.
     *
     * Used by iteration to rebuild a full handle (index + generation) from a slot.
     */
    Entity entityAt(std::size_t index) const noexcept
    {
        if (index >= kMaxEntities || !alive_[index])
            return Entity::Null;
        return makeEntity(static_cast<std::uint32_t>(index), generations_[index]);
    }

    /**
     * @brief Lightweight object to iterate on the entities having ALL the given components.
     * @throws std::out_of_range (at each() time) if one of the types is not registered.
     */
    template <class... Cs>
    View<Cs...> view()
    {
        return View<Cs...>(*this);
    }

    /**
     * @brief Shortcut for view<Cs...>().each(f).
     *
     * @code
     * reg.each<Position, Velocity>([](Position &p, const Velocity &v) { p.x += v.dx; });
     * reg.each<Position>([](Entity e, Position &p) { ... });   // handle first, if needed
     * @endcode
     */
    template <class... Cs, class F>
    void each(F &&f)
    {
        view<Cs...>().each(std::forward<F>(f));
    }

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
        return getComponents<T>().emplace(entityIndex(e), std::forward<Args>(args)...);
    }

    /**
     * @brief Detaches T from the entity. No-op if the entity has no T.
     * @throws std::invalid_argument dead handle; std::out_of_range unregistered T.
     */
    template <class T>
    void remove(Entity e)
    {
        checkAlive(e);
        getComponents<T>().erase(entityIndex(e));
    }

    /** @brief True if the entity is alive and has a T. @throws std::out_of_range unregistered T. */
    template <class T>
    bool has(Entity e) const
    {
        return alive(e) && getComponents<T>().has(entityIndex(e));
    }

    /**
     * @brief Accesses the T of the entity.
     * @throws std::invalid_argument dead handle; std::out_of_range absent or unregistered T.
     */
    template <class T>
    T &get(Entity e)
    {
        checkAlive(e);
        return getComponents<T>()[entityIndex(e)];
    }

    template <class T>
    const T &get(Entity e) const
    {
        checkAlive(e);
        return getComponents<T>()[entityIndex(e)];
    }

    /** @brief Pointer to the T of the entity, or nullptr (absent / dead). */
    template <class T>
    T *tryGet(Entity e)
    {
        return alive(e) ? getComponents<T>().tryGet(entityIndex(e)) : nullptr;
    }

    template <class T>
    const T *tryGet(Entity e) const
    {
        return alive(e) ? getComponents<T>().tryGet(entityIndex(e)) : nullptr;
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

namespace detail {

template <class...>
struct AllDifferent : std::true_type {};

template <class T, class... Rest>
struct AllDifferent<T, Rest...>
    : std::bool_constant<(!std::is_same_v<T, Rest> && ...) && AllDifferent<Rest...>::value> {};

}

/**
 * @brief Iteration on every entity that has ALL the components Cs.
 *
 * The callback is invoked as f(Entity, Cs&...) or, if it does not need the
 * handle, f(Cs&...). Entities are visited in ascending slot-index order.
 *
 * Behaviour while iterating (see docs/ecs.md):
 *  - modifying the components you receive is always fine;
 *  - removing a component or killing an entity that has NOT been visited yet
 *    is safe: it is simply not visited;
 *  - after removing a component of / killing the CURRENT entity, do not use the
 *    references you received for it any more;
 *  - entities spawned or components added during the pass are memory-safe
 *    (storages never reallocate) but may or may not be visited by this pass.
 */
template <class... Cs>
class View {
    static_assert(sizeof...(Cs) > 0, "View needs at least one component type");
    static_assert(detail::AllDifferent<Cs...>::value, "View: component types must be distinct");

public:
    explicit View(Registry &registry) : registry_(&registry) {}

    /** @throws std::out_of_range if one of the component types is not registered. */
    template <class F>
    void each(F &&f)
    {
        run(f, registry_->getComponents<Cs>()...);
    }

private:
    template <class F>
    void run(F &f, SparseArray<Cs> &...arrays)
    {
        if (((arrays.count() == 0) || ...))
            return;
        const std::size_t limit = std::min({arrays.size()...});
        for (std::size_t i = 0; i < limit; ++i) {
            if (!(arrays.has(i) && ...))
                continue;
            if constexpr (std::is_invocable_v<F &, Entity, Cs &...>) {
                f(registry_->entityAt(i), arrays[i]...);
            } else {
                static_assert(std::is_invocable_v<F &, Cs &...>,
                    "View::each callback must accept (Entity, Cs&...) or (Cs&...)");
                f(arrays[i]...);
            }
        }
    }

    Registry *registry_;
};

}

#endif