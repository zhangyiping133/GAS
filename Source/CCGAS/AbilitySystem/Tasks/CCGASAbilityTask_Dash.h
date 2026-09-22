// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CCGASAbilityTask_Dash.generated.h"

class ACharacter;

/**
 * 冲刺能力任务 — 通过 LaunchCharacter 实现角色爆发位移
 *
 * 工作原理:
 *   1. Activate — 禁用角色移动组件的刹车摩擦，执行 LaunchCharacter
 *   2. TickTask — 每帧检查是否到达持续时间，到达后恢复移动参数
 *   3. 任务自行结束，不依赖外部调用
 *
 * GAS 概念:
 *   - UAbilityTask: GAS 中的异步任务基类，支持 Tick
 *   - 技能创建任务 → 激活任务 → 任务在后台运行 → 完成后自动销毁
 *   - 用于需要在多个帧内持续更新的技能逻辑
 */
UCLASS()
class CCGAS_API UCCGASAbilityTask_Dash : public UAbilityTask
{
	GENERATED_BODY()

public:
	/**
	 * 工厂方法 — 创建冲刺任务实例
	 * NewAbilityTask<T> 是 GAS 中创建任务的推荐方式
	 *
	 * @param OwningAbility - 拥有此任务的技能
	 * @param Direction - 冲刺方向（归一化向量）
	 * @param Speed - 冲刺速度
	 * @param Duration - 冲刺持续时间
	 */
	static UCCGASAbilityTask_Dash* CreateDashTask(
		UGameplayAbility* OwningAbility,
		FVector Direction,
		float Speed,
		float Duration);

	// ========================================================================
	// UAbilityTask 重写
	// ========================================================================
	/** 任务激活 — 执行 LaunchCharacter 并记录开始时间 */
	virtual void Activate() override;

	/** 每帧更新 — 检测是否到达持续时间，到时恢复移动并结束 */
	virtual void TickTask(float DeltaTime) override;

protected:
	/** 冲刺方向（归一化） */
	FVector Direction;

	/** 冲刺速度（单位/秒） */
	float Speed;

	/** 冲刺持续时间（秒） */
	float Duration;

	/** 任务开始时的世界时间（通过 GetWorld()->GetTimeSeconds() 获取） */
	float StartTime;

	/** 被冲刺的角色 — 使用 TWeakObjectPtr 避免阻止 GC */
	TWeakObjectPtr<ACharacter> Character;
};
