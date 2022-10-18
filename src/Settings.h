#pragma once

struct Settings {
    using ISetting = AutoTOML::ISetting;
    using bSetting = AutoTOML::bSetting;
    using iSetting = AutoTOML::iSetting;

    static void load() {
        try {
            const auto table = toml::parse_file("Data/SKSE/Plugins/WhoseQuestIsItAnyway.toml"s);
            for (const auto& setting : ISetting::get_settings()) {
                setting->load(table);
            }
        } catch (const toml::parse_error& e) {
            std::ostringstream ss;
            ss << "Error parsing file \'" << *e.source().path << "\':\n"
               << '\t' << e.description() << '\n'
               << "\t\t(" << e.source().begin << ')';
            logger::error("{}"sv, ss.str());
            throw std::runtime_error("failed to load settings"s);
        }
    }

    static inline bSetting printQuestFormID{"General"s, "printQuestFormID"s, false};
    static inline iSetting totalMessageCount{"General"s, "totalMessageCount"s, 5};
    static inline bSetting useRandomMessages{"General"s, "useRandomMessages"s, false};
};