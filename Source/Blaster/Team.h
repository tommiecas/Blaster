#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_RedTeam UMETA(DisplayName = "RedTeam"),
	ET_BlueTeam UMETA(DisplayName = "BlueTeam"),
	ET_RedTeam UMETA(DisplayName = "NoTeam"),
};
	class UMyInterface : public UInterface
{
	GENERATED_BODY()
};
class IMyInterface
{
	GENERATED_BODY()

public:
};