// Copyright CCGAS. All Rights Reserved.

#include "CCGASGameplayTags.h"

/**
 * CCGASGameplayTags — 全局 GameplayTag 定义文件
 *
 * UE_DEFINE_GAMEPLAY_TAG 宏将 Tag 字符串注册到全局的 GameplayTag 管理器中。
 * 字符串采用层级命名（如 "CCGAS.Ability.Skill.Fireball"），
 * 点号分隔的每一级都可以作为父 Tag 进行匹配。
 * 双下划线的变量名（如 Ability_Skill_Fireball）仅用作 C++ 编译期引用。
 */
namespace CCGASGameplayTags
{
	// ============================================================================
	// Ability.Skill — 主动技能标签
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Fireball, "CCGAS.Ability.Skill.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Dash,    "CCGAS.Ability.Skill.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_FrostNova, "CCGAS.Ability.Skill.FrostNova");

	// ============================================================================
	// Ability.Passive — 被动技能标签
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Ability_Passive_BattleAura, "CCGAS.Ability.Passive.BattleAura");

	// ============================================================================
	// Ability.Type — 能力类型标签
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Dash, "CCGAS.Ability.Type.Dash");

	// ============================================================================
	// State.Debuff — 负面状态标签
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(State_Debuff_Frozen, "CCGAS.State.Debuff.Frozen");

	// ============================================================================
	// State.Buff — 正面状态标签
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(State_Buff_BattleAura, "CCGAS.State.Buff.BattleAura");

	// ============================================================================
	// Cooldown — 冷却标签
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Fireball,  "CCGAS.Cooldown.Fireball");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Dash,      "CCGAS.Cooldown.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_FrostNova, "CCGAS.Cooldown.FrostNova");

	// ============================================================================
	// GameplayCue — 表现层通知标签
	// 注意：GameplayCue 的 Tag 命名必须以 "GameplayCue." 开头，
	// 这是 GAS 的约定，系统会据此自动查找对应的 Cue 通知类。
	// ============================================================================
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Fireball_Impact, "GameplayCue.CCGAS.Fireball.Impact");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_FrostNova_Area,  "GameplayCue.CCGAS.FrostNova.Area");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Dash_Active,     "GameplayCue.CCGAS.Dash.Active");
}
