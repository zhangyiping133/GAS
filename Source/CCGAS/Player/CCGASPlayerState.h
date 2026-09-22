// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASPlayerState.h — 玩家状态类声明
//
// PlayerState 是 GAS 架构中持有 ASC 的推荐位置:
//   - PlayerState 在服务器和所有客户端之间自动复制
//   - 角色重生活（Respawn）时 PlayerState 保持不变，技能/属性不会丢失
//   - 角色销毁后 ASC 仍然存活，直到玩家离开服务器
//
// GAS 概念:
//   - Owner (= PlayerState): 拥有 ASC 和 AttributeSet 的逻辑所有者
//   - Avatar (= Character): 在场景中表现的角色体
//   - InitAbilityActorInfo(Owner, Avatar) 将二者关联
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CCGASPlayerState.generated.h"

class UAbilitySystemComponent;
class UCCGASAttributeSet;

/**
 * 玩家状态 — 持有 ASC 和 AttributeSet 的 GAS 核心对象。
 * 玩家角色通过此对象获取 GAS 功能。
 */
UCLASS()
class CCGAS_API ACCGASPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACCGASPlayerState();

	virtual void BeginPlay() override;

	/** 返回持有的 ASC 组件 */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|AbilitySystem")
	UAbilitySystemComponent* GetASC() const;

	/** 返回持有的属性集 */
	UFUNCTION(BlueprintCallable, Category = "CCGAS|AbilitySystem")
	UCCGASAttributeSet* GetAttributeSet() const;

protected:
	/** ASC: GAS 核心组件，处理技能执行、效果应用、属性修改、标签管理等 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CCGAS|AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/**
	 * AttributeSet: 属性集，定义游戏中的数值属性（生命、法力、攻击力等）。
	 * 不需要手动复制，ASC 会自动处理属性的网络同步。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CCGAS|AbilitySystem")
	TObjectPtr<UCCGASAttributeSet> AttributeSet;
};
