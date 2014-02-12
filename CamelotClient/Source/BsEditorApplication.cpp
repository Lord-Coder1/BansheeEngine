#include "BsEditorApplication.h"
#include "BsEditorWindowManager.h"
#include "BsMainEditorWindow.h"
#include "BsApplication.h"
#include "CmApplication.h"
#include "CmRenderWindow.h"
#include "BsEditorGUI.h"
#include "BsUndoRedo.h"

// DEBUG ONLY
#include "DbgEditorWidget1.h"
#include "DbgEditorWidget2.h"
#include "CmResources.h"
#include "CmSceneObject.h"
#include "CmImporter.h"
#include "CmGpuProgram.h"
#include "CmGpuProgramImportOptions.h"
#include "CmShader.h"
#include "CmTexture.h"
#include "CmMaterial.h"
#include "CmTechnique.h"
#include "CmPass.h"
#include "BsRenderable.h"
#include "BsDbgTestGameObjectRef.h"
#include "BsVirtualInput.h"

using namespace CamelotFramework;
using namespace BansheeEngine;

namespace BansheeEditor
{
	EditorApplication::EditorApplication(RenderSystemPlugin renderSystemPlugin)
		:mActiveRSPlugin(renderSystemPlugin)
	{
		RENDER_WINDOW_DESC renderWindowDesc;
		renderWindowDesc.width = 1280;
		renderWindowDesc.height = 720;
		renderWindowDesc.title = "BansheeEditor";
		renderWindowDesc.fullscreen = false;
		renderWindowDesc.border = WindowBorder::None;

		const String& renderSystemLibraryName = getLibraryNameForRenderSystem(renderSystemPlugin);
		gBansheeApp().startUp(renderWindowDesc, renderSystemLibraryName, "BansheeForwardRenderer", L"D:\\CamelotResourceMetas"); // TODO - Make renderer and resource cache dir customizable
		EditorGUI::startUp(cm_new<EditorGUI>());

		{
			auto inputConfig = VirtualInput::instance().getConfiguration();

			inputConfig->registerButton("Rename", BC_F2);
			inputConfig->registerButton("Undo", BC_Z, VButtonModifier::Ctrl);
			inputConfig->registerButton("Redo", BC_Y, VButtonModifier::Ctrl);
			inputConfig->registerButton("Copy", BC_C, VButtonModifier::Ctrl);
			inputConfig->registerButton("Cut", BC_X, VButtonModifier::Ctrl);
			inputConfig->registerButton("Paste", BC_V, VButtonModifier::Ctrl);
		}

		//gApplication().loadPlugin("SBansheeEditor"); // Managed part of the editor

		/************************************************************************/
		/* 								DEBUG CODE                      		*/
		/************************************************************************/

		RenderSystem* renderSystem = RenderSystem::instancePtr();
		RenderWindowPtr renderWindow = gApplication().getPrimaryWindow();

		HSceneObject testModelGO = SceneObject::create("TestMesh");
		HRenderable testRenderable = testModelGO->addComponent<Renderable>();

		WString psLoc;
		WString vsLoc;

		GpuProgramProfile psProfile;
		GpuProgramProfile vsProfile;

		String psEntry;
		String vsEntry;

		String language;

		switch (renderSystemPlugin)
		{
		case RenderSystemPlugin::DX11:
			{
				psLoc = L"C:\\Projects\\BansheeEngine\\Data\\hlsl11_ps.gpuprog";
				vsLoc = L"C:\\Projects\\BansheeEngine\\Data\\hlsl11_vs.gpuprog";
				language = "hlsl";
				psProfile = GPP_PS_4_0;
				vsProfile = GPP_VS_4_0;
				psEntry = "ps_main";
				vsEntry = "vs_main";
				break;
			}
		case RenderSystemPlugin::DX9:
			{
				psLoc = L"C:\\Projects\\BansheeEngine\\Data\\hlsl9_ps.gpuprog";
				vsLoc = L"C:\\Projects\\BansheeEngine\\Data\\hlsl9_vs.gpuprog";
				language = "hlsl";
				psProfile = GPP_PS_2_0;
				vsProfile = GPP_VS_2_0;
				psEntry = "ps_main";
				vsEntry = "vs_main";
				break;
			}
		case RenderSystemPlugin::OpenGL:
			{
				psLoc = L"C:\\Projects\\BansheeEngine\\Data\\glsl_ps.gpuprog";
				vsLoc = L"C:\\Projects\\BansheeEngine\\Data\\glsl_vs.gpuprog";
				language = "glsl";
				psProfile = GPP_PS_2_0;
				vsProfile = GPP_VS_2_0;
				psEntry = "main";
				vsEntry = "main";
				break;
			}
		}

		ImportOptionsPtr gpuProgImportOptions = Importer::instance().createImportOptions(psLoc);
		if(rtti_is_of_type<GpuProgramImportOptions>(gpuProgImportOptions))
		{
			GpuProgramImportOptions* importOptions = static_cast<GpuProgramImportOptions*>(gpuProgImportOptions.get());

			importOptions->setEntryPoint(psEntry);
			importOptions->setLanguage(language);
			importOptions->setProfile(psProfile);
			importOptions->setType(GPT_FRAGMENT_PROGRAM);
		}

		HHighLevelGpuProgram fragProgRef = Importer::instance().import(psLoc, gpuProgImportOptions);

		gpuProgImportOptions = Importer::instance().createImportOptions(vsLoc);
		if(rtti_is_of_type<GpuProgramImportOptions>(gpuProgImportOptions))
		{
			GpuProgramImportOptions* importOptions = static_cast<GpuProgramImportOptions*>(gpuProgImportOptions.get());

			importOptions->setEntryPoint(vsEntry);
			importOptions->setLanguage(language);
			importOptions->setProfile(vsProfile);
			importOptions->setType(GPT_VERTEX_PROGRAM);
		}

		HHighLevelGpuProgram vertProgRef = Importer::instance().import(vsLoc, gpuProgImportOptions);

		gResources().create(vertProgRef, L"C:\\vertProgCg.vprog", true);
		gResources().unload(vertProgRef);
		vertProgRef = gResources().load(L"C:\\vertProgCg.vprog");

		gResources().create(fragProgRef, L"C:\\fragProgCg.vprog", true);
		gResources().unload(fragProgRef);
		fragProgRef = gResources().load(L"C:\\fragProgCg.vprog");

		ShaderPtr testShader = Shader::create("TestShader");

		testShader->addParameter("matViewProjection", "matViewProjection", GPDT_MATRIX_4X4);

		if(renderSystemPlugin == RenderSystemPlugin::DX11)
				testShader->addParameter("input", "input", GPDT_STRUCT, 2, 8);

		testShader->addParameter("samp", "samp", GPOT_SAMPLER2D);
		testShader->addParameter("tex", "tex", GPOT_TEXTURE2D);
		TechniquePtr newTechniqueGL = testShader->addTechnique("GLRenderSystem", "ForwardRenderer");
		PassPtr newPassGL = newTechniqueGL->addPass();
		newPassGL->setVertexProgram(vertProgRef);
		newPassGL->setFragmentProgram(fragProgRef);

		// TODO - I need to create different techniques for different render systems (and renderers, if there were any),
		// which is redundant as some techniques can be reused. I should add a functionality that supports multiple
		// render systems/renderers per technique
		TechniquePtr newTechniqueDX = testShader->addTechnique("D3D9RenderSystem", "ForwardRenderer");
		PassPtr newPassDX = newTechniqueDX->addPass();
		newPassDX->setVertexProgram(vertProgRef);
		newPassDX->setFragmentProgram(fragProgRef);

		TechniquePtr newTechniqueDX11 = testShader->addTechnique("D3D11RenderSystem", "ForwardRenderer");
		PassPtr newPassDX11 = newTechniqueDX11->addPass();
		newPassDX11->setVertexProgram(vertProgRef);
		newPassDX11->setFragmentProgram(fragProgRef);

		HMaterial testMaterial = Material::create();
		testMaterial->setShader(testShader);

		testMaterial->setMat4("matViewProjection", Matrix4::IDENTITY);

		if(renderSystemPlugin == RenderSystemPlugin::DX11)
		{
			float dbgMultipliers1[2];
			dbgMultipliers1[0] = 0.0f;
			dbgMultipliers1[1] = 0.0f;

			float dbgMultipliers2[2];
			dbgMultipliers2[0] = 1.0f;
			dbgMultipliers2[1] = 1.0f;

			testMaterial->setStructData("input", dbgMultipliers1, sizeof(dbgMultipliers1), 0);
			testMaterial->setStructData("input", dbgMultipliers2, sizeof(dbgMultipliers2), 1);
		}

		HTexture testTexRef = static_resource_cast<Texture>(Importer::instance().import(L"C:\\ArenaTowerDFS.psd"));
		HMesh dbgMeshRef = static_resource_cast<Mesh>(Importer::instance().import(L"C:\\X_Arena_Tower.FBX"));

		gResources().create(testTexRef, L"C:\\ExportTest.tex", true);
		gResources().create(dbgMeshRef, L"C:\\ExportMesh.mesh", true);

		gResources().unload(testTexRef);
		gResources().unload(dbgMeshRef);

		testTexRef = static_resource_cast<Texture>(gResources().loadAsync(L"C:\\ExportTest.tex"));
		dbgMeshRef = static_resource_cast<Mesh>(gResources().loadAsync(L"C:\\ExportMesh.mesh"));

		dbgMeshRef.synchronize();
		testTexRef.synchronize();

		testMaterial->setTexture("tex", testTexRef);
		gResources().create(testMaterial, L"C:\\ExportMaterial.mat", true);

		gResources().unload(testMaterial);

		testMaterial = gResources().load(L"C:\\ExportMaterial.mat");

		testRenderable->setMesh(dbgMeshRef);
		testRenderable->setMaterial(0, testMaterial);

		GameObjectHandle<DbgTestGameObjectRef> dbgTestGameObjectRef = testModelGO->addComponent<DbgTestGameObjectRef>();
		dbgTestGameObjectRef->mRenderable = testRenderable;

		HSceneObject clone = testModelGO->clone();
		GameObjectHandle<DbgTestGameObjectRef> clonedDbgTestGameObjectRef = clone->getComponent<DbgTestGameObjectRef>();
		
		testModelGO->destroy();


		HTexture dbgCursor = static_resource_cast<Texture>(Importer::instance().import(L"C:\\CursorDbg.psd"));
		PixelDataPtr cursorPixelData = dbgCursor->allocateSubresourceBuffer(0);

		gMainSyncedCA().readSubresource(dbgCursor.getInternalPtr(), 0, cursorPixelData);
		gMainSyncedCA().submitToCoreThread(true);

		/************************************************************************/
		/* 							END DEBUG CODE                      		*/
		/************************************************************************/

		UndoRedo::startUp(cm_new<UndoRedo>());
		EditorWindowManager::startUp(cm_new<EditorWindowManager>());
		MainEditorWindow* mainWindow = MainEditorWindow::create(gApplication().getPrimaryWindow());

		gApplication().mainLoopCallback.connect(boost::bind(&EditorApplication::update, this));

		DbgEditorWidget1::open(); // DEBUG ONLY
		DbgEditorWidget2::open(); // DEBUG ONLY

		gBansheeApp().runMainLoop();

		EditorWindowManager::shutDown();
		UndoRedo::shutDown();


		/************************************************************************/
		/* 								DEBUG CODE                      		*/
		/************************************************************************/

		gResources().unload(testTexRef);
		gResources().unload(dbgMeshRef);
		gResources().unload(fragProgRef);
		gResources().unload(vertProgRef);
		gResources().unload(testMaterial);

		testMaterial = nullptr;
		testTexRef = nullptr;
		dbgMeshRef = nullptr;
		fragProgRef = nullptr;
		vertProgRef = nullptr;

		testModelGO->destroy();

		newPassGL = nullptr;
		newTechniqueGL = nullptr;

		newPassDX = nullptr;
		newTechniqueDX = nullptr;

		newPassDX11 = nullptr;
		newTechniqueDX11 = nullptr;

		testShader = nullptr;

		renderWindow = nullptr;

		/************************************************************************/
		/* 							END DEBUG CODE                      		*/
		/************************************************************************/

		EditorGUI::shutDown();
		gBansheeApp().shutDown();

	}

	EditorApplication::~EditorApplication()
	{
		// TODO - Move shutdown code from constructor to here. Right now I don't care because cleanup 
		// isn't working as intended and if I move stuff I will probably break it even more
	}

	void EditorApplication::runMainLoop()
	{
		// TODO - Move "runMainLoop" code from constructor to here. Right now I don't care because cleanup 
		// isn't working as intended and if I move stuff I will probably break it even more
	}

	void EditorApplication::update()
	{
		EditorWindowManager::instance().update();	
	}

	const String& EditorApplication::getLibraryNameForRenderSystem(RenderSystemPlugin plugin)
	{
		static String DX11Name = "CamelotD3D11RenderSystem";
		static String DX9Name = "CamelotD3D9RenderSystem";
		static String OpenGLName = "CamelotGLRenderSystem";

		switch(plugin)
		{
		case RenderSystemPlugin::DX11:
			return DX11Name;
		case RenderSystemPlugin::DX9:
			return DX9Name;
		case RenderSystemPlugin::OpenGL:
			return OpenGLName;
		}

		return StringUtil::BLANK;
	}
}