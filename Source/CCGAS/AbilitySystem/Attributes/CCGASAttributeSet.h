// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CCGASAttributeSet.generated.h"

/**
 * ATTRIBUTE_ACCESSORS — 为属性生成四个标准访问函数的宏
 *
 * 展开为：
 *   GetXxxAttribute()  — 返回 FGameplayAttribute（供 GE 系统绑定用）
 *   GetXxx()           — 获取当前值
 *   SetXxx()           — 设置当前值
 *   InitXxx()          — 初始化值（仅构造函数中用）
 *
 * 这是 GAS 项目的常见写法，省去手动编写样板代码。
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UCCGASAttributeSet — 角色核心属性集
 *
 * 管理角色的基础资源：Health（生命值）和 Mana（法力值）。
 * 每个属性有当前值和最大值，都支持网络复制。
 *
 * GAS 概念说明：
 * - UAttributeSet 是属性的容器，属性值本身由 FGameplayAttributeData 持有。
 * - 属性值修改必须通过 GameplayEffect（GE），不能直接 Set。
 * - PreAttributeChange 在 GE 修改前调用，用于 Clamp。
 * - PostGameplayEffectExecute 在 GE 执行后调用，用于最终 Clamp 和触发逻辑。
 */
UCLASS()
class CCGAS_API UCCGASAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCCGASAttributeSet();

	// ========================================================================
	// Attribute Replication — 属性复制配置
	// ========================================================================
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================================================
	// PreAttributeChange / PostGameplayEffectExecute
	// ========================================================================

	/**
	 * PreAttributeChange — 属性变化前的回调
	 * 在 GameplayEffect 即将修改属性时调用。
	 * 这里对 Health 和 Mana 做 Clamp，防止超出 [0, Max] 范围。
	 * 注意：此函数仅限值 Clamp，不要在这里触发副作用。
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * PostGameplayEffectExecute — GE 执行后的回调
	 * 在 GameplayEffect 对属性完成修改后调用。
	 * 此时属性值已被修改，可以再次 Clamp 确保数据安全，
	 * 也可以在此处触发死亡判定、UI 更新等逻辑。
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ========================================================================
	// Health — 生命值及其最大值
	// ========================================================================

	/** 当前生命值，降到 0 时角色死亡 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCCGASAttributeSet, Health);

	/** 最大生命值，Health 的上限 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UCCGASAttributeSet, MaxHealth);

	// ========================================================================
	// Mana — 法力值及其最大值
	// ========================================================================

	/** 当前法力值，释放技能时消耗 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Attributes|Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UCCGASAttributeSet, Mana);

	/** 最大法力值，Mana 的上限 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Attributes|Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UCCGASAttributeSet, MaxMana);

protected:
	// ========================================================================
	// OnRep — 网络复制回调
	// GAS 要求手动处理 OnRep 并调用 GAMEPLAYATTRIBUTE_REPNOTIFY，
	// 以确保 ASC 能同步感知到属性的复制更新。
	// ========================================================================
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana);
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);
};
