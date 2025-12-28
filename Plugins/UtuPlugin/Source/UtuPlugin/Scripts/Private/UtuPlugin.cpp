// Copyright Alex Quevillon. All Rights Reserved.

#include "UtuPlugin/Scripts/Public/UtuPlugin.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLog.h"
#include "UtuPlugin/Scripts/Public/UtuPluginPaths.h"
#include "UtuPlugin/Scripts/Public/UtuPluginLibrary.h"
#include "Runtime/Launch/Resources/Version.h" 

#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "JsonObjectConverter.h" 

FUtuPluginCurrentImport UUtuPlugin::currentImportJob;
FUtuPluginImportSettings UUtuPlugin::currentImportSettings;

void FUtuPluginCurrentImport::Import(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool executeFullImportOnSameFrame) {
	BeginImport(Json, AssetTypes);
	if (executeFullImportOnSameFrame) {
		while (ContinueImport(executeFullImportOnSameFrame) != true) {
			// ContinueImport
		}
	}
}

void FUtuPluginCurrentImport::BeginImport(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes) {
	TArray<EUtuAssetType> assetTypesOrder = { EUtuAssetType::Texture, EUtuAssetType::Material, EUtuAssetType::Mesh, EUtuAssetType::Animation, EUtuAssetType::PrefabFirstPass, EUtuAssetType::PrefabSecondPass, EUtuAssetType::Scene };
	assetTypesToProcess.Empty();
	for (EUtuAssetType AssetType : assetTypesOrder) {
		if (AssetTypes.Contains(AssetType)) {
			assetTypesToProcess.Add(AssetType); // Order is important
		}
	}
	countAssetProcessedForSave = 0;
	countAssetTypesToProcess = 1;
	amountAssetTypesToProcess = assetTypesToProcess.Num();
	percentAssetTypesToProcess = (float)countAssetTypesToProcess / (float)amountAssetTypesToProcess;
	json = Json;
	PopulateListOfDuplicatedAssetNames(Json);

	timestamp = FDateTime::UtcNow().ToString().Replace(TEXT("-"), TEXT("_")).Replace(TEXT("."), TEXT(""));
	UUtuPluginLog::InitializeNewLog(json.json_info.json_file_fullname, json.json_info.export_timestamp);
	// Log
	UTU_LOG_CLEAR();
	UTU_LOG_SEPARATOR_LINE();
	UTU_LOG_L("Beginning New Import...");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());;;
	UTU_LOG_L("    Utu Version: " + UUtuPlugin::GetUtuPluginVersion());
	UTU_LOG_L("    Unreal Version: " + FString::FromInt(ENGINE_MAJOR_VERSION) + "." + FString::FromInt(ENGINE_MINOR_VERSION));
	UTU_LOG_L("    Json File Fullname: " + json.json_info.json_file_fullname);
	UTU_LOG_L("        Export Name: " + json.json_info.export_name);
	UTU_LOG_L("        Export Time: " + json.json_info.export_datetime);
	UTU_LOG_L("        Export Utu Version: " + json.json_info.utu_plugin_version);
	UTU_LOG_L("        Export Scene Quantity: " + FString::FromInt(json.json_info.scenes.Num()));
	UTU_LOG_L("        Export Prefab Quantity: " + FString::FromInt(json.json_info.prefabs.Num()));
	UTU_LOG_L("        Export Mesh Quantity: " + FString::FromInt(json.json_info.meshes.Num()));
	UTU_LOG_L("        Export Material Quantity: " + FString::FromInt(json.json_info.materials.Num()));
	UTU_LOG_L("        Export Texture Quantity: " + FString::FromInt(json.json_info.textures.Num()));
	UTU_LOG_L("    Asset Types to process: ");
	for (EUtuAssetType AssetType : assetTypesToProcess) {
		UTU_LOG_L("        " + AssetTypeToString(AssetType));
	}
	if (ListOfDuplicatedAssetNames.Num() > 0)
	{
		UTU_LOG_L("    Asset with duplicated names that will be renamed during the process: ");
		for (FString Name : ListOfDuplicatedAssetNames) {
			UTU_LOG_L("        " + Name);
		}
	}

	UTU_LOG_L("    Import Options: ");
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FUtuPluginCurrentImport::StaticStruct(), this, JsonString, 0, 0);
	TArray<FString> Properties;
	JsonString.ParseIntoArrayLines(Properties);
	for (FString Property : Properties)
	{
		UTU_LOG_L("        " + Property);
	}

	UTU_LOG_SEPARATOR_LINE();
}

bool FUtuPluginCurrentImport::ContinueImport(bool executeFullImportOnSameFrame) 
{
	if (!currentAssetTypeProcessor.bIsValid && assetTypesToProcess.Num() == 0) 
	{
		//UTU_LOG_E("FUtuPluginCurrentImport::ContinueImport() Was called even though the list is already empty. This should never happen!");
		CompleteImport();
		return true;
	}
	if (!currentAssetTypeProcessor.bIsValid && assetTypesToProcess.Num() > 0) 
	{
		currentAssetTypeProcessor = FUtuPluginAssetTypeProcessor();
		currentAssetTypeProcessor.bIsValid = true;
		currentAssetTypeProcessor.ImportSettings = UUtuPlugin::currentImportSettings;
		nameUtuAssetTypesToProcess = AssetTypeToString(assetTypesToProcess[0]);
		currentAssetTypeProcessor.Import(json, assetTypesToProcess[0], executeFullImportOnSameFrame, ListOfDuplicatedAssetNames);
		assetTypesToProcess.RemoveAt(0);
		UTU_LOG_SEPARATOR_LINE();
		UTU_LOG_L("Starting to import assets of type: " + nameUtuAssetTypesToProcess + "...");
		UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
		UTU_LOG_L("    Quantity: " + FString::FromInt(currentAssetTypeProcessor.GetAssetsNum()));
		UTU_LOG_SEMI_SEPARATOR_LINE();
	}
	if (executeFullImportOnSameFrame) 
	{
		currentAssetTypeProcessor.bIsValid = false;
		countAssetTypesToProcess = amountAssetTypesToProcess;
		percentAssetTypesToProcess = (float)countAssetTypesToProcess / (float)FMath::Max(amountAssetTypesToProcess, 1);
	}
	else 
	{
		if (currentAssetTypeProcessor.ContinueImport()) 
		{
			currentAssetTypeProcessor.bIsValid = false;
			countAssetTypesToProcess++;
			percentAssetTypesToProcess = (float)countAssetTypesToProcess / (float)FMath::Max(amountAssetTypesToProcess, 1);
			countAssetProcessedForSave++;
			if (UUtuPlugin::currentImportSettings.SavingBehavior == EUtuSavingBehavior::SaveAllEveryXxAssets && countAssetProcessedForSave >= UUtuPlugin::currentImportSettings.SavingIntervals)
			{
				countAssetProcessedForSave = 0;
				UTU_LOG_L("        Saving all modified assets...");
				TArray<UPackage*> Packages;
				FEditorFileUtils::GetDirtyContentPackages(Packages);
				FEditorFileUtils::GetDirtyWorldPackages(Packages);
				FEditorFileUtils::PromptForCheckoutAndSave(Packages, false, false);
			}
		}
	}
	if (!currentAssetTypeProcessor.bIsValid && assetTypesToProcess.Num() == 0) {
		CompleteImport();
		return true;
	}
	return false;
}



void FUtuPluginCurrentImport::CompleteImport() {
	UTU_LOG_SEPARATOR_LINE();
	UTU_LOG_L("Completing Import...");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());

	if (UUtuPlugin::currentImportSettings.SavingBehavior == EUtuSavingBehavior::PromptAtEnd)
	{
		UTU_LOG_L("        Prompt user for save all...");
		TArray<UPackage*> Packages;
		FEditorFileUtils::GetDirtyContentPackages(Packages);
		FEditorFileUtils::GetDirtyWorldPackages(Packages);
		FEditorFileUtils::PromptForCheckoutAndSave(Packages, false, true);
	}
	else
	{
		UTU_LOG_L("        Saving all modified assets...");
		TArray<UPackage*> Packages;
		FEditorFileUtils::GetDirtyContentPackages(Packages);
		FEditorFileUtils::GetDirtyWorldPackages(Packages);
		FEditorFileUtils::PromptForCheckoutAndSave(Packages, false, false);
	}
	UTU_LOG_SEPARATOR_LINE();
	UTU_LOG_L("Import Completed!");
	UTU_LOG_L("    Time: " + FDateTime::UtcNow().ToString());
	EUtuLog LogState;
	int WarningCount;
	int ErrorCount;
	UUtuPluginLog::GetLogState(LogState, WarningCount, ErrorCount);
	UTU_LOG("    Warning Count: " + FString::FromInt(WarningCount), WarningCount > 0 ? EUtuLog::Warning : EUtuLog::Log);
	UTU_LOG("    Error   Count: " + FString::FromInt(ErrorCount), ErrorCount > 0 ? EUtuLog::Error : EUtuLog::Log);
	UTU_LOG_SEPARATOR_LINE();
	UUtuPluginLog::PrintIntoLogFile("", true);
}

FString FUtuPluginCurrentImport::AssetTypeToString(EUtuAssetType AssetType) {
	switch (AssetType) {
	case EUtuAssetType::Scene:
		return "Scenes";
		break;
	case EUtuAssetType::Mesh:
		return "Meshes";
		break;
	case EUtuAssetType::Animation:
		return "Animations";
		break;
	case EUtuAssetType::Material:
		return "Materials";
		break;
	case EUtuAssetType::Texture:
		return "Textures";
		break;
	case EUtuAssetType::PrefabFirstPass:
		return "Prefabs: First Pass";
		break;
	case EUtuAssetType::PrefabSecondPass:
		return "Prefabs: Second Pass";
		break;
	}
	return "";
}


void FUtuPluginCurrentImport::PopulateListOfDuplicatedAssetNames(FUtuPluginJson Json)
{
	ListOfDuplicatedAssetNames.Empty();

	FUtuPluginAssetTypeProcessor Proc = FUtuPluginAssetTypeProcessor();
	TArray<FString> UniqueNames = TArray<FString>();

	// Find all duplicated names
	if (UUtuPlugin::currentImportSettings.Scenes.AssetRenameSettings.bAutoRenameDuplicatedAssets)
	{
		for (FUtuPluginAsset Asset : Json.scenes)
		{
			TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::Level);
			if (UniqueNames.Contains(AssetNames[2])) // Already in
			{
				ListOfDuplicatedAssetNames.Add(AssetNames[2]);
			}
			UniqueNames.AddUnique(AssetNames[2]);
		}
	}
	for (FUtuPluginMesh Asset : Json.meshes)
	{
		if (Asset.is_skeletal_mesh)
		{
			if (UUtuPlugin::currentImportSettings.SkeletalMeshes.AssetRenameSettings.bAutoRenameDuplicatedAssets)
			{
				TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::SkeletalMesh);
				if (UniqueNames.Contains(AssetNames[2])) // Already in
				{
					ListOfDuplicatedAssetNames.Add(AssetNames[2]);
				}
				UniqueNames.AddUnique(AssetNames[2]);
			}
		}
		else
		{
			if (UUtuPlugin::currentImportSettings.StaticMeshes.AssetRenameSettings.bAutoRenameDuplicatedAssets)
			{
				TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::StaticMesh);
				if (UniqueNames.Contains(AssetNames[2])) // Already in
				{
					ListOfDuplicatedAssetNames.Add(AssetNames[2]);
				}
				UniqueNames.AddUnique(AssetNames[2]);
			}
		}
	}
	if (UUtuPlugin::currentImportSettings.Animations.AssetRenameSettings.bAutoRenameDuplicatedAssets)
	{
		for (FUtuPluginAsset Asset : Json.animations)
		{
			TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::Animation);
			if (UniqueNames.Contains(AssetNames[2])) // Already in
			{
				ListOfDuplicatedAssetNames.Add(AssetNames[2]);
			}
			UniqueNames.AddUnique(AssetNames[2]);
		}
	}
	if (UUtuPlugin::currentImportSettings.Materials.AssetRenameSettings.bAutoRenameDuplicatedAssets)
	{
		for (FUtuPluginAsset Asset : Json.materials)
		{
			TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::Material);
			if (UniqueNames.Contains(AssetNames[2])) // Already in
			{
				ListOfDuplicatedAssetNames.Add(AssetNames[2]);
			}
			UniqueNames.AddUnique(AssetNames[2]);
		}
	}
	if (UUtuPlugin::currentImportSettings.Textures.AssetRenameSettings.bAutoRenameDuplicatedAssets)
	{
		for (FUtuPluginAsset Asset : Json.textures)
		{
			TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::Texture);
			if (UniqueNames.Contains(AssetNames[2])) // Already in
			{
				ListOfDuplicatedAssetNames.Add(AssetNames[2]);
			}
			UniqueNames.AddUnique(AssetNames[2]);
		}
	}
	if (UUtuPlugin::currentImportSettings.Blueprints.AssetRenameSettings.bAutoRenameDuplicatedAssets)
	{
		for (FUtuPluginAsset Asset : Json.prefabs_first_pass)
		{
			TArray<FString> AssetNames = Proc.FormatRelativeFilenameForUnreal(Asset.asset_relative_filename, EUtuUnrealAssetType::Blueprint);
			if (UniqueNames.Contains(AssetNames[2])) // Already in
			{
				ListOfDuplicatedAssetNames.Add(AssetNames[2]);
			}
			UniqueNames.AddUnique(AssetNames[2]);
		}
	}
}


void UUtuPlugin::Import(FUtuPluginJson Json, TArray<EUtuAssetType> AssetTypes, bool executeFullImportOnSameFrame) {
	static FTick TickInstance;
	currentImportJob = FUtuPluginCurrentImport();
	currentImportJob.bIsValid = true;
	currentImportJob.Import(Json, AssetTypes, executeFullImportOnSameFrame);
	if (executeFullImportOnSameFrame) {
		currentImportJob.bIsValid = false;
	}
}

FUtuPluginCurrentImport UUtuPlugin::GetCurrentImportState() {
	return UUtuPlugin::currentImportJob;
}

FString UUtuPlugin::GetPluginFolderFullExports()
{
	return UtuPluginPaths::pluginFolder_Full_Exports;
}

void UUtuPlugin::SetPluginFolderFullExportsCustom(FString Path)
{
	UtuPluginPaths::pluginFolder_Full_Exports_Custom = Path;
	UtuPluginPaths::pluginFolder_Full_Exports_Custom = UtuPluginPaths::pluginFolder_Full_Exports_Custom.Replace(TEXT("\\"), TEXT("/"));
	UtuPluginPaths::pluginFolder_Full_Exports_Custom.RemoveFromEnd("/");
	if (!UtuPluginPaths::pluginFolder_Full_Exports_Custom.EndsWith("/UtuPlugin/Exports"))
	{
		UtuPluginPaths::pluginFolder_Full_Exports_Custom = UtuPluginPaths::pluginFolder_Full_Exports_Custom + "/UtuPlugin/Exports";
	}
}

void UUtuPlugin::SetImportSettings(FUtuPluginImportSettings ImportSettings)
{
	UUtuPlugin::currentImportSettings = ImportSettings;
}

FString UUtuPlugin::GetUtuPluginVersion()
{
	FString TextString;
	FString FilePath = IPluginManager::Get().FindPlugin(TEXT("UtuPlugin"))->GetBaseDir() + "/Resources/UtuPluginVersion.txt";
	FFileHelper::LoadFileToString(TextString, *FilePath);
	return TextString;
}

FString UUtuPlugin::GetUtuPluginInfo()
{
	FString TextString;
	FString FilePath = IPluginManager::Get().FindPlugin(TEXT("UtuPlugin"))->GetBaseDir() + "/Resources/UtuPluginInfo.txt";
	FFileHelper::LoadFileToString(TextString, *FilePath);
	return TextString;
}

bool UUtuPlugin::IsGarbageCollecting()
{
	return GIsGarbageCollecting;
}

FUtuPluginCurrentImport UUtuPlugin::ContinueCurrentImport() {
	if (currentImportJob.bIsValid) {
		if (currentImportJob.ContinueImport(false)) {
			currentImportJob.bIsValid = false;
		}
	}
	return currentImportJob;
}

void UUtuPlugin::CancelImport() {
	currentImportJob.bIsValid = false;
	UUtuPluginLog::PrintIntoLogFile("\n\n\n\n\n\nImport Cancelled By User!", true);
}

void FTick::Tick(float DeltaTime) {
	static int TickCount = 0; // To give enough time to the user to cancel
	TickCount++;
	if (TickCount == 10) {
		UUtuPlugin::ContinueCurrentImport();
		TickCount = 0;
	}
}

TStatId FTick::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(FTick, STATGROUP_Tickables);
}
