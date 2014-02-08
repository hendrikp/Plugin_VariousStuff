#include "StdAfx.h"
#include <IPluginVariousStuff.h>
#include "CDynMatEntity.h"

CDynMatEntity::CDynMatEntity( void )
{
}

void CDynMatEntity::Release()
{
    delete this;
}

bool CDynMatEntity::Init( IGameObject* pGameObject )
{
    SetGameObject( pGameObject );

    return true;
}


void CDynMatEntity::Update( SEntityUpdateContext& ctx, int updateSlot )
{

}

void CDynMatEntity::ProcessEvent( SEntityEvent& entityEvent )
{

}
