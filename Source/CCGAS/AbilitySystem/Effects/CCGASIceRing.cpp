// Copyright CCGAS. All Rights Reserved.

#include "CCGASIceRing.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "TimerManager.h"

ACCGASIceRing::ACCGASIceRing()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	ParticleComp->SetupAttachment(RootComponent);

	bReplicates = false;
	PrimaryActorTick.bCanEverTick = false;

	// 设置自动销毁生命周期 = 粒子持续 + 1 秒缓冲
	InitialLifeSpan = Duration + 1.0f;
}

void ACCGASIceRing::Initialize(const FVector& InLocation)
{
	SetActorLocation(InLocation);

	// 播放冰环粒子效果
	if (ParticleComp && RingParticle)
	{
		ParticleComp->SetTemplate(RingParticle);
		ParticleComp->SetRelativeScale3D(FVector(ParticleScale));
		ParticleComp->ActivateSystem();
	}

	// === 球形范围检测 — 找出半径内的所有 Actor ===
	TArray<AActor*> OverlappedActors;
	TArray<AActor*> IgnoreActors;
	if (GetInstigator())
	{
		IgnoreActors.Add(GetInstigator());  // 忽略施法者自己（免伤）
	}

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), InLocation, Radius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		AActor::StaticClass(), IgnoreActors, OverlappedActors);

	int32 HitCount = 0;
	for (AActor* A : OverlappedActors)
	{
		// 对每个拥有 ASC 的目标施加伤害效果
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(A))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				if (DamageEffectClass)
				{
					// 直接为目标 ASC 创建并应用伤害效果
					FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
					FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Ctx);
					if (Spec.IsValid())
					{
						ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
					}
				}
				HitCount++;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] IceRing - Targets hit: %d"), HitCount);

	// 延迟后停用粒子效果 — 实现"冰环扩散后消散"的视觉效果
	FTimerHandle StopHandle;
	GetWorld()->GetTimerManager().SetTimer(StopHandle,
		[this]()
		{
			if (ParticleComp)
			{
				ParticleComp->DeactivateSystem();
			}
		}, Duration, false);
}
