// Copyright CCGAS. All Rights Reserved.

#include "CCGASFrostNovaAbility.h"
#include "../../AbilitySystem/CCGASGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Character/CCGASCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "../Effects/CCGASIceRing.h"

UCCGASFrostNovaAbility::UCCGASFrostNovaAbility()
{
	// 给技能贴标签，用于按键绑定和标签匹配
	AbilityTags.AddTag(CCGASGameplayTags::Ability_Skill_FrostNova);

	// 冻结状态下无法使用冰霜新星
	ActivationBlockedTags.AddTag(CCGASGameplayTags::State_Debuff_Frozen);
}

void UCCGASFrostNovaAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] FrostNova - ActivateAbility START"));

	// CommitAbility: 检查蓝量 → 消耗 Mana → 触发 10 秒冷却
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] FrostNova - CommitAbility failed (cost/cooldown blocked)"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CastMontage)
	{
		// 播放施法动画 — 不需要响应中断事件（与火球不同）
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, CastMontage, 1.f, NAME_None, false);

		MontageTask->OnCompleted.AddDynamic(this, &UCCGASFrostNovaAbility::OnMontageComplete);
		MontageTask->OnCancelled.AddDynamic(this, &UCCGASFrostNovaAbility::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 没有动画配置 → 直接触发延迟后生成冰环
		OnMontageComplete();
	}
}

void UCCGASFrostNovaAbility::OnMontageComplete()
{
	// 动画结束后设置延迟计时器 — 模拟"蓄力"效果
	GetWorld()->GetTimerManager().SetTimer(
		DelayTimerHandle,
		[this]()
		{
			SpawnIceRing();
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		},
		CastDelay, false);
}

void UCCGASFrostNovaAbility::OnMontageCancelled()
{
	// 动画被取消 → 技能失败结束
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCCGASFrostNovaAbility::SpawnIceRing()
{
	ACharacter* Character = GetCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	FVector Origin = Character->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Cast<APawn>(Character);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 在角色位置生成冰环 Actor
	// IceRing 在其 Initialize 方法中自动检测范围内敌人并造成伤害
	if (ACCGASIceRing* IceRing = GetWorld()->SpawnActor<ACCGASIceRing>(IceRingClass, Origin, FRotator::ZeroRotator, SpawnParams))
	{
		IceRing->Initialize(Origin);
		UE_LOG(LogTemp, Warning, TEXT("[CCGAS] FrostNova - IceRing spawned"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] FrostNova - Failed to spawn IceRing"));
	}
}
