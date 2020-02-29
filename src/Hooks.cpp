#include "Hooks.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "NotificationManager.h"

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Trampoline.h"


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
			constexpr UInt8 NOP = 0x90;

			REL::Offset<std::uintptr_t> funcBase = REL::ID(50978);

			// Move InventoryEntryData into rcx
			{
				struct Patch : SKSE::CodeGenerator
				{
					Patch() : SKSE::CodeGenerator(CAVE_SIZE)
					{
						mov(rcx, r14);	// r14 = InventoryEntryData*
					}
				};

				Patch patch;
				patch.finalize();

				std::size_t i = 0;
				while (i < patch.getSize()) {
					SKSE::SafeWrite8(funcBase.GetAddress() + MOV_HOOK + i, patch.getCode()[i]);
					++i;
				}

				while (i < CAVE_SIZE) {
					SKSE::SafeWrite8(funcBase.GetAddress() + MOV_HOOK + i, NOP);
					++i;
				}
			}

			// Prevent the function from popping off the stack
			for (std::size_t i = POP_START; i < JMP_HOOK; ++i) {
				SKSE::SafeWrite8(funcBase.GetAddress() + i, NOP);
			}

			// Detour the jump
			{
				struct Patch : SKSE::CodeGenerator
				{
					Patch(std::size_t a_callAddr, std::size_t a_retAddr) : SKSE::CodeGenerator()
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

				Patch patch(unrestricted_cast<std::uintptr_t>(&Hook_DebugNotification), funcBase.GetAddress() + CALL_BACK);
				patch.finalize();

				auto trampoline = SKSE::GetTrampoline();
				trampoline->Write5Branch(funcBase.GetAddress() + JMP_HOOK, patch.getCode<std::uintptr_t>());
			}

			_MESSAGE("Installed drop hook");
		}


		void InstallStoreHook()
		{
			constexpr std::size_t CAVE_SIZE = 7;
			constexpr std::size_t MOV_HOOK = 0x375;
			constexpr std::size_t CALL_HOOK = 0x37C;
			constexpr UInt8 NOP = 0x90;

			REL::Offset<std::uintptr_t> funcBase = REL::ID(50212);

			struct Patch : SKSE::CodeGenerator
			{
				Patch() : SKSE::CodeGenerator(CAVE_SIZE)
				{
					mov(rcx, r12);	// r12 = InventoryEntryData*
				}
			};

			Patch patch;
			patch.finalize();

			std::size_t i = 0;
			while (i < patch.getSize()) {
				SKSE::SafeWrite8(funcBase.GetAddress() + MOV_HOOK + i, patch.getCode()[i]);
				++i;
			}

			while (i < CAVE_SIZE) {
				SKSE::SafeWrite8(funcBase.GetAddress() + MOV_HOOK + i, NOP);
				++i;
			}

			auto trampoline = SKSE::GetTrampoline();
			trampoline->Write5Call(funcBase.GetAddress() + CALL_HOOK, unrestricted_cast<std::uintptr_t>(&Hook_DebugNotification));

			_MESSAGE("Installed store hook");
		}
	}


	void Install()
	{
		InstallDropHook();
		InstallStoreHook();

		_MESSAGE("Hooks installed");
	}
}
