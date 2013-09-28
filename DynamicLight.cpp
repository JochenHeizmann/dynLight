//
//  DynamicLight.cpp
//  pixelgame
//
//  Created by Jochen Heizmann on 30.07.13.
//
//
#include "DynamicLight.h"

DynamicLight::DynamicLight() :
softShadows{true}, lightSize{256},
upScale{1.0f}, finalSize(lightSize * upScale),
dirty{false}, shadowRenderShader{nullptr},
shadowMapShader{nullptr}, occlusionMap{nullptr}, shadowMap1D{nullptr},
finalShadowMap{nullptr}, shadowCasters{nullptr}, bakedShadowMap{nullptr},
accuracy{1.0}, bakedMapIsValid{false}, additive{true}
{
}


DynamicLight::~DynamicLight()
{
    CC_SAFE_RELEASE(shadowRenderShader);
    CC_SAFE_RELEASE(shadowMapShader);
    CC_SAFE_RELEASE(occlusionMap);
    CC_SAFE_RELEASE(shadowMap1D);
    CC_SAFE_RELEASE(finalShadowMap);
    CC_SAFE_RELEASE(shadowCasters);
    CC_SAFE_RELEASE(bakedShadowMap);
}

bool DynamicLight::init()
{
    if (!CCNode::init())
        return false;

    shadowMapShader = this->loadShader("shader/pass.vsh", "shader/shadowMap.fsh");
    shadowRenderShader = this->loadShader("shader/pass.vsh", "shader/shadowRender.fsh");

    this->initOcclusionMap();
    this->initShadowMap1D();
    this->initFinalShadowMap();
    this->initBakedShadowMap();

    color = cocos2d::ccc4(255, 255, 255, 255);

    this->updateUniforms();

    return true;
}

void DynamicLight::initOcclusionMap()
{
    if (occlusionMap)
        occlusionMap->release();

    occlusionMap = cocos2d::CCRenderTexture::create(lightSize, lightSize);
    occlusionMap->retain();
}

void DynamicLight::initShadowMap1D()
{
    if (shadowMap1D)
        shadowMap1D->release();

    // seems like 16 pixel is the minimum height of a texture (on ios)
    shadowMap1D = cocos2d::CCRenderTexture::create(lightSize, 16);
    shadowMap1D->retain();
}

void DynamicLight::initFinalShadowMap()
{
    if (finalShadowMap)
        finalShadowMap->release();

    finalSize = lightSize * upScale;

    finalShadowMap = cocos2d::CCRenderTexture::create(finalSize, finalSize);
    finalShadowMap->retain();
}

void DynamicLight::initBakedShadowMap()
{
    if (bakedShadowMap)
        bakedShadowMap->release();

    finalSize = lightSize * upScale;

    bakedShadowMap = cocos2d::CCRenderTexture::create(finalSize, finalSize);
    bakedShadowMap->retain();
}

void DynamicLight::setShadowCasters(CCNode *casters)
{
    this->shadowCasters = casters;
    this->shadowCasters->retain();
}

void DynamicLight::updateShadowMap()
{
    this->createOcclusionMap();
    this->createShadowMap();
}

void DynamicLight::setPosition(const cocos2d::CCPoint &position)
{
    CCNode::setPosition(position);
    this->invalidateBakedMap();
}

void DynamicLight::draw()
{
    if (!this->bakedMapIsValid)
    {
        if (dirty)
        {
            this->updateUniforms();
            dirty = false;
        }

        this->updateUniforms();
        this->updateShadowMap();

        finalShadowMap->getSprite()->setColor(cocos2d::ccc3(color.r, color.g, color.b));
        finalShadowMap->getSprite()->setShaderProgram(shadowRenderShader);
        finalShadowMap->getSprite()->setAnchorPoint(ccp(0,0));
        finalShadowMap->getSprite()->setPosition(ccp(0,0));
        finalShadowMap->setScaleY(-1);

        bakedShadowMap->beginWithClear(0.0, 0.0, 0.0, 0.0);
        finalShadowMap->setAnchorPoint(ccp(0.5,0.5));
        finalShadowMap->setPosition(ccp(0,0));
        finalShadowMap->visit();
        bakedShadowMap->end();
        bakedShadowMap->setPosition(ccp(0,0));

        if (additive)
            bakedShadowMap->getSprite()->setBlendFunc({GL_SRC_ALPHA, GL_ONE});

        this->bakedMapIsValid = true;
    }
    bakedShadowMap->visit();

#if DEBUG_DRAW
    this->debugDraw();
#endif
}

void DynamicLight::invalidateBakedMap()
{
    ++updateCount;
    if (updateCount > updateFrequency)
    {
        updateCount = 0;
        this->bakedMapIsValid = false;
    }
}

void DynamicLight::debugDraw()
{
    //debug rendering
    occlusionMap->getSprite()->setColor(cocos2d::ccc3(255, 0, 0));
    occlusionMap->setAnchorPoint(ccp(0,0));
    occlusionMap->setPosition((ccp(800 - lightSize/2 - this->getPositionX(), 600 - lightSize/2 - this->getPositionY())));
    occlusionMap->visit();
    occlusionMap->getSprite()->setColor(cocos2d::ccc3(255,255,255));

    shadowMap1D->setAnchorPoint(ccp(0,0));
    shadowMap1D->setPosition((ccp(800 - lightSize/2 - this->getPositionX(), 600 - lightSize - 15 - this->getPositionY())));
    shadowMap1D->visit();
}

cocos2d::CCGLProgram* DynamicLight::loadShader(const char* vertexShader, const char* fragmentShader)
{
    cocos2d::CCGLProgram* shader = cocos2d::CCShaderCache::sharedShaderCache()->programForKey(fragmentShader);
    if (!shader)
    {
        shader = new cocos2d::CCGLProgram();
        shader->retain();
        shader->initWithVertexShaderFilename(vertexShader, fragmentShader);
        shader->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
        shader->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);
        shader->addAttribute(kCCAttributeNameColor, cocos2d::kCCVertexAttrib_Color);
        shader->link();
        shader->updateUniforms();
        shader->use();
    }

    return shader;
}

void DynamicLight::updateUniforms()
{
    shadowMapShader->use();
    GLint res = shadowMapShader->getUniformLocationForName("resolution");
    shadowMapShader->setUniformLocationWith2f(res, lightSize, lightSize);

    GLint glUpscale = shadowMapShader->getUniformLocationForName("upScale");
    shadowMapShader->setUniformLocationWith1f(glUpscale, upScale);

    GLint glAccuracy = shadowMapShader->getUniformLocationForName("accuracy");
    shadowMapShader->setUniformLocationWith1f(glAccuracy, accuracy);

    shadowRenderShader->use();
    GLint glResolution2 = shadowRenderShader->getUniformLocationForName("resolution");
    GLint glSoftShadows = shadowRenderShader->getUniformLocationForName("softShadows");

    shadowRenderShader->setUniformLocationWith2f(glResolution2, lightSize, lightSize);
    shadowRenderShader->setUniformLocationWith1f(glSoftShadows, softShadows ? 1.0f : 0.0f);

    glUniform1i(glGetUniformLocation(shadowRenderShader->getProgram(), "u_texture2"), 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap1D->getSprite()->getTexture()->getName());
    glActiveTexture(GL_TEXTURE0);
}

void DynamicLight::createOcclusionMap()
{
    if (!shadowCasters)
    {
        occlusionMap->beginWithClear(0.0f,0.0f,0.0f,0.0f);
        occlusionMap->end();
        return;
    }
    cocos2d::CCPoint p1 = shadowCasters->getAnchorPoint();
    cocos2d::CCPoint p2 = shadowCasters->getPosition();

    // Render light region to occluder FBO
    occlusionMap->beginWithClear(0.0f,0.0f,0.0f,0.0f);
    shadowCasters->setAnchorPoint(ccp(0,0));
    //ugly....
    shadowCasters->setPosition(ccp(-getPositionX() + lightSize/2 + shadowCasters->getPositionX(),-getPositionY() + lightSize/2 + shadowCasters->getPositionY()));
    shadowCasters->visit();
    occlusionMap->end();

    shadowCasters->setAnchorPoint(p1);
    shadowCasters->setPosition(p2);
}

void DynamicLight::createShadowMap()
{
    //STEP 2. build a 1D shadow map from occlude FBO
    occlusionMap->getSprite()->setShaderProgram(shadowMapShader);
    shadowMap1D->beginWithClear(0.f, 0.f, 0.f, 0.f);
    occlusionMap->setAnchorPoint(ccp(0.5,0.5));
    occlusionMap->setPosition(ccp(lightSize / 2.0, lightSize / 2.0));
    occlusionMap->visit();
    shadowMap1D->end();
//    occlusionMap->getSprite()->setShaderProgram(cocos2d::CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
}

void DynamicLight::setSoftShadows(bool shadows)
{
    if (softShadows != shadows)
    {
        dirty = true;
        softShadows = shadows;
    }
}

void DynamicLight::setLightSize(int size)
{
    if (lightSize != size)
    {
        dirty = true;
        lightSize = size;
    }
}

void DynamicLight::setUpScale(float upScale)
{
    if (this->upScale != upScale)
    {
        dirty = true;
        this->upScale = upScale;
    }
}

void DynamicLight::setAccuracy(float accuracy)
{
    if (this->accuracy != accuracy)
    {
        dirty = true;
        this->accuracy = accuracy;
    }
}

void DynamicLight::setAdditive(bool additive)
{
    this->additive = additive;
}

void DynamicLight::setColor(cocos2d::ccColor4B c)
{
    this->color = c;
}