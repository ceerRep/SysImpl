#ifndef _special_path_manager_hpp

#define _special_path_manager_hpp

#include "Object.hpp"

#include <shared_ptr.hpp>
#include <stdexcept.h>

class SpecialPathManager
{
    inline static SpecialPathManager *instance;

private:
    enum
    {
        MAX_SPECIAL_PATH_NUM = 16
    };
    struct device
    {
        char name[16];
        shared_ptr<Object> obj;
    } devices[MAX_SPECIAL_PATH_NUM];

    SpecialPathManager();

public:
    int registerObject(const char *path, shared_ptr<Object> obj);
    int removeObject(const char* path);
    shared_ptr<Object> get(const char* path);

    static SpecialPathManager *getInstance();
};

#endif
