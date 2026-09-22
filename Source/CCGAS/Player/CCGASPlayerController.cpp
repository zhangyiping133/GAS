// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASPlayerController.cpp — 玩家控制器类实现
//
// 包含: 输入映射上下文注册、技能输入绑定、通过 GameplayTag 激活能力。
// ============================================================================

#include "CCGASPlayerController.h"
#include "CCGASPlayerState.h"
#include "AbilitySystemComponent.h"
#include "../AbilitySystem/CCGASGameplayTags.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

void ACCGASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 将输入映射上下文（IMC）注册到 LocalPlayer 的 EnhancedInput 子系统
	// 优先级 0 表示默认层级，多个 IMC 的优先级决定谁先处理输入
	if (UEnhancedInputLocalPlayerSubsystem* InputSystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultIMC)
		{
			InputSystem->AddMappingContext(DefaultIMC, 0);
		}
	}
}

void ACCGASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 绑定技能输入动作（IA_xxx）到对应的回调函数
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Fireball)
		{
			EnhancedInput->BindAction(IA_Fireball, ETriggerEvent::Triggered, this, &ACCGASPlayerController::OnFireballPressed);
		}
		if (IA_Dash)
		{
			EnhancedInput->BindAction(IA_Dash, ETriggerEvent::Triggered, this, &ACCGASPlayerController::OnDashPressed);
		}
		if (IA_FrostNova)
		{
			EnhancedInput->BindAction(IA_FrostNova, ETriggerEvent::Triggered, this, &ACCGASPlayerController::OnFrostNovaPressed);
		}
	}
}

// ============================================================================
// Ability Input Callbacks
//
// 所有技能回调遵循同一模式:
//   1. 从 PlayerState 获取 ASC
//   2. 构建 FGameplayTagContainer，添加对应的技能标签
//   3. 调用 TryActivateAbilitiesByTag: GAS 内置函数，查找所有匹配该标签的能力
//      并激活第一个可用的能力（消耗资源、启动冷却、执行效果等）
// ============================================================================

void ACCGASPlayerController::OnFireballPressed()
{
	ACCGASPlayerState* PS = GetPlayerState<ACCGASPlayerState>();
	if (!PS) return;
	UAbilitySystemComponent* ASC = PS->GetASC();
	if (!ASC) return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(CCGASGameplayTags::Ability_Skill_Fireball);
	ASC->TryActivateAbilitiesByTag(TagContainer);
}

void ACCGASPlayerController::OnDashPressed()
{
	ACCGASPlayerState* PS = GetPlayerState<ACCGASPlayerState>();
	if (!PS) return;
	UAbilitySystemComponent* ASC = PS->GetASC();
	if (!ASC) return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(CCGASGameplayTags::Ability_Skill_Dash);
	ASC->TryActivateAbilitiesByTag(TagContainer);
}

void ACCGASPlayerController::OnFrostNovaPressed()
{
	ACCGASPlayerState* PS = GetPlayerState<ACCGASPlayerState>();
	if (!PS) return;
	UAbilitySystemComponent* ASC = PS->GetASC();
	if (!ASC) return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(CCGASGameplayTags::Ability_Skill_FrostNova);
	ASC->TryActivateAbilitiesByTag(TagContainer);
}
