#include "Hooks.h"

#include "skse64_common/BranchTrampoline.h"  // g_localTrampoline, g_branchTrampoline
#include "skse64_common/Relocation.h"  // RelocAddr
#include "skse64_common/SafeWrite.h"  // SafeWrite8
#include "skse64/GameSettings.h"  // g_gameSettingCollection, Setting
#include "xbyak/xbyak.h"  // xbyak

#include <cassert>  // assert
#include <cstdint>  // uintptr_t
#include <cstdlib>  // size_t
#include <string>  // string

#include "NotificationManager.h"  // NotificationManager

#include "RE/BaseExtraList.h"  // BaseExtraList
#include "RE/BGSBaseAlias.h"  // BGSBaseAlias
#include "RE/ExtraAliasInstanceArray.h"  // ExtraAliasInstanceArray
#include "RE/InventoryEntryData.h"  // InventoryEntryData
#include "RE/Misc.h"  // DebugNotification
#include "RE/TESQuest.h"  // TESQuest


void Hook_DebugNotification(RE::InventoryEntryData* a_entryData, void* a_arg2, bool a_arg3)
{
	RE::TESQuest* quest = 0;
	if (a_entryData && a_entryData->extraList) {
		RE::ExtraAliasInstanceArray* aliasInstArr = 0;
		for (auto& xList : *a_entryData->extraList) {
			aliasInstArr = xList->GetByType<RE::ExtraAliasInstanceArray>();
			if (aliasInstArr) {
				for (auto& aliasInfo : aliasInstArr->aliasInfoArr) {
					if (aliasInfo->quest && aliasInfo->alias && aliasInfo->alias->IsQuestObject()) {
						quest = aliasInfo->quest;
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
		NotificationManager* notifManager = NotificationManager::GetSingleton();
		RE::DebugNotification(notifManager->BuildNotification(quest).c_str(), a_arg2, a_arg3);
	} else {
		Setting* sDropQuestItemWarning = (*g_gameSettingCollection)->Get("sDropQuestItemWarning");
		RE::DebugNotification(sDropQuestItemWarning->data.s, a_arg2, a_arg3);
	}
}


void InstallDropHook()
{
	constexpr std::uintptr_t TARGET_FUNC = 0x0088E470;
	constexpr std::size_t CAVE_SIZE = 7;
	constexpr std::size_t MOV_HOOK = 0x41;
	constexpr std::size_t POP_START = 0x4D;
	constexpr std::size_t JMP_HOOK = 0x55;
	constexpr std::size_t CALL_BACK = 0x107;
	constexpr UInt8 NOP = 0x90;
	RelocAddr<std::uintptr_t> funcBase(TARGET_FUNC);

	// Move InventoryEntryData into rcx
	{
		struct Patch : Xbyak::CodeGenerator
		{
			Patch(void* a_buf) : Xbyak::CodeGenerator(1024, a_buf)
			{
				mov(rcx, r14);	// r14 = InventoryEntryData*
			}
		};

		void* patchBuf = g_localTrampoline.StartAlloc();
		Patch patch(patchBuf);
		g_localTrampoline.EndAlloc(patch.getCurr());

		assert(patch.getSize() <= CAVE_SIZE);

		std::size_t i = 0;
		while (i < patch.getSize()) {
			SafeWrite8(funcBase.GetUIntPtr() + MOV_HOOK + i, patch.getCode()[i]);
			++i;
		}

		std::size_t diff = CAVE_SIZE - patch.getSize();
		while (i < CAVE_SIZE) {
			SafeWrite8(funcBase.GetUIntPtr() + MOV_HOOK + i, NOP);
			++i;
		}
	}

	// Prevent the function from popping off the stack
	for (std::size_t i = POP_START; i < JMP_HOOK; ++i) {
		SafeWrite8(funcBase.GetUIntPtr() + i, NOP);
	}

	// Detour the jump
	{
		struct Patch : Xbyak::CodeGenerator
		{
			Patch(void* a_buf, std::size_t a_callAddr, std::size_t a_retAddr) : Xbyak::CodeGenerator(1024, a_buf)
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

		void* patchBuf = g_localTrampoline.StartAlloc();
		Patch patch(patchBuf, GetFnAddr(&Hook_DebugNotification), funcBase.GetUIntPtr() + CALL_BACK);
		g_localTrampoline.EndAlloc(patch.getCurr());

		g_branchTrampoline.Write5Branch(funcBase.GetUIntPtr() + JMP_HOOK, reinterpret_cast<std::uintptr_t>(patch.getCode()));
	}

	_MESSAGE("[MESSAGE] Installed drop hook");
}


void InstallStoreHook()
{
	constexpr std::uintptr_t TARGET_FUNC = 0x0085EDB0;
	constexpr std::size_t CAVE_SIZE = 7;
	constexpr std::size_t MOV_HOOK = 0x375;
	constexpr std::size_t CALL_HOOK = 0x37C;
	constexpr UInt8 NOP = 0x90;
	RelocAddr<std::uintptr_t> funcBase(TARGET_FUNC);

	struct Patch : Xbyak::CodeGenerator
	{
		Patch(void* a_buf) : Xbyak::CodeGenerator(1024, a_buf)
		{
			mov(rcx, r12);	// r12 = InventoryEntryData*
		}
	};

	void* patchBuf = g_localTrampoline.StartAlloc();
	Patch patch(patchBuf);
	g_localTrampoline.EndAlloc(patch.getCurr());

	assert(patch.getSize() <= CAVE_SIZE);

	std::size_t i = 0;
	while (i < patch.getSize()) {
		SafeWrite8(funcBase.GetUIntPtr() + MOV_HOOK + i, patch.getCode()[i]);
		++i;
	}

	while (i < CAVE_SIZE) {
		SafeWrite8(funcBase.GetUIntPtr() + MOV_HOOK + i, NOP);
		++i;
	}

	g_branchTrampoline.Write5Call(funcBase.GetUIntPtr() + CALL_HOOK, GetFnAddr(&Hook_DebugNotification));

	_MESSAGE("[MESSAGE] Installed store hook");
}


void InstallHooks()
{
	InstallDropHook();
	InstallStoreHook();
}
