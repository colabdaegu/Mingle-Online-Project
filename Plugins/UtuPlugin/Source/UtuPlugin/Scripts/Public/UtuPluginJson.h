// Copyright Alex Quevillon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "UtuPluginJson.generated.h"

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuAssetType : uint8 {
	Scene, Animation, Mesh, Material, Texture, PrefabFirstPass, PrefabSecondPass
};

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuActorType : uint8
{
	Empty, StaticMesh, SkeletalMesh, PointLight, DirectionalLight, SpotLight, Camera, Prefab
};

UENUM(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
enum class EUtuShaderOpacity : uint8
{
	Opaque, Masked, Translucent
};

// Not used in Unreal, but used in Unity
USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginSubmesh {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FString submesh_name = "";
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FVector submesh_relative_location = FVector();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FQuat submesh_relative_rotation = FQuat();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FVector submesh_relative_scale = FVector();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FVector submesh_world_location = FVector();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FQuat submesh_world_rotation = FQuat();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FVector submesh_world_scale = FVector();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FString> submesh_materials_relative_filenames = TArray<FString>();
};

			USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			struct UTUPLUGIN_API FUtuPluginActorCamera {
				GENERATED_USTRUCT_BODY()
			public:
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					FQuat camera_viewport_rect = FQuat(); //XYWH
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float camera_near_clip_plane = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float camera_far_clip_plane = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float camera_aspect_ratio = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					bool camera_is_perspective = false;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float camera_ortho_size = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float camera_persp_field_of_view = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					bool camera_is_physical = false;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float camera_phys_focal_length = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					FVector2D camera_phys_sensor_size = FVector2D();
			};

			USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			struct UTUPLUGIN_API FUtuPluginActorLight {
				GENERATED_USTRUCT_BODY()
			public:
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					FString light_color = ""; // Hex
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float light_intensity = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float light_range = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					float light_spot_angle = 0.0f;
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					bool light_is_casting_shadows = false;
			};
			
			USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			struct UTUPLUGIN_API FUtuPluginActorMesh {
				GENERATED_USTRUCT_BODY()
			public:
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					FString actor_mesh_relative_filename = ""; // Warning: if == "", means that the mesh is invalid / empty
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					FString actor_mesh_relative_filename_if_separated = ""; // Warning: if == "", means that the mesh is invalid / empty
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					TArray<FString> actor_mesh_materials_relative_filenames = TArray<FString>();
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					TArray<FString> actor_mesh_animations_relative_filenames = TArray<FString>();
			};


				USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					struct UTUPLUGIN_API FUtuPluginActorPrefabComponentOverride {
					GENERATED_USTRUCT_BODY()
				public:
					UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
						FString component_display_name = "";
					UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
						FString mesh_relative_filename = "";
					UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
						FString mesh_relative_filename_if_separated = "";
					UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
						TArray<FString> animation_relative_filenames = TArray<FString>();
					UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
						TArray<FString> material_relative_filenames = TArray<FString>();
				};


			USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			struct UTUPLUGIN_API FUtuPluginActorPrefab {
				GENERATED_USTRUCT_BODY()
			public:
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					FString actor_prefab_relative_filename = ""; // Warning: if == "", means that the mesh is invalid / empty
				UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
					TArray<FUtuPluginActorPrefabComponentOverride> actor_prefab_component_overrides = TArray<FUtuPluginActorPrefabComponentOverride>();
			};

			
		USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		struct UTUPLUGIN_API FUtuPluginActor {
			GENERATED_USTRUCT_BODY()
		public:
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				int actor_id = 0;
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				int actor_parent_id = 0;
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FString actor_display_name = "";
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FString actor_tag = "";
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				bool actor_is_visible = false;
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FVector actor_world_location = FVector();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FQuat actor_world_rotation = FQuat();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FVector actor_world_scale = FVector();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FVector actor_relative_location = FVector();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FQuat actor_relative_rotation = FQuat();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FVector actor_relative_scale = FVector();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				bool actor_is_movable = false;
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				TArray<EUtuActorType> actor_types = TArray<EUtuActorType>();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FUtuPluginActorMesh actor_mesh = FUtuPluginActorMesh();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FUtuPluginActorPrefab actor_prefab = FUtuPluginActorPrefab();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FUtuPluginActorLight actor_light = FUtuPluginActorLight();
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				FUtuPluginActorCamera actor_camera = FUtuPluginActorCamera();
		};


	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString asset_name = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString asset_relative_filename = "";
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginScene : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FUtuPluginActor> scene_actors = TArray<FUtuPluginActor>();
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginMesh : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString mesh_file_absolute_filename = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FVector mesh_import_position_offset = FVector();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FQuat mesh_import_rotation_offset = FQuat();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FVector mesh_import_scale_offset = FVector();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			float mesh_import_scale_factor = 0.0f;
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> mesh_materials_relative_filenames = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			bool is_skeletal_mesh = false;
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			bool use_file_scale = false;
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FUtuPluginSubmesh> submeshes = TArray<FUtuPluginSubmesh>();
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginMaterial : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		// Master material name
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString shader_name = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			EUtuShaderOpacity shader_opacity = EUtuShaderOpacity::Opaque;
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			bool two_sided = false;
		// If unsupported
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString main_texture = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FVector2D main_texture_scale = FVector2D();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FVector2D main_texture_offset = FVector2D();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString main_color = "";
		// All properties
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_floats_names = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<float> material_floats = TArray<float>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_ints_names = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<int> material_ints = TArray<int>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_textures_names = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_textures = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_colors_names = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_colors = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_vectors_names = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FQuat> material_vectors = TArray<FQuat>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> material_vector2s_names = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FVector2D> material_vector2s = TArray<FVector2D>();
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginAnimation : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString animation_file_absolute_filename = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> associated_skeletal_meshes_relative_filenames = TArray<FString>();
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginTexture : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString texture_file_absolute_filename = "";
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginPrefabFirstPass : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			bool has_any_static_child = false;
	};

	USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		struct UTUPLUGIN_API FUtuPluginPrefabSecondPass : public FUtuPluginAsset {
		GENERATED_USTRUCT_BODY()
		public:
			UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
				TArray<FUtuPluginActor> prefab_components = TArray<FUtuPluginActor>();
	};

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
	struct UTUPLUGIN_API FUtuPluginJsonInfo {
	GENERATED_USTRUCT_BODY()
	public:
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString export_name = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString export_datetime = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString export_timestamp = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString json_file_fullname = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			FString utu_plugin_version = "";
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> scenes = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> meshes = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> animations = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> materials = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> textures = TArray<FString>();
		UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
			TArray<FString> prefabs = TArray<FString>();
};

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginJson {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FUtuPluginJsonInfo json_info = FUtuPluginJsonInfo();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginScene> scenes = TArray<FUtuPluginScene>();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginMesh> meshes = TArray<FUtuPluginMesh>();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginAnimation> animations = TArray<FUtuPluginAnimation>();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginMaterial> materials = TArray<FUtuPluginMaterial>();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginTexture> textures = TArray<FUtuPluginTexture>();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginPrefabFirstPass> prefabs_first_pass = TArray<FUtuPluginPrefabFirstPass>();
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		TArray<FUtuPluginPrefabSecondPass> prefabs_second_pass = TArray<FUtuPluginPrefabSecondPass>();
};

USTRUCT(BlueprintType, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
struct UTUPLUGIN_API FUtuPluginConfigJson {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		FString unityFile_Full_Project = "";
};

UCLASS()
class UTUPLUGIN_API UUtuPluginJsonUtilities : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FUtuPluginJson ReadExportJsonFromFile(FString JsonFile);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static TArray<FString> GetAvailableExportJsons();
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FUtuPluginJsonInfo ReadExportJsonInfoFromFile(FString JsonFile);
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static TArray<FString> GetAvailableExportJsonInfos();
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void DeleteExportJson(FString ExportJsonFileFullname);
public:
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static FUtuPluginConfigJson ReadConfigJsonFromFile();
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Alex Quevillon Utu Plugin"), Category = "Alex Quevillon - Utu Plugin")
		static void WriteConfigJsonToFile(FUtuPluginConfigJson InConfig);
};

