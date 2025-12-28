// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPluginAssets.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "UtuPlugin/Scripts/Public/UtuPluginConstants.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLog.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"

#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "Editor/UnrealEd/Public/AssetImportTask.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Editor/UnrealEd/Classes/Factories/MaterialFactoryNew.h"
#include "Kismet/KismetMathLibrary.h"
#include "ObjectTools.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Engine/Texture2D.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Animation/Skeleton.h"
#include "InterchangeAssetImportData.h"
#include "InterchangeGenericAssetsPipeline.h"
#include "InterchangeGenericMeshPipeline.h"
#include "InterchangeGenericAnimationPipeline.h"
#include "Fbx/InterchangeFbxTranslator.h"
#include "Editor/EditorPerProjectUserSettings.h"
#else
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionComponentMask.h"
#endif

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/AssetRegistryInterface.h"
#else
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/AssetRegistry/Public/IAssetRegistry.h"
#endif

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "Engine/SkinnedAssetCommon.h"
#endif

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Runtime/Engine/Public/ComponentReregisterContext.h"
#include "Editor/UnrealEd/Classes/Factories/FbxImportUI.h"
#include "Editor/UnrealEd/Classes/Factories/FbxStaticMeshImportData.h"
#include "Editor/UnrealEd/Classes/Factories/FbxSkeletalMeshImportData.h"
#include "Factories/FbxAnimSequenceImportData.h" // UnrealEd (Editor Only)
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"
#include "Editor/UnrealEd/Public/Kismet2/KismetEditorUtilities.h"
#include "Editor/KismetCompiler/Public/KismetCompilerModule.h"
#include "Runtime/Engine/Classes/Engine/SimpleConstructionScript.h"
#include "Runtime/Engine/Classes/Engine/SCS_Node.h"
#include "Runtime/Engine/Classes/Animation/AnimSequence.h"
//#include "Editor/UnrealEd/Public/Toolkits/AssetEditorManager.h"
//#include "Editor/Kismet/Public/BlueprintEditor.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Editor/UnrealEd/Classes/Factories/WorldFactory.h"
#include "Editor/UnrealEd/Classes/ActorFactories/ActorFactoryEmptyActor.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "BlueprintEditorSettings.h"
//#include "Editor/BlueprintGraph/Public/BlueprintEditorImportSettings.h"
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/DirectionalLight.h"
#include "Runtime/Engine/Classes/Engine/PointLight.h"
#include "Runtime/Engine/Classes/Engine/SpotLight.h"
#include "Runtime/Engine/Classes/Engine/SkyLight.h"
#include "Runtime/Engine/Classes/Components/LightComponent.h"
#include "Runtime/Engine/Classes/Components/PointLightComponent.h"
#include "Runtime/Engine/Classes/Components/SpotLightComponent.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"
#include "Runtime/Engine/Classes/Components/SkyLightComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/CinematicCamera/Public/CineCameraActor.h"
#include "Runtime/CinematicCamera/Public/CineCameraComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionPanner.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Engine/TextureCube.h"
#include "MaterialEditingLibrary.h"

#include "EditorReimportHandler.h" // UnrealEd  (Editor Only)
#include "Misc/ScopedSlowTask.h" // Core 
#include "Misc/FeedbackContext.h" // Core 
#include "FbxMeshUtils.h"
#include "FbxImporter.h"
#include "Editor/UnrealEd/Private/FbxExporter.h"
#include "Exporters/FbxExportOption.h"
#include "AssetExportTask.h"
#include "UObject/GCObjectScopeGuard.h"
#include "Exporters/Exporter.h"
#include "UnrealExporter.h"

void FUtuPluginAssetTypeProcessor::Import(FUtuPluginJson Json, EUtuAssetType AssetType, bool executeFullImportOnSameFrame, TArray<FString> DuplicatedAssetNames) {
	BeginImport(Json, AssetType, DuplicatedAssetNames);
	if (executeFullImportOnSameFrame) {
		while (ContinueImport() != true) {
			// ContinueImport
		}
	}
}

void FUtuPluginAssetTypeProcessor::BeginImport(FUtuPluginJson Json, EUtuAssetType AssetType, TArray<FString> DuplicatedAssetNames) {
	AssetTools = FModuleManager::LoadModulePtr<FAssetToolsModule>("AssetTools");
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();
	bWasInterchangeEnabled = InterchangeManager.IsInterchangeImportEnabled();
	InterchangeManager.SetInterchangeImportEnabled(false);
#endif

	CopyUtuAssetsInProject();

	json = Json;
	assetType = AssetType;
	ListOfDuplicatedAssetNames = DuplicatedAssetNames;
	amountItemsToProcess = GetAssetsNum();
	countItemsToProcess = 1;
	percentItemsToProcess = (float)countItemsToProcess / (float)amountItemsToProcess;
	if (amountItemsToProcess == 0) {
		bIsValid = false;
	}
}

bool FUtuPluginAssetTypeProcessor::ContinueImport() {
	if (GetAssetsNum() == 0) {
		//UTU_LOG_E("FUtuPluginAssetTypeProcessor::ContinueImport() Was called even though the list is already empty. This should never happen!");
		CompleteImport();
		return true;
	}
	switch (assetType) {
	case EUtuAssetType::Scene:
		nameItemToProcess = json.scenes[0].asset_name;
		ProcessScene(json.scenes[0]);
		json.scenes.RemoveAt(0);
		break;
	case EUtuAssetType::Animation:
		nameItemToProcess = json.animations[0].asset_name;
		ProcessAnimation(json.animations[0]);
		json.animations.RemoveAt(0);
		break;
	case EUtuAssetType::Mesh:
		nameItemToProcess = json.meshes[0].asset_name;
		ProcessMesh(json.meshes[0]);
		json.meshes.RemoveAt(0);
		break;
	case EUtuAssetType::Material:
		nameItemToProcess = json.materials[0].asset_name;
		ProcessMaterial(json.materials[0]);
		json.materials.RemoveAt(0);
		break;
	case EUtuAssetType::Texture:
		nameItemToProcess = json.textures[0].asset_name;
		ProcessTexture(json.textures[0]);
		json.textures.RemoveAt(0);
		break;
	case EUtuAssetType::PrefabFirstPass:
		nameItemToProcess = json.prefabs_first_pass[0].asset_name;
		ProcessPrefabFirstPass(json.prefabs_first_pass[0]);
		json.prefabs_first_pass.RemoveAt(0);
		break;
	case EUtuAssetType::PrefabSecondPass:
		nameItemToProcess = json.prefabs_second_pass[0].asset_name;
		ProcessPrefabSecondPass(json.prefabs_second_pass[0]);
		json.prefabs_second_pass.RemoveAt(0);
		break;
	default:
		break;
	}
	countItemsToProcess++;
	percentItemsToProcess = (float)countItemsToProcess / (float)FMath::Max(amountItemsToProcess, 1);
	if (GetAssetsNum() == 0) {
		CompleteImport();
		return true;
	}
	return false;
}

void FUtuPluginAssetTypeProcessor::CompleteImport() 
{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	UInterchangeManager& InterchangeManager = UInterchangeManager::GetInterchangeManager();
	InterchangeManager.SetInterchangeImportEnabled(bWasInterchangeEnabled);
#endif
}





void FUtuPluginAssetTypeProcessor::CopyUtuAssetsInProject()
{
	// Get the asset tools module
	TArray<FString> AssetsToCopy = TArray<FString>();
	AssetsToCopy.Add("/UtuPlugin/Components/TX_CubeMap");
	AssetsToCopy.Add("/UtuPlugin/Default/Capsule");
	AssetsToCopy.Add("/UtuPlugin/Default/Cube");
	AssetsToCopy.Add("/UtuPlugin/Default/Cylinder");
	AssetsToCopy.Add("/UtuPlugin/Default/Normal");
	AssetsToCopy.Add("/UtuPlugin/Default/Plane");
	AssetsToCopy.Add("/UtuPlugin/Default/Quad");
	AssetsToCopy.Add("/UtuPlugin/Default/Sphere");
	AssetsToCopy.Add("/UtuPlugin/Default/Texture");
	AssetsToCopy.Add("/UtuPlugin/Default/unity_builtin_extra");
	AssetsToCopy.Add("/UtuPlugin/Shaders/HDRP_Lit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/HDRP_Unlit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/LegacyShaders_BumpedDiffuse");
	AssetsToCopy.Add("/UtuPlugin/Shaders/LegacyShaders_BumpedSpecular");
	AssetsToCopy.Add("/UtuPlugin/Shaders/LegacyShaders_Diffuse");
	AssetsToCopy.Add("/UtuPlugin/Shaders/LegacyShaders_Specular");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Mobile_BumpedDiffuse");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Mobile_BumpedSpecular");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Mobile_Diffuse");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Mobile_UnlitSupportsLightmap");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Standard");
	AssetsToCopy.Add("/UtuPlugin/Shaders/StandardSpecularsetup");
	AssetsToCopy.Add("/UtuPlugin/Shaders/UniversalRenderPipeline_BakedLit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/UniversalRenderPipeline_ComplexLit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/UniversalRenderPipeline_Lit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/UniversalRenderPipeline_SimpleLit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/UniversalRenderPipeline_Unlit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Unlit_Color");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Unlit_Texture");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Unlit_Transparent");
	AssetsToCopy.Add("/UtuPlugin/Shaders/ShaderGraphs_ArnoldStandardSurface");
	AssetsToCopy.Add("/UtuPlugin/Shaders/ShaderGraphs_ArnoldStandardSurfaceTransparent");
	AssetsToCopy.Add("/UtuPlugin/Shaders/ShaderGraphs_PhysicalMaterial3DsMax");
	AssetsToCopy.Add("/UtuPlugin/Shaders/ShaderGraphs_PhysicalMaterial3DsMaxTransparent");
	AssetsToCopy.Add("/UtuPlugin/Shaders/ShaderGraphs_VFXSpriteLit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/ShaderGraphs_VFXSpriteUnlit");
	AssetsToCopy.Add("/UtuPlugin/Shaders/Hidden_InternalErrorShader");

	for (FString AssetPath : AssetsToCopy)
	{
		FString NewPath = AssetPath;
		NewPath = NewPath.Replace(TEXT("/UtuPlugin/Components"), TEXT("/Game/Utu/Assets"));
		NewPath = NewPath.Replace(TEXT("/UtuPlugin/Default"), TEXT("/Game/Utu/Assets"));
		NewPath = NewPath.Replace(TEXT("/UtuPlugin/Shaders"), TEXT("/Game/Utu/Shaders"));

		UObject* AssetTarget = UUtuPluginLibrary::TryGetAsset(NewPath);

		if (AssetTarget == nullptr)
		{
			FScopedSlowTask SlowTask(AssetsToCopy.Num(), FText::Format(NSLOCTEXT("CopyUtuAssetsInProject", "CopyAsset", "Copying Utu assets in your project. '{0}' -> '{1}' ..."), FText::FromString(AssetPath), FText::FromString(NewPath)), true, *GWarn);
			SlowTask.MakeDialog(false/*bShowCancelButton*/);

			// Create asset
			UObject* AssetSource = UUtuPluginLibrary::TryGetAsset(AssetPath);
			AssetTarget = AssetTools->Get().DuplicateAsset(FPaths::GetBaseFilename(NewPath), FPaths::GetPath(NewPath), AssetSource);
		}

		//if (AssetTarget != nullptr)
		//{
		//	FScopedSlowTask SecondSlowTask(1, NSLOCTEXT("CopyUtuAssetsInProject", "ReplaceRefs", "Replacing old references..."), true, *GWarn);
		//	// Replace refs
		//	TArray<UObject*> SourceAssetAsArray = TArray<UObject*>({ AssetSource });
		//	ObjectTools::ForceReplaceReferences(AssetTarget, SourceAssetAsArray);
		//}

	}
}


TArray<FString> FUtuPluginAssetTypeProcessor::FormatRelativeFilenameForUnreal(FString InRelativeFilename, EUtuUnrealAssetType AssetType) {
	if (InRelativeFilename != "") 
	{
		FString Relative = InRelativeFilename;
		if (Relative.RemoveFromStart("Assets")) 
		{
			Relative = "/Game" + Relative;
		}
		if (Relative.RemoveFromStart("Packages")) 
		{
			Relative = "/Game" + Relative;
		}
		Relative = Relative.Replace(*UtuPluginPaths::backslash, *UtuPluginPaths::slash);
		
		if (!Relative.StartsWith("/Game")) 
		{
			Relative = "/Game/Utu/Assets/" + Relative.Replace(TEXT("Resources"), TEXT(""));
		}

		Relative = Relative.Replace(TEXT("\\"), TEXT("/")); // No backslash to be safe
		while (Relative.Contains("//"))
		{
			Relative = Relative.Replace(TEXT("//"), TEXT("/")); // If double slash is in the path, Unreal will crash
		}
		
		Relative = Relative.Replace(TEXT(" "), TEXT("_"));
		Relative = Relative.Replace(TEXT("\""), TEXT("_"));
		Relative = Relative.Replace(TEXT("'"), TEXT("_"));
		Relative = Relative.Replace(TEXT(" "), TEXT("_"));
		Relative = Relative.Replace(TEXT(","), TEXT("_"));
		Relative = Relative.Replace(TEXT(":"), TEXT("_"));
		Relative = Relative.Replace(TEXT("|"), TEXT("_"));
		Relative = Relative.Replace(TEXT("&"), TEXT("_"));
		Relative = Relative.Replace(TEXT("!"), TEXT("_"));
		Relative = Relative.Replace(TEXT("~"), TEXT("_"));
		Relative = Relative.Replace(TEXT("@"), TEXT("_"));
		Relative = Relative.Replace(TEXT("#"), TEXT("_"));
		Relative = Relative.Replace(TEXT("("), TEXT("_"));
		Relative = Relative.Replace(TEXT(")"), TEXT("_"));
		Relative = Relative.Replace(TEXT("{"), TEXT("_"));
		Relative = Relative.Replace(TEXT("}"), TEXT("_"));
		Relative = Relative.Replace(TEXT("["), TEXT("_"));
		Relative = Relative.Replace(TEXT("]"), TEXT("_"));
		Relative = Relative.Replace(TEXT("="), TEXT("_"));
		Relative = Relative.Replace(TEXT(";"), TEXT("_"));
		Relative = Relative.Replace(TEXT("^"), TEXT("_"));
		Relative = Relative.Replace(TEXT("%"), TEXT("_"));
		Relative = Relative.Replace(TEXT("$"), TEXT("_"));
		Relative = Relative.Replace(TEXT("`"), TEXT("_"));
		Relative = Relative.Replace(TEXT("*"), TEXT("_"));
		Relative = Relative.Replace(TEXT("?"), TEXT("_"));
		Relative = Relative.Replace(TEXT("+"), TEXT("_"));

		if (AssetType == EUtuUnrealAssetType::Material || AssetType == EUtuUnrealAssetType::MaterialInstance)
		{
			// Fix the fact that materials doesn't really exists in Unity if they are from the .fbx file -.-
			if (Relative.EndsWith(".Fbx"))
			{
				Relative.RemoveFromEnd(".Fbx");
				Relative += "_FbxMat";
			}
		}

		// Semi final path
		FString Path;
		FString Filename;
		Relative.Split("/", &Path, &Filename, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

		// Extension? 
		if (Filename.Contains("."))
		{
			// Remove it
			Filename.Split(".", &Filename, nullptr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
			Relative = Path / Filename;
			Relative = Relative.Replace(TEXT("."), TEXT("_")); // Dots in asset path? Really!? -.-
			Relative.Split("/", &Path, &Filename, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		}

		// Renaming stuff
		// Don't try to rename utu assets
		if (!Relative.StartsWith("/Game/Utu/Assets") && !Relative.StartsWith("/Game/Utu/Shaders"))
		{
			// Prefix
			if (!Filename.StartsWith(GetAssetStruct(AssetType).AssetRenameSettings.Prefix))
			{
				Filename = GetAssetStruct(AssetType).AssetRenameSettings.Prefix + Filename;
			}
			// Suffix
			if (!Filename.EndsWith(GetAssetStruct(AssetType).AssetRenameSettings.Suffix))
			{
				Filename += GetAssetStruct(AssetType).AssetRenameSettings.Suffix;
			}
			// Find and replace
			Relative = Path / Filename;
			for (FUtuFindAndReplace FindAndReplace : GetAssetStruct(AssetType).AssetRenameSettings.FindAndReplace)
			{
				Relative = Relative.Replace(*FindAndReplace.From, *FindAndReplace.To, ESearchCase::CaseSensitive);
			}
			Relative.Split("/", &Path, &Filename, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		}

		// Is this a duplicate? 
		if (ListOfDuplicatedAssetNames.Contains(Relative))
		{
			Relative += GetAssetStruct(AssetType).AssetRenameSettings.AutoRenameDuplicatedAssetSuffix;
		}

		// Final path
		Relative.Split("/", &Path, &Filename, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		return { Path, Filename, Path / Filename };
	}
	return { "", "", "" };
}


TArray<FString> FUtuPluginAssetTypeProcessor::FormatRelativeFilenameForUnrealSeparated(FString InRelativeFilename, FString InRelativeFilenameSeparated, EUtuUnrealAssetType AssetType)
{
	TArray<FString> RelativeNames = FormatRelativeFilenameForUnreal(InRelativeFilename, AssetType);
	TArray<FString> RelativeNamesSeparated = FormatRelativeFilenameForUnreal(InRelativeFilenameSeparated, AssetType);

	FString Suffix = GetAssetStruct(AssetType).AssetRenameSettings.AutoRenameDuplicatedAssetSuffix;

	FString NormalFilename = RelativeNames[1];
	NormalFilename.RemoveFromEnd(Suffix);

	// Not a duplicae?
	if (!ListOfDuplicatedAssetNames.Contains(RelativeNames[0] / NormalFilename))
	{
		return RelativeNamesSeparated;
	}

	FString SeparatedFilename = RelativeNamesSeparated[1];
	SeparatedFilename.RemoveFromStart(NormalFilename);

	FString FinalSeparatedFilename = RelativeNames[1] + SeparatedFilename;

	return TArray<FString>({ RelativeNamesSeparated[0], FinalSeparatedFilename, RelativeNamesSeparated[0] / FinalSeparatedFilename });
}


int FUtuPluginAssetTypeProcessor::GetAssetsNum() {
	switch (assetType) {
	case EUtuAssetType::Scene:
		return json.scenes.Num();
		break;
	case EUtuAssetType::Mesh:
		return json.meshes.Num();
		break;
	case EUtuAssetType::Animation:
		return json.animations.Num();
		break;
	case EUtuAssetType::Material:
		return json.materials.Num();
		break;
	case EUtuAssetType::Texture:
		return json.textures.Num();
		break;
	case EUtuAssetType::PrefabFirstPass:
		return json.prefabs_first_pass.Num();
		break;
	case EUtuAssetType::PrefabSecondPass:
		return json.prefabs_second_pass.Num();
		break;
	default:
		break;
	}
	return 0;
}

void FUtuPluginAssetTypeProcessor::ProcessScene(FUtuPluginScene InUtuScene) {
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuScene, EUtuUnrealAssetType::Level);
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames, UWorld::StaticClass())) 
	{
		// Existing Asset
		UWorld* Asset = Cast<UWorld>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		LogAssetCreateOrNot(Asset);

		if (ImportSettings.Scenes.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
		}
		else if (Asset != nullptr && ImportSettings.Scenes.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
		}
		else
		{
			// Create Asset
			if (Asset == nullptr)
			{
				UPackage* Package = CreateAssetPackage(AssetNames[2], true);
				UWorldFactory* Factory = NewObject<UWorldFactory>();
				Asset = Cast<UWorld>(Factory->FactoryCreateNew(UWorld::StaticClass(), Package, FName(*AssetNames[1]), RF_Public | RF_Standalone, NULL, GWarn));
				LogAssetImportedOrFailed(Asset, AssetNames, "", "World", { });
			}
			// Process Asset
			if (Asset != nullptr)
			{
				Asset->PreEditChange(NULL);
				// Delete Old Actors -- TODO : Analyse them and keep the good ones
				TArray<AActor*> OldActors;
				UGameplayStatics::GetAllActorsWithTag(Asset, "UtuActor", OldActors);
				if (OldActors.Num() > 0) 
				{
					UTU_LOG_L("        " + FString::FromInt(OldActors.Num()) + " old actors from previous import detected. Deleting them...");
					//UTU_LOG_L("            Plan for a future release: Analyse existing Actors and keep the good ones.");
					for (AActor* OldActor : OldActors) {
						Asset->DestroyActor(OldActor);
					}
				}
				// Start by creating a simple SkyLight
				WorldSpawnSkyLightActor(Asset);
				// Map
				TMap<int, AActor*> IdToActor;
				TMap<AActor*, int> ActorToParentId;
				// Spawn Actors
				for (FUtuPluginActor UtuActor : InUtuScene.scene_actors) 
				{
					AActor* RootActor = WorldAddRootActorForSubActorsIfNeeded(Asset, UtuActor);
					if (RootActor != nullptr) 
					{
						RootActor->Tags.Add("UtuActor");
						IdToActor.Add(UtuActor.actor_id, RootActor);
						ActorToParentId.Add(RootActor, UtuActor.actor_parent_id);
					}
					// Spawn Real Actors
					for (EUtuActorType ActorType : UtuActor.actor_types) 
					{
						AActor* Actor = nullptr;
						if (ActorType == EUtuActorType::Empty) 
						{
							// Don't care about an empty actor 'cause it's already spawned as a RootActor
						}
						else if (ActorType == EUtuActorType::StaticMesh) 
						{
							Actor = WorldSpawnStaticMeshActor(Asset, UtuActor);
						}
						else if (ActorType == EUtuActorType::SkeletalMesh) 
						{
							Actor = WorldSpawnSkeletalMeshActor(Asset, UtuActor); // TODO : Support Skeletal Mesh
							//Actor = WorldSpawnStaticMeshActor(Asset, UtuActor);
						}
						else if (ActorType == EUtuActorType::PointLight) 
						{
							Actor = WorldSpawnPointLightActor(Asset, UtuActor);
						}
						else if (ActorType == EUtuActorType::DirectionalLight) 
						{
							Actor = WorldSpawnDirectionalLightActor(Asset, UtuActor);
						}
						else if (ActorType == EUtuActorType::SpotLight) 
						{
							Actor = WorldSpawnSpotLightActor(Asset, UtuActor);
						}
						else if (ActorType == EUtuActorType::Camera) 
						{
							Actor = WorldSpawnCameraActor(Asset, UtuActor);
						}
						else if (ActorType == EUtuActorType::Prefab) 
						{
							Actor = WorldSpawnBlueprintActor(Asset, UtuActor);
						}
						// Attachment
						if (Actor != nullptr) 
						{
							Actor->GetRootComponent()->SetMobility(UtuActor.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
							if (RootActor == nullptr) 
							{
								Actor->Tags.Add("UtuActor");
								Actor->SetActorLabel(UtuActor.actor_display_name);
								Actor->Tags.Add(*FString::FromInt(UtuActor.actor_id));
								if (UtuActor.actor_tag != "Untagged") 
								{
									Actor->Tags.Add(*UtuActor.actor_tag);
								}
								Actor->SetActorHiddenInGame(!UtuActor.actor_is_visible);
								Actor->GetRootComponent()->SetVisibility(UtuActor.actor_is_visible, true);
								Actor->SetActorLocation(UtuConst::ConvertLocation(UtuActor.actor_world_location));
								Actor->SetActorRotation(UtuConst::ConvertRotation(UtuActor.actor_world_rotation));
								Actor->SetActorScale3D(UtuConst::ConvertScale(UtuActor.actor_world_scale));
								IdToActor.Add(UtuActor.actor_id, Actor);
								ActorToParentId.Add(Actor, UtuActor.actor_parent_id);
							}
							else 
							{
								Actor->AttachToActor(RootActor, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
							}
						}
					}
				}
				// Parent Actors
				UTU_LOG_L("        Parenting actors...");
				TArray<AActor*> Keys;
				ActorToParentId.GetKeys(Keys);
				for (AActor* Actor : Keys) 
				{
					int Id = ActorToParentId[Actor];
					if (Id != UtuConst::INVALID_INT) 
					{
						if (IdToActor.Contains(Id)) 
						{
							AActor* ParentActor = IdToActor[Id];
							Actor->AttachToActor(ParentActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, false));
							UTU_LOG_L("            " + Actor->GetActorLabel() + " -> " + ParentActor->GetActorLabel());
						}
						else {
							UTU_LOG_W("            Failed to find parent for: " + Actor->GetActorLabel());
							UTU_LOG_W("                Potential Causes:");
							UTU_LOG_W("                    - The desired parent of this actor is a component of a prefab.");
							UTU_LOG_W("                      (Scene edits of prefabs' components aren't supported yet.)");
							UTU_LOG_W("                    - The desired parent failed to spawn for some reason. (Missing Bp asset maybe?)");
						}
					}
				}
				Asset->MarkPackageDirty();
				Asset->PostEditChange();
			}
		}
	}
}

void FUtuPluginAssetTypeProcessor::ProcessAnimation(FUtuPluginAnimation InUtuAnimation)
{
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuAnimation, EUtuUnrealAssetType::Animation);
	TArray<FString> AssetNames_Anim = TArray<FString>({ AssetNames[0], AssetNames[1] + "_Anim", AssetNames[0] + "/" + AssetNames[1] + "_Anim" });
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames_Anim, UAnimSequence::StaticClass()) && DeleteInvalidAssetIfNeeded(AssetNames_Anim, UAnimSequence::StaticClass())) {
		// Existing Asset
		UAnimSequence* Asset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		if (Asset == nullptr)
		{
			Asset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(AssetNames_Anim[2]));
		}
		LogAssetImportOrReimport(Asset);

		if (ImportSettings.Animations.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
		}
		else if (Asset != nullptr && ImportSettings.Animations.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
		}
		else
		{
			if (Asset != nullptr && ImportSettings.Animations.ProcessingBehavior == EUtuProcessingBehavior::UpdateExisting)
			{
				UTU_LOG_L("        Asset re-import skipped because processing behavior is set to 'UpdateExisting'");
			}
			else
			{
				// Create Asset
				UTU_LOG_L("        Fbx File Fullname: " + InUtuAnimation.animation_file_absolute_filename);

				// Settings
				UFbxImportUI* Options = NewObject<UFbxImportUI>();
				// Required options to specify that we're importing an animation
				Options->bImportAnimations = true;
				// Animation only, don't import anything else
				Options->bImportTextures = false;
				Options->bImportMaterials = false;
				Options->bResetToFbxOnMaterialConflict = true;
				Options->LodNumber = 0;
				Options->OverrideAnimationName = "";
				// UFbxAssetImportData
				Options->AnimSequenceImportData->ImportTranslation = ImportSettings.Animations.ImportLocationOffset;
				Options->AnimSequenceImportData->ImportRotation = ImportSettings.Animations.ImportRotationOffset;
				Options->AnimSequenceImportData->ImportUniformScale = IsFbxExporter(InUtuAnimation.animation_file_absolute_filename) ? ImportSettings.Animations.FbxExporterImportScaleMultiplier : ImportSettings.Animations.ImportScaleMultiplier;
				Options->AnimSequenceImportData->bConvertScene = ImportSettings.Animations.bConvertScene;
				Options->AnimSequenceImportData->bForceFrontXAxis = ImportSettings.Animations.bForceFrontXAxis;
				Options->AnimSequenceImportData->bConvertSceneUnit = ImportSettings.Animations.bConvertSceneUnit;
				// UFbxAnimSequenceImportData
				Options->AnimSequenceImportData->bImportMeshesInBoneHierarchy = ImportSettings.Animations.bAnimImportMeshesInBoneHierarchy;
				Options->AnimSequenceImportData->AnimationLength = EFBXAnimationLengthImportType::FBXALIT_ExportedTime;
				Options->AnimSequenceImportData->FrameImportRange = FInt32Interval();
				Options->AnimSequenceImportData->bUseDefaultSampleRate = ImportSettings.Animations.bUseDefaultSampleRate;
				Options->AnimSequenceImportData->CustomSampleRate = ImportSettings.Animations.CustomSampleRate;
				Options->AnimSequenceImportData->bImportCustomAttribute = ImportSettings.Animations.bImportCustomAttribute;
				Options->AnimSequenceImportData->bDeleteExistingCustomAttributeCurves = ImportSettings.Animations.bDeleteExistingCustomAttributeCurves;
				Options->AnimSequenceImportData->bSetMaterialDriveParameterOnCustomAttribute = ImportSettings.Animations.bSetMaterialDriveParameterOnCustomAttribute;
				Options->AnimSequenceImportData->bRemoveRedundantKeys = ImportSettings.Animations.bRemoveRedundantKeys;
				Options->AnimSequenceImportData->bDeleteExistingMorphTargetCurves = ImportSettings.Animations.bDeleteExistingMorphTargetCurves;
				Options->AnimSequenceImportData->bDoNotImportCurveWithZero = ImportSettings.Animations.bDoNotImportCurveWithZero;
				Options->AnimSequenceImportData->bPreserveLocalTransform = ImportSettings.Animations.bPreserveLocalTransform;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 0
				Options->AnimSequenceImportData->bDeleteExistingNonCurveCustomAttributes = false;
				Options->AnimSequenceImportData->bSnapToClosestFrameBoundary = true;
#endif

				// If nothing gets imported, we'll just import it as is on a new skeletal mesh
				bool bWasSomethingImported = false;

				for (FString SkeletalMeshes : InUtuAnimation.associated_skeletal_meshes_relative_filenames)
				{
					TArray<FString> SkeletonAssetNames = FormatRelativeFilenameForUnreal(SkeletalMeshes, EUtuUnrealAssetType::SkeletalMesh);
					Options->Skeleton = Cast<USkeleton>(UUtuPluginLibrary::TryGetAsset(SkeletonAssetNames[2] + "_Skeleton"));
					if (Options->Skeleton != nullptr)
					{
						bWasSomethingImported = true;
						Options->bAutomatedImportShouldDetectType = false;
						Options->MeshTypeToImport = EFBXImportType::FBXIT_Animation;
						Options->bImportMesh = false;
						Options->bImportAsSkeletal = false;
						Options->bCreatePhysicsAsset = false;
						Options->bResetToFbxOnMaterialConflict = true;

						TArray<FString> CustomAssetNames = TArray<FString>({ AssetNames[0], SkeletonAssetNames[1] + "_" + AssetNames[1], AssetNames[0] + "/" + SkeletonAssetNames[1] + "_" + AssetNames[1] });

						// Checkk if already exists (Reimport)
						Asset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(CustomAssetNames[2]));
						if (Asset != nullptr)
						{
							// Make sure the new settings are used for the next import
							Asset->AssetImportData = Options->AnimSequenceImportData;
						}

						// Process
						AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuAnimation.animation_file_absolute_filename, CustomAssetNames, Options) });
						Asset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(CustomAssetNames[2]));
						LogAssetImportedOrFailed(Asset, CustomAssetNames, InUtuAnimation.animation_file_absolute_filename, "Animation", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
					}
				}

				if (!bWasSomethingImported)
				{
					Options->bAutomatedImportShouldDetectType = true;
					Options->MeshTypeToImport = EFBXImportType::FBXIT_Animation;
					Options->Skeleton = nullptr;
					Options->bImportMesh = true; // Import mesh if skeleton is not valid
					Options->bImportAsSkeletal = true; // Import mesh if skeleton is not valid
					Options->bCreatePhysicsAsset = true; // Import mesh if skeleton is not valid
					Options->bResetToFbxOnMaterialConflict = true;

					// Process
					AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuAnimation.animation_file_absolute_filename, AssetNames, Options) });
					Asset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(AssetNames_Anim[2]));
					LogAssetImportedOrFailed(Asset, AssetNames_Anim, InUtuAnimation.animation_file_absolute_filename, "Animation", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
				}
			}
		}
	}
}


void FUtuPluginAssetTypeProcessor::ProcessMesh(FUtuPluginMesh InUtuMesh) {
	if (InUtuMesh.asset_relative_filename.StartsWith("Assets") || InUtuMesh.asset_relative_filename.StartsWith("Packages")) { // Default Unity Mesh
		// Format Paths
		TArray<FString> AssetNames = StartProcessAsset(InUtuMesh, InUtuMesh.is_skeletal_mesh ? EUtuUnrealAssetType::SkeletalMesh : EUtuUnrealAssetType::StaticMesh);
		if (InUtuMesh.is_skeletal_mesh) {
			// Invalid Asset
			if (DeleteInvalidAssetIfNeeded(AssetNames, USkeletalMesh::StaticClass())) {
				// Existing Asset
				USkeletalMesh* Asset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetImportOrReimport(Asset);

				if (ImportSettings.SkeletalMeshes.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
				}
				else if (Asset != nullptr && ImportSettings.SkeletalMeshes.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
				}
				else
				{
					if (Asset != nullptr && ImportSettings.SkeletalMeshes.ProcessingBehavior == EUtuProcessingBehavior::UpdateExisting)
					{
						UTU_LOG_L("        Asset re-import skipped because processing behavior is set to 'UpdateExisting'");
					}
					else
					{
						// Create Asset
						UTU_LOG_L("        Fbx File Fullname: " + InUtuMesh.mesh_file_absolute_filename);
						UTU_LOG_L("        Import Scale Factor: " + FString::SanitizeFloat(InUtuMesh.mesh_import_scale_factor));
						UFbxImportUI* Options = NewObject<UFbxImportUI>();
						Options->bIsObjImport = InUtuMesh.mesh_file_absolute_filename.EndsWith(".obj", ESearchCase::IgnoreCase);
						Options->bAutomatedImportShouldDetectType = false;
						Options->bImportAnimations = false;
						Options->MeshTypeToImport = EFBXImportType::FBXIT_SkeletalMesh;
						Options->bImportMesh = true;
						Options->bImportTextures = false;
						Options->bImportMaterials = false;
						Options->bImportAsSkeletal = true;
						Options->SkeletalMeshImportData->NormalImportMethod = ImportSettings.SkeletalMeshes.NormalImportMethod;
						Options->SkeletalMeshImportData->NormalGenerationMethod = ImportSettings.SkeletalMeshes.NormalGenerationMethod;
						Options->SkeletalMeshImportData->bComputeWeightedNormals = ImportSettings.SkeletalMeshes.bComputeWeightedNormals;
						Options->SkeletalMeshImportData->bUseT0AsRefPose = ImportSettings.SkeletalMeshes.bUseT0AsRefPose;
						Options->SkeletalMeshImportData->bPreserveSmoothingGroups = ImportSettings.SkeletalMeshes.bPreserveSmoothingGroups;
						Options->SkeletalMeshImportData->bImportMeshesInBoneHierarchy = ImportSettings.SkeletalMeshes.bImportMeshesInBoneHierarchy;
						Options->SkeletalMeshImportData->bImportMorphTargets = ImportSettings.SkeletalMeshes.bImportMorphTargets;
						Options->SkeletalMeshImportData->bComputeWeightedNormals = ImportSettings.SkeletalMeshes.bComputeWeightedNormals;
						Options->SkeletalMeshImportData->bImportMeshLODs = false;
						Options->SkeletalMeshImportData->bTransformVertexToAbsolute = ImportSettings.SkeletalMeshes.bTransformVertexToAbsolute;
						Options->SkeletalMeshImportData->bConvertScene = ImportSettings.SkeletalMeshes.bConvertScene;
						Options->SkeletalMeshImportData->bForceFrontXAxis = ImportSettings.SkeletalMeshes.bForceFrontXAxis;
						Options->SkeletalMeshImportData->bConvertSceneUnit = ImportSettings.SkeletalMeshes.bConvertSceneUnit;
						Options->SkeletalMeshImportData->ImportTranslation = UtuConst::ConvertLocation(InUtuMesh.mesh_import_position_offset, true, InUtuMesh.mesh_import_rotation_offset) / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X) + ImportSettings.SkeletalMeshes.ImportLocationOffset;
						Options->SkeletalMeshImportData->ImportRotation = UKismetMathLibrary::ComposeRotators(ImportSettings.SkeletalMeshes.ImportRotationOffset, FRotator(UtuConst::ConvertRotation(InUtuMesh.mesh_import_rotation_offset, true)));
						Options->SkeletalMeshImportData->ImportUniformScale = (IsFbxExporter(InUtuMesh.mesh_file_absolute_filename) ? ImportSettings.SkeletalMeshes.FbxExporterImportScaleMultiplier : ImportSettings.SkeletalMeshes.ImportScaleMultiplier) * InUtuMesh.mesh_import_scale_factor / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);

						// Checkk if already exists (Reimport)
						Asset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
						if (Asset != nullptr)
						{
							// Make sure the new settings are used for the next import
							Asset->AssetImportData = Options->SkeletalMeshImportData;
						}

						// Process
						AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuMesh.mesh_file_absolute_filename, AssetNames, Options) });

						Asset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
						LogAssetImportedOrFailed(Asset, AssetNames, InUtuMesh.mesh_file_absolute_filename, "SkeletalMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
					}

					// Process Asset
					if (Asset != nullptr)
					{
						AssignMaterialsToMesh(InUtuMesh.mesh_materials_relative_filenames, Asset);
					}
				}
			}
		}
		else
		{
			// Invalid Asset
			if (DeleteInvalidAssetIfNeeded(AssetNames, UStaticMesh::StaticClass()))
			{
				// Existing Asset
				UStaticMesh* Asset = GetMeshAsset(AssetNames);
				LogAssetImportOrReimport(Asset);


				if (ImportSettings.StaticMeshes.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
				}
				else if (Asset != nullptr && ImportSettings.StaticMeshes.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
				}
				else
				{
					if (Asset != nullptr && ImportSettings.StaticMeshes.ProcessingBehavior == EUtuProcessingBehavior::UpdateExisting)
					{
						UTU_LOG_L("        Asset re-import skipped because processing behavior is set to 'UpdateExisting'");
					}
					else
					{
						// Create Asset
						UTU_LOG_L("        Fbx File Fullname: " + InUtuMesh.mesh_file_absolute_filename);

						// Create import options
						UFbxImportUI* Options = GetStaticMeshImportOptions(InUtuMesh, "");
						{
							UTU_LOG_L("            Import Options: ");
							TArray<FString> Prpperties = GetAllPropertiesAsString(Options);
							for (FString Property : Prpperties)
							{
								UTU_LOG_L("                - " + Property);
							}
						}

						// Checkk if already exists (Reimport)
						Asset = GetMeshAsset(AssetNames);
						if (Asset != nullptr)
						{
							// Make sure the new settings are used for the next import
							Asset->AssetImportData = Options->StaticMeshImportData;
						}

						// Process
						AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuMesh.mesh_file_absolute_filename, AssetNames, Options) });

						// Check if worked
						Asset = GetMeshAsset(AssetNames);
						LogAssetImportedOrFailed(Asset, AssetNames, InUtuMesh.mesh_file_absolute_filename, "StaticMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
					}

					// Finalize import
					if (!ImportSettings.StaticMeshes.bImportSeparated)
					{
						// Process Asset
						if (Asset != nullptr)
						{
							// Assign materials
							if (InUtuMesh.mesh_materials_relative_filenames.Num() > 0)
							{
								AssignMaterialsToMesh(InUtuMesh.mesh_materials_relative_filenames, Asset);
							}
							else
							{
								UTU_LOG_W("                    This mesh was combined during the process, but wasn't combined in Unity. Doing my best to assign materials based on submeshes, but material IDs are likely to be wrong...");

								// Find materials in sub meshes instead
								TArray<FString> Materials = TArray<FString>();
								for (int w = 0; w < InUtuMesh.submeshes.Num(); w++)
								{
									for (int x = 0; x < InUtuMesh.submeshes[w].submesh_materials_relative_filenames.Num(); x++)
									{
										Materials.AddUnique(InUtuMesh.submeshes[w].submesh_materials_relative_filenames[x]);
									}
								}

								AssignMaterialsToMesh(InUtuMesh.mesh_materials_relative_filenames, Asset);
							}
						}
					}
					else // Import separated
					{
						bool bImportedSomething = Asset != nullptr;

						// Process Asset
						if (Asset != nullptr)
						{
							// Assign materials
							AssignMaterialsToMesh(InUtuMesh.mesh_materials_relative_filenames, Asset);
						}

						// Keep track of all the meshes so we can add it to a combined blueprint later
						TArray<FUtuPluginActor> MeshesComponentsForCombinedBlueprint = TArray<FUtuPluginActor>();
						// Keep track of LODs
						TMap<int, FString> LodAbsoluteFilenames = TMap<int, FString>();

						for (FUtuPluginSubmesh SubMesh : InUtuMesh.submeshes)
						{
							// Format subasset path
							FString UnityAssetName = AssetNames[0] + "/" + AssetNames[1] + "_" + SubMesh.submesh_name;
							if (!UnityAssetName.ToLower().EndsWith(".fbx") && !UnityAssetName.ToLower().EndsWith(".obj")) // No extension?
							{
								UnityAssetName = UnityAssetName + ".fbx"; // Add one because it'll be removed on the next line
							}
							TArray<FString> SubMeshAssetNames = FormatRelativeFilenameForUnreal(UnityAssetName, EUtuUnrealAssetType::StaticMesh);

							// Get Submesh asset
							UStaticMesh* SubAsset = GetMeshAsset(SubMeshAssetNames);
							UStaticMesh* RealSubMesh = SubAsset;

							// Because sometimes the transform is applied to the first submesh of a mesh in Unity.
							// But in Unreal, if there's only 1 submesh in the mesh, it only creates the root mesh, not the submesh
							// If that happens, apply the transform to the root asset instead
							{
								bool bSameNameAsRoot = AssetNames[1] == SubMesh.submesh_name.Replace(TEXT(" "), TEXT("_"));
								bool bRootIsValid = Asset != nullptr;
								bool bSubIsNotValid = SubAsset == nullptr;
								bool bRootLocIsZero = InUtuMesh.mesh_import_position_offset.Equals(FVector(0.0f), 0.01f);
								bool bRootRotIsZero = InUtuMesh.mesh_import_rotation_offset.Equals(FRotator::ZeroRotator.Quaternion(), 1.0f);
								bool bRootScaIsZero = InUtuMesh.mesh_import_scale_offset.Equals(FVector(1.0f), 0.01f);
								bool bSubLocIsNotZero = !SubMesh.submesh_world_location.Equals(FVector(0.0f), 0.01f);
								bool bSubRotIsNotZero = !SubMesh.submesh_world_rotation.Equals(FRotator::ZeroRotator.Quaternion(), 1.0f);
								bool bSubScaIsNotZero = !SubMesh.submesh_world_scale.Equals(FVector(1.0f), 0.01f);
								if (bSameNameAsRoot && bRootIsValid && bSubIsNotValid && (bRootLocIsZero && bRootRotIsZero && bRootScaIsZero) && (bSubLocIsNotZero || bSubRotIsNotZero || bSubScaIsNotZero))
								{
									SubAsset = Asset;
								}
							}

							if (RealSubMesh != nullptr && RealSubMesh != Asset)
							{
								if (FReimportManager::Instance() != nullptr)
								{
									UFbxAssetImportData* FbxImportData = Cast<UFbxAssetImportData>(SubAsset->AssetImportData);
									if (FbxImportData != nullptr)
									{
										// Check if submesh has correct transform
										UFbxImportUI* Options = GetStaticMeshImportOptions(InUtuMesh, "");
										UFbxImportUI* DesiredOptions = GetStaticMeshImportOptions(InUtuMesh, SubMesh.submesh_name);
										bool bSameLoc = Options->StaticMeshImportData->ImportTranslation == DesiredOptions->StaticMeshImportData->ImportTranslation;
										bool bSameRot = Options->StaticMeshImportData->ImportRotation == DesiredOptions->StaticMeshImportData->ImportRotation;
										bool bSameSca = Options->StaticMeshImportData->ImportUniformScale == DesiredOptions->StaticMeshImportData->ImportUniformScale;
										if (!bSameLoc || !bSameRot || !bSameSca)
										{
											UTU_LOG_L("            Reimporting SubMesh '" + SubMesh.submesh_name + "' with correct transform...");
											FbxImportData->ImportTranslation = DesiredOptions->StaticMeshImportData->ImportTranslation;
											FbxImportData->ImportRotation = DesiredOptions->StaticMeshImportData->ImportRotation;
											FbxImportData->ImportUniformScale = DesiredOptions->StaticMeshImportData->ImportUniformScale;
											AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuMesh.mesh_file_absolute_filename, SubMeshAssetNames, DesiredOptions) });
											//FReimportManager::Instance()->Reimport(SubAsset);
										}
									}
								}
							}

							// Process SubAsset
							if (SubAsset != nullptr)
							{
								if (!bImportedSomething)
								{
									bImportedSomething = true;
									LogAssetImportedOrFailed(SubAsset, AssetNames, InUtuMesh.mesh_file_absolute_filename, "StaticMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
								}

								// Assign materials
								AssignMaterialsToMesh(SubMesh.submesh_materials_relative_filenames, SubAsset);

								// Mesh Component for Combined Blueprint
								{
									FUtuPluginActor MeshComponent = FUtuPluginActor();
									MeshComponent.actor_id = MeshesComponentsForCombinedBlueprint.Num() + 1;
									MeshComponent.actor_parent_id = -999; // Root
									MeshComponent.actor_display_name = SubMeshAssetNames[1];
									MeshComponent.actor_tag = "";
									MeshComponent.actor_is_visible = true;
									MeshComponent.actor_world_location = SubMesh.submesh_world_location;
									MeshComponent.actor_world_rotation = SubMesh.submesh_world_rotation;
									MeshComponent.actor_world_scale = SubMesh.submesh_world_scale;
									MeshComponent.actor_relative_location = SubMesh.submesh_world_location;
									MeshComponent.actor_relative_rotation = SubMesh.submesh_world_rotation;
									MeshComponent.actor_relative_scale = SubMesh.submesh_world_scale;
									MeshComponent.actor_is_movable = false;
									MeshComponent.actor_types.Add(EUtuActorType::StaticMesh);
									MeshComponent.actor_mesh.actor_mesh_relative_filename = SubMeshAssetNames[2];
									MeshComponent.actor_mesh.actor_mesh_relative_filename_if_separated = SubMeshAssetNames[2];
									MeshComponent.actor_mesh.actor_mesh_materials_relative_filenames = SubMesh.submesh_materials_relative_filenames;
									MeshesComponentsForCombinedBlueprint.Add(MeshComponent);
								}

								// LOD
								if (ImportSettings.StaticMeshes.bUtuGenerateLODs)
								{
									if (SubMeshAssetNames[1].Contains("_LOD"))
									{
										FString LodIndex = "";
										SubMeshAssetNames[1].Split("_LOD", nullptr, &LodIndex, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

										if (LodIndex.IsNumeric() && LodIndex != "0") // Ignore main LOD
										{
											FString JsonSourcePath = "";
											json.json_info.json_file_fullname.Replace(TEXT("\\"), TEXT("/")).Split("/Exports/", &JsonSourcePath, nullptr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
											FString LodFbxPath = JsonSourcePath + "/GeneratedLodFbxFiles" + SubMeshAssetNames[2] + ".fbx";

											if (!FPaths::FileExists(LodFbxPath) || ImportSettings.StaticMeshes.ProcessingBehavior == EUtuProcessingBehavior::AlwaysProcess)
											{
												// Export LOD mesh
												UTU_LOG_L("                    Generating LOD FBX for future import. '" + LodFbxPath + "'");
												UExporter* Exporter = UExporter::FindExporter(SubAsset, TEXT("fbx"));
												if (Exporter != nullptr)
												{
													Exporter->SetBatchMode(true);
													Exporter->SetCancelBatch(false);
													Exporter->SetShowExportOption(false);
													Exporter->AddToRoot();

													UAssetExportTask* ExportTask = NewObject<UAssetExportTask>();
													FGCObjectScopeGuard ExportTaskGuard(ExportTask);

													ExportTask->Object = SubAsset;
													ExportTask->Exporter = Exporter;
													ExportTask->Filename = LodFbxPath;
													ExportTask->bSelected = false;
													ExportTask->bReplaceIdentical = true;
													ExportTask->bPrompt = false;
													ExportTask->bUseFileArchive = SubAsset->IsA(UPackage::StaticClass());
													ExportTask->bWriteEmptyFiles = false;
													ExportTask->bAutomated = true;

													UFbxExportOption* Options = NewObject<UFbxExportOption>();
													Options->bASCII = false;
													Options->bForceFrontXAxis = true;
													Options->LevelOfDetail = false;
													Options->Collision = false;
													Options->bExportMorphTargets = false;
													Options->VertexColor = false;
													Options->MapSkeletalMotionToRoot = false;
													Options->bExportLocalTime = true;
													ExportTask->Options = Options;

													IFileManager::Get().MakeDirectory(*FPaths::GetPath(LodFbxPath), true);
													UExporter::RunAssetExportTask(ExportTask);
												}
											}
											if (FPaths::FileExists(LodFbxPath))
											{
												// Save for later
												LodAbsoluteFilenames.Add(FCString::Atoi(*LodIndex), LodFbxPath);
												UTU_LOG_L("                        LOD FBX generated to '" + LodFbxPath + "'");
											}
											else
											{
												UTU_LOG_W("                        Failed to generate LOD FBX to '" + LodFbxPath + "'");
											}
										}
									}
								}
							}
						}

						// Import LODs
						if (ImportSettings.StaticMeshes.bUtuGenerateLODs)
						{
							if (Asset != nullptr)
							{
								UTU_LOG_L("                    Importing LODs...");
								for (int LodIndex = 1; LodIndex < 50; LodIndex++)
								{
									if (LodAbsoluteFilenames.Contains(LodIndex))
									{
										UTU_LOG_L("                    Importing LOD from '" + LodAbsoluteFilenames[LodIndex] + "'");
										
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
										bool bResult = FbxMeshUtils::ImportStaticMeshLOD(Asset, LodAbsoluteFilenames[LodIndex], LodIndex, false);
#else
										bool bResult = FbxMeshUtils::ImportStaticMeshLOD(Asset, LodAbsoluteFilenames[LodIndex], LodIndex);
#endif
										if (!bResult)
										{
											UTU_LOG_W("                        Failed to import LOD from '" + LodAbsoluteFilenames[LodIndex] + "'");
										}
									}
									else
									{
										break;
									}
								}
							}
						}
						

						if (bImportedSomething == false)
						{
							LogAssetImportedOrFailed(nullptr, AssetNames, InUtuMesh.mesh_file_absolute_filename, "StaticMesh", { "Invalid FBX : Make sure that the Fbx file is valid by trying to import it manually in Unreal." });
						}

						// Also create a BlueprintAsset containing all the parts of this mesh
						if (MeshesComponentsForCombinedBlueprint.Num() > 1)
						{
							FUtuPluginPrefabFirstPass PrefabFirstPass = FUtuPluginPrefabFirstPass();
							PrefabFirstPass.asset_name = AssetNames[1] + "_UtuCombined";
							PrefabFirstPass.asset_relative_filename = AssetNames[2] + "_UtuCombined";
							PrefabFirstPass.has_any_static_child = true;
							ProcessPrefabFirstPass(PrefabFirstPass);

							FUtuPluginPrefabSecondPass PrefabSecondPass = FUtuPluginPrefabSecondPass();
							PrefabSecondPass.asset_name = AssetNames[1] + "_UtuCombined";
							PrefabSecondPass.asset_relative_filename = AssetNames[2] + "_UtuCombined";
							PrefabSecondPass.prefab_components = MeshesComponentsForCombinedBlueprint;
							ProcessPrefabSecondPass(PrefabSecondPass);
						}
					}
				}
			}
		}
	}
}

UPackage* FUtuPluginAssetTypeProcessor::CreateAssetPackage(FString InRelativeFilename, bool bLoadPackage) {
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	UPackage* RetPackage = CreatePackage(*InRelativeFilename);
#else
	UPackage* RetPackage = CreatePackage(NULL, *InRelativeFilename);
#endif
	RetPackage->MarkPackageDirty();
	if (bLoadPackage) {
		RetPackage->FullyLoad();
	}
	return RetPackage;
}

void FUtuPluginAssetTypeProcessor::ProcessMaterial(FUtuPluginMaterial InUtuMaterial) {
	if (!InUtuMaterial.asset_relative_filename.StartsWith("Resources")) // Default Unity Material
	{
		// Format Paths
		TArray<FString> AssetNames = StartProcessAsset(InUtuMaterial, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::MaterialInstance : EUtuUnrealAssetType::Material);
		// Invalid Asset
		if (ImportSettings.Materials.bCreateMaterialInstances)
		{
			if (DeleteInvalidAssetIfNeeded(AssetNames, UMaterialInstanceConstant::StaticClass()))
			{
				// Existing Asset
				UMaterialInstanceConstant* Asset = Cast<UMaterialInstanceConstant>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetCreateOrNot(Asset);

				if (ImportSettings.Materials.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
				}
				else if (Asset != nullptr && ImportSettings.Materials.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
				}
				else
				{
					UMaterial* ParentMaterial = GetOrCreateParentMaterial(InUtuMaterial);
					if (ParentMaterial != nullptr)
					{
						if (Asset == nullptr)
						{
							UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
							Factory->InitialParent = ParentMaterial;
							AssetTools->Get().CreateAsset(AssetNames[1], AssetNames[0], UMaterialInstanceConstant::StaticClass(), Factory);
							Asset = Cast<UMaterialInstanceConstant>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
							LogAssetImportedOrFailed(Asset, AssetNames, "", "MaterialInstance", { });
						}

						if (Asset != nullptr)
						{
							Asset->MarkPackageDirty();

							// Parent
							Asset->SetParentEditorOnly(ParentMaterial, true);

							// Clear everything
							Asset->ClearParameterValuesEditorOnly();

							// Two Sided
							Asset->BasePropertyOverrides.bOverride_TwoSided = true;
							Asset->BasePropertyOverrides.TwoSided = InUtuMaterial.two_sided;

							// Blend Mode
							Asset->BasePropertyOverrides.bOverride_BlendMode = true;
							switch (InUtuMaterial.shader_opacity)
							{
							default:
								break;
							case EUtuShaderOpacity::Opaque:
								Asset->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Opaque;
								break;
							case EUtuShaderOpacity::Masked:
								Asset->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Masked;
								break;
							case EUtuShaderOpacity::Translucent:
								Asset->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Translucent;
								break;
							}

							// Settings
							if (ImportSettings.Materials.bOverrideMetallicIntensityMultiplier)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_MetallicIntensityMultiplier"), ImportSettings.Materials.MetallicIntensityMultiplier);
							if (ImportSettings.Materials.bOverrideSpecularIntensityMultiplier)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_SpecularIntensityMultiplier"), ImportSettings.Materials.SpecularIntensityMultiplier);
							if (ImportSettings.Materials.bOverrideEmissiveIntensityMultiplier)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_EmissiveIntensityMultiplier"), ImportSettings.Materials.EmissiveIntensityMultiplier);
							if (ImportSettings.Materials.bOverrideNormalIntensityMultiplier)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_NormalIntensityMultiplier"), ImportSettings.Materials.NormalIntensityMultiplier);
							if (ImportSettings.Materials.bOverrideOcclusionIntensityMultiplier)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_OcclusionIntensityMultiplier"), ImportSettings.Materials.OcclusionIntensityMultiplier);
							if (ImportSettings.Materials.bOverrideTexturesPannerTime)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime"), ImportSettings.Materials.TexturesPannerTime);
							if (ImportSettings.Materials.bOverrideRoughnessMultiplier)
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_RoughnessMultiplier"), ImportSettings.Materials.RoughnessMultiplier);

							// Main color and texture for custom shaders
							UTexture2D* MainTextureAsset = GetTextureFromUnityRelativeFilename(InUtuMaterial.main_texture);
							if (MainTextureAsset != nullptr)
								Asset->SetTextureParameterValueEditorOnly(FName("__Main_Texture"), MainTextureAsset);
							Asset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime___Main_Texture"), ImportSettings.Materials.TexturesPannerTime);
							Asset->SetVectorParameterValueEditorOnly(FName("__Main_Color"), HexToColor(InUtuMaterial.main_color));

							// Floats
							for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_floats_names.Num(), InUtuMaterial.material_floats.Num()); Idx++)
							{
								FString Name = InUtuMaterial.material_floats_names[Idx];
								float Value = InUtuMaterial.material_floats[Idx];
								Asset->SetScalarParameterValueEditorOnly(*Name, Value);
							}

							// Textures
							for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_textures_names.Num(), InUtuMaterial.material_textures.Num()); Idx++)
							{
								FString Name = InUtuMaterial.material_textures_names[Idx];
								UTexture2D* Value = GetTextureFromUnityRelativeFilename(InUtuMaterial.material_textures[Idx]);
								if (Value != nullptr)
								{
									Asset->SetTextureParameterValueEditorOnly(*Name, Value);
								}
								Asset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime_" + Name), ImportSettings.Materials.TexturesPannerTime);
							}

							// Colors
							for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_colors_names.Num(), InUtuMaterial.material_colors.Num()); Idx++)
							{
								FString Name = InUtuMaterial.material_colors_names[Idx];
								FLinearColor Value = HexToColor(InUtuMaterial.material_colors[Idx]);
								Asset->SetVectorParameterValueEditorOnly(*Name, Value);
							}

							// Vectors
							for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vectors_names.Num(), InUtuMaterial.material_vectors.Num()); Idx++)
							{
								FString Name = InUtuMaterial.material_vectors_names[Idx];
								FQuat Value = InUtuMaterial.material_vectors[Idx];
								Asset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, Value.Z, Value.W));
							}

							// Vector2s
							for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vector2s_names.Num(), InUtuMaterial.material_vector2s.Num()); Idx++)
							{
								FString Name = InUtuMaterial.material_vector2s_names[Idx];
								FVector2D Value = InUtuMaterial.material_vector2s[Idx];
								Asset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, 0.0f, 0.0f));
							}

							// Ints
							for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_ints_names.Num(), InUtuMaterial.material_ints.Num()); Idx++)
							{
								FString Name = InUtuMaterial.material_ints_names[Idx];
								int Value = InUtuMaterial.material_ints[Idx];
								Asset->SetScalarParameterValueEditorOnly(*Name, Value);
							}

							Asset->PostEditChange();
						}
					}
					else
					{
						UTU_LOG_E("        Cannot process material instance because parent material doesn't exist.");
					}
				}
			}
		}
		else
		{
			if (DeleteInvalidAssetIfNeeded(AssetNames, UMaterial::StaticClass()))
			{
				// Existing Asset
				UMaterial* MatAsset = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetCreateOrNot(MatAsset);

				if (ImportSettings.Materials.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
				}
				else if (MatAsset != nullptr && ImportSettings.Materials.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
				{
					UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
				}
				else
				{
					UMaterial* ParentMaterial = GetOrCreateParentMaterial(InUtuMaterial);

					if (MatAsset == nullptr)
					{
						UEditorAssetLibrary::DuplicateLoadedAsset(ParentMaterial, AssetNames[2]);
						MatAsset = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
						LogAssetImportedOrFailed(MatAsset, AssetNames, "", "Material", { });
					}

					if (MatAsset != nullptr)
					{
						MatAsset->MarkPackageDirty();

						// Two Sided
						MatAsset->TwoSided = InUtuMaterial.two_sided;

						// Blend Mode
						switch (InUtuMaterial.shader_opacity)
						{
						default:
							break;
						case EUtuShaderOpacity::Opaque:
							MatAsset->BlendMode = EBlendMode::BLEND_Opaque;
							break;
						case EUtuShaderOpacity::Masked:
							MatAsset->BlendMode = EBlendMode::BLEND_Masked;
							break;
						case EUtuShaderOpacity::Translucent:
							MatAsset->BlendMode = EBlendMode::BLEND_Translucent;
							break;
						}

						// Settings
						if (ImportSettings.Materials.bOverrideMetallicIntensityMultiplier)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_MetallicIntensityMultiplier"), ImportSettings.Materials.MetallicIntensityMultiplier);
						if (ImportSettings.Materials.bOverrideSpecularIntensityMultiplier)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_SpecularIntensityMultiplier"), ImportSettings.Materials.SpecularIntensityMultiplier);
						if (ImportSettings.Materials.bOverrideEmissiveIntensityMultiplier)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_EmissiveIntensityMultiplier"), ImportSettings.Materials.EmissiveIntensityMultiplier);
						if (ImportSettings.Materials.bOverrideNormalIntensityMultiplier)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_NormalIntensityMultiplier"), ImportSettings.Materials.NormalIntensityMultiplier);
						if (ImportSettings.Materials.bOverrideOcclusionIntensityMultiplier)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_OcclusionIntensityMultiplier"), ImportSettings.Materials.OcclusionIntensityMultiplier);
						if (ImportSettings.Materials.bOverrideTexturesPannerTime)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime"), ImportSettings.Materials.TexturesPannerTime);
						if (ImportSettings.Materials.bOverrideRoughnessMultiplier)
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_RoughnessMultiplier"), ImportSettings.Materials.RoughnessMultiplier);

						// Main color and texture for custom shaders
						UTexture2D* MainTextureAsset = GetTextureFromUnityRelativeFilename(InUtuMaterial.main_texture);
						if (MainTextureAsset != nullptr)
							MatAsset->SetTextureParameterValueEditorOnly("__Main_Texture", MainTextureAsset);
						MatAsset->SetScalarParameterValueEditorOnly("Utu_TexturesPannerTime___Main_Texture", ImportSettings.Materials.TexturesPannerTime);
						MatAsset->SetVectorParameterValueEditorOnly("__Main_Color", HexToColor(InUtuMaterial.main_color));

						// Floats
						for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_floats_names.Num(), InUtuMaterial.material_floats.Num()); Idx++)
						{
							FString Name = InUtuMaterial.material_floats_names[Idx];
							float Value = InUtuMaterial.material_floats[Idx];
							MatAsset->SetScalarParameterValueEditorOnly(*Name, Value);
						}

						// Textures
						for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_textures_names.Num(), InUtuMaterial.material_textures.Num()); Idx++)
						{
							FString Name = InUtuMaterial.material_textures_names[Idx];
							UTexture2D* Value = GetTextureFromUnityRelativeFilename(InUtuMaterial.material_textures[Idx]);
							if (Value != nullptr)
							{
								MatAsset->SetTextureParameterValueEditorOnly(*Name, Value);
							}
							MatAsset->SetScalarParameterValueEditorOnly(FName("Utu_TexturesPannerTime_" + Name), ImportSettings.Materials.TexturesPannerTime);
						}

						// Colors
						for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_colors_names.Num(), InUtuMaterial.material_colors.Num()); Idx++)
						{
							FString Name = InUtuMaterial.material_colors_names[Idx];
							FLinearColor Value = HexToColor(InUtuMaterial.material_colors[Idx]);
							MatAsset->SetVectorParameterValueEditorOnly(*Name, Value);
						}

						// Vectors
						for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vectors_names.Num(), InUtuMaterial.material_vectors.Num()); Idx++)
						{
							FString Name = InUtuMaterial.material_vectors_names[Idx];
							FQuat Value = InUtuMaterial.material_vectors[Idx];
							MatAsset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, Value.Z, Value.W));
						}

						// Vector2s
						for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vector2s_names.Num(), InUtuMaterial.material_vector2s.Num()); Idx++)
						{
							FString Name = InUtuMaterial.material_vector2s_names[Idx];
							FVector2D Value = InUtuMaterial.material_vector2s[Idx];
							MatAsset->SetVectorParameterValueEditorOnly(*Name, FLinearColor(Value.X, Value.Y, 0.0f, 0.0f));
						}

						// Ints
						for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_ints_names.Num(), InUtuMaterial.material_ints.Num()); Idx++)
						{
							FString Name = InUtuMaterial.material_ints_names[Idx];
							int Value = InUtuMaterial.material_ints[Idx];
							MatAsset->SetScalarParameterValueEditorOnly(*Name, Value);
						}

						MatAsset->PostEditChange();
					}
				}
			}
		}
	}
}

UMaterial* FUtuPluginAssetTypeProcessor::GetOrCreateParentMaterial(FUtuPluginMaterial InUtuMaterial)
{
	FString MatName = InUtuMaterial.shader_name;
	MatName = MatName.Replace(TEXT(" "), TEXT(""));
	MatName = MatName.Replace(TEXT("."), TEXT("_"));
	MatName = MatName.Replace(TEXT("/"), TEXT("_"));
	MatName = MatName.Replace(TEXT("("), TEXT(""));
	MatName = MatName.Replace(TEXT(")"), TEXT(""));

	// Default material (Supported materials)
	FString MatDir = "/Game/Utu/Shaders/";
	FString MatPath = MatDir + MatName;
	UMaterial* Material = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(MatPath));
	if (Material != nullptr)
	{
		UTU_LOG_L("        Parent Material: \"" + MatPath + "\"");
		return Material;
	}

	// Custom material (Unsupported materials)
	MatDir = "/Game/Utu/CustomShaders/";
	MatPath = MatDir + MatName;
	Material = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(MatPath));
	if (Material != nullptr)
	{
		UTU_LOG_L("        Parent Material: \"" + MatPath + "\"");
		return Material;
	}

	// Create custom material
	UTU_LOG_W("        No Supported Material Found. Creating Custom Material: \"" + MatPath + "\" ...");
	if (ImportSettings.Materials.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
	{
		UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
		return nullptr;
	}

	UPackage* Package = CreateAssetPackage(MatPath, false);
	NewObject<UMaterialFactoryNew>()->FactoryCreateNew(UMaterial::StaticClass(), Package, FName(*MatName), RF_Public | RF_Standalone, NULL, GWarn);
	Material = Cast<UMaterial>(UUtuPluginLibrary::TryGetAsset(MatPath));
	LogAssetImportedOrFailed(Material, { MatDir, MatName, MatPath }, "", "Material", { });

	// Process Material
	if (Material != nullptr)
	{
		UTU_LOG_L("        Building Material...");
		Material->PreEditChange(NULL);
		Material->GetOutermost()->FullyLoad();
		Material->MarkPackageDirty();

		int H = -2000;
		int V = 700;
		// Settings
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.MetallicIntensityMultiplier, "Utu_MetallicIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.SpecularIntensityMultiplier, "Utu_SpecularIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.EmissiveIntensityMultiplier, "Utu_EmissiveIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.NormalIntensityMultiplier, "Utu_NormalIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.OcclusionIntensityMultiplier, "Utu_OcclusionIntensityMultiplier", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.TexturesPannerTime, "Utu_TexturesPannerTime", H, V, InUtuMaterial);
		H += 300;
		GetOrCreateScalarParameter(Material, ImportSettings.Materials.RoughnessMultiplier, "Utu_RoughnessMultiplier", H, V, InUtuMaterial);


		H = -2000;
		V = 1000;
		// Floats
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_floats_names.Num(), InUtuMaterial.material_floats.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_floats_names[Idx];
			float Value = InUtuMaterial.material_floats[Idx];
			UMaterialExpressionScalarParameter* Scalar = GetOrCreateScalarParameter(Material, Value, *Name, H, V, InUtuMaterial);
			H += 300;
		}

		H = -2000;
		V = 1300;
		// Textures & Panners
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_textures_names.Num(), InUtuMaterial.material_textures.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_textures_names[Idx];
			UTexture2D* Value = GetTextureFromUnityRelativeFilename(InUtuMaterial.material_textures[Idx]);
			UMaterialExpressionTextureSampleParameter2D* Texture = GetOrCreateTextureParameter(Material, Value, *Name, H + 600, V, InUtuMaterial);

			int TexCoordIndex = InUtuMaterial.material_vector2s_names.Find(Name + "_ST_TexCoord");
			int PannerIndex = InUtuMaterial.material_vector2s_names.Find(Name + "_ST_Panner");
			if (TexCoordIndex >= 0 && PannerIndex >= 0)
			{
				FString TexCoordName = InUtuMaterial.material_vector2s_names[TexCoordIndex];
				FVector2D TexCoordValue = FVector2D(1.0f);
				if (InUtuMaterial.material_vector2s.IsValidIndex(TexCoordIndex))
				{
					TexCoordValue = InUtuMaterial.material_vector2s[TexCoordIndex];
				}
				UMaterialExpressionTextureCoordinate* TexCoordNode = GetOrCreateTexCoordExpression(Material, FVector2D(1.0f, 1.0f), TexCoordName + "_Node", H, V, InUtuMaterial);
				UMaterialExpressionVectorParameter* TexCoord = GetOrCreateVectorParameter(Material, FLinearColor(TexCoordValue.X, TexCoordValue.Y, 0.0f, 0.0f), *TexCoordName, H, V + 50, InUtuMaterial);
				UMaterialExpressionComponentMask* TexCoordMask = GetOrCreateMaskExpression(Material, TexCoordName + "_Mask", H + 200, V + 50, InUtuMaterial);
				UMaterialExpressionMultiply* TexCoordMultiply = GetOrCreateMultiplyExpression(Material, TexCoordName + "_Multiply", H + 350, V, InUtuMaterial);

				FString PannerName = InUtuMaterial.material_vector2s_names[PannerIndex];
				UMaterialExpressionPanner* Panner = GetOrCreatePannerExpression(Material, FVector2D(0.0f, 0.0f), PannerName + "_Node", H + 475, V, InUtuMaterial);
				UMaterialExpressionScalarParameter* PannerTime = GetOrCreateScalarParameter(Material, ImportSettings.Materials.TexturesPannerTime, *("Utu_TexturesPannerTime_" + Name), H, V + 250, InUtuMaterial);

				if (TexCoordNode != nullptr && TexCoord != nullptr && TexCoordMask != nullptr && TexCoordMultiply != nullptr && PannerTime != nullptr && Panner != nullptr)
				{
					TexCoordNode->ConnectExpression(&TexCoordMultiply->A, 0);
					TexCoord->ConnectExpression(&TexCoordMask->Input, 2);
					TexCoordMask->ConnectExpression(&TexCoordMultiply->B, 0);
					TexCoordMultiply->ConnectExpression(&Panner->Coordinate, 0);
					PannerTime->ConnectExpression(&Panner->Time, 0);
					Panner->ConnectExpression(&Texture->Coordinates, 0);
				}
			}
			H += 900;
		}


		H = -2000;
		V = 1800;
		// Colors
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_colors_names.Num(), InUtuMaterial.material_colors.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_colors_names[Idx];
			FLinearColor Value = HexToColor(InUtuMaterial.material_colors[Idx]);
			UMaterialExpressionVectorParameter* Color = GetOrCreateVectorParameter(Material, Value, *Name, H, V, InUtuMaterial);
			H += 300;
		}


		H = -2000;
		V = 2100;
		// Vectors
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vectors_names.Num(), InUtuMaterial.material_vectors.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_vectors_names[Idx];
			FQuat Value = InUtuMaterial.material_vectors[Idx];
			UMaterialExpressionVectorParameter* Vector = GetOrCreateVectorParameter(Material, FLinearColor(Value.X, Value.Y, Value.Z, Value.W), *Name, H, V, InUtuMaterial);
			H += 300;
		}


		H = -2000;
		V = 2400;
		// Vector2s
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_vector2s_names.Num(), InUtuMaterial.material_vector2s.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_vector2s_names[Idx];
			FVector2D Value = InUtuMaterial.material_vector2s[Idx];
			if (!Name.EndsWith("_ST_TexCoord") && !Name.EndsWith("_ST_Panner"))
			{
				UMaterialExpressionVectorParameter* Vector = GetOrCreateVectorParameter(Material, FLinearColor(Value.X, Value.Y, 0.0f, 0.0f), *Name, H, V, InUtuMaterial);
				H += 300;
			}
		}


		H = -2000;
		V = 2700;
		// Ints
		for (int Idx = 0; Idx < FMath::Min(InUtuMaterial.material_ints_names.Num(), InUtuMaterial.material_ints.Num()); Idx++)
		{
			FString Name = InUtuMaterial.material_ints_names[Idx];
			int Value = InUtuMaterial.material_ints[Idx];
			UMaterialExpressionScalarParameter* Scalar = GetOrCreateScalarParameter(Material, Value, *Name, H, V, InUtuMaterial);
			H += 300;
		}


		// Custom material visual
		H = -600;
		V = 0;
		{
			// Default blend mode
			Material->BlendMode = EBlendMode::BLEND_Masked;
			// Default nodes
			UTexture2D* MainTextureAsset = GetTextureFromUnityRelativeFilename(InUtuMaterial.main_texture);
			UMaterialExpressionTextureSampleParameter2D* MainTexture = GetOrCreateTextureParameter(Material, MainTextureAsset, "__Main_Texture", H, V, InUtuMaterial);
			UMaterialExpressionPanner* MainTexturePanner = GetOrCreatePannerExpression(Material, InUtuMaterial.main_texture_offset, "__Main_Texture_panner", H - 150, V, InUtuMaterial);
			UMaterialExpressionTextureCoordinate* MainTextureTexCoord = GetOrCreateTexCoordExpression(Material, InUtuMaterial.main_texture_scale, "__Main_Texture_TexCoord", H - 350, V, InUtuMaterial);
			UMaterialExpressionScalarParameter* MainTexturePannerTime = GetOrCreateScalarParameter(Material, ImportSettings.Materials.TexturesPannerTime, "Utu_TexturesPannerTime___Main_Texture", H - 500, V + 100, InUtuMaterial);
			UMaterialExpressionVectorParameter* MainTextureColor = GetOrCreateVectorParameter(Material, HexToColor(InUtuMaterial.main_color), "__Main_Color", H, V + 250, InUtuMaterial);
			UMaterialExpressionMultiply* MainTextureMultiply = GetOrCreateMultiplyExpression(Material, "__Main_Texture_Multiply_With_Color", H + 300, V + 100, InUtuMaterial);
			UMaterialExpressionMultiply* MainTextureAlphaMultiply = GetOrCreateMultiplyExpression(Material, "__Main_Texture_Multiply_With_Color_Alpha", H + 300, V + 250, InUtuMaterial);
			if (MainTexture != nullptr && MainTextureColor != nullptr && MainTextureMultiply != nullptr && MainTextureAlphaMultiply != nullptr && MainTexturePanner != nullptr && MainTexturePannerTime != nullptr && MainTextureTexCoord != nullptr)
			{
#if ENGINE_MAJOR_VERSION >= 5
				Material->GetEditorOnlyData()->Opacity.Connect(0, MainTextureAlphaMultiply);
				Material->GetEditorOnlyData()->OpacityMask.Connect(0, MainTextureAlphaMultiply);
				Material->GetEditorOnlyData()->BaseColor.Connect(0, MainTextureMultiply);
#else
				Material->Opacity.Connect(0, MainTextureAlphaMultiply);
				Material->OpacityMask.Connect(0, MainTextureAlphaMultiply);
				Material->BaseColor.Connect(0, MainTextureMultiply);
#endif
				MainTexture->ConnectExpression(&MainTextureMultiply->A, 0);
				MainTextureColor->ConnectExpression(&MainTextureMultiply->B, 0);
				MainTexture->ConnectExpression(&MainTextureAlphaMultiply->A, 4);
				MainTextureColor->ConnectExpression(&MainTextureAlphaMultiply->B, 4);
				MainTexturePanner->ConnectExpression(&MainTexture->Coordinates, 0);
				MainTexturePannerTime->ConnectExpression(&MainTexturePanner->Time, 0);
				MainTextureTexCoord->ConnectExpression(&MainTexturePanner->Coordinate, 0);
			}
		}
	}

	Material->PostEditChange();
	FGlobalComponentReregisterContext RecreateComponents;
	return Material;
}


FLinearColor FUtuPluginAssetTypeProcessor::HexToColor(FString InHex) {
	return FLinearColor(FColor::FromHex(InHex));
}


UTexture2D* FUtuPluginAssetTypeProcessor::GetTextureFromUnityRelativeFilename(FString InUnityRelativeFilename) {
	if (InUnityRelativeFilename != "") {
		TArray<FString> TexNames = FormatRelativeFilenameForUnreal(InUnityRelativeFilename, EUtuUnrealAssetType::Texture);
		UTU_LOG_L("            Texture: " + TexNames[2]);
		UTexture2D* TextureAsset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset(TexNames[2]));
		if (TextureAsset == nullptr) {
			UTU_LOG_W("                Failed to associate texture because it doesn't exists: '" + TexNames[2] + "'");
			TextureAsset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset("/Game/Utu/Assets/Texture"));
		}
		return TextureAsset;
	}
	return nullptr;
}


UMaterialExpressionTextureSampleParameter2D* FUtuPluginAssetTypeProcessor::GetOrCreateTextureParameter(UMaterial* InMaterial, UTexture* InTexture, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionTextureSampleParameter2D* Ret = nullptr;
	UTU_LOG_L("                Texture Parameter: " + InParamName.ToString());
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionTextureSampleParameter2D* Parameter = Cast<UMaterialExpressionTextureSampleParameter2D>(Exp);
			if (Parameter != nullptr && Parameter->ParameterName == InParamName) {
				UTU_LOG_L("                    Texture Parameter found in material.");
				Ret = Parameter;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionTextureSampleParameter2D>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionTextureSampleParameter2D::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                    Texture Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Texture Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionTextureSampleParameter2D>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                    Texture Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Texture Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			if (InTexture == nullptr)
			{
				InTexture = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset("/Game/Utu/Assets/Texture"));
			}
			Ret->Texture = InTexture;
			Ret->SamplerType = SAMPLERTYPE_Color;
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
	}
	return Ret;
}

UMaterialExpressionScalarParameter* FUtuPluginAssetTypeProcessor::GetOrCreateScalarParameter(UMaterial * InMaterial, float InValue, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionScalarParameter* Ret = nullptr;
	UTU_LOG_L("                Scalar Parameter: " + InParamName.ToString());
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionScalarParameter* Parameter = Cast<UMaterialExpressionScalarParameter>(Exp);
			if (Parameter != nullptr && Parameter->ParameterName == InParamName) {
				UTU_LOG_L("                    Scalar Parameter found in material.");
				Ret = Parameter;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionScalarParameter>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionScalarParameter::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                    Scalar Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Scalar Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionScalarParameter>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                    Scalar Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Scalar Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->DefaultValue = InValue;
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
		}
	return Ret;
	}

UMaterialExpressionVectorParameter* FUtuPluginAssetTypeProcessor::GetOrCreateVectorParameter(UMaterial * InMaterial, FLinearColor InColor, FName InParamName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionVectorParameter* Ret = nullptr;
	UTU_LOG_L("                Vector Parameter: " + InParamName.ToString());
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionVectorParameter* Parameter = Cast<UMaterialExpressionVectorParameter>(Exp);
			if (Parameter != nullptr && Parameter->ParameterName == InParamName) {
				UTU_LOG_L("                    Vector Parameter found in material.");
				Ret = Parameter;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionVectorParameter>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionVectorParameter::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                    Vector Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Vector Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionVectorParameter>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                    Vector Parameter added into material.");
				Ret->ParameterName = InParamName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Vector Parameter expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->DefaultValue = InColor;
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
		}
	return Ret;
	}

UMaterialExpressionComponentMask* FUtuPluginAssetTypeProcessor::GetOrCreateMaskExpression(UMaterial * InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionComponentMask* Ret = nullptr;
	UTU_LOG_L("                Mask Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionComponentMask* Expression = Cast<UMaterialExpressionComponentMask>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    Mask Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionComponentMask>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionComponentMask::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   Mask Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Mask Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionComponentMask>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   Mask Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Mask Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
			Ret->R = true;
			Ret->G = true;
			Ret->B = false;
			Ret->A = false;
		}
		}
	return Ret;
	}

UMaterialExpressionMultiply* FUtuPluginAssetTypeProcessor::GetOrCreateMultiplyExpression(UMaterial * InMaterial, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionMultiply* Ret = nullptr;
	UTU_LOG_L("                Multiply Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionMultiply* Expression = Cast<UMaterialExpressionMultiply>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    Multiply Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionMultiply>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionMultiply::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   Multiply Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to createMultiply Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionMultiply>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   Multiply Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to createMultiply Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
		}
		}
	return Ret;
	}

UMaterialExpressionPanner* FUtuPluginAssetTypeProcessor::GetOrCreatePannerExpression(UMaterial * InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionPanner* Ret = nullptr;
	UTU_LOG_L("                Panner Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionPanner* Expression = Cast<UMaterialExpressionPanner>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    Panner Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionPanner>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionPanner::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   Panner Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create Panner Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionPanner>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   Panner Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create Panner Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
			Ret->SpeedX = InValue.X;
			Ret->SpeedY = InValue.Y;
		}
		}
	return Ret;
	}

UMaterialExpressionTextureCoordinate* FUtuPluginAssetTypeProcessor::GetOrCreateTexCoordExpression(UMaterial * InMaterial, FVector2D InValue, FString InExpressionName, int InPosX, int InPosY, FUtuPluginMaterial InUtuMaterial) {
	UMaterialExpressionTextureCoordinate* Ret = nullptr;
	UTU_LOG_L("                TexCoord Expression: " + InExpressionName);
	if (InMaterial != nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
		for (UMaterialExpression* Exp : InMaterial->GetExpressions()) {
#else
		for (UMaterialExpression* Exp : InMaterial->Expressions) {
#endif
			UMaterialExpressionTextureCoordinate* Expression = Cast<UMaterialExpressionTextureCoordinate>(Exp);
			if (Expression != nullptr && Expression->Desc == InExpressionName) {
				UTU_LOG_L("                    TexCoord Expression found in material.");
				Ret = Expression;
				break;
			}
		}
		if (Ret == nullptr) {
#if ENGINE_MAJOR_VERSION >= 5
			Ret = Cast<UMaterialExpressionTextureCoordinate>(UMaterialEditingLibrary::CreateMaterialExpression(InMaterial, UMaterialExpressionTextureCoordinate::StaticClass()));
			if (Ret != nullptr) {
				UTU_LOG_L("                   TexCoord Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->AddExpressionParameter(Ret, InMaterial->EditorParameters);
			}
			else {
				UTU_LOG_E("                    Failed to create TexCoord Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#else
			Ret = NewObject<UMaterialExpressionTextureCoordinate>(InMaterial);
			if (Ret != nullptr) {
				UTU_LOG_L("                   TexCoord Expression added into material.");
				Ret->Desc = InExpressionName;
				InMaterial->Expressions.Add(Ret);
			}
			else {
				UTU_LOG_E("                    Failed to create TexCoord Expression expression for Material: '" + InUtuMaterial.asset_relative_filename + "'. This is not normal and should never happen.");
			}
#endif
		}
		if (Ret != nullptr) {
			Ret->MaterialExpressionEditorX = InPosX;
			Ret->MaterialExpressionEditorY = InPosY;
			Ret->UTiling = InValue.X;
			Ret->VTiling = InValue.Y;
		}
		}
	return Ret;
	}

void FUtuPluginAssetTypeProcessor::ProcessTexture(FUtuPluginTexture InUtuTexture) {
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuTexture, EUtuUnrealAssetType::Texture);
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames, UTexture2D::StaticClass())) {
		// Existing Asset
		UTexture2D* Asset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		LogAssetImportOrReimport(Asset);

		if (ImportSettings.Textures.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
		}
		else if (Asset != nullptr && ImportSettings.Textures.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
		}
		else
		{
			if (Asset != nullptr && ImportSettings.Textures.ProcessingBehavior == EUtuProcessingBehavior::UpdateExisting)
			{
				UTU_LOG_L("        Asset re-import skipped because processing behavior is set to 'UpdateExisting'");
			}
			else
			{
				// Create Asset
				AssetTools->Get().ImportAssetTasks({ BuildTask(InUtuTexture.texture_file_absolute_filename, AssetNames, nullptr) });
				Asset = Cast<UTexture2D>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
				LogAssetImportedOrFailed(Asset, AssetNames, InUtuTexture.texture_file_absolute_filename, "Texture", { "Invalid Texture File : Make sure that the texture file is a supported format by trying to import it manually in Unreal." });
			}

			if (Asset != nullptr) 
			{
				Asset->PreEditChange(NULL);
				if (Asset->IsNormalMap())
				{
					Asset->bFlipGreenChannel = ImportSettings.Textures.bFlipNormalMapGreenChannel;
				}
				else
				{
					Asset->bFlipGreenChannel = false;
					Asset->CompressionSettings = ImportSettings.Textures.CompressionSettings;
				}
				Asset->Filter = ImportSettings.Textures.Filter;
				Asset->LODGroup = ImportSettings.Textures.LODGroup;
				Asset->SRGB = ImportSettings.Textures.SRGB;
				Asset->MaxTextureSize = ImportSettings.Textures.MaxTextureSize;
				Asset->CompressionQuality = ImportSettings.Textures.CompressionQuality;
				Asset->MipGenSettings = ImportSettings.Textures.MipGenSettings;
				Asset->DeferCompression = false;
				Asset->PostEditChange();
			}
		}
	}
}

void FUtuPluginAssetTypeProcessor::LogAssetCreateOrNot(UObject * InAsset) {
	if (InAsset == nullptr) {
		UTU_LOG_L("    New Asset. Creating...");
	}
	else {
		UTU_LOG_L("    Existing Asset.");
	}
}

void FUtuPluginAssetTypeProcessor::LogAssetImportOrReimport(UObject * InAsset) {
	if (InAsset == nullptr) {
		UTU_LOG_L("    New Asset. Importing...");
	}
	else {
		UTU_LOG_L("    Existing Asset. Re-Importing...");
	}
}

void FUtuPluginAssetTypeProcessor::LogAssetImportedOrFailed(UObject * InAsset, TArray<FString> InAssetNames, FString InSourceFileFullname, FString InAssetType, TArray<FString> InPotentialCauses) {
	if (InAsset != nullptr) {
		FAssetRegistryModule::AssetCreated(InAsset);
		UTU_LOG_L("            Asset Created.");
	}
	else {
		UTU_LOG_E("    Asset Name: " + InAssetNames[1]);
		UTU_LOG_E("        Unreal Asset Relative Filename: " + InAssetNames[2]);
		if (InSourceFileFullname != "") {
			UTU_LOG_E("        Source File Fullname: " + InSourceFileFullname);
		}
		UTU_LOG_E("            Failed to create new " + InAssetType);
		UTU_LOG_E("            Potential Causes:");
		if (InPotentialCauses.Num() == 0) {
			InPotentialCauses.Add("No Potential Causes known yet.");
		}
		for (FString x : InPotentialCauses) {
			UTU_LOG_E("                - " + x);
		}
		UTU_LOG_E("            Asset skipped.");
	}
}


void FUtuPluginAssetTypeProcessor::ProcessPrefabFirstPass(FUtuPluginPrefabFirstPass InUtuPrefabFirstPass) {
	// Make sure it does not save the bp on compile
	UBlueprintEditorSettings* Settings = GetMutableDefault<UBlueprintEditorSettings>();
	ESaveOnCompile OriginalSaveOnCompile = Settings->SaveOnCompile;
	Settings->SaveOnCompile = ESaveOnCompile::SoC_Never;
	Settings->SaveConfig();
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuPrefabFirstPass, EUtuUnrealAssetType::Blueprint);
	// Invalid Asset
	if (DeleteInvalidAssetIfNeeded(AssetNames, UBlueprint::StaticClass()))
	{
		// Existing Asset
		UBlueprint* Asset = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
		LogAssetCreateOrNot(Asset);

		if (ImportSettings.Blueprints.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
		}
		else if (Asset != nullptr && ImportSettings.Blueprints.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
		{
			UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
		}
		else
		{
			// Create Asset
			IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
			if (Asset == nullptr)
			{
				// Create Bp
				UPackage* Package = CreateAssetPackage(AssetNames[2], false);
				UClass* BlueprintClass = nullptr;
				UClass* BlueprintGeneratedClass = nullptr;
				KismetCompilerModule.GetBlueprintTypesForClass(AActor::StaticClass(), BlueprintClass, BlueprintGeneratedClass);
				Asset = FKismetEditorUtilities::CreateBlueprint(AActor::StaticClass(), Package, *AssetNames[1], BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass, FName("LevelEditorActions"));
				LogAssetImportedOrFailed(Asset, AssetNames, "", "Blueprint", { });
				// Create Root Node
				BpAddRootComponent(Asset, InUtuPrefabFirstPass.has_any_static_child);
			}
			if (Asset != nullptr)
			{
				Asset->PreEditChange(NULL);
				TArray<USCS_Node*> ExistingNodes = Asset->SimpleConstructionScript->GetAllNodes();
				if (ExistingNodes.Num() > 1) 
				{
					UTU_LOG_L("    Detected existing Nodes. Deleting them and re-building the Blueprint from scratch.");
					while (ExistingNodes.Num() > 1) {
						TArray<USCS_Node*> RootNodes = Asset->SimpleConstructionScript->GetRootNodes();
						for (USCS_Node* Node : ExistingNodes) 
						{
							if (!RootNodes.Contains(Node)) 
							{
								Asset->SimpleConstructionScript->RemoveNode(Node);
							}
						}
						Asset->SimpleConstructionScript->FixupRootNodeParentReferences();
						ExistingNodes = Asset->SimpleConstructionScript->GetAllNodes();
					}
				}
				Cast<USceneComponent>(ExistingNodes[0]->ComponentTemplate)->SetMobility(InUtuPrefabFirstPass.has_any_static_child ? EComponentMobility::Static : EComponentMobility::Movable);
				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Asset);
				FKismetEditorUtilities::CompileBlueprint(Asset);
				Asset->PostEditChange();
			}
		}
	}
	// Restore Save On Compile
	Settings->SaveOnCompile = OriginalSaveOnCompile;
	Settings->SaveConfig();
}

void FUtuPluginAssetTypeProcessor::ProcessPrefabSecondPass(FUtuPluginPrefabSecondPass InUtuPrefabSecondPass) 
{
	// Make sure it does not save the bp on compile
	UBlueprintEditorSettings* Settings = GetMutableDefault<UBlueprintEditorSettings>();
	ESaveOnCompile OriginalSaveOnCompile = Settings->SaveOnCompile;
	Settings->SaveOnCompile = ESaveOnCompile::SoC_Never;
	Settings->SaveConfig();
	// Format Paths
	TArray<FString> AssetNames = StartProcessAsset(InUtuPrefabSecondPass, EUtuUnrealAssetType::Blueprint);
	// Existing Asset
	UBlueprint* Asset = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));

	if (ImportSettings.Blueprints.ProcessingBehavior == EUtuProcessingBehavior::DoNotProcess)
	{
		UTU_LOG_L("        Asset skipped because processing behavior is set to 'DoNotProcess'");
	}
	else if (Asset != nullptr && ImportSettings.Blueprints.ProcessingBehavior == EUtuProcessingBehavior::SkipExisting)
	{
		UTU_LOG_L("        Asset skipped because processing behavior is set to 'SkipExisting' and asset exists.");
	}
	else
	{
		// Skipping Asset. Should already be created by FirstPass
		if (Asset == nullptr)
		{
			UTU_LOG_W("    Asset Name: " + AssetNames[1]);
			UTU_LOG_W("        Unreal Asset Relative Path: " + AssetNames[2]);
			UTU_LOG_W("            Cannot setup Blueprint because the asset does not exist.");
			UTU_LOG_W("            Potential Causes:");
			UTU_LOG_W("                - PrefabFirstPass failed to create the asset.");
			UTU_LOG_W("            Asset skipped.");
		}
		else
		{
			Asset->PreEditChange(NULL);
			// Process Asset
			// Maps
			TMap<int, USCS_Node*> IdToNode;
			TMap<USCS_Node*, int> NodeToParentId;
			// Get Root Node
			int RootId = UtuConst::INVALID_INT;
			USCS_Node* RootNode = Asset->SimpleConstructionScript->GetRootNodes()[0];
			IdToNode.Add(RootId, RootNode);
			// Add Real Components
			TArray<FString> UniqueNames;
			UTU_LOG_L("    Adding real components...");
			for (FUtuPluginActor PrefabComponent : InUtuPrefabSecondPass.prefab_components) 
			{
				// New Component
				FString ComponentName = BpMakeUniqueName(PrefabComponent.actor_display_name, UniqueNames);
				USCS_Node* ComponentNode = nullptr;
				bool bComponentCreated = BpAddRootComponentForSubComponentsIfNeeded(Asset, PrefabComponent, ComponentName, ComponentNode);
				if (bComponentCreated) 
				{
					IdToNode.Add(PrefabComponent.actor_id, ComponentNode);
					NodeToParentId.Add(ComponentNode, PrefabComponent.actor_parent_id);
				}
				// Real Components for real this time
				for (EUtuActorType CompType : PrefabComponent.actor_types) 
				{
					USCS_Node* SubComponentNode = nullptr;
					FString SubComponentName = bComponentCreated ? BpMakeUniqueName(PrefabComponent.actor_display_name, UniqueNames) : ComponentName;
					if (CompType == EUtuActorType::Empty) 
					{
						BpAddEmptyComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::StaticMesh) 
					{
						BpAddStaticMeshComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::SkeletalMesh) 
					{
						BpAddSkeletalMeshComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::PointLight) 
					{
						BpAddPointLightComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::DirectionalLight) 
					{
						BpAddDirectionalLightComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::SpotLight) 
					{
						BpAddSpotLightComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::Camera) 
					{
						BpAddCameraComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					else if (CompType == EUtuActorType::Prefab) 
					{
						BpAddChildActorComponent(Asset, PrefabComponent, SubComponentName, SubComponentNode, bComponentCreated);
					}
					// Attachment
					if (bComponentCreated) 
					{
						ComponentNode->AddChildNode(SubComponentNode);
					}
					else 
					{
						IdToNode.Add(PrefabComponent.actor_id, SubComponentNode);
						NodeToParentId.Add(SubComponentNode, PrefabComponent.actor_parent_id);
					}
				}
			}
			// Parent Nodes
			UTU_LOG_L("        Parenting components...");
			TArray<USCS_Node*> Keys;
			NodeToParentId.GetKeys(Keys);
			for (USCS_Node* Node : Keys) 
			{
				if (Node != nullptr)
				{
					int Id = NodeToParentId[Node];
					if (IdToNode.Contains(Id))
					{
						USCS_Node* ParentNode = IdToNode[Id];
						if (ParentNode != nullptr)
						{
							ParentNode->AddChildNode(Node);
							UTU_LOG_L("            " + Node->GetName() + " -> " + ParentNode->GetName());
						}
						else
						{
							UTU_LOG_W("            Parent node for: " + Node->GetName() + " is not valid. Parent node id should have been: " + FString::FromInt(Id) + ". Attaching to root instead.");
						}
					}
					else
					{
						UTU_LOG_W("            Couldn't find parent node for: " + Node->GetName() + ". Parent node id should have been: " + FString::FromInt(Id) + ". I don't know how this can happen.");
					}
				}
			}
			// Dirty
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Asset);
			FKismetEditorUtilities::CompileBlueprint(Asset);
			Asset->MarkPackageDirty();
			Asset->PostEditChange();
		}
	}
	// Restore Save On Compile
	Settings->SaveOnCompile = OriginalSaveOnCompile;
	Settings->SaveConfig();
	Settings->RemoveFromRoot();
}

void FUtuPluginAssetTypeProcessor::BpAddRootComponent(UBlueprint * InAsset, bool bStatic) 
{
	int RootNodeId = UtuConst::INVALID_INT;
	UTU_LOG_L("    Adding Root component...");
	UTU_LOG_L("        Component Name: 'Root'");
	UTU_LOG_L("        Component ID: " + FString::FromInt(RootNodeId));
	UTU_LOG_L("        Component Class: 'USceneComponent'");
	USceneComponent* Root = NewObject<USceneComponent>(InAsset, TEXT("Root"));
	Root->SetMobility(bStatic ? EComponentMobility::Static : EComponentMobility::Movable);
	Root->ComponentTags.Add(*FString::FromInt(RootNodeId));
	USCS_Node* RootNode = InAsset->SimpleConstructionScript->CreateNode(Root->GetClass(), Root->GetFName());
	//InAsset->SimpleConstructionScript->RemoveNode(InAsset->SimpleConstructionScript->GetRootNodes()[0]);
	InAsset->SimpleConstructionScript->AddNode(RootNode); // Making it the new root
	//USCS_Node* RootNode = InAsset->SimpleConstructionScript->GetRootNodes()[0];
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Root, RootNode->ComponentTemplate);
	//FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(InAsset);
	// TODO : Handle Root Node! Can't Remove it because it's the root
	//OutRootNode = InAsset->SimpleConstructionScript->CreateNode(OutRoot->GetClass(), OutRoot->GetFName());
	//UEditorEngine::CopyPropertiesForUnrelatedObjects(OutRoot, OutRootNode->ComponentTemplate);
	//InAsset->SimpleConstructionScript->AddNode(RootNode); // Making it the new root
	//InAsset->SimpleConstructionScript->FixupRootNodeParentReferences();
}



AActor* FUtuPluginAssetTypeProcessor::WorldAddRootActorForSubActorsIfNeeded(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	AActor* RetActor = nullptr;
	UTU_LOG_L("        Processing '" + InUtuActor.actor_display_name + "'...");
	if (InUtuActor.actor_types.Num() != 1 || InUtuActor.actor_types[0] == EUtuActorType::Empty) { // If actor_types == 1, we don't need to have an empty root above the other actors.
		UTU_LOG_L("            Because there was more than one supported components on this GameObject in Unity, creating another 'Root' to hold them...");
		UTU_LOG_L("                Adding Root actor...");
		UTU_LOG_L("                    Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_L("                    Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_L("                    Actor Class: 'AActor'");
		UTU_LOG_L("                    Actor Tag: '" + InUtuActor.actor_tag + "'");
		// Create Empty Root Actor
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<AActor>(Params);
		if (RetActor != nullptr) {
			USceneComponent* RootComponent = NewObject<USceneComponent>(RetActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
			if (RootComponent != nullptr) {
				RetActor->SetRootComponent(RootComponent);
				RetActor->AddInstanceComponent(RootComponent);
				RootComponent->RegisterComponent();
				RetActor->SetActorLabel(InUtuActor.actor_display_name);
				RetActor->Tags.Add(*FString::FromInt(InUtuActor.actor_id));
				if (InUtuActor.actor_tag != "Untagged") {
					RetActor->Tags.Add(*InUtuActor.actor_tag);
				}
				RetActor->SetActorHiddenInGame(!InUtuActor.actor_is_visible);
				RetActor->GetRootComponent()->SetVisibility(InUtuActor.actor_is_visible, true);
				RetActor->SetActorLocation(UtuConst::ConvertLocation(InUtuActor.actor_world_location));
				RetActor->SetActorRotation(UtuConst::ConvertRotation(InUtuActor.actor_world_rotation));
				RetActor->SetActorScale3D(UtuConst::ConvertScale(InUtuActor.actor_world_scale));
				RetActor->GetRootComponent()->SetMobility(InUtuActor.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
			}
			else {
				UTU_LOG_E("            Failed to spawn Root Actor's Root Component...");
				UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
				UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
				UTU_LOG_E("                Potential Causes:");
				UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
				return nullptr; // Don't even bother returning the actor
			}
		}
		else {
			UTU_LOG_E("            Failed to spawn Root Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnStaticMeshActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Static Mesh Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'AStaticMeshActor'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");

	// Check if not LOD
	if (ImportSettings.StaticMeshes.bUtuGenerateLODs)
	{
		TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_relative_filename, EUtuUnrealAssetType::StaticMesh);
		if (MeshNames[1].Contains("_LOD"))
		{
			FString LodIndex = "";
			MeshNames[1].Split("_LOD", nullptr, &LodIndex, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

			if (LodIndex.IsNumeric() && LodIndex != "0") // Keep main LOD
			{
				UTU_LOG_L("                Actor skipped because it's a LOD and not a real mesh. LODs should already be included in LOD0 of this mesh.");
				return nullptr;
			}
		}
	}

	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* RetActor = InAsset->SpawnActor<AStaticMeshActor>(Params);
	if (RetActor != nullptr) {
		TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_relative_filename, EUtuUnrealAssetType::StaticMesh);
		TArray<FString> MeshNamesSeparated = FormatRelativeFilenameForUnrealSeparated(InUtuActor.actor_mesh.actor_mesh_relative_filename, InUtuActor.actor_mesh.actor_mesh_relative_filename_if_separated, EUtuUnrealAssetType::StaticMesh);
		UTU_LOG_L("            Associating Static Mesh to Static Mesh Actor...");
		UTU_LOG_L("                Unreal Asset Relative Path: " + MeshNames[2]);
		UTU_LOG_L("                Unreal Asset Relative Path If Separated: " + MeshNamesSeparated[2]);
		UStaticMesh* StaticMeshAsset = GetMeshAsset(ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames);
		if (StaticMeshAsset == nullptr) {
			StaticMeshAsset = GetMeshAsset(!ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames);
		}
		if (StaticMeshAsset == nullptr) {
			UTU_LOG_W("                Failed to assign Static Mesh because it doesn't exists: '" + (ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames)[2] + "'");
		}
		RetActor->GetStaticMeshComponent()->SetStaticMesh(StaticMeshAsset);
		AssignMaterialsToMesh(InUtuActor.actor_mesh.actor_mesh_materials_relative_filenames, RetActor->GetStaticMeshComponent());
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
	}
	else {
		UTU_LOG_E("            Failed to spawn StaticMesh Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnSkeletalMeshActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Skeletal Mesh Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'ASkeletalMeshActor'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASkeletalMeshActor* RetActor = InAsset->SpawnActor<ASkeletalMeshActor>(Params);
	if (RetActor != nullptr && RetActor->GetSkeletalMeshComponent() != nullptr) {
		TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_relative_filename, EUtuUnrealAssetType::SkeletalMesh);
		UTU_LOG_L("            Associating Skeletal Mesh to Skeletal Mesh Actor...");
		UTU_LOG_L("                Unreal Asset Relative Path: " + MeshNames[2]);
		USkeletalMesh* SkeletalMeshAsset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
		RetActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkeletalMeshAsset);
		if (SkeletalMeshAsset == nullptr) {
			UTU_LOG_W("                Failed to assign Skeletal Mesh because it doesn't exists: '" + MeshNames[2] + "'");
		}
		
		AssignMaterialsToMesh(InUtuActor.actor_mesh.actor_mesh_materials_relative_filenames, RetActor->GetSkeletalMeshComponent());
		
		if (InUtuActor.actor_mesh.actor_mesh_animations_relative_filenames.Num() > 0)
		{
			UTU_LOG_L("            Associating Animation to Skeletal Mesh...");
			UAnimSequence* AnimAsset = nullptr;
			TArray<FString> AnimNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_mesh.actor_mesh_animations_relative_filenames[0], EUtuUnrealAssetType::Animation);
			if (RetActor->GetSkeletalMeshComponent()->SkeletalMesh != nullptr)
			{
				TArray<FString> CustomAssetNames = TArray<FString>({ AnimNames[0], RetActor->GetSkeletalMeshComponent()->SkeletalMesh->GetName() + "_" + AnimNames[1], AnimNames[0] + "/" + RetActor->GetSkeletalMeshComponent()->SkeletalMesh->GetName() + "_" + AnimNames[1] });
				UTU_LOG_L("                Animation : " + CustomAssetNames[2]);
				AnimAsset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(CustomAssetNames[2]));
				if (AnimAsset == nullptr) {
					UTU_LOG_W("                    Failed to assign animation because it doesn't exists: '" + AnimNames[2] + "'");
				}
			}
			else
			{
				UTU_LOG_W("                    Failed to assign animation because Skeletal Mesh is not valid doesn't exists: '" + AnimNames[2] + "'");
			}

			RetActor->GetSkeletalMeshComponent()->SetAnimation(AnimAsset);
			RetActor->GetSkeletalMeshComponent()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			RetActor->GetSkeletalMeshComponent()->AnimationData.AnimToPlay = AnimAsset;
		}
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
	}
	else {
		UTU_LOG_E("            Failed to spawn StaticMesh Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnBlueprintActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	TArray<FString> BpNames = FormatRelativeFilenameForUnreal(InUtuActor.actor_prefab.actor_prefab_relative_filename, EUtuUnrealAssetType::Blueprint);
	UTU_LOG_L("            Adding Blueprint Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'AActor'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	UTU_LOG_L("                Unreal Asset Relative Path: " + BpNames[2]);
	AActor* RetActor = nullptr;
	UBlueprint* BlueprintAsset = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(BpNames[2]));
	if (BlueprintAsset != nullptr) 
	{
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<AActor>(BlueprintAsset->GeneratedClass, Params);
		if (RetActor != nullptr) 
		{
			RetActor->SetActorLabel(InUtuActor.actor_display_name);
			TSet<UActorComponent*> Comps = RetActor->GetComponents();
			if (Comps.Num() > 0) 
			{
				UTU_LOG_L("                Applying scene overrides if needed...");
				for (FUtuPluginActorPrefabComponentOverride Override : InUtuActor.actor_prefab.actor_prefab_component_overrides) 
				{
					for (UActorComponent* Comp : Comps)
					{
						if (Comp->GetName() == Override.component_display_name)
						{
							// Mesh
							{
								UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Comp);
								if (MeshComp != nullptr)
								{
									// Mesh Component Specific
									TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(Override.mesh_relative_filename, EUtuUnrealAssetType::StaticMesh);
									TArray<FString> MeshNamesSeparated = FormatRelativeFilenameForUnrealSeparated(Override.mesh_relative_filename, Override.mesh_relative_filename_if_separated, EUtuUnrealAssetType::StaticMesh);
									UStaticMesh* StaticMeshAsset = GetMeshAsset(ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames);
									if (StaticMeshAsset == nullptr) {
										StaticMeshAsset = GetMeshAsset(!ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames); // Try the other one
									}
									if (StaticMeshAsset == nullptr) {
										UTU_LOG_W("                    Failed to assign Static Mesh because it doesn't exists: '" + (ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames)[2] + "'");
									}
									MeshComp->SetStaticMesh(StaticMeshAsset);

									// Materials
									AssignMaterialsToMesh(Override.material_relative_filenames, MeshComp);
								}
							}

							// Skeletal Mesh
							{
								USkeletalMeshComponent* SkelMeshComp = Cast<USkeletalMeshComponent>(Comp);
								if (SkelMeshComp != nullptr)
								{
									// Mesh Component Specific
									TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(Override.mesh_relative_filename, EUtuUnrealAssetType::SkeletalMesh);
									USkeletalMesh* SkeletalMeshAsset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
									if (SkeletalMeshAsset == nullptr) {
										UTU_LOG_W("                    Failed to assign Skeletal Mesh because it doesn't exists: '" + MeshNames[2] + "'");
									}
									SkelMeshComp->SetSkeletalMesh(SkeletalMeshAsset);

									// Materials
									AssignMaterialsToMesh(Override.material_relative_filenames, SkelMeshComp);
								}
							}

							// Animation
							{
								if (Override.animation_relative_filenames.Num() > 0)
								{
									USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(Comp);
									if (SkelComp != nullptr && SkelComp->SkeletalMesh != nullptr)
									{
										TArray<FString> AnimNames = FormatRelativeFilenameForUnreal(Override.animation_relative_filenames[0], EUtuUnrealAssetType::Animation);
										TArray<FString> CustomAssetNames = TArray<FString>({ AnimNames[0], SkelComp->SkeletalMesh->GetName() + "_" + AnimNames[1], AnimNames[0] + "/" + SkelComp->SkeletalMesh->GetName() + "_" + AnimNames[1] });
										UAnimSequence* AnimAsset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(CustomAssetNames[2]));
										SkelComp->SetAnimation(AnimAsset);
										SkelComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
										SkelComp->AnimationData.AnimToPlay = AnimAsset;
									}
								}
							}
						}
					}
				}

				// Replace prefab by SM if that's what we want 
				if (ImportSettings.Scenes.MeshSpawnBehavior != EUtuMeshSpawnBehavior::AllPrefab)
				{
					TArray<USceneComponent*> SceneComps = TArray<USceneComponent*>();
					TArray<UStaticMeshComponent*> StaticMeshComps = TArray<UStaticMeshComponent*>();
					for (UActorComponent* Comp : Comps)
					{
						if (Cast<UStaticMeshComponent>(Comp))
						{
							StaticMeshComps.Add(Cast<UStaticMeshComponent>(Comp));
						}
						else if (Cast<USceneComponent>(Comp))
						{
							SceneComps.Add(Cast<USceneComponent>(Comp));
						}
					}
					// Only if all the components are either scene or static meshes
					if (StaticMeshComps.Num() > 0 && (SceneComps.Num() + StaticMeshComps.Num() == Comps.Num()))
					{
						if (ImportSettings.Scenes.MeshSpawnBehavior == EUtuMeshSpawnBehavior::AllStaticMesh || (ImportSettings.Scenes.MeshSpawnBehavior == EUtuMeshSpawnBehavior::StaticMeshIfAloneInPrefab && StaticMeshComps.Num() == 1))
						{
							// Do we need a parent root actor?
							if (StaticMeshComps.Num() == 1 && StaticMeshComps[0]->GetRelativeTransform().Equals(FTransform::Identity))
							{
								// We don't
								AStaticMeshActor* MeshActor = InAsset->SpawnActor<AStaticMeshActor>(Params);
								if (MeshActor != nullptr)
								{
									if (MeshActor->GetStaticMeshComponent() != nullptr)
									{
										MeshActor->SetActorLabel(InUtuActor.actor_display_name);
										MeshActor->GetStaticMeshComponent()->SetStaticMesh(StaticMeshComps[0]->GetStaticMesh());
										// Copy paste materials
										for (int m = 0; m < StaticMeshComps[0]->GetNumMaterials(); m++)
										{
											MeshActor->GetStaticMeshComponent()->SetMaterial(m, StaticMeshComps[0]->GetMaterial(m));
										}
										// Replace RetActor
										RetActor->Destroy();
										RetActor = MeshActor;
									}
									else
									{
										MeshActor->Destroy();
									}
								}
							}
							else
							{
								// We do
								// Create Empty Root Actor
								AActor* RootActor = InAsset->SpawnActor<AActor>(Params);
								if (RootActor != nullptr) {
									USceneComponent* RootComponent = NewObject<USceneComponent>(RootActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
									if (RootComponent != nullptr) {
										RootActor->SetRootComponent(RootComponent);
										RootActor->AddInstanceComponent(RootComponent);
										RootComponent->RegisterComponent();
										RootComponent->SetMobility(InUtuActor.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);

										// Spawn meshes
										for (UStaticMeshComponent* SmComp : StaticMeshComps)
										{
											AStaticMeshActor* MeshActor = InAsset->SpawnActor<AStaticMeshActor>(Params);
											if (MeshActor != nullptr)
											{
												if (MeshActor->GetStaticMeshComponent() != nullptr)
												{
													MeshActor->Tags.Add("UtuActor");
													MeshActor->GetRootComponent()->SetMobility(InUtuActor.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
													MeshActor->AttachToActor(RootActor, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
													MeshActor->SetActorLabel(InUtuActor.actor_display_name);
													MeshActor->SetActorTransform(SmComp->GetComponentTransform());
													MeshActor->GetStaticMeshComponent()->SetStaticMesh(SmComp->GetStaticMesh());
													// Copy paste materials
													for (int m = 0; m < SmComp->GetNumMaterials(); m++)
													{
														MeshActor->GetStaticMeshComponent()->SetMaterial(m, SmComp->GetMaterial(m));
													}
												}
												else
												{
													MeshActor->Destroy();
												}
											}
										}
									}
									// Replace RetActor
									RetActor->Destroy();
									RetActor = RootActor;
								}
							}
						}
					}
				}
			}
		}
		else {
			UTU_LOG_E("            Failed to spawn Blueprint Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Unreal Asset Relative Path: " + BpNames[2]);
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	else {
		UTU_LOG_W("            Cannot spawn Blueprint Actor because Blueprint asset does not exist...");
		UTU_LOG_W("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_W("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_W("                Unreal Asset Relative Path: " + BpNames[2]);
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnSkyLightActor(UWorld * InAsset) {
	UTU_LOG_L("            Adding Point Sky Actor...");
	UTU_LOG_L("                Actor Name: 'SkyLight'");
	UTU_LOG_L("                Actor Class: 'ASkyLight'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASkyLight* RetActor = InAsset->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), Params);
	if (RetActor != nullptr && Cast<USkyLightComponent>(RetActor->GetLightComponent()) != nullptr) {
		USkyLightComponent* Comp = Cast<USkyLightComponent>(RetActor->GetLightComponent());
		RetActor->Tags.Add("UtuActor");
		RetActor->SetActorLabel("SkyLight");
		RetActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		Comp->Intensity = ImportSettings.Lights.SkyLightIntensity;
		Comp->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
		Comp->Cubemap = Cast<UTextureCube>(UUtuPluginLibrary::TryGetAsset("/Game/Utu/Assets/TX_CubeMap"));
	}
	else {
		UTU_LOG_E("            Failed to spawn Point Light Actor...");
		UTU_LOG_E("                Actor Name: 'SkyLight'");
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}


AActor* FUtuPluginAssetTypeProcessor::WorldSpawnPointLightActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Point Light Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'APointLight'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APointLight* RetActor = InAsset->SpawnActor<APointLight>(APointLight::StaticClass(), Params);
	if (RetActor != nullptr && Cast<UPointLightComponent>(RetActor->GetLightComponent()) != nullptr) {
		UPointLightComponent* Comp = Cast<UPointLightComponent>(RetActor->GetLightComponent());
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
		Comp->Intensity = InUtuActor.actor_light.light_intensity * ImportSettings.Lights.LightIntensityMultiplier;
		Comp->bUseInverseSquaredFalloff = false;
		Comp->SetLightFalloffExponent(ImportSettings.Lights.LightFalloffExponent);
		Comp->SetLightColor(HexToColor(InUtuActor.actor_light.light_color));
		Comp->AttenuationRadius = InUtuActor.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier;
		Comp->SetAttenuationRadius(InUtuActor.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier);
		Comp->CastShadows = InUtuActor.actor_light.light_is_casting_shadows;
	}
	else {
		UTU_LOG_E("            Failed to spawn Point Light Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}


AActor* FUtuPluginAssetTypeProcessor::WorldSpawnDirectionalLightActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Directional Light Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'ADirectionalLight'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ADirectionalLight* RetActor = InAsset->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Params);
	if (RetActor != nullptr && RetActor->GetLightComponent() != nullptr) {
		ULightComponent* Comp = RetActor->GetLightComponent();
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
		Comp->Intensity = InUtuActor.actor_light.light_intensity * ImportSettings.Lights.LightIntensityMultiplier;
		Comp->SetLightColor(HexToColor(InUtuActor.actor_light.light_color));
		Comp->CastShadows = InUtuActor.actor_light.light_is_casting_shadows;
	}
	else {
		UTU_LOG_E("            Failed to spawn Directional Light Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnSpotLightActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	UTU_LOG_L("            Adding Spot Light Actor...");
	UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
	UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
	UTU_LOG_L("                Actor Class: 'ASpotLight'");
	UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASpotLight* RetActor = InAsset->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Params);
	if (RetActor != nullptr && Cast<USpotLightComponent>(RetActor->GetLightComponent()) != nullptr) {
		USpotLightComponent* Comp = Cast<USpotLightComponent>(RetActor->GetLightComponent());
		RetActor->SetActorLabel(InUtuActor.actor_display_name);
		Comp->Intensity = InUtuActor.actor_light.light_intensity * ImportSettings.Lights.LightIntensityMultiplier;
		Comp->bUseInverseSquaredFalloff = false;
		Comp->SetLightFalloffExponent(ImportSettings.Lights.LightFalloffExponent);
		Comp->SetLightColor(HexToColor(InUtuActor.actor_light.light_color));
		Comp->AttenuationRadius = InUtuActor.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier;
		Comp->SetAttenuationRadius(InUtuActor.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier);
		Comp->CastShadows = InUtuActor.actor_light.light_is_casting_shadows;
		Comp->InnerConeAngle = ImportSettings.Lights.LightSpotInnerConeAngle;
		Comp->OuterConeAngle = InUtuActor.actor_light.light_spot_angle * ImportSettings.Lights.LightSpotAngleMultiplier;
	}
	else {
		UTU_LOG_E("            Failed to spawn Spot Light Actor...");
		UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_E("                Potential Causes:");
		UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
	}
	return RetActor;
}

AActor* FUtuPluginAssetTypeProcessor::WorldSpawnCameraActor(UWorld * InAsset, FUtuPluginActor InUtuActor) {
	AActor* RetActor = nullptr;
	if (InUtuActor.actor_camera.camera_is_physical) {
		UTU_LOG_L("            Adding Cine Camera Actor...");
		UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_L("                Actor Class: 'ACineCameraActor'");
		UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<ACineCameraActor>(ACineCameraActor::StaticClass(), Params);
		if (RetActor != nullptr && Cast<ACineCameraActor>(RetActor)->GetCineCameraComponent() != nullptr) {
			UCineCameraComponent* Comp = Cast<ACineCameraActor>(RetActor)->GetCineCameraComponent();
			RetActor->SetActorLabel(InUtuActor.actor_display_name);
			//InUtuActor.actor_camera.camera_viewport_rect;
			Comp->OrthoNearClipPlane = InUtuActor.actor_camera.camera_near_clip_plane;
			Comp->OrthoFarClipPlane = InUtuActor.actor_camera.camera_far_clip_plane;
			Comp->AspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
			Comp->ProjectionMode = InUtuActor.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
			Comp->OrthoWidth = InUtuActor.actor_camera.camera_ortho_size;
			Comp->FieldOfView = InUtuActor.actor_camera.camera_persp_field_of_view;
			Comp->CurrentFocalLength = InUtuActor.actor_camera.camera_phys_focal_length;
#if ENGINE_MINOR_VERSION >= 24 || ENGINE_MAJOR_VERSION >= 5
			Comp->Filmback.SensorWidth = InUtuActor.actor_camera.camera_phys_sensor_size.X;
			Comp->Filmback.SensorHeight = InUtuActor.actor_camera.camera_phys_sensor_size.Y;
			Comp->Filmback.SensorAspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
#else
			Comp->FilmbackImportSettings.SensorWidth = InUtuActor.actor_camera.camera_phys_sensor_size.X;
			Comp->FilmbackImportSettings.SensorHeight = InUtuActor.actor_camera.camera_phys_sensor_size.Y;
			Comp->FilmbackImportSettings.SensorAspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
#endif
		}
		else {
			UTU_LOG_E("            Failed to spawn Cine Camera Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	else {
		UTU_LOG_L("            Adding Camera Actor...");
		UTU_LOG_L("                Actor Name: '" + InUtuActor.actor_display_name + "'");
		UTU_LOG_L("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
		UTU_LOG_L("                Actor Class: 'ACameraActor'");
		UTU_LOG_L("                Actor Tag: '" + InUtuActor.actor_tag + "'");
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RetActor = InAsset->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Params);
		if (RetActor != nullptr && Cast<ACameraActor>(RetActor)->GetCameraComponent() != nullptr) {
			UCameraComponent* Comp = Cast<ACameraActor>(RetActor)->GetCameraComponent();
			RetActor->SetActorLabel(InUtuActor.actor_display_name);
			//InUtuActor.actor_camera.camera_viewport_rect;
			Comp->OrthoNearClipPlane = InUtuActor.actor_camera.camera_near_clip_plane;
			Comp->OrthoFarClipPlane = InUtuActor.actor_camera.camera_far_clip_plane;
			Comp->AspectRatio = InUtuActor.actor_camera.camera_aspect_ratio;
			Comp->ProjectionMode = InUtuActor.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
			Comp->OrthoWidth = InUtuActor.actor_camera.camera_ortho_size;
			Comp->FieldOfView = InUtuActor.actor_camera.camera_persp_field_of_view;
		}
		else {
			UTU_LOG_E("            Failed to spawn Camera Actor...");
			UTU_LOG_E("                Actor Name: '" + InUtuActor.actor_display_name + "'");
			UTU_LOG_E("                Actor ID: " + FString::FromInt(InUtuActor.actor_id));
			UTU_LOG_E("                Potential Causes:");
			UTU_LOG_E("                    - No Potential Causes known yet, but it should never happen.");
		}
	}
	return RetActor;
}

bool FUtuPluginAssetTypeProcessor::BpAddRootComponentForSubComponentsIfNeeded(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode) {
	OutComponentNode = nullptr;
	UTU_LOG_L("        Processing '" + InUniqueName + "'...");
	if (InPrefabComponent.actor_types.Num() != 1) { // If actor_types == 1, we don't need to have an empty root above the other components.
		UTU_LOG_L("            Because there was more than one supported components on this GameObject in Unity, creating another 'Root' to hold them...");
		UTU_LOG_L("                Adding Root component...");
		UTU_LOG_L("                    Component Name: '" + InUniqueName + "'");
		UTU_LOG_L("                    Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
		UTU_LOG_L("                    Component Class: 'USceneComponent'");
		UTU_LOG_L("                    Component Tag: '" + InPrefabComponent.actor_tag + "'");
		// Create Component
		USceneComponent* Component = NewObject<USceneComponent>(InAsset, *InUniqueName);
		Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
		// Create Component Node
		OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
		UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
		return true;
	}
	return false;
}

void FUtuPluginAssetTypeProcessor::BpAddEmptyComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	// If we get here, it's only because theres only one empty component to create. Should be almost the same as creating an intermediary root component
	UTU_LOG_L("            Adding Empty component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	UTU_LOG_L("                Component Class: 'USceneComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	USceneComponent* Component = NewObject<USceneComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddStaticMeshComponent(UBlueprint* InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node* &OutComponentNode, bool bInRootCreated) 
{
	UTU_LOG_L("            Adding StaticMesh component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) 
	{
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UStaticMeshComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");

	// Check if not LOD
	if (ImportSettings.StaticMeshes.bUtuGenerateLODs)
	{
		TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_relative_filename, EUtuUnrealAssetType::StaticMesh);

		for (FString Name : TArray<FString>({ InUniqueName, MeshNames[1] }))
		{
			if (Name.Contains("_LOD"))
			{
				FString LodIndex = "";
				Name.Split("_LOD", nullptr, &LodIndex, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

				if (LodIndex.IsNumeric() && LodIndex != "0") // Keep main LOD
				{
					UTU_LOG_L("                Component skipped because it's a LOD and not a real mesh. LODs should already be included in LOD0 of this mesh.");
					return;
				}
			}
		}
	}

	// Create Component
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) 
	{
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") 
		{
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Mesh Component Specific
	TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_relative_filename, EUtuUnrealAssetType::StaticMesh);
	TArray<FString> MeshNamesSeparated = FormatRelativeFilenameForUnrealSeparated(InPrefabComponent.actor_mesh.actor_mesh_relative_filename, InPrefabComponent.actor_mesh.actor_mesh_relative_filename_if_separated, EUtuUnrealAssetType::StaticMesh);
	UStaticMesh* StaticMeshAsset = GetMeshAsset(ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames);
	if (StaticMeshAsset == nullptr) {
		StaticMeshAsset = GetMeshAsset(!ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames);
	}
	if (StaticMeshAsset == nullptr) {
		UTU_LOG_W("                Failed to assign Static Mesh because it doesn't exists: '" + (ImportSettings.StaticMeshes.bImportSeparated ? MeshNamesSeparated : MeshNames)[2] + "'");
	}
	Component->SetStaticMesh(StaticMeshAsset);
	AssignMaterialsToMesh(InPrefabComponent.actor_mesh.actor_mesh_materials_relative_filenames, Component);
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddSkeletalMeshComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding SkeletalMesh component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'USkeletalMeshComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		// Do not use the mesh transform since it's driven by the bones (Unity is dumb)
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Mesh Component Specific
	TArray<FString> MeshNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_relative_filename, EUtuUnrealAssetType::SkeletalMesh);
	USkeletalMesh* SkeletalMeshAsset = Cast<USkeletalMesh>(UUtuPluginLibrary::TryGetAsset(MeshNames[2]));
	Component->SetSkeletalMesh(SkeletalMeshAsset);
	if (SkeletalMeshAsset != nullptr) 
	{
		AssignMaterialsToMesh(InPrefabComponent.actor_mesh.actor_mesh_materials_relative_filenames, Component);
		if (InPrefabComponent.actor_mesh.actor_mesh_animations_relative_filenames.Num() > 0)
		{
			UAnimSequence* AnimAsset = nullptr;
			if (Component->SkeletalMesh != nullptr)
			{
				TArray<FString> AnimNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_mesh.actor_mesh_animations_relative_filenames[0], EUtuUnrealAssetType::Animation);
				TArray<FString> CustomAssetNames = TArray<FString>({ AnimNames[0], Component->SkeletalMesh->GetName() + "_" + AnimNames[1], AnimNames[0] + "/" + Component->SkeletalMesh->GetName() + "_" + AnimNames[1] });
				AnimAsset = Cast<UAnimSequence>(UUtuPluginLibrary::TryGetAsset(CustomAssetNames[2]));
			}
			Component->SetAnimation(AnimAsset);
			Component->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			Component->AnimationData.AnimToPlay = AnimAsset;
		}
	}
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddPointLightComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding PointLight component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UPointLightComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UPointLightComponent* Component = NewObject<UPointLightComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Point Light Component Specific
	Component->Intensity = InPrefabComponent.actor_light.light_intensity * ImportSettings.Lights.LightIntensityMultiplier;
	Component->bUseInverseSquaredFalloff = false;
	Component->SetLightFalloffExponent(ImportSettings.Lights.LightFalloffExponent);
	Component->SetLightColor(HexToColor(InPrefabComponent.actor_light.light_color));
	Component->AttenuationRadius = InPrefabComponent.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier;
	Component->SetAttenuationRadius(InPrefabComponent.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier);
	Component->CastShadows = InPrefabComponent.actor_light.light_is_casting_shadows;
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddDirectionalLightComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding DirectionalLight component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UDirectionalLightComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UDirectionalLightComponent* Component = NewObject<UDirectionalLightComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Point Light Component Specific
	Component->Intensity = InPrefabComponent.actor_light.light_intensity * ImportSettings.Lights.LightIntensityMultiplier;
	Component->SetLightColor(HexToColor(InPrefabComponent.actor_light.light_color));
	Component->CastShadows = InPrefabComponent.actor_light.light_is_casting_shadows;
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddSpotLightComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding SpotLight component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'USpotLightComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	USpotLightComponent* Component = NewObject<USpotLightComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Point Light Component Specific
	Component->Intensity = InPrefabComponent.actor_light.light_intensity * ImportSettings.Lights.LightIntensityMultiplier;
	Component->bUseInverseSquaredFalloff = false;
	Component->SetLightFalloffExponent(ImportSettings.Lights.LightFalloffExponent);
	Component->SetLightColor(HexToColor(InPrefabComponent.actor_light.light_color));
	Component->AttenuationRadius = InPrefabComponent.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier;
	Component->SetAttenuationRadius(InPrefabComponent.actor_light.light_range * ImportSettings.Lights.LightRangeMultiplier);
	Component->CastShadows = InPrefabComponent.actor_light.light_is_casting_shadows;
	Component->InnerConeAngle = ImportSettings.Lights.LightSpotInnerConeAngle;
	Component->OuterConeAngle = InPrefabComponent.actor_light.light_spot_angle * ImportSettings.Lights.LightSpotAngleMultiplier;
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

void FUtuPluginAssetTypeProcessor::BpAddCameraComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	if (InPrefabComponent.actor_camera.camera_is_physical) {
		UTU_LOG_L("            Adding Cine Camera Component...");
		UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
		if (!bInRootCreated) {
			UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
		}
		UTU_LOG_L("                Component Class: 'UCineCameraComponent'");
		UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
		// Create Component
		UCineCameraComponent* Component = NewObject<UCineCameraComponent>(InAsset, *InUniqueName);
		Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
		if (!bInRootCreated) {
			Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
			if (InPrefabComponent.actor_tag != "Untagged") {
				Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
			}
			Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
			Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
			Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
			Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
			Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
		}
		// Cine Camera Component Specific
		//InPrefabComponent.actor_camera.camera_viewport_rect;
		Component->OrthoNearClipPlane = InPrefabComponent.actor_camera.camera_near_clip_plane;
		Component->OrthoFarClipPlane = InPrefabComponent.actor_camera.camera_far_clip_plane;
		Component->AspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
		Component->ProjectionMode = InPrefabComponent.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
		Component->OrthoWidth = InPrefabComponent.actor_camera.camera_ortho_size;
		Component->FieldOfView = InPrefabComponent.actor_camera.camera_persp_field_of_view;
		Component->CurrentFocalLength = InPrefabComponent.actor_camera.camera_phys_focal_length;
#if ENGINE_MINOR_VERSION >= 24 || ENGINE_MAJOR_VERSION >= 5
		Component->Filmback.SensorWidth = InPrefabComponent.actor_camera.camera_phys_sensor_size.X;
		Component->Filmback.SensorHeight = InPrefabComponent.actor_camera.camera_phys_sensor_size.Y;
		Component->Filmback.SensorAspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
#else
		Component->FilmbackImportSettings.SensorWidth = InPrefabComponent.actor_camera.camera_phys_sensor_size.X;
		Component->FilmbackImportSettings.SensorHeight = InPrefabComponent.actor_camera.camera_phys_sensor_size.Y;
		Component->FilmbackImportSettings.SensorAspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
#endif
		// Create Component Node
		OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
		UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
	}
	else {
		UTU_LOG_L("            Adding Camera Actor...");
		UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
		if (!bInRootCreated) {
			UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
		}
		UTU_LOG_L("                Component Class: 'UCameraComponent'");
		UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
		// Create Component
		UCameraComponent* Component = NewObject<UCameraComponent>(InAsset, *InUniqueName);
		Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
		if (!bInRootCreated) {
			Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
			if (InPrefabComponent.actor_tag != "Untagged") {
				Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
			}
			Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
			Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
			Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
			Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
			Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
		}
		// Camera Component Specific
		//InPrefabComponent.actor_camera.camera_viewport_rect;
		Component->OrthoNearClipPlane = InPrefabComponent.actor_camera.camera_near_clip_plane;
		Component->OrthoFarClipPlane = InPrefabComponent.actor_camera.camera_far_clip_plane;
		Component->AspectRatio = InPrefabComponent.actor_camera.camera_aspect_ratio;
		Component->ProjectionMode = InPrefabComponent.actor_camera.camera_is_perspective ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
		Component->OrthoWidth = InPrefabComponent.actor_camera.camera_ortho_size;
		Component->FieldOfView = InPrefabComponent.actor_camera.camera_persp_field_of_view;
		// Create Component Node
		OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
		UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
	}
}

void FUtuPluginAssetTypeProcessor::BpAddChildActorComponent(UBlueprint * InAsset, FUtuPluginActor InPrefabComponent, FString InUniqueName, USCS_Node * &OutComponentNode, bool bInRootCreated) {
	UTU_LOG_L("            Adding Prefab component...");
	UTU_LOG_L("                Component Name: '" + InUniqueName + "'");
	if (!bInRootCreated) {
		UTU_LOG_L("                Component ID: " + FString::FromInt(InPrefabComponent.actor_id));
	}
	UTU_LOG_L("                Component Class: 'UChildActorComponent'");
	UTU_LOG_L("                Component Tag: '" + InPrefabComponent.actor_tag + "'");
	// Create Component
	UChildActorComponent* Component = NewObject<UChildActorComponent>(InAsset, *InUniqueName);
	Component->SetMobility(InPrefabComponent.actor_is_movable ? EComponentMobility::Movable : EComponentMobility::Static);
	if (!bInRootCreated) {
		Component->ComponentTags.Add(*FString::FromInt(InPrefabComponent.actor_id));
		if (InPrefabComponent.actor_tag != "Untagged") {
			Component->ComponentTags.Add(*InPrefabComponent.actor_tag);
		}
		Component->SetHiddenInGame(!InPrefabComponent.actor_is_visible);
		Component->SetVisibility(InPrefabComponent.actor_is_visible, true);
		Component->SetRelativeLocation(UtuConst::ConvertLocation(InPrefabComponent.actor_relative_location));
		Component->SetRelativeRotation(UtuConst::ConvertRotation(InPrefabComponent.actor_relative_rotation));
		Component->SetRelativeScale3D(UtuConst::ConvertScale(InPrefabComponent.actor_relative_scale));
	}
	// Child Actor Specific
	TArray<FString> BpNames = FormatRelativeFilenameForUnreal(InPrefabComponent.actor_prefab.actor_prefab_relative_filename, EUtuUnrealAssetType::Blueprint);
	UBlueprint* BpChild = Cast<UBlueprint>(UUtuPluginLibrary::TryGetAsset(BpNames[2]));
	Component->SetChildActorClass(TSubclassOf<AActor>(BpChild->GeneratedClass));
	// Create Component Node
	OutComponentNode = InAsset->SimpleConstructionScript->CreateNode(Component->GetClass(), *InUniqueName);
	UEditorEngine::CopyPropertiesForUnrelatedObjects(Component, OutComponentNode->ComponentTemplate);
}

FString FUtuPluginAssetTypeProcessor::BpMakeUniqueName(FString InDesiredName, TArray<FString>&InOutUsedNames) {
	if (InOutUsedNames.Contains(InDesiredName)) {
		for (int x = 1; x < 1000; x++) {
			FString PotentialName = InDesiredName + "_" + FString::FromInt(x);
			if (!InOutUsedNames.Contains(PotentialName)) {
				InOutUsedNames.Add(PotentialName);
				return PotentialName;
			}
		}
	}
	InOutUsedNames.Add(InDesiredName);
	return InDesiredName;
}

UStaticMesh* FUtuPluginAssetTypeProcessor::GetMeshAsset(TArray<FString> AssetNames)
{
	UStaticMesh* Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2]));
	if (Asset == nullptr)
	{
		// Try separated way
		Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2] + "_" + AssetNames[1]));
	}
	if (Asset == nullptr)
	{
		// Try separated way in 5.5 and up
		FString Filename = AssetNames[1];
		while (Filename.Contains("_") && Asset == nullptr)
		{
			Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[0] + "/" + Filename));
			// Remove underscore
			FString Right = FString();
			Filename.Split("_", nullptr, &Right, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			Filename = Right;
		}
	}
	if (Asset == nullptr)
	{
		// Try with LOD names
		Asset = Cast<UStaticMesh>(UUtuPluginLibrary::TryGetAsset(AssetNames[2] + "_" + AssetNames[1] + "_LOD0"));
	}
	return Asset;
}


TArray<FString> FUtuPluginAssetTypeProcessor::CalculateMaterialsSlotOrder(TArray<FString> Materials, TArray<FStaticMaterial> Slots)
{
	// Get the slots
	TArray<int> UsedMaterialIdx = TArray<int>();

	// Populate list of materials to slot
	TMap<int, FString> SlotIdsToMaterial = TMap<int, FString>();

	// Try to find slot for name
	for (int Idx = 0; Idx < Materials.Num(); Idx++)
	{
		FString Material = Materials[Idx];
		// Still invalid
		if (!UsedMaterialIdx.Contains(Idx))
		{
			for (EUtuUnrealAssetType MatType : TArray<EUtuUnrealAssetType>({ EUtuUnrealAssetType::Material , EUtuUnrealAssetType::MaterialInstance }))
			{
				TArray<FString> MatNames = FormatRelativeFilenameForUnreal(Material, MatType);
				bool bFound = false;

				FString SlotName = MatNames[1];

				if (GetAssetStruct(MatType).AssetRenameSettings.Prefix != "")
				{
					SlotName.RemoveFromStart(GetAssetStruct(MatType).AssetRenameSettings.Prefix, ESearchCase::CaseSensitive);
				}
				if (GetAssetStruct(MatType).AssetRenameSettings.AutoRenameDuplicatedAssetSuffix != "")
				{
					SlotName.RemoveFromEnd(GetAssetStruct(MatType).AssetRenameSettings.AutoRenameDuplicatedAssetSuffix, ESearchCase::CaseSensitive);
				}
				if (GetAssetStruct(MatType).AssetRenameSettings.Suffix != "")
				{
					SlotName.RemoveFromEnd(GetAssetStruct(MatType).AssetRenameSettings.Suffix, ESearchCase::CaseSensitive);
				}
				SlotName.RemoveFromEnd("_FbxMat", ESearchCase::CaseSensitive);

				// Find good slot
				for (int X = 0; X < Slots.Num(); X++)
				{
					// Not already used
					if (!SlotIdsToMaterial.Contains(X))
					{
						if (Slots[X].ImportedMaterialSlotName.ToString() == SlotName || Slots[X].MaterialSlotName.ToString() == SlotName)
						{
							SlotIdsToMaterial.Add(X, Material);
							UsedMaterialIdx.Add(Idx);
							bFound = true;
							break;
						}
					}
				}
				if (bFound)
				{
					break;
				}
			}
		}
	}

	// Try to find slot for name. Try again, but with contains instead
	for (int Idx = 0; Idx < Materials.Num(); Idx++)
	{
		FString Material = Materials[Idx];
		// Still invalid
		if (!UsedMaterialIdx.Contains(Idx))
		{
			for (EUtuUnrealAssetType MatType : TArray<EUtuUnrealAssetType>({ EUtuUnrealAssetType::Material , EUtuUnrealAssetType::MaterialInstance }))
			{
				TArray<FString> MatNames = FormatRelativeFilenameForUnreal(Material, MatType);
				bool bFound = false;

				FString SlotName = MatNames[1];
				if (GetAssetStruct(MatType).AssetRenameSettings.Prefix != "")
				{
					SlotName.RemoveFromStart(GetAssetStruct(MatType).AssetRenameSettings.Prefix, ESearchCase::CaseSensitive);
				}
				if (GetAssetStruct(MatType).AssetRenameSettings.AutoRenameDuplicatedAssetSuffix != "")
				{
					SlotName.RemoveFromEnd(GetAssetStruct(MatType).AssetRenameSettings.AutoRenameDuplicatedAssetSuffix, ESearchCase::CaseSensitive);
				}
				if (GetAssetStruct(MatType).AssetRenameSettings.Suffix != "")
				{
					SlotName.RemoveFromEnd(GetAssetStruct(MatType).AssetRenameSettings.Suffix, ESearchCase::CaseSensitive);
				}
				SlotName.RemoveFromEnd("_FbxMat", ESearchCase::CaseSensitive);

				// Find good slot
				for (int X = 0; X < Slots.Num(); X++)
				{
					// Not already used
					if (!SlotIdsToMaterial.Contains(X))
					{
						if (Slots[X].ImportedMaterialSlotName.ToString().Contains(SlotName) || Slots[X].MaterialSlotName.ToString().Contains(SlotName))
						{
							SlotIdsToMaterial.Add(X, Material);
							UsedMaterialIdx.Add(Idx);
							bFound = true;
							break;
						}
					}
				}
				if (bFound)
				{
					break;
				}
			}
		}
	}

	// Try matching material id with empty slot id
	for (int Idx = 0; Idx < Materials.Num(); Idx++)
	{
		// Still invalid
		if (!UsedMaterialIdx.Contains(Idx))
		{
			// Not already used
			if (!SlotIdsToMaterial.Contains(Idx) && Idx < Slots.Num())
			{
				SlotIdsToMaterial.Add(Idx, Materials[Idx]);
				UsedMaterialIdx.Add(Idx);
				break;
			}
		}
	}

	// Make sure there's no more materials without a slot
	for (int Idx = 0; Idx < Materials.Num(); Idx++)
	{
		FString Material = Materials[Idx];

		// Still invalid
		if (!UsedMaterialIdx.Contains(Idx))
		{
			// Find good slot
			for (int X = 0; X < Slots.Num(); X++)
			{
				// Not already used
				if (!SlotIdsToMaterial.Contains(X))
				{
					SlotIdsToMaterial.Add(X, Material);
					UsedMaterialIdx.Add(Idx);
					break;
				}
			}
		}
	}

	TArray<FString> MaterialsOrder = TArray<FString>();

	for (int X = 0; X < Materials.Num(); X++)
	{
		if (SlotIdsToMaterial.Contains(X))
		{
			MaterialsOrder.Add(SlotIdsToMaterial[X]);
		}
		else
		{
			MaterialsOrder.Add("");
		}
	}

	return MaterialsOrder;
}


void FUtuPluginAssetTypeProcessor::AssignMaterialsToMesh(TArray<FString> Materials, UStaticMesh* StaticMesh)
{
	if (StaticMesh == nullptr)
	{
		return;
	}

	Materials.Remove("");
	if (Materials.Num() == 0)
	{
		UTU_LOG_L("                No Materials to assign to Static Mesh.");
		return;
	}

	UTU_LOG_L("                Associating Materials to Static Mesh...");
	
	// Get the Ids
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 0
	TArray<FStaticMaterial> Slots = StaticMesh->GetStaticMaterials();
#else
	TArray<FStaticMaterial> Slots = StaticMesh->StaticMaterials;
#endif
	TArray<FString> SortedMaterials = CalculateMaterialsSlotOrder(Materials, Slots);

	// Check if all materials are used
	for (FString Material : Materials)
	{
		// Slot for this mat?
		if (!SortedMaterials.Contains(Material))
		{
			UTU_LOG_W("                    No slot found for material: " + Material);
			continue;
		}
	}

	// Assign materials for real
	for (int MatId = 0; MatId < SortedMaterials.Num(); MatId++)
	{
		FString Material = SortedMaterials[MatId];

		// Get Material asset (try both material and material instance)
		TArray<FString> MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::MaterialInstance : EUtuUnrealAssetType::Material);
		UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		if (MaterialAsset == nullptr)
		{
			MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::Material : EUtuUnrealAssetType::MaterialInstance);
			MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		}

		// Find ID
		FString SlotName = FString::FromInt(MatId);
		if (Slots.IsValidIndex(MatId))
		{
			SlotName = Slots[MatId].MaterialSlotName.ToString();
		}
		UTU_LOG_L("                    MaterialSlot[" + SlotName + "] : " + MatNames[2]);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 0
		// Apply material
		if (Slots.IsValidIndex(MatId))
		{
			Slots[MatId].MaterialInterface = MaterialAsset;
		}
#else
		// Apply material
		if (StaticMesh->StaticMaterials.IsValidIndex(MatId))
		{
			StaticMesh->StaticMaterials[MatId].MaterialInterface = MaterialAsset;
		}
#endif
		if (MaterialAsset == nullptr)
		{
			UTU_LOG_W("                        Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
		}
	}
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 0
	StaticMesh->SetStaticMaterials(Slots);
#endif
	StaticMesh->Modify();
	StaticMesh->PostEditChange();
}


void FUtuPluginAssetTypeProcessor::AssignMaterialsToMesh(TArray<FString> Materials, UStaticMeshComponent* StaticMeshComponent)
{
	if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr)
	{
		return;
	}

	if (Materials.Num() == 0)
	{
		UTU_LOG_L("                No Materials to assign to Static Mesh Component.");
		return;
	}

	UTU_LOG_L("                Associating Materials to Static Mesh Component...");

	// Get the Ids
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 0
	TArray<FStaticMaterial> Slots = StaticMeshComponent->GetStaticMesh()->GetStaticMaterials();
#else
	TArray<FStaticMaterial> Slots = StaticMeshComponent->GetStaticMesh()->StaticMaterials;
#endif
	TArray<FString> SortedMaterials = CalculateMaterialsSlotOrder(Materials, Slots);

	// Check if all materials are used
	for (FString Material : Materials)
	{
		// Slot for this mat?
		if (!SortedMaterials.Contains(Material))
		{
			UTU_LOG_W("                    No slot found for material: " + Material);
			continue;
		}
	}

	// Assign materials for real
	for (int MatId = 0; MatId < SortedMaterials.Num(); MatId++)
	{
		FString Material = SortedMaterials[MatId];

		// Get Material asset (try both material and material instance)
		TArray<FString> MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::MaterialInstance : EUtuUnrealAssetType::Material);
		UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		if (MaterialAsset == nullptr)
		{
			MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::Material : EUtuUnrealAssetType::MaterialInstance);
			MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		}

		// Find ID
		FString SlotName = FString::FromInt(MatId);
		if (Slots.IsValidIndex(MatId))
		{
			SlotName = Slots[MatId].MaterialSlotName.ToString();
		}
		UTU_LOG_L("                    MaterialSlot[" + SlotName + "] : " + MatNames[2]);

		// Apply material
		StaticMeshComponent->SetMaterial(MatId, MaterialAsset);
		if (MaterialAsset == nullptr)
		{
			UTU_LOG_W("                        Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
		}
	}
}

void FUtuPluginAssetTypeProcessor::AssignMaterialsToMesh(TArray<FString> Materials, USkeletalMesh* SkeletalMesh)
{
	if (SkeletalMesh == nullptr)
	{
		return;
	}

	if (Materials.Num() == 0)
	{
		UTU_LOG_L("                No Materials to assign to Skeletal Mesh.");
		return;
	}

	UTU_LOG_L("                Associating Materials to Skeletal Mesh...");

	// Get the Ids
	TArray<FSkeletalMaterial> SkeletalSlots = SkeletalMesh->Materials;

	// Convert to static
	TArray<FStaticMaterial> Slots = TArray<FStaticMaterial>();
	for (FSkeletalMaterial Slot : SkeletalSlots)
	{
		Slots.Add(FStaticMaterial(nullptr, Slot.MaterialSlotName, Slot.ImportedMaterialSlotName));
	}

	TArray<FString> SortedMaterials = CalculateMaterialsSlotOrder(Materials, Slots);

	// Assign materials for real
	SkeletalMesh->PreEditChange(NULL);
	SkeletalMesh->Materials.Empty();

	// Check if all materials are used
	for (FString Material : Materials)
	{
		// Slot for this mat?
		if (!SortedMaterials.Contains(Material))
		{
			UTU_LOG_W("                    No slot found for material: " + Material);
			continue;
		}
	}

	// Assign materials for real
	for (int MatId = 0; MatId < SortedMaterials.Num(); MatId++)
	{
		FString Material = SortedMaterials[MatId];

		// Get Material asset (try both material and material instance)
		TArray<FString> MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::MaterialInstance : EUtuUnrealAssetType::Material);
		UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		if (MaterialAsset == nullptr)
		{
			MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::Material : EUtuUnrealAssetType::MaterialInstance);
			MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		}

		// Find ID
		FString SlotName = FString::FromInt(MatId);
		if (Slots.IsValidIndex(MatId))
		{
			SlotName = Slots[MatId].MaterialSlotName.ToString();
		}
		UTU_LOG_L("                    MaterialSlot[" + SlotName + "] : " + MatNames[2]);

		// Apply material
		while (SkeletalMesh->Materials.Num() <= MatId)
		{
			SkeletalMesh->Materials.Add(nullptr);
		}
		if (SkeletalMesh->Materials.IsValidIndex(MatId))
		{
			SkeletalMesh->Materials[MatId] = MaterialAsset;
		}
		if (MaterialAsset == nullptr)
		{
			UTU_LOG_W("                        Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
		}
	}

	SkeletalMesh->Modify();
	SkeletalMesh->PostEditChange();
}

void FUtuPluginAssetTypeProcessor::AssignMaterialsToMesh(TArray<FString> Materials, USkeletalMeshComponent* SkeletalMeshComponent)
{
	if (SkeletalMeshComponent == nullptr || SkeletalMeshComponent->SkeletalMesh == nullptr)
	{
		return;
	}

	if (Materials.Num() == 0)
	{
		UTU_LOG_L("                No Materials to assign to Skeletal Mesh Component.");
		return;
	}

	UTU_LOG_L("                Associating Materials to Skeletal Mesh Component...");

	// Get the Ids
	TArray<FSkeletalMaterial> SkeletalSlots = SkeletalMeshComponent->SkeletalMesh->Materials;

	// Convert to static
	TArray<FStaticMaterial> Slots = TArray<FStaticMaterial>();
	for (FSkeletalMaterial Slot : SkeletalSlots)
	{
		Slots.Add(FStaticMaterial(nullptr, Slot.MaterialSlotName, Slot.ImportedMaterialSlotName));
	}

	TArray<FString> SortedMaterials = CalculateMaterialsSlotOrder(Materials, Slots);

	// Check if all materials are used
	for (FString Material : Materials)
	{
		// Slot for this mat?
		if (!SortedMaterials.Contains(Material))
		{
			UTU_LOG_W("                    No slot found for material: " + Material);
			continue;
		}
	}

	// Assign materials for real
	for (int MatId = 0; MatId < SortedMaterials.Num(); MatId++)
	{
		FString Material = SortedMaterials[MatId];

		// Get Material asset (try both material and material instance)
		TArray<FString> MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::MaterialInstance : EUtuUnrealAssetType::Material);
		UMaterialInterface* MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		if (MaterialAsset == nullptr)
		{
			MatNames = FormatRelativeFilenameForUnreal(Material, ImportSettings.Materials.bCreateMaterialInstances ? EUtuUnrealAssetType::Material : EUtuUnrealAssetType::MaterialInstance);
			MaterialAsset = Cast<UMaterialInterface>(UUtuPluginLibrary::TryGetAsset(MatNames[2]));
		}

		// Find ID
		FString SlotName = FString::FromInt(MatId);
		if (Slots.IsValidIndex(MatId))
		{
			SlotName = Slots[MatId].MaterialSlotName.ToString();
		}
		UTU_LOG_L("                    MaterialSlot[" + SlotName + "] : " + MatNames[2]);

		// Apply material
		SkeletalMeshComponent->SetMaterial(MatId, MaterialAsset);
		if (MaterialAsset == nullptr)
		{
			UTU_LOG_W("                        Failed to assign material because it doesn't exists: '" + MatNames[2] + "'");
		}
	}
}

UFbxImportUI* FUtuPluginAssetTypeProcessor::GetStaticMeshImportOptions(FUtuPluginMesh InUtuMesh, FString SpecificSubmesh)
{
	UFbxImportUI* Options = NewObject<UFbxImportUI>();
	Options->bIsObjImport = InUtuMesh.mesh_file_absolute_filename.EndsWith(".obj", ESearchCase::IgnoreCase);
	Options->bAutomatedImportShouldDetectType = false;
	Options->bImportAnimations = false;
	Options->MeshTypeToImport = EFBXImportType::FBXIT_StaticMesh;
	Options->bImportMesh = true;
	Options->bImportTextures = false;
	Options->bImportMaterials = false;
	Options->bImportAsSkeletal = false;
	Options->bResetToFbxOnMaterialConflict = true;
	if (ImportSettings.StaticMeshes.bImportSeparated)
	{
		Options->StaticMeshImportData->bCombineMeshes = false;
		Options->StaticMeshImportData->bTransformVertexToAbsolute = true;
	}
	else
	{
		Options->StaticMeshImportData->bCombineMeshes = true;
		Options->StaticMeshImportData->bTransformVertexToAbsolute = false;
	}
	Options->StaticMeshImportData->bBakePivotInVertex = ImportSettings.StaticMeshes.bBakePivotInVertex;
	Options->StaticMeshImportData->bConvertScene = ImportSettings.StaticMeshes.bConvertScene;
	Options->StaticMeshImportData->bForceFrontXAxis = ImportSettings.StaticMeshes.bForceFrontXAxis;
	Options->StaticMeshImportData->bConvertSceneUnit = ImportSettings.StaticMeshes.bConvertSceneUnit;
	Options->StaticMeshImportData->bAutoGenerateCollision = ImportSettings.StaticMeshes.bAutoGenerateCollision;
	Options->StaticMeshImportData->bGenerateLightmapUVs = ImportSettings.StaticMeshes.bGenerateLightmapUVs;
	Options->StaticMeshImportData->bRemoveDegenerates = ImportSettings.StaticMeshes.bRemoveDegenerates;
	Options->StaticMeshImportData->NormalImportMethod = ImportSettings.StaticMeshes.NormalImportMethod;
	Options->StaticMeshImportData->NormalGenerationMethod = ImportSettings.StaticMeshes.NormalGenerationMethod;
	Options->StaticMeshImportData->bComputeWeightedNormals = ImportSettings.StaticMeshes.bComputeWeightedNormals;

	// Figure out transform
	FVector Loc = UtuConst::ConvertLocation(InUtuMesh.mesh_import_position_offset, true, InUtuMesh.mesh_import_rotation_offset) / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X) + ImportSettings.StaticMeshes.ImportLocationOffset;
	FRotator Rot = UKismetMathLibrary::ComposeRotators(ImportSettings.StaticMeshes.ImportRotationOffset, FRotator(UtuConst::ConvertRotation(InUtuMesh.mesh_import_rotation_offset, true)));
	float ScaleMultiplier = IsFbxExporter(InUtuMesh.mesh_file_absolute_filename) ? (InUtuMesh.use_file_scale ? ImportSettings.StaticMeshes.FbxExporterImportScaleMultiplierIfUseFileScale : ImportSettings.StaticMeshes.FbxExporterImportScaleMultiplierIfNotUseFileScale) : (InUtuMesh.use_file_scale ? ImportSettings.StaticMeshes.ImportScaleMultiplierIfUseFileScale : ImportSettings.StaticMeshes.ImportScaleMultiplierIfNotUseFileScale);
	float Scale = InUtuMesh.mesh_import_scale_factor * ScaleMultiplier;
	float Sca = Scale / FMath::Max(0.0001f, InUtuMesh.mesh_import_scale_offset.X);
	
	// If separated, use the most popular import transform
	if (ImportSettings.StaticMeshes.bImportSeparated && InUtuMesh.submeshes.Num() > 0)
	{
		// Calculate them all
		TMap<FString, int> PossibleTransformsString = TMap<FString, int>();
		TMap<FString, FTransform> PossibleTransforms = TMap<FString, FTransform>();
		for (FUtuPluginSubmesh SubMesh : InUtuMesh.submeshes)
		{
			if (SpecificSubmesh == "" || SubMesh.submesh_name == SpecificSubmesh)
			{
				FVector L = UtuConst::ConvertLocation(SubMesh.submesh_world_location, true, SubMesh.submesh_world_rotation) / FMath::Max(0.0001f, SubMesh.submesh_world_scale.X);
				FRotator R = UKismetMathLibrary::ComposeRotators(ImportSettings.StaticMeshes.ImportRotationOffset, FRotator(UtuConst::ConvertRotation(SubMesh.submesh_world_rotation, true)));
				float S = Scale / FMath::Max(0.0001f, SubMesh.submesh_world_scale.X);
				FString TransformString = FIntVector(L * 100).ToString() + "_" + FIntVector(R.Pitch * 100, R.Yaw * 100, R.Roll * 100).ToString() + "_" + FString::FromInt(FMath::RoundToInt(S * 100));
				if (PossibleTransforms.Contains(TransformString))
				{
					PossibleTransformsString[TransformString]++;
				}
				else
				{
					FTransform T = FTransform(R, L, FVector(S));
					PossibleTransformsString.Add(TransformString, 1);
					PossibleTransforms.Add(TransformString, T);
				}
			}
		}

		// Get most popular
		FTransform Transform = FTransform(Rot, Loc, FVector(Sca));
		int TransformCount = 0;
		TArray<FString> Keys = TArray<FString>();
		PossibleTransformsString.GetKeys(Keys);
		for (FString Str : Keys)
		{
			if (PossibleTransformsString[Str] > TransformCount)
			{
				Transform = PossibleTransforms[Str];
				TransformCount = PossibleTransformsString[Str];
			}
		}

		// Set final transform
		Loc = Transform.GetLocation();
		Rot = Transform.GetRotation().Rotator();
		Sca = Transform.GetScale3D().X;
	}

	Options->StaticMeshImportData->ImportTranslation = Loc;
	Options->StaticMeshImportData->ImportRotation = Rot;
	Options->StaticMeshImportData->ImportUniformScale = Sca;
	return Options;
}


TArray<FString> FUtuPluginAssetTypeProcessor::GetAllPropertiesAsString(UObject* Object)
{
	if (Object == nullptr)
	{
		return TArray<FString>();
	}

	TArray<FString> ResultString;

	for (TFieldIterator<FProperty> PropertyIt(Object->GetClass(), EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;

		// No properties of "Object" type, we only want properties with value data.
		if (Property->IsA<FDelegateProperty>()) continue;
		if (Property->IsA<FMulticastDelegateProperty>()) continue;
		if (Property->IsA<FMulticastInlineDelegateProperty>()) continue;
		if (Property->IsA<FObjectProperty>() && CastField<FObjectProperty>(Property) != nullptr)
		{
			FObjectProperty* ObjProp = CastField<FObjectProperty>(Property);
			ResultString.Append(GetAllPropertiesAsString(ObjProp->GetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(Object))));
		}

		FString ValueText;
		if (Property->ExportText_InContainer(0, ValueText, Object, Object, Object, PPF_None))
		{
			if (ValueText != "" && ValueText != "None")
			{
				ResultString.Add(Property->GetFName().ToString() + " = " + ValueText);
			}
		}
	}
	return ResultString;
}


UAssetImportTask* FUtuPluginAssetTypeProcessor::BuildTask(FString InSource, TArray<FString> InAssetNames, UObject * InOptions) {
	UAssetImportTask* RetTask = NewObject<UAssetImportTask>();
	RetTask->Filename = InSource;
	RetTask->DestinationPath = InAssetNames[0];
	RetTask->DestinationName = InAssetNames[1];
	RetTask->bSave = false;
	RetTask->bAutomated = true;
	RetTask->bReplaceExisting = true;
	RetTask->Options = InOptions;
	return RetTask;
}

TArray<FString> FUtuPluginAssetTypeProcessor::StartProcessAsset(FUtuPluginAsset InUtuAsset, EUtuUnrealAssetType AssetType) {
	TArray<FString> RetAssetNames = FormatRelativeFilenameForUnreal(InUtuAsset.asset_relative_filename, AssetType);
	UTU_LOG_EMPTY_LINE();
	UTU_LOG_L("Asset Name: " + RetAssetNames[1]);
	UTU_LOG_L("    Unity  Asset Relative Path: " + InUtuAsset.asset_relative_filename);
	UTU_LOG_L("    Unreal Asset Relative Path: " + RetAssetNames[2]);
	UTU_LOG_L("Begin Creation ...");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
	return RetAssetNames;
}

bool FUtuPluginAssetTypeProcessor::DeleteInvalidAssetIfNeeded(TArray<FString> InAssetNames, UClass* InClass) 
{
	UObject* Asset = UUtuPluginLibrary::TryGetAsset(InAssetNames[2]);
	if (Asset != nullptr && InClass != nullptr) 
	{
		if (Asset->GetClass() != InClass) 
		{
			if (ImportSettings.bDeleteInvalidAssets) 
			{
				UTU_LOG_L("    Existing Invalid Asset detected, deleting ...");
				if (UUtuPluginLibrary::DeleteAsset(Asset) && UUtuPluginLibrary::TryGetAsset(InAssetNames[2]) == nullptr) 
				{
					UTU_LOG_L("        Invalid Asset deleted.");
				}
				else 
				{
					UTU_LOG_E("    Asset Name: " + InAssetNames[1]);
					UTU_LOG_E("        Unreal Asset Relative Path: " + InAssetNames[2]);
					UTU_LOG_E("            You are trying to import a '" + InClass->GetDisplayNameText().ToString() + "' over an already existing asset that is of type '" + Asset->GetClass()->GetDisplayNameText().ToString() + "'.");
					UTU_LOG_E("            Tried to delete the Invalid Asset, but failed to delete it.");
					UTU_LOG_E("            Potential Causes:");
					UTU_LOG_E("                - The asset might be referenced by other content. Please delete it manually.");
					UTU_LOG_E("            Asset skipped.");
					return false;
				}
			}
			else 
			{
				UTU_LOG_W("    Asset Name: " + InAssetNames[1]);
				UTU_LOG_W("        Unreal Asset Relative Path: " + InAssetNames[2]);
				UTU_LOG_W("            You are trying to import a '" + InClass->GetDisplayNameText().ToString() + "' over an already existing asset that is of type '" + Asset->GetClass()->GetDisplayNameText().ToString() + "'.");
				UTU_LOG_W("            If you want to process this asset, please delete the current asset or enable the 'Delete Invalid Assets' functionnality.");
				UTU_LOG_W("            Asset skipped.");
				return false;
			}
		}
	}
	return true;
}

bool FUtuPluginAssetTypeProcessor::IsFbxExporter(FString Path)
{
	return Path.Contains("ExportedFbxFiles");
}

FUtuPluginImportSettings_AllAssets FUtuPluginAssetTypeProcessor::GetAssetStruct(EUtuUnrealAssetType AssetType)
{
	switch (AssetType)
	{
	case EUtuUnrealAssetType::Level:
		return ImportSettings.Scenes;
		break;
	case EUtuUnrealAssetType::Animation:
		return ImportSettings.Animations;
		break;
	case EUtuUnrealAssetType::StaticMesh:
		return ImportSettings.StaticMeshes;
		break;
	case EUtuUnrealAssetType::SkeletalMesh:
		return ImportSettings.SkeletalMeshes;
		break;
	case EUtuUnrealAssetType::Material:
		return ImportSettings.Materials;
		break;
	case EUtuUnrealAssetType::MaterialInstance:
		return ImportSettings.MaterialInstances;
		break;
	case EUtuUnrealAssetType::Texture:
		return ImportSettings.Textures;
		break;
	case EUtuUnrealAssetType::Blueprint:
		return ImportSettings.Blueprints;
		break;
	default:
		break;
	}
	return FUtuPluginImportSettings_AllAssets();
}
