// Copyright CCGAS. All Rights Reserved.

#include "CCGASAbilityTask_Dash.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCCGASAbilityTask_Dash* UCCGASAbilityTask_Dash::CreateDashTask(
	UGameplayAbility* OwningAbility,
	FVector Direction,
	float Speed,
	float Duration)
{
	// NewAbilityTask<T>: GAS 方法，在技能的内存空间中创建任务实例
	// 自动处理任务与技能的生命周期绑定
	UCCGASAbilityTask_Dash* Task = NewAbilityTask<UCCGASAbilityTask_Dash>(OwningAbility);
	if (Task)
	{
		Task->Direction = Direction;
		Task->Speed = Speed;
		Task->Duration = Duration;
	}
	return Task;
}

void UCCGASAbilityTask_Dash::Activate()
{
	Super::Activate();

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Dash START - Direction: %s, Speed: %f"), *Direction.ToString(), Speed);

	if (!Ability)
	{
		EndTask();
		return;
	}

	AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
	Character = Cast<ACharacter>(Avatar);

	if (!Character.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Dash - No valid Character!"));
		EndTask();
		return;
	}

	// 关闭刹车摩擦 — 让角色在冲刺时保持速度不衰减
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (Movement)
	{
		Movement->BrakingFrictionFactor = 0.f;
	}

	// 使用 LaunchCharacter 推动角色 — 这是 UE 推荐的安全移动方式
	// 相比直接设置 Velocity，LaunchCharacter 能与 CharacterMovementComponent
	// 正确协作（包括碰撞响应和网络同步）
	// 参数: Velocity, bOverrideXY, bOverrideZ
	FVector LaunchVelocity = Direction * Speed;
	Character->LaunchCharacter(LaunchVelocity, true, true);

	StartTime = GetWorld()->GetTimeSeconds();
}

void UCCGASAbilityTask_Dash::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!Character.IsValid())
	{
		EndTask();
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - StartTime;
	if (Elapsed >= Duration)
	{
		// 冲刺结束 — 恢复移动参数
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			Movement->BrakingFrictionFactor = 2.f;  // 恢复默认摩擦
			Movement->StopMovementImmediately();     // 立即停止惯性移动
		}

		UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Dash END"));
		EndTask();
	}
}
