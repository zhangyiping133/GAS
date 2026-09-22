// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CCGASGameplayAbility.h"
#include "CCGASFireballAbility.generated.h"

class UGameplayEffect;
class UAnimMontage;
class ACCGASProjectile;

/**
 * 火球术技能 — 向角色前方发射火球投射物
 *
 * 技能流程:
 *   1. CommitAbility — 检查蓝量 + 消耗 Mana + 启动冷却
 *   2. PlayMontageAndWait — 播放施法动画 (CastMontage)
 *   3. 动画结束后 Spawn 火球投射物 (ACCGASProjectile)
 *   4. 投射物自主飞行、碰撞、造成伤害
 *
 * 使用的 GAS 概念:
 *   - UGameplayAbility: GAS 中技能的基础类
 *   - CommitAbility: 检查并消耗技能成本 (Mana) 并触发冷却
 *   - UAbilityTask_PlayMontageAndWait: 播放蒙太奇并等待完成的异步任务
 */
UCLASS()
class CCGAS_API UCCGASFireballAbility : public UCCGASGameplayAbility
{
	GENERATED_BODY()

public:
	UCCGASFireballAbility();

	/**
	 * ActivateAbility — 技能激活时的入口
	 * GAS 在玩家按下技能键后自动调用此函数
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 动画播放完成回调 — 生成火球 */
	UFUNCTION()
	void OnMontageComplete();

	/** 动画被中断回调 — 结束技能 */
	UFUNCTION()
	void OnMontageInterrupted();

	/** 动画被取消回调 — 结束技能 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 在角色前方生成火球投射物 */
	void SpawnProjectile();

	/** 伤害效果的 GameplayEffect 类 — 应用到被击中的目标 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Fireball")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 火球投射物的 Actor 类 — 在蓝图中指定 BP 版本 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Fireball")
	TSubclassOf<ACCGASProjectile> ProjectileClass;

	/** 技能消耗的法力值 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Fireball")
	float ManaCost = 15.f;

	/** 技能冷却时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Fireball")
	float CooldownDuration = 3.f;

	/** 火球造成的伤害数值 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Fireball")
	float DamageAmount = 25.f;

	/** 施法动画蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> CastMontage;
};
