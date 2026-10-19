/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SystemManager.hpp
*/

#ifndef ECS_SYSTEM_MANAGER_HPP
    #define ECS_SYSTEM_MANAGER_HPP

    #include <functional>
    #include <stdexcept>
    #include <string>
    #include <utility>
    #include <vector>

    #include "Registry.hpp"

namespace r_type::ecs {

/**
 * @brief Ordered list of named systems, run once per frame.
 *
 * A system is anything callable as void(Registry &, float dt): a lambda, a
 * free function or a functor with its own state. dt is in seconds and comes
 * from the caller (fixed timestep), never from a clock inside the system.
 * Execution order = registration order.
 *
 * Do not call add() from inside a running system (undefined behaviour).
 * An exception thrown by a system leaves run(): the following systems are
 * skipped for that pass.
 */
class SystemManager {
public:
    using SystemFn = std::function<void(Registry &, float)>;

    /** @throws std::invalid_argument if the name is empty/duplicated or fn is empty. */
    void add(std::string name, SystemFn fn)
    {
        if (name.empty() || !fn)
            throw std::invalid_argument("SystemManager: invalid system");
        for (const Entry &e : systems_)
            if (e.name == name)
                throw std::invalid_argument("SystemManager: duplicate system '" + name + "'");
        systems_.push_back(Entry{std::move(name), std::move(fn)});
    }

    /** @brief Runs every system once, in registration order. */
    void run(Registry &registry, float dt)
    {
        for (const Entry &e : systems_)
            e.fn(registry, dt);
    }

    std::size_t size() const noexcept { return systems_.size(); }

private:
    struct Entry {
        std::string name;
        SystemFn fn;
    };

    std::vector<Entry> systems_;
};

}

#endif