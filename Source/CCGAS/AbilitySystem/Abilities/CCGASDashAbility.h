// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CCGASGameplayAbility.h"
#include "CCGASDashAbility.generated.h"

/**
 * 冲刺技能 — 朝当前移动方向短暂爆发加速
 *
 * 技能流程:
 *   1. CommitAbility — 消耗 Mana + 启动冷却
 *   2. 通过自定义任务 CCGASAbilityTask_Dash 执行 LaunchCharacter
 *   3. Skill 立即结束，DashTask 独立控制角色移动和恢复
 *
 * 使用的 GAS 概念:
 *   - UAbilityTask: 自定义异步任务，在 Tick 中处理持续逻辑
 *   - InstancedPerActor: 每个角色持有一个技能实例（而非每次激活都新建）
 *   - BlockAbilitiesWithTag / CancelAbilitiesWithTag: 控制技能间的互斥
 */
UCLASS()
class CCGAS_API UCCGASDashAbility : public UCCGASGameplayAbility
{
	GENERATED_BODY()

public:
	UCCGASDashAbility();

	/**
	 * ActivateAbility — 冲刺技能入口
	 * 提交消耗后立即创建 DashTask 并结束自身
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 冲刺速度（单位/秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Dash")
	float DashSpeed = 1500.f;

	/** 冲刺持续时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Dash")
	float DashDuration = 0.25f;

	/** 技能消耗的法力值 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Dash")
	float ManaCost = 10.f;

	/** 冲刺时播放的粒子特效 */
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> DashEffect;
};
