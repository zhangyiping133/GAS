// Copyright CCGAS. All Rights Reserved.

#include "CCGASGameplayCue_Impact.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

bool UCCGASGameplayCue_Impact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	UE_LOG(LogTemp, Warning, TEXT("[CCGAS] GameplayCue executed: %s, Location: %s"),
		*Parameters.MatchedTagName.ToString(), *Parameters.Location.ToString());

	// 确定特效播放位置：优先使用参数中的位置，回退到目标 Actor 的位置
	const FVector LocationToUse = Parameters.Location.IsNearlyZero()
		? (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector)
		: Parameters.Location;

	// 播放粒子特效（Niagara 优先）
	if (NiagaraEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			NiagaraEffect,
			LocationToUse,
			FRotator::ZeroRotator,
			FVector(EffectScale));
	}
	else if (ParticleEffect)
	{
		// 备选：Cascade 粒子系统
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ParticleEffect,
			LocationToUse,
			FRotator::ZeroRotator,
			FVector(EffectScale),
			true);
	}

	// 播放音效
	if (SoundEffect)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundEffect, LocationToUse);
	}

	return true;
}
