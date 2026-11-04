// Copyright CCGAS. All Rights Reserved.

#include "CCGASIceRing.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Attributes/CCGASAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"

ACCGASIceRing::ACCGASIceRing()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	ParticleComp->SetupAttachment(RootComponent);

	bReplicates = true;  // 服务器生成后复制到客户端，客户端只做视觉表现
	PrimaryActorTick.bCanEverTick = false;

	// 设置自动销毁生命周期 = 粒子持续 + 1 秒缓冲
	InitialLifeSpan = Duration + 1.0f;
}

void ACCGASIceRing::BeginPlay()
{
	Super::BeginPlay();

	// 粒子在 BeginPlay 中激活 — 服务器生成、客户端经复制创建时都会播放，
	// 保证联机下两端都能看到冰环视觉。
	if (ParticleComp && RingParticle)
	{
		ParticleComp->SetTemplate(RingParticle);
		ParticleComp->SetRelativeScale3D(FVector(ParticleScale));
		ParticleComp->ActivateSystem();
	}
}

void ACCGASIceRing::Initialize(const FVector& InLocation)
{
	SetActorLocation(InLocation);

	// 只有服务器结算范围伤害；客户端冰环是复制代理，只负责视觉表现
	if (!HasAuthority())
	{
		return;
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
								// 直接对目标 Health 施加 -DamageAmount 的加法修改（不依赖 GE 配置）
				ASC->ApplyModToAttribute(
					UCCGASAttributeSet::GetHealthAttribute(),
					EGameplayModOp::Additive,
					-DamageAmount);
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
