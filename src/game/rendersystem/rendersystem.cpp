#include "convar.h"
#include "cdll_int.h"
#include "eiface.h"
#include "materialsystem/imaterialsystem.h"
#include "tier1/utlvector.h"
#include "tier1/utlstack.h"
#include "view_shared.h"
#include "ivrenderview.h"
#include "rendersystem/irendersystemrenderables.h"
#include "rendersystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CRenderSystem g_RenderSystem;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CRenderSystem, IRenderSystem, RENDERSYSTEM_INTERFACE_VERSION, g_RenderSystem);

bool CRenderSystem::Connect(CreateInterfaceFn factory)
{
	if (!BaseClass::Connect(factory))
		return false;

	if ((materials = (IMaterialSystem*)factory(MATERIAL_SYSTEM_INTERFACE_VERSION, NULL)) == NULL)
		return false;

	return true;
}

void CRenderSystem::Disconnect()
{
	return BaseClass::Disconnect();
}

void* CRenderSystem::QueryInterface(const char* pInterfaceName)
{
	// Loading the rendersystem DLL mounts *all* interfaces
	CreateInterfaceFn factory = Sys_GetFactoryThis();
	return factory(pInterfaceName, NULL);
}

InitReturnVal_t CRenderSystem::Init()
{
	return INIT_OK;
}

void CRenderSystem::Shutdown()
{
	return BaseClass::Shutdown();
}

void CRenderSystem::StartFrame()
{
	// prepare frame here or something idk
}

void CRenderSystem::Render(const CViewSetup& view, int nClearFlags, int whatToDraw)
{
	Push3DView(view);

	DrawRenderables();

	PopView();
}

void CRenderSystem::EndFrame()
{
	m_Renderables.Purge();
}

void CRenderSystem::DrawRenderables()
{
	FOR_EACH_VEC(m_Renderables, i)
	{
		m_Renderables[i]->Draw();
	}
}

void CRenderSystem::PushRenderable(IRenderSystem_Renderable* pRenderable)
{
	m_Renderables.AddToTail(pRenderable);
}

void CameraIdentityMatrix(VMatrix& Matrix)
{
	// This function produces a transform which transforms from
	// material system camera space to quake camera space

	// Camera right axis lies along the world X axis.
	Matrix[0][0] = 1;
	Matrix[0][1] = 0;
	Matrix[0][2] = 0;
	Matrix[0][3] = 0;

	// Camera up axis lies along the world Z axis.
	Matrix[1][0] = 0;
	Matrix[1][1] = 0;
	Matrix[1][2] = 1;
	Matrix[1][3] = 0;

	// Camera forward axis lies along the negative world Y axis.
	Matrix[2][0] = 0;
	Matrix[2][1] = -1;
	Matrix[2][2] = 0;
	Matrix[2][3] = 0;

	Matrix[3][0] = 0;
	Matrix[3][1] = 0;
	Matrix[3][2] = 0;
	Matrix[3][3] = 1;
}

void RotateAroundAxis(VMatrix& Matrix, float fDegrees, int nAxis)
{
	int a, b;

	if (fDegrees == 0)
		return;

	if (nAxis == 0)
	{
		a = 1; b = 2;
	}
	else if (nAxis == 1)
	{
		a = 0; b = 2;
	}
	else
	{
		a = 0; b = 1;
	}

	float fRadians = DEG2RAD(fDegrees);

	float fSin = (float)sin(fRadians);
	float fCos = (float)cos(fRadians);

	if (nAxis == 1)
		fSin = -fSin;

	float Temp0a = Matrix[0][a] * fCos + Matrix[0][b] * fSin;
	float Temp1a = Matrix[1][a] * fCos + Matrix[1][b] * fSin;
	float Temp2a = Matrix[2][a] * fCos + Matrix[2][b] * fSin;
	float Temp3a = Matrix[3][a] * fCos + Matrix[3][b] * fSin;

	if (nAxis == 1)
		fSin = -fSin;

	float Temp0b = Matrix[0][a] * -fSin + Matrix[0][b] * fCos;
	float Temp1b = Matrix[1][a] * -fSin + Matrix[1][b] * fCos;
	float Temp2b = Matrix[2][a] * -fSin + Matrix[2][b] * fCos;
	float Temp3b = Matrix[3][a] * -fSin + Matrix[3][b] * fCos;

	Matrix[0][a] = Temp0a;
	Matrix[1][a] = Temp1a;
	Matrix[2][a] = Temp2a;
	Matrix[3][a] = Temp3a;

	Matrix[0][b] = Temp0b;
	Matrix[1][b] = Temp1b;
	Matrix[2][b] = Temp2b;
	Matrix[3][b] = Temp3b;
}

void CRenderSystem::BuildViewMatrix(const CViewSetup& view, VMatrix& matview)
{
	// The camera transformation is produced by multiplying roll * yaw * pitch.
	// This will transform a point from world space into quake camera space, 
	// which is exactly what we want for our view matrix. However, quake
	// camera space isn't the same as material system camera space, so
	// we're going to have to apply a transformation that goes from quake
	// camera space to material system camera space.

	CameraIdentityMatrix(matview);

	RotateAroundAxis(matview, view.angles[PITCH], 0);
	RotateAroundAxis(matview, view.angles[ROLL], 1);
	RotateAroundAxis(matview, -view.angles[YAW] + 90, 2); // HACKHACK: this shouldn't be like this, right?

	// Translate the viewpoint to the world origin.
	VMatrix TempMatrix;
	TempMatrix.Identity();
	TempMatrix.SetTranslation(-view.origin);

	matview = matview * TempMatrix;
}

void CRenderSystem::BuildProjMatrix(const CViewSetup& view, VMatrix& matproj)
{
	memset(&matproj, 0, sizeof(matproj));
	VMatrix& m = matproj;

	if (view.m_bOrtho)
	{
		// same as D3DXMatrixOrthoRH
		float w = (float)view.width;
		float h = (float)view.height;

		m[0][0] = 2 / w;
		m[1][1] = 2 / h;

		m[2][2] = 1 / (view.zNear - view.zFar);
		m[2][3] = view.zNear / (view.zNear - view.zFar);

		m[3][3] = 1;
	}
	else
	{
		// same as D3DXMatrixPerspectiveRH
		float w = 2 * view.zNear * tan(view.fov * M_PI / 360.0);
		float h = (w * float(view.height)) / float(view.width);

		m[0][0] = 2 * view.zNear / w;
		m[1][1] = 2 * view.zNear / h;

		m[2][2] = view.zFar / (view.zNear - view.zFar);
		m[2][3] = (view.zNear * view.zFar) / (view.zNear - view.zFar);

		m[3][2] = -1;
	}
}

void CRenderSystem::Push3DView(const CViewSetup& view)
{
	m_ViewStack.Push(view);
	
	VMatrix mView, mProj;
	BuildViewMatrix(view, mView);
	BuildProjMatrix(view, mProj);

	CMatRenderContextPtr pRenderContext(materials);

	pRenderContext->MatrixMode(MATERIAL_VIEW);
	pRenderContext->PushMatrix();
	pRenderContext->LoadMatrix(mView);

	pRenderContext->MatrixMode(MATERIAL_PROJECTION);
	pRenderContext->PushMatrix();
	pRenderContext->LoadMatrix(mProj);

	pRenderContext->MatrixMode(MATERIAL_MODEL);
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();
}

void CRenderSystem::PopView()
{
	CMatRenderContextPtr pRenderContext(materials);

	pRenderContext->MatrixMode(MATERIAL_VIEW);
	pRenderContext->PopMatrix();

	pRenderContext->MatrixMode(MATERIAL_PROJECTION);
	pRenderContext->PopMatrix();

	pRenderContext->MatrixMode(MATERIAL_MODEL);
	pRenderContext->PopMatrix();

	m_ViewStack.Pop();
}
