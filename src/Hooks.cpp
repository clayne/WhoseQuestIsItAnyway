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

namespace Hooks
{
	namespace
	{
		void Hook_DebugNotification(
			RE::InventoryEntryData* a_entryData,
			const char* a_soundToPlay,
			bool a_cancelIfAlreadyQueued)
		{
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
				static auto dropQuestItemWarning = RE::GameSettingCollection::GetSingleton()->GetSetting("sDropQuestItemWarning");
				RE::DebugNotification(dropQuestItemWarning->GetString(), a_soundToPlay, a_cancelIfAlreadyQueued);
			}
		}

		void InstallDropHook()
		{
			constexpr std::size_t MOV_HOOK = 0x3D;
			constexpr std::size_t JMP_HOOK = 0x54;
			constexpr std::size_t CALL_BACK = 0x108;

			REL::Relocation<std::uintptr_t> root{ REL::ID(51857) };

			REL::make_pattern<"48 8B 0D ?? ?? ?? ?? 41 B0 01 33 D2 4C 8B 74 24 ?? 48 83 C4 48 5D 5B E9 ?? ?? ?? ??">()
				.match_or_fail(root.address() + MOV_HOOK);

			REL::make_pattern<"48 83 C4 48 5D 5B C3">()
				.match_or_fail(root.address() + CALL_BACK);

			// Move InventoryEntryData into rcx
			{
				struct Patch :
					public Xbyak::CodeGenerator
				{
					Patch()
					{
						mov(rcx, r14);  // r14 = InventoryEntryData*
					}
				};

				Patch p;
				p.ready();

				assert(p.getSize() <= JMP_HOOK - MOV_HOOK);
				REL::safe_fill(root.address() + MOV_HOOK, REL::NOP, JMP_HOOK - MOV_HOOK);
				REL::safe_write(
					root.address() + MOV_HOOK,
					std::span{ p.getCode<const std::byte*>(), p.getSize() });
			}

			// Detour the jump
			{
				struct Patch :
					public Xbyak::CodeGenerator
				{
					Patch(std::size_t a_callAddr, std::size_t a_retAddr)
					{
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

				Patch p(
					reinterpret_cast<std::uintptr_t>(Hook_DebugNotification),
					root.address() + CALL_BACK);
				p.ready();

				auto& trampoline = SKSE::GetTrampoline();
				trampoline.write_branch<5>(
					root.address() + JMP_HOOK,
					trampoline.allocate(p));
			}

			logger::debug("installed drop hook"sv);
		}

		void InstallStoreHook()
		{
			constexpr std::size_t MOV_HOOK = 0x3A4;
			constexpr std::size_t CALL_HOOK = 0x3AB;

			REL::Relocation<std::uintptr_t> root{ REL::ID(51141) };

			REL::make_pattern<"48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? C6 44 24 ?? ?? E9 ?? ?? ?? ??">()
				.match_or_fail(root.address() + MOV_HOOK);

			struct Patch :
				public Xbyak::CodeGenerator
			{
				Patch()
				{
					mov(rcx, r12);  // r12 = InventoryEntryData*
				}
			};

			Patch p;
			p.ready();

			assert(p.getSize() <= CALL_HOOK - MOV_HOOK);
			REL::safe_fill(root.address() + MOV_HOOK, REL::NOP, CALL_HOOK - MOV_HOOK);
			REL::safe_write(
				root.address() + MOV_HOOK,
				std::span{ p.getCode<const std::byte*>(), p.getSize() });

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_call<5>(
				root.address() + CALL_HOOK,
				reinterpret_cast<std::uintptr_t>(Hook_DebugNotification));

			logger::debug("installed store hook"sv);
		}
	}

	void Install()
	{
		InstallDropHook();
		InstallStoreHook();

		logger::debug("hooks installed"sv);
	}
}
