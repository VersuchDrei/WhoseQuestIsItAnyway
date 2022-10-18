#include "Hooks.h"

#include "Notification.h"

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include <xbyak/xbyak.h>

namespace Hooks {
    namespace {
        void Hook_DebugNotification(RE::InventoryEntryData* a_entryData, const char* a_soundToPlay,
                                    bool a_cancelIfAlreadyQueued) {
            const RE::TESQuest* quest = 0;
            if (a_entryData && a_entryData->extraLists) {
                for (auto& xList : *a_entryData->extraLists) {
                    auto xAliasInstArr = xList->GetByType<RE::ExtraAliasInstanceArray>();
                    if (xAliasInstArr) {
                        for (auto& instance : xAliasInstArr->aliases) {
                            if (instance->quest && instance->alias && instance->alias->IsQuestObject()) {
                                quest = instance->quest;
                                break;
                            }
                        }
                    }
                    if (quest) {
                        break;
                    }
                }
            }

            if (quest) {
                RE::DebugNotification(Notification::Build(*quest).c_str(), a_soundToPlay, a_cancelIfAlreadyQueued);
            } else {
                static auto dropQuestItemWarning =
                    RE::GameSettingCollection::GetSingleton()->GetSetting("sDropQuestItemWarning");
                RE::DebugNotification(dropQuestItemWarning->GetString(), a_soundToPlay, a_cancelIfAlreadyQueued);
            }
        }

        void InstallDropHook() {
            bool isSE = REL::Module::GetRuntime() != REL::Module::Runtime::AE;

            const std::size_t MOV_START = isSE ? 0x41 : 0x3D;
            const std::size_t MOV_END = isSE ? 0x48 : 0x44;
            const std::size_t POP_START = isSE ? 0x4D : 0x49;
            const std::size_t JMP_HOOK = isSE ? 0x55 : 0x54;
            const std::size_t CALL_BACK = isSE ? 0x107 : 0x108;

            REL::Relocation<std::uintptr_t> root{RELOCATION_ID(50978, 51857)};

            // Move InventoryEntryData into rcx
            {
                struct Patch : public Xbyak::CodeGenerator {
                    Patch() {
                        mov(rcx, r14);  // r14 = InventoryEntryData*
                    }
                };

                Patch p;
                p.ready();

                assert(p.getSize() <= MOV_END - MOV_START);
                REL::safe_fill(root.address() + MOV_START, REL::NOP, MOV_END - MOV_START);
                REL::safe_write(root.address() + MOV_START, std::span{p.getCode<const std::byte*>(), p.getSize()});
            }

            // Prevent the function from popping off the stack
            REL::safe_fill(root.address() + POP_START, REL::NOP, JMP_HOOK - POP_START);

            // Detour the jump
            {
                struct Patch : public Xbyak::CodeGenerator {
                    Patch(std::size_t a_callAddr, std::size_t a_retAddr) {
                        Xbyak::Label callLbl;
                        Xbyak::Label retLbl;

                        call(ptr[rip + callLbl]);
                        jmp(ptr[rip + retLbl]);

                        L(callLbl);
                        dq(a_callAddr);

                        L(retLbl);
                        dq(a_retAddr);
                    }
                };

                Patch p(reinterpret_cast<std::uintptr_t>(Hook_DebugNotification), root.address() + CALL_BACK);
                p.ready();

                auto& trampoline = SKSE::GetTrampoline();
                trampoline.write_branch<5>(root.address() + JMP_HOOK, trampoline.allocate(p));
            }

            logger::debug("installed drop hook"sv);
        }

        void InstallStoreHook() {
            bool isSE = REL::Module::GetRuntime() != REL::Module::Runtime::AE;

            const std::size_t MOV_HOOK = isSE? 0x375 : 0x3A4;
            const std::size_t CALL_HOOK = isSE ? 0x37C : 0x3AB;

            REL::Relocation<std::uintptr_t> root{RELOCATION_ID(50212, 51141)};

            struct Patch : public Xbyak::CodeGenerator {
                Patch() {
                    mov(rcx, r12);  // r12 = InventoryEntryData*
                }
            };

            Patch p;
            p.ready();

            assert(p.getSize() <= CALL_HOOK - MOV_HOOK);
            REL::safe_fill(root.address() + MOV_HOOK, REL::NOP, CALL_HOOK - MOV_HOOK);
            REL::safe_write(root.address() + MOV_HOOK, std::span{p.getCode<const std::byte*>(), p.getSize()});

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_call<5>(root.address() + CALL_HOOK,
                                     reinterpret_cast<std::uintptr_t>(Hook_DebugNotification));

            logger::debug("installed store hook"sv);
        }
    }  // namespace

    void Install() {
        InstallDropHook();
        InstallStoreHook();

        logger::debug("hooks installed"sv);
    }
}  // namespace Hooks