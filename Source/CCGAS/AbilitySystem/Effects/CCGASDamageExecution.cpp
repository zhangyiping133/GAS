// Copyright CCGAS. All Rights Reserved.

#include "CCGASDamageExecution.h"
#include "../Attributes/CCGASAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayTags.h"

UCCGASDamageExecution::UCCGASDamageExecution()
{
	// 在构造函数中声明"需要捕获的目标属性"
	//
	// FGameplayEffectAttributeCaptureDefinition 定义了一个属性捕获：
	// - AttributeToCapture：要捕获哪个属性
	// - AttributeSource：从源（Source，即攻击者）还是目标（Target，即受伤者）捕获
	// - bSnapshot：是否在 GE 创建时（而非执行时）就捕获快照值
	//
	// 这里捕获了目标的 Health 属性（虽然本 ExecCalc 不直接读它，
	// 只是演示捕获机制的写法；实际使用中捕获防御力等属性会更典型）。
	FGameplayEffectAttributeCaptureDefinition HealthDef;
	HealthDef.AttributeToCapture = UCCGASAttributeSet::GetHealthAttribute();
	HealthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	HealthDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(HealthDef);
}

void UCCGASDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// --- 第一步：计算伤害数值 ---
	float DamageAmount = DefaultDamageAmount;

	// 尝试从 GE Spec 中获取 SetByCaller 传入的伤害值
	// SetByCaller 机制允许技能在运行时动态设定 GE 的数值，
	// 用 Tag "Data.Damage" 作为键来查找。
	// 如果技能设定了 SetByCaller 值，则覆盖默认伤害。
	const float SetByCallerValue = ExecutionParams.GetOwningSpec().GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.Damage")));
	if (SetByCallerValue > 0.f)
	{
		DamageAmount = SetByCallerValue;
	}

	// --- 第二步：将伤害输出到目标的 Health 属性 ---
	//
	// FGameplayModifierEvaluatedData 定义了一个"已计算出结果的修改器"：
	// - 第一个参数：要修改的目标属性（这里目标的 Health）
	// - 第二个参数：修改方式（Additive = 加法修改，传入负数就是扣血）
	// - 第三个参数：修改值（-DamageAmount，负值表示减少）
	//
	// AddOutputModifier 将此修改器追加到执行输出中，
	// GAS 会最终将这个修改应用到目标的属性上。
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UCCGASAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-DamageAmount));
}
