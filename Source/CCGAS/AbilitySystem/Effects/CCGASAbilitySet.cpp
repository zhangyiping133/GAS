// Copyright CCGAS. All Rights Reserved.

#include "CCGASAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

void UCCGASAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] AbilitySet - GiveToAbilitySystem: ASC is null!"));
		return;
	}

	// --- 第一步：授予所有技能 ---
	int32 AbilitiesGranted = 0;
	for (const FCCGASAbilitySet_GameplayAbility& Item : GrantedAbilities)
	{
		if (!Item.AbilityClass)
		{
			continue;
		}

		// 创建 FGameplayAbilitySpec（技能规格），包含技能类、等级、输入标签
		// INDEX_NONE 表示不绑定特定的输入 ID（后续通过 InputTag 绑定）
		// this 是 SourceObject，指向此 DataAsset
		FGameplayAbilitySpec Spec(Item.AbilityClass, Item.AbilityLevel, INDEX_NONE, this);

		// 将 InputTag 添加到 Spec 的动态标签中
		// 这样在输入处理时可以根据 Tag 找到对应的技能
		if (Item.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Item.InputTag);
		}

		// 将技能规格注册到 ASC，GiveAbility 返回一个 Handle 用于后续操作
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		AbilitiesGranted++;

		UE_LOG(LogTemp, Warning, TEXT("[CCGAS] AbilitySet - Granted ability: %s (Level %d, InputTag: %s)"),
			*Item.AbilityClass->GetName(), Item.AbilityLevel, *Item.InputTag.ToString());

		// 如果技能是"被动技能"，授予后立即激活
		// 判断方式：检查 InputTag 是否匹配被动标签 "CCGAS.Ability.Passive.BattleAura"
		static const FGameplayTag PassiveTag = FGameplayTag::RequestGameplayTag(TEXT("CCGAS.Ability.Passive.BattleAura"));
		if (Item.InputTag.MatchesTag(PassiveTag))
		{
			if (ASC->TryActivateAbility(Handle))
			{
				UE_LOG(LogTemp, Warning, TEXT("[CCGAS] AbilitySet - Auto-activated passive: %s"),
					*Item.AbilityClass->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[CCGAS] AbilitySet - Failed to auto-activate passive: %s"),
					*Item.AbilityClass->GetName());
			}
		}
	}

	// --- 第二步：应用所有常驻效果 ---
	int32 EffectsGranted = 0;
	for (const FCCGASAbilitySet_GameplayEffect& Item : GrantedEffects)
	{
		if (!Item.EffectClass)
		{
			continue;
		}

		// MakeEffectContext：创建效果上下文（包含施法者信息）
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

		// MakeOutgoingSpec：创建 GE 规格，传入类和等级
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Item.EffectClass, Item.EffectLevel, ContextHandle);
		if (SpecHandle.IsValid())
		{
			// ApplyGameplayEffectSpecToSelf：将 GE 应用到自身（常驻效果通常是作用于自身）
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			EffectsGranted++;

			UE_LOG(LogTemp, Warning, TEXT("[CCGAS] AbilitySet - Applied effect: %s (Level %.1f)"),
				*Item.EffectClass->GetName(), Item.EffectLevel);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] AbilitySet - GiveToAbilitySystem complete: %d abilities, %d effects"),
		AbilitiesGranted, EffectsGranted);
}
