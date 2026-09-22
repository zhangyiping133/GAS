// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CCGASStatusWidget.generated.h"

class UAbilitySystemComponent;
class UCCGASAttributeSet;
class UTextBlock;
class UProgressBar;
struct FOnAttributeChangeData;

/**
 * 状态信息 Widget — 显示角色的 Health / Mana 数值和进度条
 *
 * 工作原理:
 *   1. BindToASC — 绑定到角色的 UAbilitySystemComponent
 *   2. 通过 AttributeSet 获取初始值并注册属性变化回调
 *   3. 属性变化时自动更新 UI（RefreshUI）
 *   4. NativeDestruct — 移除回调绑定，防止野指针
 *
 * GAS 概念:
 *   - UAbilitySystemComponent::GetGameplayAttributeValueChangeDelegate
 *     监听属性变化的委托，属性改变时自动通知
 *   - UCCGASAttributeSet: 自定义属性集，包含 Health / Mana / MaxHealth / MaxMana
 */
UCLASS()
class CCGAS_API UCCGASStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/**
	 * 绑定到 ASC — 获取初始属性值并注册变化回调
	 * @param InASC - 要绑定的 AbilitySystemComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|UI")
	void BindToASC(UAbilitySystemComponent* InASC);

	/** 血量数值文本（绑定 UMG 同名控件） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	/** 法力数值文本（绑定 UMG 同名控件） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ManaText;

	/** 血量进度条（绑定 UMG 同名控件） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	/** 法力进度条（绑定 UMG 同名控件） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> ManaBar;

protected:
	/** Health 属性变化回调 — 更新当前血量值并刷新 UI */
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	/** Mana 属性变化回调 — 更新当前法力值并刷新 UI */
	void OnManaChanged(const FOnAttributeChangeData& Data);

	/** 刷新所有 UI 控件 — 使用当前数值更新进度条和文本 */
	void RefreshUI();

	/** 绑定的 ASC（弱引用，避免循环引用） */
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	/** 绑定的 AttributeSet（弱引用） */
	TWeakObjectPtr<UCCGASAttributeSet> AttributeSet;

	/** 当前血量 */
	float CurrentHealth = 100.f;

	/** 最大血量 */
	float MaxHealth = 100.f;

	/** 当前法力值 */
	float CurrentMana = 100.f;

	/** 最大法力值 */
	float MaxMana = 100.f;
};
