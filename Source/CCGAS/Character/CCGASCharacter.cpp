// Copyright CCGAS. All Rights Reserved.

// ============================================================================
// CCGASCharacter.cpp — 玩家角色类实现
//
// 包含: 角色构造、ASC 缓存初始化、技能授予、输入绑定、回血效果应用。
// ============================================================================

#include "CCGASCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/CCGASAbilitySet.h"
#include "GameplayEffect.h"
#include "UI/CCGASStatusWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Player/CCGASPlayerState.h"
#include "AbilitySystem/Attributes/CCGASAttributeSet.h"

ACCGASCharacter::ACCGASCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// --- SpringArm 摄像机悬臂 ---
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	// 略微俯视角度
	FRotator SpringArmRotation = SpringArm->GetRelativeRotation();
	SpringArmRotation.Pitch = -20.f;
	SpringArm->SetRelativeRotation(SpringArmRotation);

	// --- Camera ---
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// 将骨骼网格体位置对齐到胶囊体底部
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// --- 第三人称旋转设置 ---
	// 不随 Controller 旋转偏航/俯仰/翻滚（由移动方向控制朝向）
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 角色朝向跟随移动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

// ============================================================================
// IAbilitySystemInterface
// GAS 要求实现此接口，以便 ASC 能通过 AvatarActor 找到 Owner。
// 这里从 PlayerState 获取 ASC，遵循 GAS 推荐的架构。
// ============================================================================
UAbilitySystemComponent* ACCGASCharacter::GetAbilitySystemComponent() const
{
	if (const ACCGASPlayerState* PS = GetPlayerState<ACCGASPlayerState>())
	{
		return PS->GetASC();
	}
	return nullptr;
}

// ============================================================================
// Lifecycle
// ============================================================================
void ACCGASCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 初始化 ASC 并执行"ASC 就绪后"的一次性初始化。
	// 注意：BeginPlay 时角色往往还没被 Controller 接管、PlayerState 尚未就绪，
	// 因此授技/回血/HUD 统一放到 InitializeAbilitySystem 里，并会在
	// PossessedBy / OnRep_PlayerState 中再次触发（幂等）。
	InitializeAbilitySystem();
}

/**
 * PossessedBy: 服务器端调用。
 * 当 Controller 控制此 Pawn 时，重新初始化 ASC 信息（确保 Owner 和 Avatar 正确关联）。
 */
void ACCGASCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController)
	{
		InitializeAbilitySystem();
	}
}

/**
 * OnRep_PlayerState: 客户端专用。
 * PlayerState 复制完成后，初始化 ASC 信息以建立客户端 GAS 绑定。
 */
void ACCGASCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 客户端在 PlayerState 复制完成后初始化 ASC
	InitializeAbilitySystem();
}

// ============================================================================
// Input Setup — 绑定 EnhancedInput 动作到回调函数
// ============================================================================
void ACCGASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Move)
		{
			EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ACCGASCharacter::Input_Move);
		}
		if (IA_Look)
		{
			EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ACCGASCharacter::Input_Look);
		}
	}
}

// ============================================================================
// Internal — 初始化 ASC 缓存
// InitAbilityActorInfo: GAS 内置函数，建立 Owner（持有 ASC 的对象）
// 和 Avatar（表现层角色）之间的双向引用。必须调用后 ASC 才能正常工作。
// ============================================================================
void ACCGASCharacter::InitAbilityActorInfo()
{
	ACCGASPlayerState* PS = GetPlayerState<ACCGASPlayerState>();
	if (!PS)
	{
		return;
	}

	ASC = PS->GetASC();
	AttributeSet = PS->GetAttributeSet();

	if (ASC)
	{
		// Owner = PlayerState, Avatar = this (Character)
		ASC->InitAbilityActorInfo(PS, this);
	}
}

// ============================================================================
// InitializeAbilitySystem — ASC 就绪后的一次性初始化
//
// 授技能、回血 GE、HUD 都不应放在 BeginPlay 里做：BeginPlay 时角色往往
// 还没被 Controller 接管，GetPlayerState() 返回 null，ASC 拿不到，导致
// 授技/HUD 被跳过且不再重试（联机时客户端玩家无法放技能、没有血条）。
// 正确做法：等 ASC 真正可用后，在服务器（PossessedBy）和客户端
// （OnRep_PlayerState）分别触发。靠 bAbilitiesGranted / bStatusWidgetCreated
// 两个标志保证每件事只做一次。
// ============================================================================
void ACCGASCharacter::InitializeAbilitySystem()
{
	InitAbilityActorInfo();

	if (!ASC)
	{
		return;  // PlayerState 尚未就绪，等 PossessedBy / OnRep_PlayerState 再触发
	}

	// --- 绑定生命值变化监听（仅一次，服务器与客户端各自绑定）---
	if (!bHealthListenerBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UCCGASAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ACCGASCharacter::OnHealthChanged);
		bHealthListenerBound = true;
	}

	// --- 服务器：授予默认技能 + 应用持续恢复（各仅一次）---
	if (HasAuthority() && !bAbilitiesGranted)
	{
		if (DefaultAbilitySet)
		{
			DefaultAbilitySet->GiveToAbilitySystem(ASC);
		}

		if (RegenEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(RegenEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Regen GE applied"));
			}
		}

		// 重生/首次生成时重置生命与法力到满值
		if (AttributeSet)
		{
			AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
			AttributeSet->SetMana(AttributeSet->GetMaxMana());
		}

		bAbilitiesGranted = true;
		UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Character - Abilities granted on server"));
	}

	// --- 本地玩家：创建并绑定 HUD 状态栏（仅一次）---
	if (!bStatusWidgetCreated && StatusWidgetClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PC->IsLocalPlayerController())
			{
				UCCGASStatusWidget* StatusWidget = CreateWidget<UCCGASStatusWidget>(PC, StatusWidgetClass);
				if (StatusWidget)
				{
					StatusWidget->AddToViewport();
					StatusWidget->BindToASC(ASC);
					bStatusWidgetCreated = true;
					UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Character - StatusWidget created and bound"));
				}
			}
		}
	}
}

// ============================================================================
// Death — 生命值归零时的死亡与重生处理
// ============================================================================
void ACCGASCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 只在服务器处理死亡；客户端属性变化来自复制，忽略
	if (HasAuthority() && !bIsDead && Data.NewValue <= 0.f)
	{
		bIsDead = true;
		HandleDeath();
	}
}

void ACCGASCharacter::HandleDeath()
{
	// 记录死亡位置，供重生使用
	const FVector DeathLocation = GetActorLocation();

	// 禁用移动与碰撞（服务器权威）
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}
	SetActorEnableCollision(false);

	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Player died: %s"), *GetName());

	// 保存控制器/世界的引用（本 Pawn 稍后会被销毁，不能捕获 this）
	APlayerController* PC = Cast<APlayerController>(GetController());
	TWeakObjectPtr<APlayerController> WeakPC(PC);
	UWorld* World = GetWorld();

	// 延迟重生：先解除控制并销毁尸体，再在死亡位置上方重生
	FTimerHandle RespawnTimer;
	World->GetTimerManager().SetTimer(RespawnTimer,
		[World, WeakPC, DeathLocation]()
		{
			if (!WeakPC.IsValid() || !World)
			{
				return;
			}

			// 1. 解除控制并销毁旧 Pawn（尸体）
			if (APawn* OldPawn = WeakPC->GetPawn())
			{
				WeakPC->UnPossess();
				OldPawn->Destroy();
			}

			// 2. 在死亡位置上方重生
			if (AGameModeBase* GM = World->GetAuthGameMode())
			{
				const FTransform SpawnTransform(FRotator::ZeroRotator, DeathLocation + FVector(0.f, 0.f, 100.f));
				GM->RestartPlayerAtTransform(WeakPC.Get(), SpawnTransform);
			}
		}, RespawnDelay, false);
}

// ============================================================================
// Input Callbacks
// ============================================================================
void ACCGASCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (Controller)
	{
		// 以控制器的偏航角为基准计算前后方向
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, Input.Y);
		AddMovementInput(Right,   Input.X);
	}
}

void ACCGASCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(Input.X);
		AddControllerPitchInput(Input.Y);
	}
}
