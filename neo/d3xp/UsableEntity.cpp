/*
 * UsableEntity.cpp
 *	the source code contained in this file is under the terms of the GPL v3 license
 *  Created on: 09/01/2015
 *      Author: biel
 */

/*
===============================================================================

	blUsable

    idEntity whit interaction capabilities
    you can set it with "powered_off" and with "start_on"
    it won't fire of it's targets unless is powered and turned on

===============================================================================
*/

CLASS_DECLARATION( idEntity, blUsable )
	EVENT( EV_Activate,				blUsable::Event_Activate )
END_CLASS


/*
================
blUsable::blUsable
================
*/
blUsable::blUsable( void ) {
    turned		= false;
    powered		= true;
    activated 	= false;
    //dest_angles.Zero();
	//dest_position.Zero();
}

/*
================
blUsable::Spawn
================
*/
void blUsable::Spawn( void ) {
	boolean powered_off;
	boolean	start_on;
    //idEntity	*ent;

    //gameLocal.Printf( "IT SPAWNS!\n" );
	spawnArgs.GetBool( "powered_off", "0", powered_off );
	spawnArgs.GetBool( "start_on", "0", start_on );
	if ( powered_off ) {
		powered = false;
	} else {
		powered = true;
	}

	if ( start_on ) {
		turned = true;
	} else {
		turned = false;
	}

	if ( powered && turned ) {
		activated = true;
	} else {
		activated = false;
	}

/*
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


*/
    fl.takedamage = false;
}

void blUsable::TestFireTargets( void ) {
	if ( powered && turned ) {
		activated = false;
	} else {
		activated = false;
	}

	if ( activated ) {
		gameLocal.Printf( "activated\n" );
	} else {
		gameLocal.Printf( "non activated\n" );
	}
}

void blUsable::Event_Activate( idEntity* activator )
{
	if ( powered ) {
		powered = false;
		gameLocal.DPrintf( "powered OFF\n" );
	} else {
		powered = true;
		gameLocal.DPrintf( "powered ON\n" );
	}

	TestFireTargets();
}

void blUsable::Interact( void ) {
	if ( turned ) {
		turned = false
		gameLocal.DPrintf( "turned OFF\n" );
	} else {
		turned = true;
		gameLocal.DPrintf( "turned ON\n" );
	}

	TestFireTargets();
}
