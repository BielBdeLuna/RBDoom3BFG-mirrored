/*
 * staticParticles.cpp
 *
 *  Created on: 24/07/2016
 *      Author: biel
 */

#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

static const char* staticParticle_SnapshotName = "_StaticParticle_Snapshot_";

blStaticParticles::blStaticParticles() {
	Hidden = false;
	part = NULL;
	memset( &renderEntity, 0, sizeof( renderEntity ) );
	h_renderEntity = -1;

}

blStaticParticles::~blStaticParticles() {
	Shutdown();
}

void blStaticParticles::Init() {
	memset( &renderEntity, 0, sizeof( renderEntity ) );

	renderEntity.bounds.Clear();
	renderEntity.axis = mat3_identity;
	renderEntity.shaderParms[ SHADERPARM_RED ]		= 1;
	renderEntity.shaderParms[ SHADERPARM_GREEN ]	= 1;
	renderEntity.shaderParms[ SHADERPARM_BLUE ]		= 1;
	renderEntity.shaderParms[ SHADERPARM_ALPHA ] 	= 1;

	renderEntity.hModel = renderModelManager->AllocModel();
	renderEntity.hModel->InitEmpty( staticParticle_SnapshotName );

	// we certainly don't want particle shadows //we don't?
	renderEntity.noShadow = 1;

	// huge bounds, so it will be present in every world area //FIXME this is no longer relevant to the static particles, bounds should be as neded per particle effect
	renderEntity.bounds.AddPoint( idVec3( -100000, -100000, -100000 ) );
	renderEntity.bounds.AddPoint( idVec3( 100000,  100000,  100000 ) );

	//renderEntity.callback = idSmokeParticles::ModelCallback; //what is this? an update?
	// add to renderer list
	h_renderEntity = gameRenderWorld->AddEntityDef( &renderEntity );
}

void blStaticParticles::Shutdown() {
	ShowParticles();
	if( h_renderEntity != -1 ) {
		gameRenderWorld->FreeEntityDef( h_renderEntity );
		h_renderEntity = -1;
	}
	if( renderEntity.hModel != NULL ) {
		renderModelManager->FreeModel( renderEntity.hModel );
		renderEntity.hModel = NULL;
	}
}
bool blStaticParticles::EmitParticles( const idDeclParticle* part, const int systemStartTime, const float diversity,
		   	   	   	   	   	   	   	   const idVec3& origin, const idMat3& axis, int timeGroup /*_D3XP*/ ) {
	bool	continues = false;
	SetTimeState ts( timeGroup );

	if( !part )
	{
		return false;
	}

	if( !gameLocal.isNewFrame )
	{
		return false;
	}
	// dedicated doesn't smoke. No UpdateRenderEntity, so they would not be freed
	if( gameLocal.GetLocalClientNum() < 0 )
	{
		return false;
	}

	assert( gameLocal.time == 0 || systemStartTime <= gameLocal.time );
	if( systemStartTime > gameLocal.time )
	{
		return false;
	}

	idRandom steppingRandom( 0xffff * diversity );


}

void blStaticParticles::KillParticles() {
	Shutdown();
}
void blStaticParticles::ShowParticles() {
	if ( Hidden ) {
		Hidden = false;
		Show(); //TODO since those are not Entities they don't have a Show function nor any Hide function!
	}
}
void blStaticParticles::HideParticles() {
	if ( !Hidden ) {
		Hidden = true;
		Hide(); //TODO since those are not Entities they don't have a Show function nor any Hide function!
	}
}
