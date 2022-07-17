#ifndef IFIREWORK_H
#define IFIREWORK_H

#define FIREWORK_INTERFACE_VERSION "IFireWorkRenderer_001"

class IMaterialSystem;
class IFireWorkRenderer_Renderable;

class IFireWorkRenderer : public IAppSystem
{
public:
	virtual void StartFrame() = 0;
	virtual void Render(const CViewSetup& view, int nClearFlags, int whatToDraw) = 0;
	virtual void EndFrame() = 0;
	
	virtual void PushRenderable(IFireWorkRenderer_Renderable* pRenderable) = 0;
	virtual const CViewSetup& GetViewSetup() = 0;
};
#endif // !IFIREWORK_H