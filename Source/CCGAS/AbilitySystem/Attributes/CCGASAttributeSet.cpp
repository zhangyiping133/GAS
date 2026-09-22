// Copyright CCGAS. All Rights Reserved.

#include "CCGASAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UCCGASAttributeSet::UCCGASAttributeSet()
{
	// 初始化默认值：满血满蓝
	InitMaxHealth(100.f);
	InitMaxMana(100.f);
	InitHealth(GetMaxHealth());
	InitMana(GetMaxMana());
}

// ============================================================================
// GetLifetimeReplicatedProps — 注册需要网络同步的属性
//
// DOREPLIFETIME_CONDITION_NOTIFY 的三个参数：
//   1. 类名
//   2. 属性名
//   3. 复制条件（COND_None = 无条件复制给所有客户端）
//   4. REPNOTIFY_Always = 即使值没变也通知 OnRep
// GAS 推荐使用 REPNOTIFY_Always，确保 ASC 能正确追踪属性变化。
// ============================================================================
void UCCGASAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCCGASAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCGASAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCGASAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCCGASAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

// ============================================================================
// OnRep — 网络复制回调
//
// GAMEPLAYATTRIBUTE_REPNOTIFY 告诉 ASC："这个属性的旧值是 OldValue，
// 新值已经通过复制到了。请更新你的内部缓存。"
// 如果不调用这个宏，ASC 将不知道属性被复制更新，可能导致判定错误。
// ============================================================================
void UCCGASAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCCGASAttributeSet, Health, OldHealth);
}

void UCCGASAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCCGASAttributeSet, MaxHealth, OldMaxHealth);
}

void UCCGASAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCCGASAttributeSet, Mana, OldMana);
}

void UCCGASAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCCGASAttributeSet, MaxMana, OldMaxMana);
}

// ============================================================================
// PreAttributeChange — GE 应用前对属性值做 Clamp
//
// 调用时机：一个 GameplayEffect 正要修改属性时。
// 这里做的是"门卫"（Guard）——在修改生效前把 NewValue 限制在合法范围内。
// 注意：此函数可能会被多次调用（比如一个 GE 修改多个属性时），
// 所以不适合在这里触发死亡判定等一次性逻辑。
// ============================================================================
void UCCGASAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

// ============================================================================
// PostGameplayEffectExecute — GE 执行后的最终处理
//
// 调用时机：GameplayEffect 已经完成了对属性的修改。
// 这里重新 Clamp 一次，确保即使 PreAttributeChange 漏掉的情况也被兜住。
// 后续可以在适当时机添加响应逻辑，比如：
// - Health <= 0 时触发角色死亡
// - 广播属性变化事件给 UI
//
// Data.EvaluatedData.Attribute 是本次 GE 实际修改的属性。
// ============================================================================
void UCCGASAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] AttributeSet - PostGameplayEffectExecute: Health=%f, Mana=%f"),
		Health.GetCurrentValue(), Mana.GetCurrentValue());
}
