// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CCGASDamageExecution.generated.h"

/**
 * UCCGASDamageExecution — 伤害计算的执行类
 *
 * 继承 UGameplayEffectExecutionCalculation（简称 ExecCalc），
 * 是 GAS 中实现"自定义复杂计算逻辑"的方式。
 *
 * GAS 概念说明：
 * - GameplayEffectExecutionCalculation 是 GE 的"执行计算"模块。
 * - 普通的 GE 修改（如 +10 攻击力）不需要写 ExecCalc，
 *   直接在 GE 蓝图里配 Modifier 就行。
 * - 当计算逻辑复杂时（如：伤害 = 基础伤害 x 技能倍率 - 目标防御力），
 *   就需要通过 ExecCalc 来自定义计算流程。
 * - 本 ExecCalc 支持两种伤害数值的传入方式：
 *   1. DefaultDamageAmount（编辑器配的默认值）
 *   2. SetByCaller（技能动态设置的数值，覆盖默认值）
 */
UCLASS()
class CCGAS_API UCCGASDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UCCGASDamageExecution();

	/**
	 * Execute_Implementation — 执行伤害计算的核心函数
	 *
	 * @param ExecutionParams      执行参数（含源和目标 ASC、GE 规格等）
	 * @param OutExecutionOutput   输出参数（追加修改器，修改目标属性）
	 */
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	/** 默认伤害值。如果技能没用 SetByCaller 传值，就使用此默认值 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DefaultDamageAmount = 25.f;
};
