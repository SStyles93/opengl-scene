# GPR5300 - ComputerGraphics

This project was done at the SAE Institute Geneva during the GPR5300-ComputerGraphics module.
The project is a grouping of all the code done while learning [OpenGL](https://www.opengl.org/) and its creation 
is resumed in my blogpost that you can find [here](https://sstyles93.github.io/).

[Folders](#folders) | [Folder content](#folder-content) |  
----- | -----
[Data](#data) - [Include](#include) - [Main](#main) - [Src](#src) |  [Main](#pr-main) - [Include](#pr-include) - [Source](#pr-source)


## Folders

-	<h4 id="data"><a href="https://github.com/SStyles93/opengl-scene/tree/main/data">Data</a></h4>  
	Contains the shaders used for the project. You can download the textures and objects [here](https://drive.google.com/drive/u/0/folders/1r60pUaqA7q4aLQIBk-h2yQ0TdRas9ExQ).	

-	<h4 id="include"><a href="https://github.com/SStyles93/opengl-scene/tree/main/include">Include</a></h4>  
	Contains all the header files

-	<h4 id="main"><a href="https://github.com/SStyles93/opengl-scene/tree/main/main">Main</a></h4>  
	Contains all the Scenes that illustrate the different steps of OpenGL

-	<h4 id="src"><a href="https://github.com/SStyles93/opengl-scene/tree/main/src">Src</a></h4>  
	Contains all the .cpp files

## Folder content

<details>
<summary> <h3 id="pr-main"> Main </h3> </summary>
	<p>         

-	[00_triangle_test](https://github.com/SStyles93/opengl-scene/blob/main/main/00_triangle_test.cpp)  
	Introduction to OpenGL, drawing the first triangle.

-	[01_light_test](https://github.com/SStyles93/opengl-scene/blob/main/main/01_light_test.cpp)  
	Implementation of simple cube, point, spot, flash and directional light.

-	[02_model_test](https://github.com/SStyles93/opengl-scene/blob/main/main/02_model_test.cpp)  
	First implementation of model loading.

-	[03_stensil_test](https://github.com/SStyles93/opengl-scene/blob/main/main/03_stensil_test.cpp)  
	Test at Depth/Stensil buffers.

-	[04_blend_test](https://github.com/SStyles93/opengl-scene/blob/main/main/04_blend_test.cpp)  
	Blending implementation.

-	[05_skybox_test](https://github.com/SStyles93/opengl-scene/blob/main/main/05_skybox_test.cpp)  
	First Skybox.

-	[06_instancing_test](https://github.com/SStyles93/opengl-scene/blob/main/main/06_instancing_test.cpp)  
	Object Instancing example.

-	[07_shadowMapping_test](https://github.com/SStyles93/opengl-scene/blob/main/main/07_shadowMapping_test.cpp)  
	Shadow mapping, shadows created with a directional light.

-	[08_point_shadows_test](https://github.com/SStyles93/opengl-scene/blob/main/main/08_point_shadows_test.cpp)  
	Shadows created with point lights.

-	[09_normal_test](https://github.com/SStyles93/opengl-scene/blob/main/main/09_normal_test.cpp)  
	Implementation of normal maps.

-	[10_hdr_test](https://github.com/SStyles93/opengl-scene/blob/main/main/10_hdr_test.cpp)  
	Hdr buffer tests.

-	[11_bloom_test](https://github.com/SStyles93/opengl-scene/blob/main/main/11_bloom_test.cpp)  
	Bloom and blur test with Reinhard tone mapping.

-	[12_deferred_shading_test](https://github.com/SStyles93/opengl-scene/blob/main/main/12_deferred_shading_test.cpp)  
	First implementation of deferred rendering.

-	[13_ssao_test](https://github.com/SStyles93/opengl-scene/blob/main/main/13_ssao_test.cpp)  
	Screen Space Ambient Occlusion.

-	[14_deferred_pbr](https://github.com/SStyles93/opengl-scene/blob/main/main/14_deferred_pbr.cpp)  
	First test with deferred rendering in PBR (physically based rendering).

-	[15_deferred_object](https://github.com/SStyles93/opengl-scene/blob/main/main/15_deferred_object.cpp)  
	Abstraction of objects in an Object class.

-	[16_deferred_pbr_skybox](https://github.com/SStyles93/opengl-scene/blob/main/main/16_deferred_pbr_skybox.cpp)  
	Second implementation of deferred rendering in PBR with skybox.

-	[17_def_pbr_sky_instances](https://github.com/SStyles93/opengl-scene/blob/main/main/17_def_pbr_sky_instances.cpp)  
	Third implementation of deferred rendering in PBR with skybox and instancing.

-	[18_def_pbr_sky_instances_ssao](https://github.com/SStyles93/opengl-scene/blob/main/main/18_def_pbr_sky_instances_ssao.cpp)  
	Fourth implementation of deferred rendering in PBR with skybox, instancing and SSAO

-	[IBL_simple_test](https://github.com/SStyles93/opengl-scene/blob/main/main/IBL_simple_test.cpp)  
	First Image Based Lighting (IBL) with no materials

-	[IBL_test](https://github.com/SStyles93/opengl-scene/blob/main/main/IBL_test.cpp)  
	Second Image Based Lighting (IBL) with materials

-	[19_All_in_with_IBL](https://github.com/SStyles93/opengl-scene/blob/main/main/19_All_in_with_IBL.cpp)  
	Second Image Based Lighting (IBL) with implementation of All previous elements

-	[20_IBL_Shadow](https://github.com/SStyles93/opengl-scene/blob/main/main/20_IBL_Shadow.cpp)  
	All previous elements with implementation of shadow mapping

-	[21_ALL_with_Bloom](https://github.com/SStyles93/opengl-scene/blob/main/main/21_ALL_with_Bloom.cpp)  
	All previous elements with implementation of Blur and Bloom with Narkowicz ACES tone mapping.

	</p>
</details>

<details>
<summary> <h3 id="pr-include"> Include </h3> </summary>
	<p> 

-	[bloom.h](https://github.com/SStyles93/opengl-scene/blob/main/include/bloom.h)  
	The include file with the class used for bloom effect, uses Narkowicz ACES tone mapping.

-	[camera.h](https://github.com/SStyles93/opengl-scene/blob/main/include/camera.h)  
	The camera class header file.

-	[engine.h](https://github.com/SStyles93/opengl-scene/blob/main/include/engine.h)  
	The engine class header file.

-	[file_utility.h](https://github.com/SStyles93/opengl-scene/blob/main/include/file_utility.h)  
	File loading utility.

-	[mesh.h](https://github.com/SStyles93/opengl-scene/blob/main/include/mesh.h)  
  	The mesh class header file.

-	[model.h](https://github.com/SStyles93/opengl-scene/blob/main/include/model.h)  
	The model class header file.

-	[object.h](https://github.com/SStyles93/opengl-scene/blob/main/include/object.h)  
	The object class header file.

-	[pipeline.h](https://github.com/SStyles93/opengl-scene/blob/main/include/object.h)  
	The pipeline class header file.

-	[scene.h](https://github.com/SStyles93/opengl-scene/blob/main/include/scene.h)  
	The scene class header file

-	[settings.h](https://github.com/SStyles93/opengl-scene/blob/main/include/settings.h)  
	The general settings used for the project

	</p>
</details>

<details>
<summary> <h3 id="pr-source"> Source </h3> </summary>
	<p>  

-	[camera.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/camera.cpp)  
	The camera class source file.

-	[engine.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/engine.cpp)  
	The engine class source file.

-	[file_utility.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/file_utility.cpp)  
	The file utility class source file.

-	[mesh.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/mesh.cpp)  
	The mesh class source file.

-	[model.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/model.cpp)  
	The model class source file.

-	[object.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/object.cpp)  
	The object class source file.

-	[pipeline.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/pipeline.cpp)  
	The pipeline class source file.

-	[scene.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/scene.cpp)  
	The scene class source file.

-	[stb_image.cpp](https://github.com/SStyles93/opengl-scene/blob/main/src/stb_image.cpp)  
	Implementation of the stb_image library.

	</p>
</details>


Note that this project was built using [CMake](https://cmake.org/) and [VCPKG](https://vcpkg.io/en/).

