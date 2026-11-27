#pragma once

#include <api/Shadow.inl>

struct CSDCast
{
    uint8_t padding[0x144];
};

struct CSDCastLink
{
   uint32_t ChildCastIndex;
   uint32_t SiblingCastIndex;
};

struct CSDSceneIndex
{
    xpointer<const char> pSceneName;
    uint32_t SceneIndex;  
};


struct CSDCastIndex
{
    xpointer<const char> pCastName;
    uint32_t CastNodeIndex;
    uint32_t CastIndex;
};


struct CSDCastNode
{
    uint32_t CastCount;
    xpointer<xpointer<CSDCast>> pCasts;
    uint32_t RootCastIndex;
    xpointer<CSDCastLink> pCastLinks;
};


struct CSDScene
{
    uint8_t padding[8];
    float FPS;       
    //uint32_t padding2[8];
    uint32_t padding2[6];
    uint32_t CastNodeCount;
    xpointer<CSDCastNode> pCastNodes; 
    uint32_t CastCount;      
    xpointer<CSDCastIndex> pCastIndices; 
  
};

struct CSDSceneNodeIndex
{
    xpointer<const char> pSceneNodeName; 
    uint32_t SceneNodeIndex;             
};

struct CSDSceneNode
{
    uint32_t SceneCount;      
    xpointer<xpointer<CSDScene>> pScenes;
    xpointer<CSDSceneIndex> pSceneIndices;
    uint32_t SceneNodeCount;  
    xpointer<CSDSceneNode> pSceneNodes;
    xpointer<CSDSceneNodeIndex> pSceneNodeIndices;
};
