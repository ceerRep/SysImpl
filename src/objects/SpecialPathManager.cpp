#include <objects/SpecialPathManager.hpp>

#include <errno.h>
#include <string.h>

SpecialPathManager::SpecialPathManager()
{
    for (int i = 0; i < MAX_SPECIAL_PATH_NUM; i++)
        devices[i].name[0] = 0;
}

int SpecialPathManager::registerObject(const char *path, shared_ptr<Object> obj)
{
    removeObject(path);

    for (int i = 0; i < MAX_SPECIAL_PATH_NUM; i++)
    {
        if (devices[i].name[0] == 0)
        {
            strlcpy(devices[i].name, path, 16);
            devices[i].obj = obj;

            return 0;
        }
    }

    return ENOSPC;
}

int SpecialPathManager::removeObject(const char* path)
{
    for (int i = 0; i < MAX_SPECIAL_PATH_NUM; i++)
    {
        if (strcasecmp(devices[i].name, path) == 0)
        {
            devices[i].name[0] = 0;
            devices[i].obj = nullptr;

            return 0;
        }
    }

    return ENOENT;
}

shared_ptr<Object> SpecialPathManager::get(const char* path)
{
    for (int i = 0; i < MAX_SPECIAL_PATH_NUM; i++)
    {
        if (strcasecmp(devices[i].name, path) == 0)
        {
            return devices[i].obj;
        }
    }

    return {};
}

SpecialPathManager *SpecialPathManager::getInstance()
{
    if (!instance)
        instance = new SpecialPathManager();

    return instance;
}
