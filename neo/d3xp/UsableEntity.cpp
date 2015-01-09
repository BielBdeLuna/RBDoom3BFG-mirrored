/*
 * UsableEntity.cpp
 *	the source code contained in this file is under the terms of the GPL v3 license
 *  Created on: 09/01/2015
 *      Author: biel
 */


#include "UsableEntity.h"

const idEventDef EV_Use( "use", "v");

/*
===============================================================================

	blUsableEntity

    idEntity whith interaction capabilities

===============================================================================
*/

//ABSTRACT_DECLARATION( idEntity, blUsableEntity )
CLASS_DECLARATION( idEntity, blUsableEntity )
	EVENT( EV_Use,				blUsableEntity::Event_Use )
    //EVENT( EV_Interact,         blUsableEntity::Event_Interact )
END_CLASS


/*
================
blUsableEntity::blUsableEntity
================
*/
blUsableEntity::blUsableEntity( void ) {
    activated = false;
    dest_angles.Zero();
	dest_position.Zero();
}

/*
================
blUsableEntity::Spawn
================
*/
void blUsableEntity::Spawn( void ) {
    idEntity	*ent;

    gameLocal.Printf( "IT SPAWNS!\n" );
    activated = false;

	//ent = this;
    dest_position = GetPhysics()->GetOrigin();
	dest_angles = GetPhysics()->GetAxis().ToAngles();

	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetClipMask( MASK_SOLID );

    //physicsObj.SetContents( 1 );
	//physicsObj.SetPusher( 1 );

	physicsObj.SetLinearExtrapolation( EXTRAPOLATION_NONE, 0, 0, dest_position, vec3_origin, vec3_origin );
	physicsObj.SetAngularExtrapolation( EXTRAPOLATION_NONE, 0, 0, dest_angles, ang_zero, ang_zero );
	SetPhysics( &physicsObj );



    fl.takedamage = false;
}

void blUsableEntity::Event_Use( void ) {
    gameLocal.Printf( "it works!\n" );
    activated = true;
    return;
}

void blUsableEntity::Event_Interact( void ) {
    gameLocal.Printf( "it works!\n" );
    activated = true;
    return;
}




