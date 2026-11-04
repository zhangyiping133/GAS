// Copyright CCGAS. All Rights Reserved.

#include "CCGASStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "../AbilitySystem/Attributes/CCGASAttributeSet.h"
#include "../Player/CCGASPlayerState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

bool UCCGASStatusWidget::Initialize()
{
	return Super::Initialize();
}

void UCCGASStatusWidget::NativeDestruct()
{
	// Widget 销毁时移除属性变化回调绑定
	// 防止 ASC 仍持有对已销毁 Widget 的引用导致崩溃
	if (ASC.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UCCGASAttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UCCGASAttributeSet::GetManaAttribute()).RemoveAll(this);
	}

	if (PlayerState.IsValid())
	{
		PlayerState->OnKillsChanged.RemoveDynamic(this, &UCCGASStatusWidget::OnKillsChanged);
	}

	Super::NativeDestruct();
}

void UCCGASStatusWidget::BindToASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC.IsValid())
	{
		return;
	}

	// 获取 AttributeSet 并读取初始属性值
	AttributeSet = const_cast<UCCGASAttributeSet*>(ASC->GetSet<UCCGASAttributeSet>());
	if (AttributeSet.IsValid())
	{
		MaxHealth = AttributeSet->GetMaxHealth();
		MaxMana = AttributeSet->GetMaxMana();
		CurrentHealth = AttributeSet->GetHealth();
		CurrentMana = AttributeSet->GetMana();
	}

	// === 注册属性变化回调 ===
	// GetGameplayAttributeValueChangeDelegate: 返回一个多播委托
	// 当指定属性发生变化时触发，参数 FOnAttributeChangeData 包含新/旧值
	ASC->GetGameplayAttributeValueChangeDelegate(UCCGASAttributeSet::GetHealthAttribute()).AddUObject(this, &UCCGASStatusWidget::OnHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UCCGASAttributeSet::GetManaAttribute()).AddUObject(this, &UCCGASStatusWidget::OnManaChanged);

	// === 绑定击杀数 ===
	// ASC 挂在 PlayerState 上，Owner 即 PlayerState
	if (ACCGASPlayerState* PS = Cast<ACCGASPlayerState>(ASC->GetOwner()))
	{
		PlayerState = PS;
		PS->OnKillsChanged.AddDynamic(this, &UCCGASStatusWidget::OnKillsChanged);
		OnKillsChanged(PS->GetKills());
	}

	RefreshUI();
}

void UCCGASStatusWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 血量变化时更新本地缓存并刷新 UI
	CurrentHealth = Data.NewValue;
	RefreshUI();
}

void UCCGASStatusWidget::OnManaChanged(const FOnAttributeChangeData& Data)
{
	// 法力值变化时更新本地缓存并刷新 UI
	CurrentMana = Data.NewValue;
	RefreshUI();
}

void UCCGASStatusWidget::OnKillsChanged(int32 NewKills)
{
	if (KillsText)
	{
		KillsText->SetText(FText::FromString(FString::Printf(TEXT("击杀: %d"), NewKills)));
	}
}

void UCCGASStatusWidget::RefreshUI()
{
	if (MaxHealth > 0.f)
	{
		if (HealthBar)
		{
			HealthBar->SetPercent(CurrentHealth / MaxHealth);
		}
		if (HealthText)
		{
			HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)));
		}
	}

	if (MaxMana > 0.f)
	{
		if (ManaBar)
		{
			ManaBar->SetPercent(CurrentMana / MaxMana);
		}
		if (ManaText)
		{
			ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentMana, MaxMana)));
		}
	}
}
