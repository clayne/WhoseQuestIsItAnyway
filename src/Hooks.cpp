#include "Hooks.h"

#include "NotificationManager.h"
#include "RE/Skyrim.h"
#include "REL/Relocation.h"


namespace Hooks
{
	namespace
	{
		void Hook_DebugNotification(RE::InventoryEntryData* a_entryData, const char* a_soundToPlay, bool a_cancelIfAlreadyQueued)
		{
			RE::TESQuest* quest = 0;
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
				logger::debug("Found quest {}", quest->fullName);
				auto mgr = NotificationManager::GetSingleton();
				RE::DebugNotification(mgr->BuildNotification(quest).c_str(), a_soundToPlay, a_cancelIfAlreadyQueued);
			} else {
				static auto dropQuestItemWarning = RE::GameSettingCollection::GetSingleton()->GetSetting("sDropQuestItemWarning");
				RE::DebugNotification(dropQuestItemWarning->GetString(), a_soundToPlay, a_cancelIfAlreadyQueued);
			}
		}


		void InstallDropHook()
		{
			constexpr std::size_t CAVE_SIZE = 7;
			constexpr std::size_t MOV_HOOK = 0x41;
			constexpr std::size_t POP_START = 0x4D;
			constexpr std::size_t JMP_HOOK = 0x55;
			constexpr std::size_t CALL_BACK = 0x107;

			REL::Relocation<std::uintptr_t> funcBase{ REL::ID(50978) };

			// Move InventoryEntryData into rcx
			{
				struct Patch : Xbyak::CodeGenerator
				{
					Patch() :Xbyak::CodeGenerator(CAVE_SIZE)
					{
						mov(rcx, r14);	// r14 = InventoryEntryData*
					}
				};

				Patch patch;
				patch.ready();

				std::size_t i = 0;
				while (i < patch.getSize()) {
					REL::safe_write<std::uint8_t>(funcBase.address() + MOV_HOOK + i, patch.getCode()[i]);
					++i;
				}

				while (i < CAVE_SIZE) {
					REL::safe_write<std::uint8_t>(funcBase.address() + MOV_HOOK + i, REL::NOP);
					++i;
				}
			}

			// Prevent the function from popping off the stack
			for (std::size_t i = POP_START; i < JMP_HOOK; ++i) {
				REL::safe_write<std::uint8_t>(funcBase.address() + i, REL::NOP);
			}

			// Detour the jump
			{
				struct Patch : Xbyak::CodeGenerator
				{
					Patch(std::uintptr_t a_func, std::uintptr_t a_target)
					{
						Xbyak::Label funcLbl;
						Xbyak::Label returnLbl;

						call(ptr[rip + funcLbl]);  // new function
						jmp(ptr[rip + returnLbl]);

						L(funcLbl);
						dq(a_func);

						L(returnLbl);
						dq(a_target);
					}
				};

				Patch patch(reinterpret_cast<std::uintptr_t>(Hook_DebugNotification), (std::uintptr_t)(funcBase.address() + CALL_BACK));
				patch.ready();

				auto& trampoline = SKSE::GetTrampoline();
				trampoline.write_branch<5>(funcBase.address() + JMP_HOOK, trampoline.allocate(patch));
				logger::debug(fmt::format("Writing branch code at {:0X} for {:0X}", funcBase.address() + JMP_HOOK, patch.getCode<std::uintptr_t>()));

			}

			logger::info(fmt::format("Installed drop hook at {:0X} for {:0X}", funcBase.address() + JMP_HOOK, SKSE::stl::unrestricted_cast<std::uintptr_t>(&Hook_DebugNotification)));
		}


		void InstallStoreHook()
		{
			constexpr std::size_t CAVE_SIZE = 7;
			#ifndef SKYRIMVR
			constexpr std::size_t MOV_HOOK = 0x375;
			constexpr std::size_t CALL_HOOK = 0x37C;
#else
			constexpr std::size_t MOV_HOOK = 0x378;
			constexpr std::size_t CALL_HOOK = 0x37F;
			#endif

			REL::Relocation<std::uintptr_t> funcBase{ REL::ID(50212) };

			struct Patch : Xbyak::CodeGenerator
			{
				Patch(): Xbyak::CodeGenerator(CAVE_SIZE)
				{
					mov(rcx, r12);	// r12 = InventoryEntryData*
				}
			};

			Patch patch;
			patch.ready();

			std::size_t i = 0;
			while (i < patch.getSize()) {
				REL::safe_write<std::uint8_t>(funcBase.address() + MOV_HOOK + i, patch.getCode()[i]);
				++i;
			}

			while (i < CAVE_SIZE) {
				REL::safe_write<std::uint8_t>(funcBase.address() + MOV_HOOK + i, REL::NOP);
				++i;
			}

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_call<5>(funcBase.address() + CALL_HOOK, SKSE::stl::unrestricted_cast<std::uintptr_t>(&Hook_DebugNotification));

			logger::info(fmt::format("Installed store hook at {:0X} for {:0X}", funcBase.address() + CALL_HOOK, SKSE::stl::unrestricted_cast<std::uintptr_t>(&Hook_DebugNotification)));
		}
		std::uintptr_t Hook_address;
	}


	void Install()
	{
		InstallDropHook();
		InstallStoreHook();

		logger::info("Hooks installed");
	}
}
