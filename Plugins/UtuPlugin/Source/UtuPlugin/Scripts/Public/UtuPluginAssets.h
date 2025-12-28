// Copyright Alex Quevillon. All Rights Reserved.

#pragma once
#include "UtuPlugin/Scripts/Public/UtuPluginJson.h"
#include "Runtime/Launch/Resources/Version.h" 
#include "CoreMinimal.h"
#include "Factories/FbxMeshImportData.h"
#include "Engine/Texture.h"
#include "UtuPluginAssets.generated.h"

class FAssetToolsModule;
class UAssetImportTask;
class USceneComponent;
class UBlueprint;
class USCS_Node;
class UPackage;
class ACineCamera;
class UCineCameraComponent;
class UMaterialExpressionTextureSampleParameter2D;
class UMaterialExpressionScalarParameter;
class UMaterialExpressionVectorParameter;
class UMaterial;
class UTexture;
class UMaterialExpressionMultiply;
class UMaterialExpressionPanner;
class UMaterialExpressionTextureCoordinate;
class UMaterialExpressionComponentMask;
class UStaticMesh;
class USkeletalMesh;
class USkeletalMeshComponent;
class UTexture2D;
struct FStaticMaterial;

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuUnrealAssetType : uint8
{
	Level, Animation, StaticMesh, SkeletalMesh, Material, MaterialInstance, Texture, Blueprint
};

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuProcessingBehavior : uint8
{
	AlwaysProcess, UpdateExisting, SkipExisting, DoNotProcess
};

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuSavingBehavior : uint8
{
	SaveAllEveryXxAssets, SaveAllAtEnd, PromptAtEnd
};

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuMeshSpawnBehavior : uint8
{
	AllPrefab, StaticMeshIfAloneInPrefab, AllStaticMesh
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuFindAndReplace
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FString From = "";
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FString To = "";
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuRenameSettings 
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	EUtuUnrealAssetType AssetType = EUtuUnrealAssetType::Level;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FString Prefix = "";
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FString Suffix = "";
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FString AutoRenameDuplicatedAssetSuffix = "";
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bAutoRenameDuplicatedAssets = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TArray<FUtuFindAndReplace> FindAndReplace = TArray<FUtuFindAndReplace>();
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_AllAssets 
{
	GENERATED_USTRUCT_BODY()

public:
	// Assets
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuRenameSettings AssetRenameSettings = FUtuRenameSettings();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	EUtuProcessingBehavior ProcessingBehavior = EUtuProcessingBehavior::AlwaysProcess;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_StaticMeshes : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// All meshes
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FVector ImportLocationOffset = FVector(0.0f);
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FRotator ImportRotationOffset = FRotator(0.0f);
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float ImportScaleMultiplierIfUseFileScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float ImportScaleMultiplierIfNotUseFileScale = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float FbxExporterImportScaleMultiplierIfUseFileScale = 0.0001f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float FbxExporterImportScaleMultiplierIfNotUseFileScale = 0.0001f;

	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bConvertScene = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bForceFrontXAxis = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bConvertSceneUnit = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bBakePivotInVertex = false;

	// Static Mesh

	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bImportSeparated = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bAutoGenerateCollision = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bImportLods = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bGenerateLightmapUVs = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bRemoveDegenerates = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum EFBXNormalImportMethod> NormalImportMethod = EFBXNormalImportMethod::FBXNIM_ComputeNormals;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum EFBXNormalGenerationMethod::Type> NormalGenerationMethod = EFBXNormalGenerationMethod::BuiltIn;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bComputeWeightedNormals = true;

	// LOD
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bUtuGenerateLODs = true;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_SkeletalMeshes : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// All meshes
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FVector ImportLocationOffset = FVector(0.0f);
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FRotator ImportRotationOffset = FRotator(0.0f);
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float ImportScaleMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float FbxExporterImportScaleMultiplier = 0.0001f;

	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bConvertScene = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bForceFrontXAxis = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bConvertSceneUnit = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bTransformVertexToAbsolute = true;

	// Skeletal Mesh
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bUseT0AsRefPose = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bPreserveSmoothingGroups = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bImportMeshesInBoneHierarchy = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bImportMorphTargets = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum EFBXNormalImportMethod> NormalImportMethod = EFBXNormalImportMethod::FBXNIM_ComputeNormals;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum EFBXNormalGenerationMethod::Type> NormalGenerationMethod = EFBXNormalGenerationMethod::BuiltIn;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bComputeWeightedNormals = true;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_Animations : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// All meshes
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FVector ImportLocationOffset = FVector(0.0f);
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FRotator ImportRotationOffset = FRotator(0.0f);
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float ImportScaleMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float FbxExporterImportScaleMultiplier = 0.0001f;

	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bConvertScene = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bForceFrontXAxis = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bConvertSceneUnit = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bTransformVertexToAbsolute = true;

	// Animations
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bAnimImportMeshesInBoneHierarchy = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bUseDefaultSampleRate = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	int CustomSampleRate = 0;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bImportCustomAttribute = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bDeleteExistingCustomAttributeCurves = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bSetMaterialDriveParameterOnCustomAttribute = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bRemoveRedundantKeys = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bDeleteExistingMorphTargetCurves = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bDoNotImportCurveWithZero = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bPreserveLocalTransform = false;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_Blueprints : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// Blueprints
};

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_Scenes : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// Scenes
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	EUtuMeshSpawnBehavior MeshSpawnBehavior = EUtuMeshSpawnBehavior::StaticMeshIfAloneInPrefab;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_Lights : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// Lights
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float SkyLightIntensity = 1.5f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float LightIntensityMultiplier = 10.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float LightRangeMultiplier = 100.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float LightFalloffExponent = 2.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float LightSpotInnerConeAngle = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float LightSpotAngleMultiplier = 0.5f;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_Textures : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// Textures
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum TextureCompressionSettings> CompressionSettings = TextureCompressionSettings::TC_Default;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum TextureFilter> Filter = TextureFilter::TF_Default;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum TextureGroup> LODGroup = TextureGroup::TEXTUREGROUP_World;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool SRGB = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bFlipNormalMapGreenChannel = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	int32 MaxTextureSize = 0;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum ETextureCompressionQuality> CompressionQuality = ETextureCompressionQuality::TCQ_Default;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	TEnumAsByte<enum TextureMipGenSettings> MipGenSettings = TextureMipGenSettings::TMGS_FromTextureGroup;
};



USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_Materials : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// Materials
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bCreateMaterialInstances = true;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideTexturesPannerTime = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float TexturesPannerTime = 5.45f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideMetallicIntensityMultiplier = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float MetallicIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideSpecularIntensityMultiplier = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float SpecularIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideEmissiveIntensityMultiplier = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float EmissiveIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideNormalIntensityMultiplier = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float NormalIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideOcclusionIntensityMultiplier = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float OcclusionIntensityMultiplier = 1.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bOverrideRoughnessMultiplier = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	float RoughnessMultiplier = 1.0f;
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings_MaterialInstances : public FUtuPluginImportSettings_AllAssets
{
	GENERATED_USTRUCT_BODY()

public:
	// MaterialInstances
};



USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginImportSettings 
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	bool bDeleteInvalidAssets = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	EUtuSavingBehavior SavingBehavior = EUtuSavingBehavior::PromptAtEnd;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	int SavingIntervals = 100;

public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_StaticMeshes StaticMeshes = FUtuPluginImportSettings_StaticMeshes();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_SkeletalMeshes SkeletalMeshes = FUtuPluginImportSettings_SkeletalMeshes();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_Animations Animations = FUtuPluginImportSettings_Animations();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_Scenes Scenes = FUtuPluginImportSettings_Scenes();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_Lights Lights = FUtuPluginImportSettings_Lights();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_Materials Materials = FUtuPluginImportSettings_Materials();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_MaterialInstances MaterialInstances = FUtuPluginImportSettings_MaterialInstances();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_Textures Textures = FUtuPluginImportSettings_Textures();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	FUtuPluginImportSettings_Blueprints Blueprints = FUtuPluginImportSettings_Blueprints();
};


USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginAssetTypeProcessor {
	GENERATED_USTRUCT_BODY()
public:
	void Import(FUtuPluginJson Json, EUtuAssetType AssetType, bool executeFullImportOnSameFrame, TArray<FString> DuplicatedAssetNames);
	void BeginImport(FUtuPluginJson Json, EUtuAssetType AssetType, TArray<FString> DuplicatedAssetNames);
	bool ContinueImport();
	void CompleteImport();

	TArray<FString> ListOfDuplicatedAssetNames = TArray<FString>();

public:
	int GetAssetsNum();
	TArray<FString> FormatRelativeFilenameForUnreal(FString InRelativeFilename, EUtuUnrealAssetType AssetType); //[0] = path, [1] = name, [2] = relative filename
	TArray<FString> FormatRelativeFilenameForUnrealSeparated(FString InRelativeFilename, FString InRelativeFilenameSeparated, EUtuUnrealAssetType AssetType);

private:
	void CopyUtuAssetsInProject();
	void ProcessScene(FUtuPluginScene InUtuScene);
	void ProcessAnimation(FUtuPluginAnimation InUtuAnimation);
	void ProcessMesh(FUtuPluginMesh InUtuMesh);
	void ProcessMaterial(FUtuPluginMaterial InUtuMaterial);
	class UMaterial* GetOrCreateParentMaterial(FUtuPluginMaterial InUtuMaterial);
	void ProcessTexture(FUtuPluginTexture InUtuTexture);
	void ProcessPrefabFirstPass(FUtuPluginPrefabFirstPass InUtuPrefabFirstPass);
	void ProcessPrefabSecondPass(FUtuPluginPrefabSecondPass InUtuPrefabSecondPass);
	UAssetImportTask* BuildTask(FString InSource, TArray<FString> InAssetNames, UObject* InOptions);

	TArray<FString> StartProcessAsset(FUtuPluginAsset InUtuAsset, EUtuUnrealAssetType AssetType);
	bool DeleteInvalidAssetIfNeeded(TArray<FString> InAssetNames, UClass* InClass);

	UPackage* CreateAssetPackage(FString InRelativeFilename, bool bLoadPackage);
	void LogAssetCreateOrNot(UObject* InAsset);
	void LogAssetImportOrReimport(UObject* InAsset);
	void LogAssetImportedOrFailed(UObject* InAsset, TArray<FString> InAssetNames, FString InSourceFileFullname, FString InAssetType, TArray<FString> InPotentialCauses);

	AActor* WorldAddRootActorForSubActorsIfNeeded(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnStaticMeshActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnSkeletalMeshActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnBlueprintActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnSkyLightActor(UWorld* InAsset);
	AActor* WorldSpawnPointLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnDirectionalLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnSpotLightActor(UWorld* InAsset, FUtuPluginActor InUtuActor);
	AActor* WorldSpawnCameraActor(UWorld* InAsset, FUtuPluginActor InUtuActor);

	void BpAddRootComponent(UBlueprint* InAsset, bool bStatic);
	bool BpAddRootComponentForSubComponentsIfNeeded(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode);
	void BpAddEmptyComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddStaticMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddSkeletalMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddPointLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddDirectionalLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddSpotLightComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddCameraComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);
	void BpAddChildActorComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node*& OutComponentNode, bool bInRootCreated);

	FString BpMakeUniqueName(FString InDesiredName, TArray<FString>& InOutUsedNames);
	UStaticMesh* GetMeshAsset(TArray<FString> AssetNames);
	TArray<FString> CalculateMaterialsSlotOrder(TArray<FString> Materials, TArray<FStaticMaterial> Slots);
	void AssignMaterialsToMesh(TArray<FString> Materials, UStaticMesh* StaticMesh);
	void AssignMaterialsToMesh(TArray<FString> Materials, UStaticMeshComponent* StaticMeshComponent);
	void AssignMaterialsToMesh(TArray<FString> Materials, USkeletalMesh* SkeletalMesh);
	void AssignMaterialsToMesh(TArray<FString> Materials, USkeletalMeshComponent* SkeletalMeshComponent);

public:
	static TArray<FString> GetAllPropertiesAsString(UObject* Object);
private:
	class UFbxImportUI* GetStaticMeshImportOptions(FUtuPluginMesh InUtuMesh, FString SpecificSubmesh);

	FLinearColor HexToColor(FString InHex);
	UTexture2D* GetTextureFromUnityRelativeFilename(FString InUnityRelativeFilename);
	UMaterialExpressionTextureSampleParameter2D* GetOrCreateTextureParameter(UMaterial* InMaterial, UTexture* InTexture, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionScalarParameter* GetOrCreateScalarParameter(UMaterial* InMaterial, float InValue, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionVectorParameter* GetOrCreateVectorParameter(UMaterial* InMaterial, FLinearColor InColor, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionComponentMask* GetOrCreateMaskExpression(UMaterial* InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionMultiply* GetOrCreateMultiplyExpression(UMaterial* InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionPanner* GetOrCreatePannerExpression(UMaterial* InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);
	UMaterialExpressionTextureCoordinate* GetOrCreateTexCoordExpression(UMaterial* InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial);

	bool IsFbxExporter(FString Path);
	FUtuPluginImportSettings_AllAssets GetAssetStruct(EUtuUnrealAssetType AssetType);

private:
	bool bWasInterchangeEnabled = true;

public:
	FAssetToolsModule* AssetTools;
	// Global
	FUtuPluginJson json;
	EUtuAssetType assetType;
	// Delayed Specific
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		bool bIsValid = false;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int countItemsToProcess = 1;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		int amountItemsToProcess = 0;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		float percentItemsToProcess = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FString nameItemToProcess = "";

	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FUtuPluginImportSettings ImportSettings;
};