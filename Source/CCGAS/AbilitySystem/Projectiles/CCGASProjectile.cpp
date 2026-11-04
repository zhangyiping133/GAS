// Copyright CCGAS. All Rights Reserved.

#include "CCGASProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GameplayCue_Types.h"
#include "AbilitySystem/Attributes/CCGASAttributeSet.h"
#include "Player/CCGASPlayerState.h"

ACCGASProjectile::ACCGASProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;             // 网络复制 — 服务器生成后同步到所有客户端
	SetReplicateMovement(true);     // 复制移动，让客户端看到投射物飞行轨迹

	// === 碰撞组件 ===
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(50.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);   // 撞墙
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);  // 撞动态物体
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);          // 撞角色
	RootComponent = CollisionComp;

	CollisionComp->OnComponentHit.AddDynamic(this, &ACCGASProjectile::OnProjectileHit);

	// === 投射物移动组件 — UE 内置，处理匀速直线运动 ===
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = FlightSpeed;
	ProjectileMovement->MaxSpeed = FlightSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;  // 投射物朝向速度方向
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;     // 无重力

	// === 粒子特效组件 ===
	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	ParticleComp->SetupAttachment(CollisionComp);
	ParticleComp->SetAutoActivate(true);

	// 超时自动销毁（防止飞行未命中时永久残留）
	InitialLifeSpan = 5.f;
}

void ACCGASProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 设置飞行尾迹粒子效果
	if (ParticleComp && FlightEffect)
	{
		ParticleComp->SetTemplate(FlightEffect);
		ParticleComp->SetRelativeScale3D(FVector(2.0f));
		ParticleComp->ActivateSystem();
	}

	// 忽略发射者的碰撞 — 防止火球刚生成就打到自己
	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

void ACCGASProjectile::InitializeProjectile(const FVector& InTargetLocation, AActor* InTargetActor)
{
	TargetLocation = InTargetLocation;
	TargetActor = InTargetActor;

	// 计算朝向目标的归一化方向向量，设置移动速度
	const FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * FlightSpeed;
	}
}

void ACCGASProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 忽略自身（与发射者碰撞不处理）
	if (OtherActor == GetInstigator())
	{
		return;
	}

	// 只有服务器权威结算碰撞与伤害；客户端投射物是模拟代理，不参与结算
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Projectile hit: %s"), *OtherActor->GetName());

	// === 1. 对目标应用伤害（服务器权威）===
	bool bAppliedDamage = false;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OtherActor))
	{
		UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
		if (TargetASC && DamageEffectClass)
		{
			// 创建 GameplayEffect Context 和 Spec
			FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle Spec = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				// SetByCallerMagnitude: 允许在运行时动态设置伤害数值
				// 对应 GameplayEffect 中设置的 SetByCaller 修饰符
				Spec.Data->SetSetByCallerMagnitude(
					FGameplayTag::RequestGameplayTag(FName("Data.Damage")), DamageAmount);
				TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				bAppliedDamage = true;
			}

			// 击杀判定：目标血量归零则击杀者 +1
			if (bAppliedDamage)
			{
				if (const UCCGASAttributeSet* TargetAttrs = TargetASC->GetSet<UCCGASAttributeSet>())
				{
					if (TargetAttrs->GetHealth() <= 0.f)
					{
						if (APawn* InstigatorPawn = GetInstigator())
						{
							if (ACCGASPlayerState* PS = InstigatorPawn->GetPlayerState<ACCGASPlayerState>())
							{
								PS->AddKill();
							}
						}
					}
				}
			}
		}
	}

	// === 2. 广播命中表现给所有客户端 ===
	MulticastPlayImpact(FVector_NetQuantize(Hit.ImpactPoint), DamageAmount, bAppliedDamage);

	// 自销毁
	Destroy();
}

void ACCGASProjectile::MulticastPlayImpact_Implementation(FVector_NetQuantize ImpactLocation, float Damage, bool bShowDamageNumber)
{
	// 击中粒子特效
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, ImpactLocation,
			FRotator::ZeroRotator, FVector(1.f), true);
	}

	// 伤害数字浮窗
	if (bShowDamageNumber)
	{
		ShowDamageNumber(ImpactLocation, Damage);
	}

	// GameplayCue: GAS 的事件系统，用于播放音效/VFX 等一次性的表现效果
	if (GetInstigator())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetInstigator()))
		{
			UAbilitySystemComponent* InstigatorASC = ASI->GetAbilitySystemComponent();
			if (InstigatorASC)
			{
				FGameplayCueParameters CueParams;
				CueParams.Location = ImpactLocation;
				CueParams.EffectCauser = this;
				InstigatorASC->ExecuteGameplayCue(
					FGameplayTag::RequestGameplayTag(FName("GameplayCue.CCGAS.Fireball.Impact")), CueParams);
			}
		}
	}
}

void ACCGASProjectile::ShowDamageNumber(const FVector& Location, float Damage)
{
	if (!DamageNumberWidgetClass || !GetWorld())
	{
		return;
	}

	// 创建一个浮动伤害数字 Widget
	UUserWidget* DamageWidget = CreateWidget<UUserWidget>(GetWorld(), DamageNumberWidgetClass);
	if (!DamageWidget)
	{
		return;
	}

	// 通过名称查找 Widget 内部的 TextBlock 并设置伤害数值
	if (UTextBlock* TextBlock = Cast<UTextBlock>(DamageWidget->GetWidgetFromName(TEXT("DamageText"))))
	{
		TextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Damage)));
	}

	// 创建临时 Actor 作为 Widget 的挂载点（用于世界坐标 → 屏幕坐标转换）
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* WidgetAnchor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	if (!WidgetAnchor)
	{
		return;
	}

	// 使用 WidgetComponent 将 UMG Widget 挂载到 Actor 上，使用 Screen Space
	UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(WidgetAnchor);
	WidgetComp->SetWidget(DamageWidget);
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);  // 屏幕空间 — Widget 面向摄像机
	WidgetComp->SetDrawSize(FVector2D(200.f, 50.f));
	WidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));  // 偏移 100 单位高度
	WidgetComp->RegisterComponent();
	WidgetAnchor->SetRootComponent(WidgetComp);

	// 生命周期结束后自动销毁
	WidgetAnchor->SetLifeSpan(DamageNumberDuration);
}
