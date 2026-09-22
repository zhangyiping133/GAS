// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CCGASGameplayAbility.generated.h"

class UAbilitySystemComponent;
class ACCGASCharacter;
class UGameplayEffect;

/**
 * UCCGASGameplayAbility — 所有技能的基类
 *
 * 继承 UGameplayAbility，提供项目中常用的辅助函数：
 * - 获取持有者的 ASC 和 Character
 * - 向目标应用 GameplayEffect
 *
 * GAS 概念说明：
 * - UGameplayAbility 是"技能"的载体，定义了技能激活、消耗、冷却等行为。
 * - InstancingPolicy 决定技能是否需要实例化：
 *     InstancedPerActor = 每个 ASC 持有者一个实例，保存运行时状态。
 * - NetExecutionPolicy 决定技能的网络的执行策略：
 *     LocalPredicted = 客户端本地先执行，再与服务端同步修正。
 */
UCLASS()
class CCGAS_API UCCGASGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCCGASGameplayAbility();

protected:
	// ========================================================================
	// Actor Info Helpers — 获取技能持有者相关的对象
	// ========================================================================

	/** 获取当前技能所属的 AbilitySystemComponent */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|Ability")
	UAbilitySystemComponent* GetASCFromActorInfo() const;

	/** 获取当前技能的拥有者角色（转换为 ACCGASCharacter） */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|Ability")
	ACCGASCharacter* GetCharacterFromActorInfo() const;

	// ========================================================================
	// Effect Application Helper — 向目标应用 GameplayEffect
	// ========================================================================

	/**
	 * 向指定目标 ASC 应用一个 GameplayEffect。
	 * @param Target       目标 ASC
	 * @param EffectClass  要应用的 GE 类
	 * @param Level        GE 等级（可用于缩放效果数值）
	 */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|Ability")
	void ApplyEffectToTarget(UAbilitySystemComponent* Target, TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f);
};
