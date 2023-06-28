#ifndef IRENDERSYSTEM_H
#define IRENDERSYSTEM_H

#define RENDERSYSTEM_INTERFACE_VERSION "IRenderSystem_001"

class IMaterialSystem;
class IRenderSystem_Renderable;

class IRenderSystem : public IAppSystem
{
public:
	virtual void StartFrame() = 0;
	virtual void Render(const CViewSetup& view, int nClearFlags, int whatToDraw) = 0;
	virtual void EndFrame() = 0;
	
	virtual void PushRenderable(IRenderSystem_Renderable* pRenderable) = 0;
	virtual const CViewSetup& GetViewSetup() = 0;
};
#endif // !IRENDERSYSTEM_H