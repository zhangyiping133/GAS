// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CCGASGameplayAbility.h"
#include "CCGASFrostNovaAbility.generated.h"

class UGameplayEffect;
class UAnimMontage;
class ACCGASIceRing;

/**
 * 冰霜新星技能 — 范围 AOE 冰冻攻击
 *
 * 技能流程:
 *   1. CommitAbility — 消耗 Mana + 启动冷却（10 秒）
 *   2. PlayMontageAndWait — 播放施法动画
 *   3. 动画完成后，延迟 CastDelay 秒
 *   4. 生成冰环 (ACCGASIceRing) — 冰环检测周围敌人并造成伤害
 *
 * 与火球术的区别:
 *   - 范围技能而非单体投射物
 *   - 动画结束后有延迟才生效
 *   - 冷却时间更长 (10s vs 3s)
 */
UCLASS()
class CCGAS_API UCCGASFrostNovaAbility : public UCCGASGameplayAbility
{
	GENERATED_BODY()

public:
	UCCGASFrostNovaAbility();

	/**
	 * ActivateAbility — 冰霜新星入口
	 * 检查消耗 → 播放动画 → 延迟后生成冰环
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** 动画播放完成回调 — 启动延迟计时器 */
	UFUNCTION()
	void OnMontageComplete();

	/** 动画被取消回调 — 结束技能 */
	UFUNCTION()
	void OnMontageCancelled();

	/** 在角色位置生成冰环 Actor */
	void SpawnIceRing();

	/** 伤害效果的 GameplayEffect 类 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|FrostNova")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 冰环 Actor 类 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|FrostNova")
	TSubclassOf<ACCGASIceRing> IceRingClass;

	/** 技能消耗的法力值 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|FrostNova")
	float ManaCost = 30.f;

	/** 技能冷却时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|FrostNova")
	float CooldownDuration = 10.f;

	/** 动画结束到冰环生成的延迟时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|FrostNova")
	float CastDelay = 0.3f;

	/** 施法动画蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> CastMontage;

private:
	/** 延迟计时器句柄 — 用于 CastDelay 后的冰环生成 */
	FTimerHandle DelayTimerHandle;
};
