// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "CCGASGameplayCue_Impact.generated.h"

class UNiagaraSystem;
class USoundCue;
class UParticleSystem;

/**
 * 通用命中/范围效果的 GameplayCue — 播放 VFX 和音效的一次性通知
 *
 * GameplayCue 概念（GAS）:
 *   - 与 GameplayEffect 不同，GameplayCue 不修改属性，只做表现层
 *   - 通过标签触发（如 GameplayCue.CCGAS.Fireball.Impact）
 *   - 继承自 UGameplayCueNotify_Static（无状态，不需要 Tick）
 *   - OnExecute: 触发时执行（适用于射击命中、爆炸等一次性效果）
 *   - 可在蓝图中子类化设置具体的 GameplayCueTag
 *
 * 支持两种粒子系统:
 *   - Niagara (优先)
 *   - Cascade ParticleSystem (备选，当 NiagaraEffect 为空时使用)
 */
UCLASS(Blueprintable, Category = "CCGAS|GameplayCue")
class CCGAS_API UCCGASGameplayCue_Impact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

protected:
	/**
	 * OnExecute — GameplayCue 触发时调用
	 * 在命中/技能位置播放粒子和音效
	 */
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	/** Niagara 粒子特效（优先使用） */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> NiagaraEffect;

	/** Cascade 粒子特效（Niagara 为空时的备选） */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> ParticleEffect;

	/** 音效 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<USoundCue> SoundEffect;

	/** 特效缩放倍数 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float EffectScale = 1.f;
};
