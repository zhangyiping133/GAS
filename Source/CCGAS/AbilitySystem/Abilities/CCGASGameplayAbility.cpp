// Copyright CCGAS. All Rights Reserved.

#include "CCGASGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/CCGASCharacter.h"

UCCGASGameplayAbility::UCCGASGameplayAbility()
{
	// InstancingPolicy: InstancedPerActor 表示每个 ASC 持有者创建一个技能实例，
	// 实例中保存冷却、状态等运行时数据，技能间互不干扰。
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// NetExecutionPolicy: LocalPredicted 表示客户端可以先预测性地执行技能，
	// 然后服务端再验证和修正。这能减少网络延迟带来的操作卡顿感。
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] GameplayAbility base class constructed"));
}

// ============================================================================
// GetASCFromActorInfo — 从技能 ActorInfo 中获取 ASC
//
// CurrentActorInfo 是 UGameplayAbility 内置的成员变量，
// 在技能激活时会自动填充，包含了技能持有者的各种信息。
// ============================================================================
UAbilitySystemComponent* UCCGASGameplayAbility::GetASCFromActorInfo() const
{
	return CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
}

// ============================================================================
// GetCharacterFromActorInfo — 从技能 ActorInfo 中获取持有者角色
//
// AvatarActor 是 ASC 绑定的"形象 Actor"，通常是 Character。
// 需要强制转换回 ACCGASCharacter。
// ============================================================================
ACCGASCharacter* UCCGASGameplayAbility::GetCharacterFromActorInfo() const
{
	if (!CurrentActorInfo)
	{
		return nullptr;
	}
	return Cast<ACCGASCharacter>(CurrentActorInfo->AvatarActor.Get());
}

// ============================================================================
// ApplyEffectToTarget — 向目标 ASC 应用一个 GameplayEffect
//
// 流程：
//   1. 用 MakeOutgoingSpec 创建一个 GE 规格（Spec），
//      传入 GE 类、等级和上下文（EffectContext）。
//   2. 调用 ApplyGameplayEffectSpecToTarget 将规格应用到目标 ASC 上。
//
// MakeOutgoingSpec 创建的是"来源规格"，包含了施加者的信息。
// ApplyGameplayEffectSpecToTarget 执行最终计算并修改目标属性。
// ============================================================================
void UCCGASGameplayAbility::ApplyEffectToTarget(UAbilitySystemComponent* Target, TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	if (!Target || !EffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetASCFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, SourceASC->MakeEffectContext());
	if (SpecHandle.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), Target);
	}
}
