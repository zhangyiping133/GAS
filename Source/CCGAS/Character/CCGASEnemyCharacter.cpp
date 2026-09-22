// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASEnemyCharacter.cpp — 敌人角色类实现
//
// 包含: 敌人 ASC 初始化、生命值变化监听、死亡流程。
// ============================================================================

#include "CCGASEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CCGASAttributeSet.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ACCGASEnemyCharacter::ACCGASEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// --- GAS 组件：敌人自己持有 ASC 和 AttributeSet ---
	// 与玩家不同，敌人没有 PlayerState，所以 ASC 直接挂载在 Character 上
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AttributeSet = CreateDefaultSubobject<UCCGASAttributeSet>(TEXT("AttributeSet"));

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// --- AI 自动控制：在世界中放置或生成后自动绑定 AI Controller ---
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ACCGASEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
	{
		// Owner 和 Avatar 都是自身（this），这是最简 GAS 初始化形式
		ASC->InitAbilityActorInfo(this, this);
	}

	const float Health = AttributeSet ? AttributeSet->GetHealth() : -1.f;
	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Enemy BeginPlay - ASC Owner: %s, Avatar: %s, Health: %f"),
		ASC ? *ASC->GetOwner()->GetName() : TEXT("NULL"),
		this ? *GetName() : TEXT("NULL"),
		Health);

	// 订阅 Health 属性变化委托
	// GetGameplayAttributeValueChangeDelegate: GAS 提供的属性值变化事件，
	// 当 Health 变化时自动触发 CheckHealthChange 回调。
	if (ASC && AttributeSet)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UCCGASAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ACCGASEnemyCharacter::CheckHealthChange);
	}
}

UAbilitySystemComponent* ACCGASEnemyCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

/**
 * CheckHealthChange: 生命值变化时的回调。
 * 当 Health <= 0 且尚未死亡时触发死亡流程。
 */
void ACCGASEnemyCharacter::CheckHealthChange(const FOnAttributeChangeData& Data)
{
	if (!bIsDead && Data.NewValue <= 0.f)
	{
		bIsDead = true;
		OnDeath();
	}
}

/**
 * OnDeath: 执行角色死亡逻辑。
 * 1. 碰撞关闭
 * 2. AI Controller 停止移动并解除控制
 * 3. 播放死亡蒙太奇
 * 4. 禁用移动组件
 * 5. 延迟 Destroy()（使用 Timer 避免 bCanEverTick=false 阻塞生命周期）
 */
void ACCGASEnemyCharacter::OnDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Enemy died: %s"), *GetName());

	SetActorEnableCollision(false);

	if (AController* Ctrl = GetController())
	{
		Ctrl->StopMovement();
		Ctrl->UnPossess();
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	// 使用 Timer 延迟销毁，避免直接 SetLifeSpan 被 bCanEverTick=false 阻塞的问题
	// TWeakObjectPtr 防止 Lambda 中访问已销毁的 this
	FTimerHandle DeathTimer;
	TWeakObjectPtr<ACCGASEnemyCharacter> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(DeathTimer,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->Destroy();
			}
		}, DeathDuration, false);
}
