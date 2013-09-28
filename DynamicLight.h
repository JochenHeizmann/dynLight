//
//  DynamicLight.h
//  pixelgame
//
//  Created by Jochen Heizmann on 30.07.13.
//
//

#ifndef __pixelgame__DynamicLight__
#define __pixelgame__DynamicLight__

#include "cocos2d.h"

#define DEBUG_DRAW 0

class DynamicLight : public cocos2d::CCNode
{
public:
    CREATE_FUNC(DynamicLight);

    DynamicLight();
    ~DynamicLight();

    bool init() override;
    void draw() override;
    void debugDraw();

    void initOcclusionMap();
    void initShadowMap1D();
    void initFinalShadowMap();
    void initBakedShadowMap();
    void invalidateBakedMap();
    void setPosition(const cocos2d::CCPoint &position) override;

    void updateShadowMap();
    void setSoftShadows(bool shadows);
    void setLightSize(int size);
    void setUpScale(float upScale);
    void setAccuracy(float accuracy);
    void setAdditive(bool additive);
    void setColor(cocos2d::ccColor4B c);
    
    void setShadowCasters(CCNode *casters);
    
    cocos2d::CCGLProgram* loadShader(const char* vertexShader, const char* fragmentShader);
    void updateUniforms();
    void createOcclusionMap();
    void createShadowMap();

    cocos2d::CCNode *shadowCasters;

    cocos2d::CCRenderTexture *occlusionMap;
    cocos2d::CCRenderTexture *shadowMap1D;
    cocos2d::CCRenderTexture *finalShadowMap;
    cocos2d::CCRenderTexture *bakedShadowMap;

    cocos2d::CCGLProgram *shadowMapShader;
    cocos2d::CCGLProgram *shadowRenderShader;

    int updateFrequency = 3;
    int updateCount = 0;

    bool bakedMapIsValid;
    bool dirty;
    cocos2d::ccColor4B color;

    bool softShadows;
    int lightSize;
    float upScale;
    float finalSize;
    float accuracy;
    bool additive;
};

#endif /* defined(__pixelgame__DynamicLight__) */
