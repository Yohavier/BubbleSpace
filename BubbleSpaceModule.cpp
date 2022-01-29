// Find out your copyright notice in the Description page of Project Settings.


#include "BubbleSpaceModule.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UBubbleSpaceModule::UBubbleSpaceModule()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBubbleSpaceModule::BeginPlay()
{
	Super::BeginPlay();
	//Init the a buffer that saves the last 4 values 
	historyBuffer.Init(0, 4);
	previousActorLocation == GetOwner()->GetActorLocation();
}

// Called every frame
void UBubbleSpaceModule::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	IsActorMoving();

	if (stopFrameCounter < 16)
	{
		//Rotate over 4 frames
		frameCounter++;

		DetectAzimuthPlane();
		DetectZenithPlane();

		CalculateBubbleTargetValue();

		azimuthValues.Empty();
		zenithValues.Empty();
	}

	if (debug)
	{
		DrawDebugBubble();
	}
}

void UBubbleSpaceModule::DetectAzimuthPlane()
{
	FHitResult Hit;

	offsetRotation = GetOffsetRotation(frameCounter);

	double rayAngleSteps = 360 / azimuthRayAmount;
	for (int i = 0; i < azimuthRayAmount; i++)
	{
		if (ShootRay(GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + (offsetRotation + FRotator(0, rayAngleSteps * i, 0)).RotateVector(FVector(1, 0, 0)) * maxRayLenght, Hit))
		{
			azimuthValues.Add((GetOwner()->GetActorLocation() - Hit.Location).Size());
			if (debug)
			{
				DrawDebugRay(GetOwner()->GetActorLocation(), Hit.Location);
			}
		}
		else
		{
			azimuthValues.Add(maxRayLenght);
		}
	}
}

void UBubbleSpaceModule::DetectZenithPlane() 
{
	FHitResult Hit;
	double rayAngleSteps = 360 / zenithRayAmount;

	for (int i = 0; i < zenithRayAmount; i++)
	{
		auto startPos = GetOwner()->GetActorLocation() + FRotator(0, rayAngleSteps * i, 0).RotateVector(FVector(200, 0, 0));
		if (ShootRay(startPos, startPos + FVector(0,0,1) * maxRayLenght, Hit))
		{
			zenithValues.Add((GetOwner()->GetActorLocation() - Hit.Location).Size());
			if (debug)
			{
				DrawDebugRay(startPos, Hit.Location);
			}	
		}
		else
		{
			zenithValues.Add(maxRayLenght);
		}
	}
}

bool UBubbleSpaceModule::ShootRay(const FVector& start, const FVector& end, FHitResult& Hit)
{
	return GetWorld()->LineTraceSingleByChannel(Hit, start, end, ECollisionChannel::ECC_Visibility);
}



void UBubbleSpaceModule::CalculateBubbleTargetValue()
{
	double newTargetValue = 0;
	double biggestValue = 0;

	double azimuthAverage = 0;
	double zenithAverage = 0;

	for (auto i : azimuthValues)
	{
		azimuthAverage += i;
		if (i > biggestValue) 
		{
			biggestValue = i;
		}
	}
	azimuthAverage -= biggestValue;

	biggestValue = 0;
		
	for (auto i : zenithValues)
	{
		zenithAverage += i;
		if (i > biggestValue)
		{
			biggestValue = i;
		}
	}
	zenithAverage -= biggestValue;

	newTargetValue = (azimuthAverage / 3) * azimuthWeight + (zenithAverage / 2) * zenithWeight;

	SetHistoryBuffer(newTargetValue);
	if (bubbleTargetValue != GetSmoothHistoryBufferValue()) 
	{
		bubbleTargetValue = GetSmoothHistoryBufferValue();
		SmoothValueOverTime(true);
	}
	currentSetBubbleValue = SmoothValueOverTime(false);
	SetRTPC(currentSetBubbleValue);
}

FRotator UBubbleSpaceModule::GetOffsetRotation(int& frameCount)
{
	FRotator offset = FRotator(0, frameCount * 30, 0);
	
	if (frameCount == 4)
	{
		frameCount = 0;
	}
	else if (frameCount > 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overflowing frame counter!"));
	}

	return offset;
}

void UBubbleSpaceModule::SetHistoryBuffer(const double& value)
{
	for (int i = 1; i < historyBuffer.Num(); i++) 
	{
		historyBuffer[i - 1] = historyBuffer[i];
	}
	historyBuffer[historyBuffer.Num() - 1] = value;
}

double UBubbleSpaceModule::GetSmoothHistoryBufferValue()
{
	double addedUpValue = 0;
	double largestValue = 0;
	for (auto i : historyBuffer)
	{
		if (i > largestValue) 
		{
			largestValue = i;
		}

		addedUpValue += i;
	}
	addedUpValue -= largestValue;
	return addedUpValue / 3;
}

double UBubbleSpaceModule::SmoothValueOverTime(const bool& reset)
{
	if (reset)
	{
		currentSmoothFrameCount = 0;
	}

	if (currentSmoothFrameCount == smoothFrames)
	{
		return bubbleTargetValue;
	}

	double smoothSteps = (bubbleTargetValue - currentSetBubbleValue) / smoothFrames;
	currentSetBubbleValue += smoothSteps;
	currentSmoothFrameCount++;
	return currentSetBubbleValue;
}

void UBubbleSpaceModule::SetRTPC(double setValue)
{
	if (FMath::Abs(setValue - successfulSetRTPC) / successfulSetRTPC > 0.05)
	{
		successfulSetRTPC = setValue;
		double bubbleSpaceInMeters = setValue / 100;
		//UE_LOG(LogTemp, Warning, TEXT("%f"), setValue / 100);
		UAkGameplayStatics::SetRTPCValue(RtpcValue, bubbleSpaceInMeters, 0, GetOwner(), "");
	}
}

bool UBubbleSpaceModule::IsActorMoving()
{
	if (previousActorLocation != GetOwner()->GetActorLocation())
	{
		stopFrameCounter = 0;
		previousActorLocation = GetOwner()->GetActorLocation();
		return true;
	}
	else
	{
		if(stopFrameCounter < 17)
			stopFrameCounter++;
		return false;
	}
}


//Debug
void UBubbleSpaceModule::DrawDebugRay(const FVector& start, const FVector& end)
{
	DrawDebugLine(GetWorld(), start, end, FColor::Yellow, false);
}

void UBubbleSpaceModule::DrawDebugBubble()
{
	DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation(), currentSetBubbleValue, 50, FColor::Green, false);
}