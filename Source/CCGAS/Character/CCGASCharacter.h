// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASCharacter.h — 玩家角色类声明
//
// 职责:
//   1. 玩家的第三人称角色，挂载 SpringArm 和 Camera 组件
//   2. 实现 IAbilitySystemInterface，将 GetAbilitySystemComponent()
//      委托给 PlayerState，遵循 GAS 推荐架构: ASC 由 PlayerState 持有，
//      角色作为 Avatar
//   3. 使用 EnhancedInput 处理移动/视角输入
//   4. 游戏开始时通过 DefaultAbilitySet 授予技能，绑定 HUD
//
// GAS 架构要点:
//   - PlayerState (Owner) 拥有 ASC + AttributeSet，保证跨服务器/客户端同步
//   - Character (Avatar) 是表现层，负责动画、输入、物理
//   - InitAbilityActorInfo(Owner, Avatar) 建立二者的双向引用
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CCGASCharacter.generated.h"

class UAbilitySystemComponent;
class UCCGASAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCCGASAbilitySet;
class UCCGASStatusWidget;
class UGameplayEffect;

/**
 * 玩家角色类 — 持有 ASC 的间接引用（实际 ASC 在 PlayerState 中），
 * 负责第三人称移动/视角和技能 HUD 的初始化。
 */
UCLASS()
class CCGAS_API ACCGASCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACCGASCharacter();

	// ========================================================================
	// IAbilitySystemInterface — GAS 要求角色实现此接口以暴露 ASC
	// ========================================================================
	/** 返回 ASC 指针，实际从 PlayerState 获取 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ========================================================================
	// AActor / APawn Lifecycle
	// ========================================================================
	virtual void BeginPlay() override;

	/** PossessedBy: 服务器端在 Controller 控制此 Pawn 时调用，初始化 ASC */
	virtual void PossessedBy(AController* NewController) override;

	/** OnRep_PlayerState: 客户端在 PlayerState 同步完成后调用，初始化 ASC */
	virtual void OnRep_PlayerState() override;

	/** 绑定 EnhancedInput 输入动作到回调函数 */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ========================================================================
	// Cached GAS pointers — 缓存的 GAS 引用，方便其他系统直接访问
	// ========================================================================
	/** 返回缓存的 AttributeSet 指针 */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|AbilitySystem")
	UCCGASAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	// ========================================================================
	// Input Callbacks
	// ========================================================================
	void Input_Move(const struct FInputActionValue& Value);
	void Input_Look(const struct FInputActionValue& Value);

	// ========================================================================
	// Internal — 初始化 ASC + AttributeSet 的缓存指针，并调用 InitAbilityActorInfo
	// ========================================================================
	void InitAbilityActorInfo();

	// ========================================================================
	// Startup Abilities — 通过 AbilitySet 数据资产批量授予初始技能
	// ========================================================================
	/** 默认技能集数据资产，在 BeginPlay 中授予 ASC */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Abilities")
	TObjectPtr<UCCGASAbilitySet> DefaultAbilitySet;

	/** HUD 状态栏控件类，只在本地玩家端创建 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCCGASStatusWidget> StatusWidgetClass;

	// ========================================================================
	// Components
	// ========================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CCGAS|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CCGAS|Components")
	TObjectPtr<UCameraComponent> Camera;

	// ========================================================================
	// Cached GAS references — 从 PlayerState 获取的缓存，避免反复查找
	// ========================================================================
	/** 指向 ASC 的缓存指针，从 PlayerState 获取 */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	/** 指向 AttributeSet 的缓存指针，从 PlayerState 获取 */
	UPROPERTY()
	TObjectPtr<UCCGASAttributeSet> AttributeSet;

	/** 防止技能重复授予的标志 */
	bool bAbilitiesGranted = false;

	// ========================================================================
	// Enhanced Input — 在蓝图或 C++ 默认值中设置
	// ========================================================================
	/** 默认输入映射上下文，BeginPlay 时注册到 EnhancedInput 子系统 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CCGAS|Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	/** 移动输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CCGAS|Input")
	TObjectPtr<UInputAction> IA_Move;

	/** 视角输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CCGAS|Input")
	TObjectPtr<UInputAction> IA_Look;

	// ========================================================================
	// Health / Mana Regen — 回血/回蓝的周期效果
	// ========================================================================
	/** 持续恢复效果的 GameplayEffect 类，BeginPlay 时应用 */
	UPROPERTY(EditDefaultsOnly, Category = "CCGAS|Regen")
	TSubclassOf<UGameplayEffect> RegenEffectClass;
};
