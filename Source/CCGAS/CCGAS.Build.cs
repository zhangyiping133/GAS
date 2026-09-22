// Copyright Epic Games, Inc. All Rights Reserved.

// ============================================================================
// CCGAS.Build.cs — 模块构建配置
// UnrealBuildTool (UBT) 据此决定模块的依赖项和包含路径。
// ============================================================================

using UnrealBuildTool;

public class CCGAS : ModuleRules
{
	public CCGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		// 使用显式或共享 PCH（预编译头），加快编译速度
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// --- 公开依赖项（本模块 .h 可直接 include 的模块） ---
		// 核心 UE 模块: Core, CoreUObject, Engine, InputCore
		// EnhancedInput: UE5 新版输入系统
		// GameplayAbilities: GAS 核心（ASC、GameplayEffect、GameplayAbility）
		// GameplayTags: GAS 中通过 FGameplayTag 标识技能、状态等
		// GameplayTasks: GAS 内部使用的异步任务系统
		// Niagara: UE5 粒子系统（技能特效）
		// UMG: UI 框架（血条、技能冷却 UI）
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "Niagara", "UMG" });

		// 让编译器能在 "CCGAS/" 前缀下找到头文件
		PublicIncludePaths.AddRange(new string[] { "CCGAS" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
