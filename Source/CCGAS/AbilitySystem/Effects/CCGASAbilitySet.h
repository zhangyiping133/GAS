// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CCGASAbilitySet.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;

/**
 * FCCGASAbilitySet_GameplayAbility — 单个要授予的技能及其输入标签
 *
 * 存储一个技能类、等级和对应的输入标签（InputTag）。
 * InputTag 用于在蓝图中将技能绑定到按键输入。
 */
USTRUCT(BlueprintType)
struct FCCGASAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	/** 要授予的技能类 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** 授予时技能的初始等级 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Ability")
	int32 AbilityLevel = 1;

	/**
	 * 输入标签。用于技能和按键之间的映射。
	 * 例如 "Ability.Input.Slot1" 对应键盘的 1 键。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Ability")
	FGameplayTag InputTag;
};

/**
 * FCCGASAbilitySet_GameplayEffect — 单个要授予的 GameplayEffect
 *
 * 存储一个 GE 类以及等级。
 * 常用于给角色附加常驻效果（如被动属性加成）。
 */
USTRUCT(BlueprintType)
struct FCCGASAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	/** 要授予的 GameplayEffect 类 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Effect")
	TSubclassOf<UGameplayEffect> EffectClass;

	/** 授予时效果的初始等级 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Effect")
	float EffectLevel = 1.f;
};

/**
 * UCCGASAbilitySet — 能力集数据资产
 *
 * 继承 UPrimaryDataAsset，可以在编辑器中创建 DataAsset 实例。
 * 每个实例里配置一组技能和效果，一次性授予给一个 ASC。
 * 典型的用法是：每个角色职业（法师、战士等）对应一个 AbilitySet 实例。
 *
 * GAS 概念说明：
 * - 数据资产（DataAsset）是 UE 的资源，用于在编辑器中配置数据。
 * - UPrimaryDataAsset 是带标识符的 DataAsset，便于资源管理。
 */
UCLASS(BlueprintType, Category = "CCGAS")
class CCGAS_API UCCGASAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 此能力集包含的所有技能（含输入标签绑定） */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TArray<FCCGASAbilitySet_GameplayAbility> GrantedAbilities;

	/** 此能力集包含的所有常驻效果 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TArray<FCCGASAbilitySet_GameplayEffect> GrantedEffects;

	/**
	 * 将此能力集中的所有技能和效果授予指定的 ASC。
	 * @param ASC 目标 AbilitySystemComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|AbilitySet")
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC);
};
