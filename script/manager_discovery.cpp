#include "skeet/script/manager.hpp"
#include "manager_detail.hpp"
#include <algorithm>
#include <stdexcept>

namespace skeet::script {
bool script_is_loaded(std::u16string_view name, std::span<const BuiltinModule> builtins,
                      const std::set<std::u16string>& loaded_files) {
    if (name.empty())
        return false;
    if (name.front() != u'*')
        return loaded_files.contains(std::u16string(name));
    const auto hash = script_name_hash(name);
    return std::any_of(builtins.begin(), builtins.end(), [&](const auto& module) {
        return module.enabled && script_name_hash(std::string_view(module.name)) == hash;
    });
}
std::vector<ScriptRecord> discover_scripts(std::span<const BuiltinModule> builtins,
                                           ScriptFileSystem& files,
                                           const SettingsPersistenceState* settings) {
    std::vector<ScriptRecord> discovered;
    for (const auto& module : builtins) {
        const auto hash = script_name_hash(std::string_view(module.name)) ^ 0x92CA5DF1;
        bool startup = true;
        if (settings)
            startup = std::find(settings->script_hashes.begin(), settings->script_hashes.end(),
                                hash) == settings->script_hashes.end();
        discovered.push_back(
            {module.modified, std::uint8_t(startup), manager_detail::utf16(module.name)});
    }
    if (auto directory = files.current_directory()) {
        for (unsigned pass = 0; pass != 2; ++pass) {
            for (const auto& file : files.enumerate_lua(*directory)) {
                if (file.filename.size() < 4)
                    throw std::invalid_argument("NT script filename is shorter than its extension");
                auto name = file.filename.substr(0, file.filename.size() - 4);
                if (!settings)
                    throw std::logic_error("Script discovery requires the native settings owner");
                const auto hash = script_name_hash(std::u16string_view(name)) ^ 0x92CA5DF1;
                bool startup = false;
                for (auto saved : settings->script_hashes) {
                    if (!saved)
                        break;
                    if (saved == hash) {
                        startup = true;
                        break;
                    }
                }
                auto path = *directory;
                if (!path.empty() && path.back() != u'\\')
                    path.push_back(u'\\');
                path += file.filename;
                discovered.push_back({script_file_time(file.last_write_time), std::uint8_t(startup),
                                      std::move(name), std::move(path)});
            }
            if (pass == 0) {
                if (!directory->empty() && directory->back() != u'\\')
                    directory->push_back(u'\\');
                directory->append(u"lua");
            }
        }
    }
    return discovered;
}
} // namespace skeet::script
