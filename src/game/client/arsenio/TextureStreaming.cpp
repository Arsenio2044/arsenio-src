#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "engine/ivmodelinfo.h"
#include "iclientrenderable.h"
#include "convar.h"
#include "materialsystem/imaterialproxy.h"
#include "toolframework_client.h"

// Define the maximum distance at which to switch to low render textures (adjust as needed)
const float MAX_TEXTURE_DISTANCE = 500.0f;

// Define ConVars
ConVar cl_texturestreaming_debug("cl_texturestreaming_debug", "0", FCVAR_ARCHIVE, "Enable texture streaming debug mode.");
ConVar cl_texturestreaming_lowres("cl_texturestreaming_lowres", "0", FCVAR_ARCHIVE, "Force textures to stay in low texture mode.");

// Function to check if a material has a low render texture
bool HasLowRenderTexture(IMaterial* material)
{
    if (material)
    {
        // Check if the material name contains "_lr"
        const char* materialName = material->GetName();
        if (Q_stristr(materialName, "_lr"))
        {
            return true;
        }
    }
    return false;
}

// Custom material proxy to handle texture streaming
class CTextureStreamingMaterialProxy : public IMaterialProxy
{
public:
    virtual bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
    {
        m_iBaseTextureVar = pMaterial->FindVar("$basetexture", nullptr);
        return m_iBaseTextureVar != nullptr;
    }

    virtual void OnBind(void* pRenderable) override
    {
        if (!m_iBaseTextureVar || cl_texturestreaming_debug.GetInt() || cl_texturestreaming_lowres.GetInt())
            return;

        IClientRenderable* pRenderableInterface = static_cast<IClientRenderable*>(pRenderable);
        if (!pRenderableInterface)
            return;

        // Get the current position of the player or camera
        Vector playerPosition; // Set this to the player's position
        Vector renderablePosition = pRenderableInterface->GetRenderOrigin();

        // Calculate the distance between the player and the renderable
        float distanceToPlayer = playerPosition.DistTo(renderablePosition);

        // Get the material of the renderable
        IMaterial* pMaterial = m_iBaseTextureVar->GetOwningMaterial();
        if (pMaterial)
        {
            if (HasLowRenderTexture(pMaterial) && distanceToPlayer > MAX_TEXTURE_DISTANCE)
            {
                // Replace the material with the low render material
                const char* materialName = pMaterial->GetName();
                char lowResMaterialName[256];
                Q_snprintf(lowResMaterialName, sizeof(lowResMaterialName), "%s_lr", materialName);

                IMaterial* pLowResMaterial = materials->FindMaterial(lowResMaterialName, TEXTURE_GROUP_MODEL);
                if (pLowResMaterial)
                {
                    m_iBaseTextureVar->SetTextureValue(pLowResMaterial->GetTextureValue());
                }
            }
        }
    }

    virtual void Release() override
    {
        delete this;
    }

private:
    IMaterialVar* m_iBaseTextureVar;
};

// Create the custom material proxy
EXPOSE_INTERFACE(CTextureStreamingMaterialProxy, IMaterialProxy, "TextureStreamingMaterialProxy" IMATERIAL_PROXY_INTERFACE_VERSION);

// Call this function during initialization to register the custom material proxy
void RegisterTextureStreamingMaterialProxy()
{
    static bool bRegistered = false;
    if (!bRegistered)
    {
        bRegistered = true;
        MaterialSystem()->AddMaterialProxy(CreateInterfaceFn(CreateTextureStreamingMaterialProxy), "TextureStreamingMaterialProxy");
    }
}

// Call this function for each renderable entity in your scene to apply texture streaming
void ApplyTextureStreamingToScene()
{
    // Iterate through all renderables in the scene
    int count = entitylist->GetHighestEntityIndex() + 1;
    for (int i = 0; i < count; i++)
    {
        IClientRenderable* renderable = entitylist->GetClientRenderable(i);
        if (renderable)
        {
            renderable->GetMaterial()->Refresh();
        }
    }
}
