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

	// 初始化 ASC / AttributeSet 缓存
	InitAbilityActorInfo();

	if (ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Character BeginPlay - ASC Owner: %s, Avatar: %s"),
			*ASC->GetOwner()->GetName(), *ASC->GetAvatarActor()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[CCGAS] Character BeginPlay - ASC is NULL!"));
	}

	// 通过 AbilitySet 授予默认技能（仅一次）
	if (ASC && DefaultAbilitySet && !bAbilitiesGranted)
	{
		DefaultAbilitySet->GiveToAbilitySystem(ASC);
		bAbilitiesGranted = true;
	}

	// 为本地玩家创建并绑定 HUD 状态栏
	if (ASC && StatusWidgetClass)
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
					UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Character - StatusWidget created and bound"));
				}
			}
		}
	}

	// 应用持续恢复效果（回血/回蓝）
	// GameplayEffectSpec 是 GE 的运行态实例，MakeOutgoingSpec 创建它，
	// ApplyGameplayEffectSpecToSelf 将其应用到自身 ASC
	if (ASC && RegenEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(RegenEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			UE_LOG(LogTemp, Warning, TEXT("[CCGAS] Regen GE applied"));
		}
	}
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
		InitAbilityActorInfo();
	}
}

/**
 * OnRep_PlayerState: 客户端专用。
 * PlayerState 复制完成后，初始化 ASC 信息以建立客户端 GAS 绑定。
 */
void ACCGASCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 客户端初始化 ASC
	InitAbilityActorInfo();
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
