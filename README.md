dynLight
========

Dynamic Lightning for cocos2d-x

This is a simple cocos2d-x version of 2d pixel perfect shadows based on these [techniques].

### Note

Make sure that the shader files are in your resource path, so they can be found when the DynamicLight class tries to load them up.

On mobile devices this is very slow because the shaders are quite complex and multiple render passes are required. I'm sure this can be optimized with a little effort :)

### Usage

```c++
    light = DynamicLight::create();
    light->setColor(ccc4(64,130,77,255));
    light->setAccuracy(2.0);
    light->setSoftShadows(true);
    light->setUpScale(2.0);
    light->setLightSize(32);
    light->setAdditive(true);
    
    // this will set the shadow casters
    light->setShadowCasters(dynamic_cast<CCTMXTiledMap*>(this->getParent())->layerNamed("myMap"));

    light->initOcclusionMap();
    light->initShadowMap1D();
    light->initFinalShadowMap();
    light->initBakedShadowMap();
    
    [....]

    void MyClass::update(float fDelta)
    {
        light->setPosition(somePos);
        // setPosition will call invalidateBakedMap() and therefore
        // recalculate the shadows
    }
```

[here]: https://github.com/mattdesl/lwjgl-basics/wiki/2D-Pixel-Perfect-Shadows
