/* VariousStuff_Plugin - for licensing and copyright see license.txt */

#include <IPluginBase.h>

#pragma once

/**
* @brief VariousStuff Plugin Namespace
*/
namespace VariousStuffPlugin
{
    /**
    * @brief plugin VariousStuff concrete interface
    */
    struct IPluginVariousStuff
    {
        /**
        * @brief Get Plugin base interface
        */
        virtual PluginManager::IPluginBase* GetBase() = 0;

        // TODO: Add your concrete interface declaration here
    };
};