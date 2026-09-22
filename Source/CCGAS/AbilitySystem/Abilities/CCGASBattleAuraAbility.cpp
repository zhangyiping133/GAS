// Copyright CCGAS. All Rights Reserved.

#include "CCGASBattleAuraAbility.h"
#include "../../AbilitySystem/CCGASGameplayTags.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "Character/CCGASCharacter.h"

UCCGASBattleAuraAbility::UCCGASBattleAuraAbility()
{
	// 标记为被动光环技能标签
	AbilityTags.AddTag(CCGASGameplayTags::Ability_Passive_BattleAura);

	// LocalPredicted: 客户端预测执行，然后与服务端结果同步
	// 这使光环在单机和网络环境下都能工作
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// InstancedPerActor: 角色持有一个持久实例（被动技能通常是持久的）
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCCGASBattleAuraAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] BattleAura - ActivateAbility START"));

	// 被动技能：不调用 CommitAbility（光环没有消耗和冷却）
	// 启动定期检测计时器
	GetWorld()->GetTimerManager().SetTimer(
		TickTimerHandle,
		this, &UCCGASBattleAuraAbility::OnAuraTick,
		TickInterval, true);  // true = 循环定时器

	// 立即执行第一次检测，不需要等第一个 TickInterval 过去
	OnAuraTick();

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] BattleAura - Timer started, interval: %f"), TickInterval);
}

void UCCGASBattleAuraAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 防止重复进入 EndAbility（例如定时器回调中可能触发多次）
	if (bIsEnding)
	{
		return;
	}
	bIsEnding = true;

	if (GetWorld())
	{
		// 清理定时器 — 避免技能结束后定时器仍触发野指针
		GetWorld()->GetTimerManager().ClearTimer(TickTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] BattleAura - EndAbility"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCCGASBattleAuraAbility::OnAuraTick()
{
	ACharacter* Character = GetCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	FVector Origin = Character->GetActorLocation();

	TArray<AActor*> OverlappedActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);  // 不给自己加光环效果

	// 球形范围检测 — 找出半径 AuraRadius 内的所有 Actor
	// 不限定 ObjectType（检测所有类型）
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Origin,
		AuraRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		AActor::StaticClass(),
		IgnoreActors,
		OverlappedActors);

	int32 BuffedCount = 0;

	for (AActor* Actor : OverlappedActors)
	{
		// 只对实现了 AbilitySystemInterface 的 Actor 施加效果
		// 即只给拥有 GAS 组件的单位施加 Buff
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
		{
			UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
			if (TargetASC)
			{
				// ApplyEffectToTarget: UCCGASGameplayAbility 中的辅助方法
				// 创建 GameplayEffect Spec 并应用到目标 ASC
				ApplyEffectToTarget(TargetASC, BuffEffectClass, 1.f);
				BuffedCount++;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] BattleAura tick - Allies in range: %d"), BuffedCount);
}
