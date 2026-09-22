// Copyright CCGAS. All Rights Reserved.

#include "CCGASFireballAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "../Projectiles/CCGASProjectile.h"
#include "../../AbilitySystem/CCGASGameplayTags.h"

UCCGASFireballAbility::UCCGASFireballAbility()
{
	// 给技能贴上标签，用于 GAS 的标签匹配系统（例如按键绑定通过标签查找技能）
	AbilityTags.AddTag(CCGASGameplayTags::Ability_Skill_Fireball);

	// 当角色处于"冻结"状态时，此技能无法激活
	ActivationBlockedTags.AddTag(CCGASGameplayTags::State_Debuff_Frozen);
}

void UCCGASFireballAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Fireball - ActivateAbility START"));

	// CommitAbility: GAS 内置方法 — 检查蓝量是否足够，消耗 Mana，并启动冷却
	// 如果失败（蓝量不足或在冷却中），直接结束技能
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Fireball - CommitAbility failed (cost/cooldown blocked)"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Fireball - No Avatar!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CastMontage)
	{
		// PlayMontageAndWait: GAS 异步任务 — 播放蒙太奇，等待完成/中断/取消
		// "Proxy" 后缀表示这是一个异步任务代理，支持动态绑定回调
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("CastMontage"), CastMontage, 1.0f, NAME_None, true, 1.0f, 0.0f);

		// 绑定动画的三个可能结果
		MontageTask->OnCompleted.AddDynamic(this, &UCCGASFireballAbility::OnMontageComplete);
		MontageTask->OnInterrupted.AddDynamic(this, &UCCGASFireballAbility::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UCCGASFireballAbility::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
	else
	{
		// 没有配置动画蒙太奇 — 直接生成火球（备选路径）
		SpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UCCGASFireballAbility::OnMontageComplete()
{
	// 动画正常播完 → 生成火球并结束技能
	SpawnProjectile();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UCCGASFireballAbility::OnMontageInterrupted()
{
	// 动画被更高优先级动画打断 → 技能失败结束
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCCGASFireballAbility::OnMontageCancelled()
{
	// 动画被取消（如通过蓝图取消） → 技能失败结束
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UCCGASFireballAbility::SpawnProjectile()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	// 计算火球生成位置：角色前方 100 单位，高度偏移 50 单位
	FVector Direction = Avatar->GetActorForwardVector();
	FVector SpawnLoc = Avatar->GetActorLocation() + Direction * 100.f + FVector(0.f, 0.f, 50.f);
	FVector TargetLoc = SpawnLoc + Direction * 3000.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ACCGASProjectile* Projectile = GetWorld()->SpawnActor<ACCGASProjectile>(
		ProjectileClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams))
	{
		// InitializeProjectile 设置飞行目标和速度方向
		Projectile->InitializeProjectile(TargetLoc, nullptr);
		UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Fireball - Projectile fired"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Fireball - Failed to spawn projectile"));
	}
}
