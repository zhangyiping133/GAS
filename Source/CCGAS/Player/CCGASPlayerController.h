// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASPlayerController.h — 玩家控制器类声明
//
// 负责:
//   1. 注册 EnhancedInput 映射上下文（MappingContext）
//   2. 绑定技能按键输入并通过 GAS 标签激活对应能力
//
// GAS 输入方式:
//   本项目的技能激活使用 "标签模式"（TryActivateAbilitiesByTag），
//   按下按键 → 构造 Abilitiy_Skill_xxx 标签 → ASC 查找并激活匹配的能力。
//   这种方式比直接绑定 GA 类更灵活，支持改名/替换而不改代码。
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CCGASPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * 玩家控制器 — 处理技能输入和 EnhancedInput 映射上下文的注册。
 * 技能触发方式: 按键 → GameplayTag → TryActivateAbilitiesByTag。
 */
UCLASS()
class CCGAS_API ACCGASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// ========================================================================
	// AActor / APlayerController Lifecycle
	// ========================================================================
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	// ========================================================================
	// Ability Input Callbacks — 各技能按键对应的回调
	// ========================================================================
	void OnFireballPressed();
	void OnDashPressed();
	void OnFrostNovaPressed();

	// ========================================================================
	// Enhanced Input Assets — 在蓝图或 C++ 默认值中设置
	// ========================================================================
	/** 默认输入映射上下文，BeginPlay 时注册到 LocalPlayer 的 EnhancedInput 子系统 */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	/** 火球术技能输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> IA_Fireball;

	/** 冲刺技能输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> IA_Dash;

	/** 冰霜新星技能输入动作 */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> IA_FrostNova;
};
