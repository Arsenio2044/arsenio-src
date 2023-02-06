#ifndef CRENDERSYSTEM_H
#define CRENDERSYSTEM_H

#include "rendersystem/irendersystem.h"
#include "tier2/tier2.h"

class CRenderSystem : public CBaseAppSystem<IRenderSystem>
{
	typedef CBaseAppSystem< IRenderSystem > BaseClass;

	// Methods of IAppSystem 
public:
	virtual bool Connect(CreateInterfaceFn factory);
	virtual void Disconnect();
	virtual void* QueryInterface(const char* pInterfaceName);
	virtual InitReturnVal_t Init();
	virtual void Shutdown();
	
	virtual void StartFrame();
	virtual void Render(const CViewSetup& view, int nClearFlags, int whatToDraw);
	virtual void EndFrame();

	virtual void PushRenderable(IRenderSystem_Renderable* pRenderable);
#if !defined (CLIENT_DLL)
	virtual const CViewSetup& GetViewSetup() { return m_ViewStack.Top(); };
#endif
private:
	void DrawRenderables();

	void BuildViewMatrix(const CViewSetup& view, VMatrix& matview);
	void BuildProjMatrix(const CViewSetup& view, VMatrix &matproj);

	void Push3DView(const CViewSetup& view);
	void PopView();

#if !defined (CLIENT_DLL)
	CUtlVector<IRenderSystem_Renderable*> m_Renderables;
	CUtlStack<CViewSetup> m_ViewStack;
#endif
};


#endif // !CRENDERSYSTEM_H