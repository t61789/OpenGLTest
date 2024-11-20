#include "ResourceMgr.h"

std::vector<ResourceBase*> ResourceMgr::s_ptr;
std::unordered_map<std::string, ResourceInfo> ResourceMgr::s_pathMap;
std::unordered_map<RESOURCE_ID, ResourceInfo> ResourceMgr::s_resourceInfoMap;
