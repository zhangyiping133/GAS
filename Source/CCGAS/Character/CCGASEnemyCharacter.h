// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASEnemyCharacter.h — 敌人角色类声明
//
// 与玩家角色不同，敌人角色自身持有 ASC 和 AttributeSet 组件
// （不通过 PlayerState），适用于 AI 控制的非玩家单位。
//
// GAS 架构要点:
//   敌人角色的 ASC Owner 和 Avatar 都是自身（this），简化了生命周期管理，
//   但无法利用 PlayerState 的跨回合持久化特性。
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CCGASEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UCCGASAttributeSet;
class UAnimMontage;
class UUserWidget;
struct FOnAttributeChangeData;

/**
 * 敌人角色 — ASC 和 AttributeSet 由自身直接持有（非 PlayerState 模式），
 * 用作玩家技能的 AI 目标。包含死亡动画触发和血条显示功能。
 */
UCLASS()
class CCGAS_API ACCGASEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACCGASEnemyCharacter();

	// ========================================================================
	// IAbilitySystemInterface — 返回自身持有的 ASC
	// ========================================================================
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ========================================================================
	// AActor Lifecycle
	// ========================================================================
	virtual void BeginPlay() override;

	// ========================================================================
	// GAS Components — 敌人直接持有，不依赖 PlayerState
	// ========================================================================
	/** 敌人自身的 ASC 组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CCGAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	/** 敌人自身的属性集组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CCGAS")
	TObjectPtr<UCCGASAttributeSet> AttributeSet;

	// ========================================================================
	// Config — 可在蓝图或构造函数中调整
	// ========================================================================
	/** 最大生命值（用于初始设置） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CCGAS")
	float MaxHealth = 100.f;

	// ========================================================================
	// Death — 死亡播放蒙太奇与销毁逻辑
	// ========================================================================
	/** 死亡动画蒙太奇 */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** 死亡后延迟销毁的时长 */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathDuration = 3.f;

	/** 敌人头顶血条控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "HealthDisplay")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	/** 是否已死亡（防止重复触发死亡逻辑） */
	bool bIsDead = false;

	/** 触发死亡流程（禁用碰撞、播放蒙太奇、延迟销毁） */
	UFUNCTION(BlueprintCallable, Category = "CCGAS")
	void OnDeath();

	/**
	 * 生命值变化回调 — 通过 ASC 的 AttributeChangeDelegate 绑定。
	 * 当 Health <= 0 时调用 OnDeath()。
	 */
	void CheckHealthChange(const FOnAttributeChangeData& Data);
};
