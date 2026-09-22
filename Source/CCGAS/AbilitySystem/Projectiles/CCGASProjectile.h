// Copyright CCGAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCGASProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UParticleSystem;
class UGameplayEffect;
class UUserWidget;

/**
 * 火球投射物 — 继承自 AActor，由 FireballAbility 生成
 *
 * 行为流程:
 *   1. InitializeProjectile — 设置目标方向和速度
 *   2. 自动飞行（使用 UProjectileMovementComponent）
 *   3. OnProjectileHit — 碰撞时:
 *      a) 对目标应用伤害 GameplayEffect
 *      b) 显示伤害数字 Widget
 *      c) 播放击中粒子特效
 *      d) 触发 GameplayCue（用于音效/附加 VFX）
 *      e) 自销毁
 *
 * 网络: bReplicates = false（仅本地模拟，简化网络处理）
 */
UCLASS()
class CCGAS_API ACCGASProjectile : public AActor
{
	GENERATED_BODY()

public:
	ACCGASProjectile();

	virtual void BeginPlay() override;

	/**
	 * 初始化投射物
	 * @param InTargetLocation - 目标位置（用于计算飞行方向）
	 * @param InTargetActor - 可选的目标 Actor（预留，当前未使用）
	 */
	void InitializeProjectile(const FVector& InTargetLocation, AActor* InTargetActor);

protected:
	/** 碰撞事件回调 — 命中目标后的处理逻辑 */
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** 碰撞检测组件（球形） */
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComp;

	/** UE 自带的投射物移动组件 — 处理匀速飞行 */
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 粒子特效组件 — 飞行时显示火焰尾迹 */
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UParticleSystemComponent> ParticleComp;

	/** 应用到碰撞目标的伤害 GameplayEffect 类 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 飞行时的粒子特效 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TObjectPtr<UParticleSystem> FlightEffect;

	/** 击中时的粒子特效 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TObjectPtr<UParticleSystem> ImpactEffect;

	/** 飞行速度（单位/秒） */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float FlightSpeed = 2000.f;

	/** 造成的伤害数值 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DamageAmount = 25.f;

	/** 伤害数字浮窗 Widget 类 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageDisplay")
	TSubclassOf<UUserWidget> DamageNumberWidgetClass;

	/** 伤害数字显示时长（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "DamageDisplay")
	float DamageNumberDuration = 1.5f;

	/** 在命中位置创建伤害数字浮窗 */
	void ShowDamageNumber(const FVector& Location, float Damage);

	/** 目标位置（用于计算飞行方向） */
	FVector TargetLocation;

	/** 目标 Actor（预留） */
	TWeakObjectPtr<AActor> TargetActor;
};
