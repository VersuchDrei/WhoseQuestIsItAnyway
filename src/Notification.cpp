#include "Notification.h"

#pragma warning(push, 0)
#include <pcg_random.hpp>
#pragma warning(pop)

namespace Notification {
    namespace {
        [[nodiscard]] auto Roll() noexcept -> std::uint32_t {
            static struct {
                std::mutex mutex;
                ::pcg32 rng = []() {
                    pcg_extras::seed_seq_from<std::random_device> src;
                    return ::pcg32(src);
                }();
                std::uniform_int_distribution<std::uint32_t> dist{
                    0, static_cast<std::uint32_t>(std::max(*Settings::totalMessageCount - 1, 0ll))};
            } s;

            const std::lock_guard _(s.mutex);
            return s.dist(s.rng);
        }
    }  // namespace

    auto Build(const RE::TESQuest& a_quest) -> std::string {
        auto msg = fmt::format("$WHQA_Msg{:02}"sv, *Settings::useRandomMessages ? Roll() : 0);
        msg += "{";

        bool foundQuestName = false;
        if (!a_quest.fullName.empty()) {
            msg += a_quest.fullName.c_str();
            foundQuestName = true;
        } else if (!a_quest.formEditorID.empty()) {
            msg += a_quest.formEditorID.c_str();
            foundQuestName = true;
        }

        if (*Settings::printQuestFormID) {
            if (foundQuestName) {
                msg += " ";
            }
            msg += fmt::format("[{:08X}]"sv, a_quest.GetFormID());
        }

        msg += "}";
        return msg;
    }
}  // namespace Notification