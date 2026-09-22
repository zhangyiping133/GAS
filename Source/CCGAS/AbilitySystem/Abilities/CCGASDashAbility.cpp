// Copyright CCGAS. All Rights Reserved.

#include "CCGASDashAbility.h"
#include "../../AbilitySystem/CCGASGameplayTags.h"
#include "../Tasks/CCGASAbilityTask_Dash.h"
#include "Character/CCGASCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

UCCGASDashAbility::UCCGASDashAbility()
{
	// 为技能打标签 — 用于按键绑定和 GameplayEffect 的标签匹配
	AbilityTags.AddTag(CCGASGameplayTags::Ability_Skill_Dash);
	AbilityTags.AddTag(CCGASGameplayTags::Ability_Type_Dash);

	// BlockAbilitiesWithTag: 冲刺时阻止其他冲刺技能激活（防止反复冲刺）
	// CancelAbilitiesWithTag: 冲刺时取消正在进行的其他冲刺
	BlockAbilitiesWithTag.AddTag(CCGASGameplayTags::Ability_Type_Dash);
	CancelAbilitiesWithTag.AddTag(CCGASGameplayTags::Ability_Type_Dash);

	// InstancedPerActor: 每个角色拥有此技能的一个持久实例（而非激活时临时创建）
	// 适用于冲刺这种需要维护内部状态（如冷却）的技能
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCCGASDashAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Dash - ActivateAbility START"));

	// CommitAbility: 检查蓝量 → 消耗 Mana → 触发冷却
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Dash - CommitAbility failed (cost/cooldown blocked)"));
		return;
	}

	ACharacter* Character = GetCharacterFromActorInfo();
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Dash - No Character!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 计算冲刺方向：优先使用当前输入方向，无输入时朝角色面朝方向
	FVector Direction = Character->GetLastMovementInputVector();
	if (Direction.IsNearlyZero())
	{
		Direction = Character->GetActorForwardVector();
	}
	Direction.Normalize();

	// 创建自定义冲刺任务 — 在其中通过 LaunchCharacter 推动角色
	UCCGASAbilityTask_Dash* DashTask = UCCGASAbilityTask_Dash::CreateDashTask(
		this, Direction, DashSpeed, DashDuration);

	if (!DashTask)
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Dash - Failed to create DashTask"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 生成冲刺粒子特效（位于角色位置）
	// 粒子生命周期手动管理：0.3 秒后停用，再延迟 0.5 秒后销毁组件
	if (DashEffect)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), DashEffect, Character->GetActorLocation(),
			FRotator::ZeroRotator, FVector(1.5f), false);

		if (PSC)
		{
			TWeakObjectPtr<UParticleSystemComponent> WeakPSC(PSC);
			FTimerHandle CleanupHandle;
			GetWorld()->GetTimerManager().SetTimer(CleanupHandle,
				[WeakPSC]()
				{
					if (UParticleSystemComponent* P = WeakPSC.Get())
					{
						P->DeactivateSystem();
						FTimerHandle NestedHandle;
						P->GetWorld()->GetTimerManager().SetTimer(NestedHandle,
							[WPSC = WeakPSC]()
							{
								if (UParticleSystemComponent* PP = WPSC.Get())
								{
									PP->DestroyComponent();
								}
							}, 0.5f, false);
					}
				}, 0.3f, false);
		}
	}

	// 激活冲刺任务（开始计时和移动）
	DashTask->ReadyForActivation();

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Dash - Dash activated, Direction: %s"), *Direction.ToString());

	// 立即结束技能自身 — DashTask 会独立运行并在完成后自清理
	// 技能 Spec 仍保留在 ASC 中，冷却会阻止立刻再次激活
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
