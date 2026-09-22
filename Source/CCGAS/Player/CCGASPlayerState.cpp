// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASPlayerState.cpp — 玩家状态类实现
//
// 负责 ASC 和 AttributeSet 的创建、复制模式设置和初始化。
// ============================================================================

#include "CCGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CCGASAttributeSet.h"

ACCGASPlayerState::ACCGASPlayerState()
{
	// 创建 ASC 子对象（挂载在 PlayerState 上）
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// 启用 ASC 的网络复制
	AbilitySystemComponent->SetIsReplicated(true);

	/**
	 * SetReplicationMode:
	 *   Mixed: 对自身应用的效果使用 Minimal（只复制到拥有该 ASC 的客户端），
	 *          对其他人应用的效果使用 Full（复制到所有客户端）。
	 *   这是多人游戏中 PlayerState 上 ASC 的推荐模式。
	 */
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// 创建 AttributeSet 子对象
	AttributeSet = CreateDefaultSubobject<UCCGASAttributeSet>(TEXT("AttributeSet"));
}

void ACCGASPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// InitAbilityActorInfo: 初始化 Owner/ Avatar 关联。
	// 此处 Owner = PlayerState, Avatar = GetPawn()（即 Character）。
	// 此时 GetPawn() 可能为 nullptr（角色尚未生成），
	// 后续 Character::OnRep_PlayerState / PossessedBy 中会重新调用。
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	// 提高 PlayerState 的网络更新频率，使属性值同步更及时
	SetNetUpdateFrequency(100.f);

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] PlayerState BeginPlay - ASC valid: %s, Health: %f"),
		AbilitySystemComponent ? TEXT("YES") : TEXT("NO"),
		AttributeSet ? AttributeSet->GetHealth() : -1.f);
}

UAbilitySystemComponent* ACCGASPlayerState::GetASC() const
{
	return AbilitySystemComponent;
}

UCCGASAttributeSet* ACCGASPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
