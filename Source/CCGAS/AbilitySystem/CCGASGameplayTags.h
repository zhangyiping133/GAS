// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * CCGASGameplayTags — 全局 GameplayTag 声明头文件
 *
 * GameplayTag 是 GAS 中用于标记能力、状态、冷却、特效等的轻量层级标签
 * (例如 "CCGAS.Ability.Skill.Fireball")。
 * 这里使用 UE_DECLARE_GAMEPLAY_TAG_EXTERN 做外部声明，
 * 对应的 UE_DEFINE_GAMEPLAY_TAG 定义在 .cpp 文件中。
 *
 * 所有用到这些 Tag 的地方只需 #include 本文件即可。
 */
namespace CCGASGameplayTags
{
	// ============================================================================
	// Ability.Skill — 主动技能标签
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Fireball);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_FrostNova);

	// ============================================================================
	// Ability.Passive — 被动技能标签
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Passive_BattleAura);

	// ============================================================================
	// Ability.Type — 能力类型标签（用于分类，比如区分冲刺类能力）
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Dash);

	// ============================================================================
	// State.Debuff — 负面状态标签（如冰冻）
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Debuff_Frozen);

	// ============================================================================
	// State.Buff — 正面状态标签（如战斗光环）
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Buff_BattleAura);

	// ============================================================================
	// Cooldown — 冷却标签，用于追踪各技能的冷却状态
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Fireball);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_FrostNova);

	// ============================================================================
	// GameplayCue — 视觉/音效等表现层的触发标签
	// GameplayCue 是 GAS 中"只表现、不修改属性"的轻量通知机制。
	// ============================================================================
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Fireball_Impact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_FrostNova_Area);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Dash_Active);
}
