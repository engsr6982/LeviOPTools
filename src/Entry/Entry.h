#pragma once

#include <ll/api/plugin/NativePlugin.h>

namespace entry {

class entry {
    entry();

public:
    entry(entry&&)                 = delete;
    entry(const entry&)            = delete;
    entry& operator=(entry&&)      = delete;
    entry& operator=(const entry&) = delete;

    static entry& getInstance();

    [[nodiscard]] ll::plugin::NativePlugin& getSelf() const;

    /// @return True if the plugin is loaded successfully.
    bool load(ll::plugin::NativePlugin&);

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

private:
    ll::plugin::NativePlugin* mSelf{};
};

} // namespace entry
