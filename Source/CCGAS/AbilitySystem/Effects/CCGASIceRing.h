// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCGASIceRing.generated.h"

class UParticleSystem;
class UParticleSystemComponent;
class UGameplayEffect;

/**
 * 冰环特效 Actor — 冰霜新星技能生成的冰霜范围效果
 *
 * 行为流程:
 *   1. 在角色位置生成粒子特效 (RingParticle)
 *   2. 立即用 SphereOverlap 检测范围内所有拥有 ASC 的目标
 *   3. 对每个目标应用伤害 GameplayEffect
 *   4. 等待 Duration 秒后停止粒子，Actor 在 InitialLifeSpan 后自销毁
 *
 * 不继承自 UGameplayEffect — 此 Actor 只负责视觉表现和范围检测逻辑
 */
UCLASS()
class CCGAS_API ACCGASIceRing : public AActor
{
	GENERATED_BODY()

public:
	ACCGASIceRing();

	/**
	 * 初始化冰环 — 设置位置、播放粒子、检测敌人
	 * @param InLocation - 冰环生成的世界位置
	 */
	void Initialize(const FVector& InLocation);

protected:
	/** 冰环粒子效果 */
	UPROPERTY(EditDefaultsOnly, Category = "IceRing")
	TObjectPtr<UParticleSystem> RingParticle;

	/** 对范围内目标施加的伤害 GameplayEffect 类 */
	UPROPERTY(EditDefaultsOnly, Category = "IceRing")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 伤害检测半径（单位） */
	UPROPERTY(EditDefaultsOnly, Category = "IceRing")
	float Radius = 500.f;

	/** 粒子持续显示时间（秒）— 之后粒子停用 */
	UPROPERTY(EditDefaultsOnly, Category = "IceRing")
	float Duration = 1.0f;

	/** 粒子缩放倍数 */
	UPROPERTY(EditDefaultsOnly, Category = "IceRing")
	float ParticleScale = 3.0f;

	/** 粒子特效组件 */
	UPROPERTY(VisibleAnywhere, Category = "IceRing")
	TObjectPtr<UParticleSystemComponent> ParticleComp;
};
