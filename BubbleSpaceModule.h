// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AkGameplayStatics.h"
#include "../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"
#include "BubbleSpaceModule.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUBBLESPACE_API UBubbleSpaceModule : public UActorComponent
{
	GENERATED_BODY()

public:
	UBubbleSpaceModule();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	void DetectAzimuthPlane();
	void DetectZenithPlane();

	bool ShootRay(const FVector& start, const FVector& end, FHitResult& Hit);
	void DrawDebugRay(const FVector& start, const FVector& end);
	void DrawDebugBubble();
	void CalculateBubbleTargetValue();

	FRotator GetOffsetRotation(int& frameCount);

	void SetHistoryBuffer(const double& value);
	double GetSmoothHistoryBufferValue();

	double SmoothValueOverTime(const bool& reset);
	void SetRTPC(double value);

	bool IsActorMoving();

public:
	UPROPERTY(EditAnywhere)
	UAkRtpc* RtpcValue;
	
	UPROPERTY(EditAnywhere)
	int maxRayLenght = 1000;

	UPROPERTY(EditAnywhere)
	int azimuthRayAmount = 3;

	UPROPERTY(EditAnywhere)
	int zenithRayAmount = 3;

	UPROPERTY(EditAnywhere)
	double zenithWeight = .6;

	UPROPERTY(EditAnywhere)
	double azimuthWeight = .4;

	UPROPERTY(EditAnywhere)
	bool debug = false;

private:
	double bubbleTargetValue = 0;
	double currentSetBubbleValue = 0;
	double successfulSetRTPC = 0;

	TArray<double> azimuthValues;
	TArray<double> zenithValues;

	int frameCounter = 0;

	FRotator offsetRotation;

	TArray<int> historyBuffer;

	int smoothFrames = 16;
	int currentSmoothFrameCount = 0;

	int stopFrameCounter = 0;
	FVector previousActorLocation;
};
