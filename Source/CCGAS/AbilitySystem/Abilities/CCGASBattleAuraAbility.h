// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CCGASGameplayAbility.h"
#include "CCGASBattleAuraAbility.generated.h"

class UGameplayEffect;

/**
 * 战斗光环被动技能 — 周期性为范围内友军施加增益效果
 *
 * 技能行为:
 *   1. ActivateAbility — 启动定期计时器 (TickInterval = 2s)
 *   2. OnAuraTick — 每次触发时用 SphereOverlap 检测周围友军
 *   3. 对每个拥有 ASC 的 Actor 施加 BuffEffect（持续 BuffDuration 秒）
 *   4. EndAbility — 停止定时器（通常在角色移除技能时调用）
 *
 * 使用的 GAS 概念:
 *   - InstancedPerActor: 技能实例随角色存在而存在
 *   - LocalPredicted: 在网络环境中先本地执行再与服务端同步
 *   - ApplyEffectToTarget: 向目标 ASC 应用 GameplayEffect
 *   - 被动技能通常不调用 CommitAbility（没有消耗/冷却）
 */
UCLASS()
class CCGAS_API UCCGASBattleAuraAbility : public UCCGASGameplayAbility
{
	GENERATED_BODY()

public:
	UCCGASBattleAuraAbility();

	/**
	 * ActivateAbility — 光环激活入口
	 * 启动周期性检测计时器
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/**
	 * EndAbility — 光环结束
	 * 清理定时器，防止野指针
	 */
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** 给友军施加的增益效果 GameplayEffect 类 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|BattleAura")
	TSubclassOf<UGameplayEffect> BuffEffectClass;

	/** 光环检测半径（单位） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|BattleAura")
	float AuraRadius = 800.f;

	/** 光环检测间隔（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|BattleAura")
	float TickInterval = 2.f;

	/** 增益效果的持续时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|BattleAura")
	float BuffDuration = 2.5f;

private:
	/** 定时器回调 — 执行一次光环范围检测 */
	UFUNCTION()
	void OnAuraTick();

	/** 防止 EndAbility 重复进入的守卫标志 */
	bool bIsEnding = false;

	/** 光环检测定时器句柄 */
	FTimerHandle TickTimerHandle;
};
