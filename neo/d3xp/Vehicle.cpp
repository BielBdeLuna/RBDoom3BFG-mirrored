/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
===============================================================================

  idAFEntity_Vehicle

===============================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

static const float BOUNCE_SOUND_MIN_VELOCITY	= 80.0f;
static const float BOUNCE_SOUND_MAX_VELOCITY	= 200.0f;


CLASS_DECLARATION( idAFEntity_Base, idAFEntity_Vehicle )
END_CLASS

/*
================
idAFEntity_Vehicle::idAFEntity_Vehicle
================
*/
idAFEntity_Vehicle::idAFEntity_Vehicle()
{
	player				= NULL;
	eyesJoint			= INVALID_JOINT;
	steeringWheelJoint	= INVALID_JOINT;
	wheelRadius			= 0.0f;
	steerAngle			= 0.0f;
	steerSpeed			= 0.0f;
	dustSmoke			= NULL;
	lightShaft			= NULL;
	headLights			= 0;
	engine				= -1;
	brakes				= 1.0f;
	driveable			= true;

	//vehicle physical capabilities
	veh_velocity            = 1000;
	veh_force               = 400; //50000
	veh_suspensionUp        = 32;
	veh_suspensionDown      = 20;
	veh_suspensionKCompress = 200;
	veh_suspensionDamping   = 400;
	veh_tireFriction        = 0.8;

	headlightsJoint		= INVALID_JOINT;
	headlightsEnd       = 0;
	headlightsHandle    = -1;
}

/*
================
idAFEntity_Vehicle::Spawn
================
*/
void idAFEntity_Vehicle::Spawn()
{
	const char* eyesJointName = spawnArgs.GetString( "eyesJoint", "eyes" );
	const char* steeringWheelJointName = spawnArgs.GetString( "steeringWheelJoint", "steeringWheel" );
	const char* headlightsName = spawnArgs.GetString( "headlightsJoint", "headlights" );
	const char* smokeName = spawnArgs.GetString( "smoke_dustsmoke", "muzzlesmoke" );
	const char* prtLightShaftName = spawnArgs.GetString( "prt_lightShaft", "light_fog.prt" );
	const char* trakDeclName = spawnArgs.GetString( "mtr_tiretrack", "" );

	LoadAF();

	SetCombatModel();

	SetPhysics( af.GetPhysics() );

	fl.takedamage = true;

	if( !eyesJointName[0] )
	{
		gameLocal.Error( "idAFEntity_Vehicle '%s' no eyes joint specified", name.c_str() );
	}
	eyesJoint = animator.GetJointHandle( eyesJointName );
	if( !steeringWheelJointName[0] )
	{
		gameLocal.Error( "idAFEntity_Vehicle '%s' no steering wheel joint specified", name.c_str() );
	}
	steeringWheelJoint = animator.GetJointHandle( steeringWheelJointName );
	if ( !headlightsName[0] ) {
		gameLocal.Error( "idAFEntity_Vehicle '%s' no headlights joint specified", name.c_str() );
	}
	headlightsJoint = animator.GetJointHandle( headlightsName );
	if ( !smokeName[0] ) {
		gameLocal.Warning( "idAFEntity_Vehicle '%s' no smoke name specified", name.c_str() );
	}
	dustSmoke = static_cast<const idDeclParticle*>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	if ( !prtLightShaftName[0] ) {
		gameLocal.Warning( "idAFEntity_Vehicle '%s' no lightShaft particle name specified", name.c_str() );
	}
	setupEmitterParticles( prtLightShaftName );
	/*
	if( *smokeName != '\0' )
	{
		dustSmoke = static_cast<const idDeclParticle*>( declManager->FindType( DECL_PARTICLE, smokeName ) );
	}
	*/
	if ( !trakDeclName[0] ) {
		gameLocal.Warning( "idAFEntity_Vehicle '%s' no track declaration specified", name.c_str() );
	}
	trackDecal = declManager->FindMaterial( spawnArgs.GetString( "mtr_tiretrack", "" ) );

	spawnArgs.GetFloat( "wheelRadius", "20", wheelRadius );
	spawnArgs.GetFloat( "steerSpeed", "5", steerSpeed );

    spawnArgs.GetFloat( "velocity", "1000", veh_velocity );
    spawnArgs.GetFloat( "force", "50000", veh_force );
    spawnArgs.GetFloat( "suspensionUp", "32", veh_suspensionUp );
    spawnArgs.GetFloat( "suspensionDown", "20", veh_suspensionDown );
    spawnArgs.GetFloat( "suspensionKCompress", "200", veh_suspensionKCompress );
    spawnArgs.GetFloat( "suspensionDamping", "400", veh_suspensionDamping );
    spawnArgs.GetFloat( "tireFriction", "0.95", veh_tireFriction );

    spawnArgs.GetInt( "engine", "-1", engine);
    spawnArgs.GetFloat( "brakes", "1.0", brakes );
    spawnArgs.GetBool( "driveable", "1", driveable );
    spawnArgs.GetBool( "hasBackAlarm", "1", hasBackAlarm );

	player 				= NULL;
	steerAngle 			= 0.0f;
	engine	   			= -1;
	backAlarm			= false;
	gasSound			= false;
	idleMotor			= false;
	klaxonEngaged		= false;

	const char *shader;

	headLights = 0;
	headlightsHandle = 0;

	const idMaterial*	headlightsShader;
	idVec3				headlightsTarget;
	idVec3				headlightsUp;
	idVec3				headlightsRight;
	float				headlightsRadius;
	bool				headlightsPointLight;

	spawnArgs.GetString( "mtr_headlights", "", &shader );
	headlightsShader = declManager->FindMaterial( shader, false );
	headlightsPointLight = spawnArgs.GetBool( "headlightPointLight", "1" );
	spawnArgs.GetVector( "headlightColor", "1 1 1", headlightsColor );
	headlightsRadius		= (float)spawnArgs.GetInt( "headlightRadius" );	// if 0, no light will spawn
	headlightsTime          = SEC2MS( spawnArgs.GetFloat( "headlightTime", "0.25" ) );
	headlightsTarget		= spawnArgs.GetVector( "headlightTarget" );
	headlightsUp			= spawnArgs.GetVector( "headlightUp" );
	headlightsRight         = spawnArgs.GetVector( "headlightRight" );

	memset( &l_headlights, 0, sizeof( l_headlights ) );

	l_headlights.pointLight								= headlightsPointLight;
	l_headlights.shader									= headlightsShader;
	l_headlights.shaderParms[ SHADERPARM_RED ]			= headlightsColor[0];
	l_headlights.shaderParms[ SHADERPARM_GREEN ]		= headlightsColor[1];
	l_headlights.shaderParms[ SHADERPARM_BLUE ]			= headlightsColor[2];
	l_headlights.shaderParms[ SHADERPARM_TIMESCALE ]	= 1.0f;

	l_headlights.lightRadius[0]							= headlightsRadius;
	l_headlights.lightRadius[1]							= headlightsRadius;
	l_headlights.lightRadius[2]							= headlightsRadius;

	if ( !headlightsPointLight ) {
		l_headlights.target								= headlightsTarget;
		l_headlights.up									= headlightsUp;
		l_headlights.right								= headlightsRight;
		l_headlights.end								= headlightsTarget;
	}

	af.GetPhysics()->SetComeToRest( false );
	af.GetPhysics()->Activate();
}

void idAFEntity_Vehicle::Save( idSaveGame *savefile ) const {

	bool vehicleHasPlayer;
	if ( player != NULL ) {
		vehicleHasPlayer = true;
		savefile->WriteBool( vehicleHasPlayer );
		player->Save( savefile );
	} else {
		vehicleHasPlayer = false;
		savefile->WriteBool( vehicleHasPlayer );
	}

	savefile->WriteJoint(eyesJoint);
	savefile->WriteJoint(steeringWheelJoint);

	savefile->WriteJoint(headlightsJoint);

	savefile->WriteInt(headLights);
	savefile->WriteVec3(headlightsColor);
	savefile->WriteInt(headlightsEnd);
	savefile->WriteInt(headlightsTime);
	savefile->WriteInt(headlightsHandle);
	savefile->WriteVec3(headlightsOrigin);
	savefile->WriteMat3(headlightsAxis);
	savefile->WriteRenderLight( l_headlights );

	savefile->WriteFloat(wheelRadius);
	savefile->WriteFloat(steerAngle);
	savefile->WriteFloat(steerSpeed);

	savefile->WriteInt(engine);
	savefile->WriteFloat(brakes);
	//savefile->WriteFloat(torque);
	//savefile->WriteFloat(forwardTorque);
	//savefile->WriteFloat(backwardTorque);
	//savefile->WriteFloat(maxTorque);

	savefile->WriteParticle(dustSmoke);
	savefile->WriteObject(lightShaft);
	savefile->WriteMaterial(trackDecal);

	savefile->WriteBool( gasSound );
	savefile->WriteBool( idleMotor );
	savefile->WriteBool( backAlarm );
	savefile->WriteBool( klaxonEngaged );
	savefile->WriteBool( hasBackAlarm );
	savefile->WriteBool( driveable );

	savefile->WriteFloat(veh_velocity);
	savefile->WriteFloat(veh_force);
	savefile->WriteFloat(veh_suspensionUp);
	savefile->WriteFloat(veh_suspensionDown);
	savefile->WriteFloat(veh_suspensionKCompress);
	savefile->WriteFloat(veh_suspensionDamping);
	savefile->WriteFloat(veh_tireFriction);
}

void idAFEntity_Vehicle::Restore( idRestoreGame *savefile ) {

	gameLocal.Printf( "Restoring vehicle %s now!\n", this->name.c_str() ); //debug

	//gameLocal.Printf( "Trying to restore the player if any... " ); //debug
	//bool vehicleHasPlayer;
	//savefile->ReadBool( vehicleHasPlayer );
	//if ( vehicleHasPlayer ) {
	//	player->Restore( savefile );
	//	gameLocal.Printf( "player has been restored!\n" ); //debug
	//} else {
		player = NULL;
	//	gameLocal.Printf( "player is null!\n" ); //debug
	//}

	savefile->ReadJoint(eyesJoint);
	savefile->ReadJoint(steeringWheelJoint);

	savefile->ReadJoint(headlightsJoint);

	savefile->ReadInt(headLights);
	savefile->ReadVec3(headlightsColor);
	savefile->ReadInt(headlightsEnd);
	savefile->ReadInt(headlightsTime);
	savefile->ReadVec3(headlightsOrigin);
	savefile->ReadMat3(headlightsAxis);
	savefile->ReadInt(headlightsHandle);
	savefile->ReadRenderLight(l_headlights);
	if ( headlightsHandle >= 0 ) {
		headlightsHandle = gameRenderWorld->AddLightDef( &l_headlights );
	}

	savefile->ReadFloat(wheelRadius);
	savefile->ReadFloat(steerAngle);
	savefile->ReadFloat(steerSpeed);
	savefile->ReadInt(engine);
	savefile->ReadFloat(brakes);
	//savefile->ReadFloat(torque);
	//savefile->ReadFloat(forwardTorque);
	//savefile->ReadFloat(backwardTorque);
	//savefile->ReadFloat(maxTorque);

	savefile->ReadParticle(dustSmoke);
	savefile->ReadObject( reinterpret_cast<idClass*&>( lightShaft ) );
	savefile->ReadMaterial(trackDecal);

	savefile->ReadBool( gasSound );
	savefile->ReadBool( idleMotor );
	savefile->ReadBool( backAlarm );
	savefile->ReadBool( klaxonEngaged );
	savefile->ReadBool( hasBackAlarm );
	savefile->ReadBool( driveable );

	savefile->ReadFloat(veh_velocity);
	savefile->ReadFloat(veh_force);
	savefile->ReadFloat(veh_suspensionUp);
	savefile->ReadFloat(veh_suspensionDown);
	savefile->ReadFloat(veh_suspensionKCompress);
	savefile->ReadFloat(veh_suspensionDamping);
	savefile->ReadFloat(veh_tireFriction);

	af.GetPhysics()->Activate();

	gameLocal.Printf( "Vehicle  %s restored!\n",  this->name.c_str() ); //debug
}

float idAFEntity_Vehicle::GetThirdPersonRange() {
	return spawnArgs.GetFloat( "thirdPersonRange", "150" );
}
float idAFEntity_Vehicle::GetThirdPersonAngle() {
	return spawnArgs.GetFloat( "thirdPersonAngle", "0" );
}
float idAFEntity_Vehicle::GetThirdPersonHeight() {
	return spawnArgs.GetFloat( "thirdPersonHeight", "0" );
}

/*
================
idAFEntity_Vehicle::Use
================
*/
void idAFEntity_Vehicle::Use( idPlayer* other )
{
	idVec3 origin;
	idMat3 axis;

	if( player )
	{
		if( player == other )
		{
			AbandonVehicle();
		} else {
			//another player tries to get to the car
			gameLocal.Printf( "another player is already using the car\n" );
		}
	}
	else
	{
		player = other;
		animator.GetJointTransform( eyesJoint, gameLocal.time, origin, axis );
		origin = renderEntity.origin + origin * renderEntity.axis;
		player->GetPhysics()->SetOrigin( origin );
		player->BindToBody( this, 0, true );
	}
}
void idAFEntity_Vehicle::AbandonVehicle() {
	if ( driveable ) {
		if ( hasBackAlarm && backAlarm ) {
			backAlarm = false;
			StopSound(SND_CHANNEL_DEMONIC, NULL); // stop backwards Alarm
		}
		if ( gasSound ) {
			StopSound(SND_CHANNEL_BODY, NULL);
			engine = 0;
			gasSound = false;
			idleMotor = true;
			StartSound( "snd_engine_idle", SND_CHANNEL_BODY, 0, false, NULL );
		}
	}

	player->Unbind();
	player = NULL;
	af.GetPhysics()->SetComeToRest( true );
}
void idAFEntity_Vehicle::ForceAbandonVehicle() {
	player->RetifyVehicle();
	AbandonVehicle();
}

void idAFEntity_Vehicle::ToggleBrakes() {
	if( brakes == 0.0f ) {
		//release it
		brakes = 1.0f;
	} else {
		//pull it
		brakes = 0.0f;
	}
}

void idAFEntity_Vehicle::ToggleHeadLights() {
	if ( headLights > -1 ) {
		if ( headLights == 1 ) {
			headLights = 0;
			gameLocal.Printf( "lights OFF\n" ); //debug
			StartSound( "snd_switch_off", SND_CHANNEL_ANY, 0, false, NULL );
			lightShaft->Event_Activate(NULL);
			lightShaft->Hide();

		} else {
			if ( headlightsJoint == INVALID_JOINT ) {
				return;
			}

			if ( !l_headlights.lightRadius[0] ) {
				return;
			}

			headLights = 1;
			gameLocal.Printf( "lights ON\n" ); //debug
			StartSound( "snd_switch_on", SND_CHANNEL_ANY, 0, false, NULL );

			lightShaft->Event_Activate(NULL);
			l_headlights.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
			l_headlights.shaderParms[ SHADERPARM_DIVERSITY ] = renderEntity.shaderParms[ SHADERPARM_DIVERSITY ];

			headlightsEnd = gameLocal.time + headlightsTime;

			if ( headlightsHandle != -1 ) {
				gameRenderWorld->UpdateLightDef( headlightsHandle, &l_headlights );
			} else {
				headlightsHandle = gameRenderWorld->AddLightDef( &l_headlights );
			}
		}
	} else if ( headLights == -2 ) { //keep playing the sounds of the switch but don't turn on the lights
		headLights = -1;
		StartSound( "snd_switch_off", SND_CHANNEL_ANY, 0, false, NULL );
	} else {
		headLights = -2;
		StartSound( "snd_switch_on", SND_CHANNEL_ANY, 0, false, NULL );
	}
}

void idAFEntity_Vehicle::ToggleEngine() {
	if ( engine >= 0 ) {
		engine = -1;
		//gameLocal.Printf( "engine OFF\n" ); //debug
		StopSound(SND_CHANNEL_BODY, NULL);
		StartSound( "snd_stop_engine", SND_CHANNEL_VOICE, 0, false, NULL );
	} else {
		engine = 0;
		if( brakes == 0.0f ) {
			ToggleBrakes();
		}
		//gameLocal.Printf( "engine ON\n" ); //debug
		StartSound( "snd_start_engine", SND_CHANNEL_VOICE, 0, false, NULL );
		gasSound = false;
		idleMotor = true;
		StartSound( "snd_engine_idle", SND_CHANNEL_BODY, 0, false, NULL );
	}
}

bool idAFEntity_Vehicle::TestDriveable() {
	if ( driveable ) {
		//do more checks
		return true;
	} else {
		if ( player ) {
			ForceAbandonVehicle();
		}
		return false;
	}
}
/*
================
idAFEntity_Base::Collide
================
*/
bool idAFEntity_Vehicle::Collide( const trace_t& collision, const idVec3& velocity )
{
	float v, f;

	idEntity* other = gameLocal.entities[ collision.c.entityNum ];

	if( af.IsActive() )	{
		if ( other ) {
			if( other->fl.takedamage ) {
				if ( player ) {
					other->Damage( this, player, collision.c.normal, "damage_roadkill", 1.f, CLIPMODEL_ID_TO_JOINT_HANDLE( collision.c.id ) );
				} else {
					other->Damage( this, NULL, collision.c.normal, "damage_roadkill", 1.f, CLIPMODEL_ID_TO_JOINT_HANDLE( collision.c.id ) );
				}
				StartSound( "snd_roadkill", SND_CHANNEL_ANY, 0, false, NULL );
			}
		}

		v = -( velocity * collision.c.normal );
		if( v > BOUNCE_SOUND_MIN_VELOCITY && gameLocal.time > nextSoundTime ) {
			f = v > BOUNCE_SOUND_MAX_VELOCITY ? 1.0f : idMath::Sqrt( v - BOUNCE_SOUND_MIN_VELOCITY ) * ( 1.0f / idMath::Sqrt( BOUNCE_SOUND_MAX_VELOCITY - BOUNCE_SOUND_MIN_VELOCITY ) );
			//SetSoundVolume( f );
			gameLocal.Printf( "force f: %f\n", f ); //debug
			if ( f >= 0.75 ) {
				StartSound( "snd_hard_bounce", SND_CHANNEL_ANY, 0, false, NULL );
			} else {
				StartSound( "snd_bounce", SND_CHANNEL_ANY, 0, false, NULL );
			}
			if ( player ) {
				const float new_damage = 0.6f * f;
				player->Damage( this, this, collision.c.normal, "damage_banging", new_damage, NULL );
			}
			nextSoundTime = gameLocal.time + 500;
		}
	}

	return false;
}
/*
=====================
idPlayer::Collide
=====================
*//*
bool idAFEntity_Vehicle::Collide( const trace_t& collision, const idVec3& velocity )
{
	idEntity* other;

	if( common->IsClient() && spectating == false )
	{
		return false;
	}

	other = gameLocal.entities[ collision.c.entityNum ];
	if( other )
	{
		other->Signal( SIG_TOUCH );
		if( !spectating )
		{
			if( other->RespondsTo( EV_Touch ) )
			{
				other->ProcessEvent( &EV_Touch, this, &collision );
			}
		}
		else
		{
			if( other->RespondsTo( EV_SpectatorTouch ) )
			{
				other->ProcessEvent( &EV_SpectatorTouch, this, &collision );
			}
		}
	}
	return false;
}*/

/*
=====================
idAFEntity_Gibbable::Collide
=====================
*//*
bool idAFEntity_Vehicle::Collide( const trace_t& collision, const idVec3& velocity )
{

	if( !gibbed && wasThrown )
	{

		// Everything gibs (if possible)
		if( spawnArgs.GetBool( "gib" ) )
		{
			idEntity*	ent;

			ent = gameLocal.entities[ collision.c.entityNum ];
			if( ent->fl.takedamage )
			{
				ent->Damage( this, gameLocal.GetLocalPlayer(), collision.c.normal, "damage_thrown_ragdoll", 1.f, CLIPMODEL_ID_TO_JOINT_HANDLE( collision.c.id ) );
			}

			idVec3 vel = velocity;
			vel.NormalizeFast();
			Gib( vel, "damage_gib" );
		}
	}

	return idAFEntity_Base::Collide( collision, velocity );
}*/

/*
================
idAFEntity_Vehicle::GetSteerAngle
================
*/
float idAFEntity_Vehicle::GetSteerAngle()
{
	float idealSteerAngle, angleDelta;

	idealSteerAngle = player->usercmd.rightmove * ( 30.0f / 128.0f );
	angleDelta = idealSteerAngle - steerAngle;

	if( angleDelta > steerSpeed )
	{
		steerAngle += steerSpeed;
	}
	else if( angleDelta < -steerSpeed )
	{
		steerAngle -= steerSpeed;
	}
	else
	{
		steerAngle = idealSteerAngle;
	}

	return steerAngle;
}

/*
==============
idAFEntity_Vehicle::TireTrack
==============
*/
void idAFEntity_Vehicle::TireTrack( const idVec3 &origin, float angle, const idMaterial* material ) {
	float s, c;
	idVec3	angles, target;
	idMat3 axis, axistemp;
	idFixedWinding winding;
	idVec3 windingOrigin, projectionOrigin;

	float halfSize 	= 12.0f;
	float depth 	= 48.0f;

	idVec3 verts[] = {	idVec3( 0.0f, +halfSize, +halfSize ),
						idVec3( 0.0f, +halfSize, -halfSize ),
						idVec3( 0.0f, -halfSize, -halfSize ),
						idVec3( 0.0f, -halfSize, +halfSize ) };
	idTraceModel trm;
	idClipModel mdl;
	trace_t results;
	idVec3 dir = idVec3(  0.0f, 0.0f, -1.0f );	//renderEntity.axis.ToAngles().ToUp() * -1.0f;
	//gameLocal.Printf( "DIR VEC: %s\n", dir.ToString() );
	trm.SetupPolygon( verts, 4 );
	mdl.LoadModel( trm );
	target = origin + dir * 2.0;
	gameLocal.clip.Translation( results, origin, target, &mdl, mat3_identity, CONTENTS_SOLID, NULL );
	idVec3 decpos = results.endpos;

	static idVec3 decalWinding[4] = {
		idVec3(  1.0f,  1.0f, 0.0f ),
		idVec3( -1.0f,  1.0f, 0.0f ),
		idVec3( -1.0f, -1.0f, 0.0f ),
		idVec3(  1.0f, -1.0f, 0.0f )
	};

	// rotate the decal winding
	//angles = renderEntity.axis.ToAngles().ToForward();	// buggy angle
	//angles.Normalize();

	idMath::SinCos16( angle, s, c );

	// winding orientation
	axis[2] = dir;
	axis[2].Normalize();
	axis[2].NormalVectors( axistemp[0], axistemp[1] );
	axis[0] = axistemp[ 0 ] * c + axistemp[ 1 ] * -s;
	axis[1] = axistemp[ 0 ] * -s + axistemp[ 1 ] * -c;

	windingOrigin 		= decpos + depth * axis[2];
	projectionOrigin 	= decpos - depth * axis[2];

	winding.Clear();
	winding += idVec5( windingOrigin + ( axis * decalWinding[0] ) * halfSize, idVec2( 1, 1 ) );
	winding += idVec5( windingOrigin + ( axis * decalWinding[1] ) * halfSize, idVec2( 0, 1 ) );
	winding += idVec5( windingOrigin + ( axis * decalWinding[2] ) * halfSize, idVec2( 0, 0 ) );
	winding += idVec5( windingOrigin + ( axis * decalWinding[3] ) * halfSize, idVec2( 1, 0 ) );
	gameRenderWorld->ProjectDecalOntoWorld( winding, projectionOrigin, true, 48.0f, material, gameLocal.time );
}
/*
================
idAFEntity_Vehicle::GetGlobalJointTransform

This returns the offset and axis of a bone in world space, suitable for attaching models or lights
================
*/
bool idAFEntity_Vehicle::GetGlobalJointTransform( const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis ) {
    if ( animator.GetJointTransform( jointHandle, gameLocal.time, offset, axis ) ) {
			offset = offset * af.GetPhysics()->GetAxis() + af.GetPhysics()->GetOrigin();
			axis = axis * af.GetPhysics()->GetAxis();
			return true;
	}
	offset = af.GetPhysics()->GetOrigin();
	axis = af.GetPhysics()->GetAxis();
	return false;
}
/*
================
idAFEntity_Vehicle::UpdatelightPositions
================
*/
void idAFEntity_Vehicle::UpdatelightPositions() {
	// the flash has an explicit joint for locating it
	GetGlobalJointTransform( headlightsJoint, l_headlights.origin, l_headlights.axis );

	//light models
	updateEmitterPos();

    /*
	// if the desired point is inside or very close to a wall, back it up until it is clear
	idVec3	start = r_headlights.origin - vehicleOrigin[0] * 16;
	idVec3	end = r_headlights.origin + vehicleAxis[0] * 8;
	trace_t	tr;
	gameLocal.clip.TracePoint( tr, start, end, MASK_SHOT_RENDERMODEL, car );
	// be at least 8 units away from a solid
	r_headlights.origin = tr.endpos - vehicleAxis[0] * 8;
    */
}
/*
================
idAFEntity_Vehicle::setupEmitterParticles
================
*/
void idAFEntity_Vehicle::setupEmitterParticles( const char* lightParticles ){
	idDict args;

	const idDeclEntityDef* emitterDef = gameLocal.FindEntityDef( "func_emitter", false );
	args = emitterDef->dict;
	args.Set( "model", lightParticles );
	args.SetBool( "start_off", true );

	idEntity* ent;
	gameLocal.SpawnEntityDef( args, &ent, false );
	lightShaft = ( idFuncEmitter* )ent;

	if( lightShaft != NULL )
	{
		lightShaft->BecomeActive( TH_THINK );
	} else {
		gameLocal.Warning( "idAFEntity_Vehicle '%s' no lightShaft particle emitter created", name.c_str() );
	}
}

/*
================
idAFEntity_Vehicle::updateEmitterPos
================
*/
void idAFEntity_Vehicle::updateEmitterPos() {
	//Manually update the position of the emitter so it follows the weapon
	renderEntity_t* rendEnt = lightShaft->GetRenderEntity();
	GetGlobalJointTransform( headlightsJoint, rendEnt->origin, rendEnt->axis );

	if( lightShaft->GetModelDefHandle() != -1 )
	{
		gameRenderWorld->UpdateEntityDef( lightShaft->GetModelDefHandle(), rendEnt );
	}
}

/*
===============================================================================

  idAFEntity_VehicleSimple

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Vehicle, idAFEntity_VehicleSimple )
END_CLASS

/*
================
idAFEntity_VehicleSimple::idAFEntity_VehicleSimple
================
*/
idAFEntity_VehicleSimple::idAFEntity_VehicleSimple()
{
	wheelModel = NULL;
	int i;
	for( i = 0; i < 4; i++ )
	{
		suspension[i] = NULL;
	}
}

/*
================
idAFEntity_VehicleSimple::~idAFEntity_VehicleSimple
================
*/
idAFEntity_VehicleSimple::~idAFEntity_VehicleSimple()
{
	delete wheelModel;
	wheelModel = NULL;
}

/*
================
idAFEntity_VehicleSimple::Spawn
================
*/
void idAFEntity_VehicleSimple::Spawn()
{
	static const char* wheelJointKeys[] =
	{
		"wheelJointFrontLeft",
		"wheelJointFrontRight",
		"wheelJointRearLeft",
		"wheelJointRearRight"
	};
	static idVec3 wheelPoly[4] = { idVec3( 2, 2, 0 ), idVec3( 2, -2, 0 ), idVec3( -2, -2, 0 ), idVec3( -2, 2, 0 ) };

	int i;
	idVec3 origin;
	idMat3 axis;
	idTraceModel trm;

	trm.SetupPolygon( wheelPoly, 4 );
	trm.Translate( idVec3( 0, 0, -wheelRadius ) );
	wheelModel = new( TAG_PHYSICS_CLIP_AF ) idClipModel( trm );

	for( i = 0; i < 4; i++ )
	{
		const char* wheelJointName = spawnArgs.GetString( wheelJointKeys[i], "" );
		if( !wheelJointName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleSimple '%s' no '%s' specified", name.c_str(), wheelJointKeys[i] );
		}
		wheelJoints[i] = animator.GetJointHandle( wheelJointName );
		if( wheelJoints[i] == INVALID_JOINT )
		{
			gameLocal.Error( "idAFEntity_VehicleSimple '%s' can't find wheel joint '%s'", name.c_str(), wheelJointName );
		}

		GetAnimator()->GetJointTransform( wheelJoints[i], 0, origin, axis );
		origin = renderEntity.origin + origin * renderEntity.axis;

		suspension[i] = new( TAG_PHYSICS_AF ) idAFConstraint_Suspension();
		suspension[i]->Setup( va( "suspension%d", i ), af.GetPhysics()->GetBody( 0 ), origin, af.GetPhysics()->GetAxis( 0 ), wheelModel );
		suspension[i]->SetSuspension(	veh_suspensionUp,
										veh_suspensionDown,
										veh_suspensionKCompress,
										veh_suspensionDamping,
										veh_tireFriction );

		af.GetPhysics()->AddConstraint( suspension[i] );
	}

	memset( wheelAngles, 0, sizeof( wheelAngles ) );
	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_VehicleSimple::Think
================
*/
void idAFEntity_VehicleSimple::Think()
{
	int i;
	float force = 0.0f, velocity = 0.0f, steerAngle = 0.0f, directionalForce = 0.0f;
	idVec3 origin;
	idMat3 axis;
	idRotation wheelRotation, steerRotation;
	float tempAngles[4] = { 0.0f };

	float ang;
	idAngles angle;

	if( thinkFlags & TH_THINK )
	{
		velocity = veh_velocity;

		if( player ) {
			steerAngle = GetSteerAngle();

			if ( ( player->usercmd.forwardmove < 0 ) && ( engine  >= 0 ) ) {
				velocity = -velocity;
				if ( (engine == 1 ) && !backAlarm && hasBackAlarm ) {
					//since this sound loops I use this in order to start it only once
					backAlarm = true;
					StartSound( "snd_alarm", SND_CHANNEL_DEMONIC, 0, false, NULL );
				}
			} else if ( hasBackAlarm ) {
				backAlarm = false;
				StopSound(SND_CHANNEL_DEMONIC, NULL); // stop backwards Alarm

			}
			//TODO RESTORE usercmd.upmov
			//if ( player->usercmd.upmove > 0 ) {
			if ( ( player->usercmd.buttons & BUTTON_JUMP ) ? 127 : 0 ) {
				if ( brakes == 1.0f ) {
					brakes = 0.0f;
					StartSound( "snd_brakes", SND_CHANNEL_VOICE, 0, false, NULL );
				}
			} else {
				brakes = 1.0f;
			}
			//if ( player->usercmd.upmove < 0 ) {
			if ( ( player->usercmd.buttons & BUTTON_CROUCH ) ? 127 : 0 ) {
				if ( !klaxonEngaged ) {
					klaxonEngaged = true;
					StartSound( "snd_klaxon_loop", SND_CHANNEL_VOICE, 0, false, NULL );
				}
			} else {
				if ( klaxonEngaged ) {
					klaxonEngaged = false;
					StopSound(SND_CHANNEL_VOICE, NULL);
				}
			}


			if ( ( player->usercmd.forwardmove > 0 ) &&  ( player->usercmd.forwardmove < 0 ) ) {
				if ( engine != 0 ) {
					engine = 0; // engine on but no gas
				}
			} else if ( ( player->usercmd.forwardmove > 0 ) ||  ( player->usercmd.forwardmove < 0 ) ) {
				if( engine  >= 0 ) {
					engine  = 1; // gas

				}
			} else {
				if ( engine == 1 ) {
					engine = 0;
				}
			}

			if ( engine == 1 ) {
				force = idMath::Fabs( player->usercmd.forwardmove * veh_force ) * ( 1.0f / 128.0f ) * brakes;

				if ( !gasSound ) {
					gasSound = true;
					if ( idleMotor ) {
						idleMotor = false;
						StopSound(SND_CHANNEL_BODY, NULL); // stop the idle sound
					}
					StartSound( "snd_engine_gas", SND_CHANNEL_BODY, 0, false, NULL );
				}
			} else if ( engine == 0 ) {
				force = 0.0f;

				if ( !idleMotor ) {
					idleMotor = true;
					if ( gasSound ) {
						gasSound = false;
						StopSound(SND_CHANNEL_BODY, NULL); // stop the gas sound
					}
					StartSound( "snd_engine_idle", SND_CHANNEL_BODY, 0, false, NULL );
				}
			} else {
				force = 0.0f;

				if ( idleMotor || gasSound ) {
					idleMotor = false;
					gasSound = false;
					StopSound(SND_CHANNEL_BODY, NULL); // stop any motor sound
				}
			}
		} else {
			force = 0.0f;

			if ( engine == 1 ) {
				engine = 0;
				if ( !idleMotor ) {
					idleMotor = true;
					if ( gasSound ) {
						gasSound = false;
						StopSound(SND_CHANNEL_BODY, NULL); // stop the gas sound
					}
					StartSound( "snd_engine_idle", SND_CHANNEL_BODY, 0, false, NULL );
				}
			}

			if ( hasBackAlarm ) {
				backAlarm = false;
				StopSound(SND_CHANNEL_DEMONIC, NULL); // stop backwards Alarm
			}

			if ( klaxonEngaged ) {
				klaxonEngaged = false;
				StopSound(SND_CHANNEL_VOICE, NULL);
			}

		}

		// update the wheel motor force and steering
		for( i = 0; i < 2; i++ )
		{

			// front wheel drive
			if( velocity != 0.0f )
			{
				suspension[i]->EnableMotor( true );
			}
			else
			{
				suspension[i]->EnableMotor( false );
			}
			suspension[i]->SetMotorVelocity( velocity );
			suspension[i]->SetMotorForce( force );

			// update the wheel steering
			suspension[i]->SetSteerAngle( steerAngle );
		}

		// adjust wheel velocity for better steering because there are no differentials between the wheels
		if( steerAngle < 0.0f )
		{
			suspension[0]->SetMotorVelocity( velocity * 0.5f );
		}
		else if( steerAngle > 0.0f )
		{
			suspension[1]->SetMotorVelocity( velocity * 0.5f );
		}

		// update suspension with latest cvar settings
		for( i = 0; i < 4; i++ )
		{
			suspension[i]->SetSuspension(	veh_suspensionUp,
											veh_suspensionDown,
											veh_suspensionKCompress,
											veh_suspensionDamping,
											veh_tireFriction );
		}

		// run the physics
		RunPhysics();

		// move and rotate the wheels visually
		for( i = 0; i < 4; i++ )
		{
			idAFBody* body = af.GetPhysics()->GetBody( 0 );

			origin = suspension[i]->GetWheelOrigin();
			velocity = body->GetPointVelocity( origin ) * body->GetWorldAxis()[0];
			wheelAngles[i] += velocity * MS2SEC( gameLocal.time - gameLocal.previousTime ) / wheelRadius;

			// additional rotation about the wheel axis
			wheelRotation.SetAngle( RAD2DEG( wheelAngles[i] ) );
			wheelRotation.SetVec( 0, -1, 0 );

			if( i < 2 )
			{
				// rotate the wheel for steering
				steerRotation.SetAngle( steerAngle );
				steerRotation.SetVec( 0, 0, 1 );
				// set wheel rotation
				animator.SetJointAxis( wheelJoints[i], JOINTMOD_WORLD, wheelRotation.ToMat3() * steerRotation.ToMat3() );
			}
			else
			{
				// set wheel rotation
				animator.SetJointAxis( wheelJoints[i], JOINTMOD_WORLD, wheelRotation.ToMat3() );
			}

			// set wheel position for suspension
			origin = ( origin - renderEntity.origin ) * renderEntity.axis.Transpose();
			GetAnimator()->SetJointPos( wheelJoints[i], JOINTMOD_WORLD_OVERRIDE, origin );

			// Place tire tracks
			float offset = velocity;
			if ( offset < 0.0f )
			{
				offset *= -1.0f;
			}

			//if ( i < 2 ) {
			float diff = 1.4f - offset / 1000.0f;	// adjust decal placement to speed
			// adjust decal placement for differential while turning
			float maxSteerAngle = 30.0f;
			offset = ( steerAngle / maxSteerAngle ) * 0.17f;
			if ( steerAngle > 0.0f )
			{
				diff -= offset;
			}
			if ( steerAngle < 0.0f )
			{
				diff += offset;
			}
			if ( wheelAngles[ i ] - tempAngles[ i ] >= diff || tempAngles[ i ] - wheelAngles[ i ] >= diff )
			{
				tempAngles[ i ] = wheelAngles[ i ];
				origin = suspension[ i ]->GetWheelOrigin();
				idVec3 vecRight = renderEntity.axis.ToAngles().ToRight();
				if ( i == 0 || i == 2 )
				{
					origin += vecRight * 5;
				}
				else
				{
					origin -= vecRight * 5;
				}
				angle = renderEntity.axis.ToAngles();
				ang = DEG2RAD( angle[ 1 ] * -1.0f );
				if ( i < 2 )
				{
					ang += DEG2RAD( steerAngle );
				}
				TireTrack( origin, ang, trackDecal );
				if ( force != 0.0f && !( gameLocal.framenum & 7 ) ) {
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), suspension[ i ]->GetWheelOrigin(), renderEntity.axis, timeGroup /*_D3XP*/ );
				}
			}
		}
		/*
				// spawn dust particle effects
				if ( force != 0.0f && !( gameLocal.framenum & 7 ) ) {
					int numContacts;
					idAFConstraint_Contact *contacts[2];
					for ( i = 0; i < 4; i++ ) {
						numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[i]->GetClipModel()->GetId(), contacts, 2 );
						for ( int j = 0; j < numContacts; j++ ) {
							gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[j]->GetContact().point, contacts[j]->GetContact().normal.ToMat3() );
						}
					}
				}
		*/
	}

	UpdateAnimation();
	if( thinkFlags & TH_UPDATEVISUALS )
	{
		Present();
		LinkCombat();
	}

	UpdatelightPositions();
	if ( headlightsHandle != -1 ) {
		gameRenderWorld->UpdateLightDef( headlightsHandle, &l_headlights );
	}
}


/*
===============================================================================

  idAFEntity_VehicleFourWheels

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Vehicle, idAFEntity_VehicleFourWheels )
END_CLASS


/*
================
idAFEntity_VehicleFourWheels::idAFEntity_VehicleFourWheels
================
*/
idAFEntity_VehicleFourWheels::idAFEntity_VehicleFourWheels()
{
	int i;

	for( i = 0; i < 4; i++ )
	{
		wheels[i]		= NULL;
		wheelJoints[i]	= INVALID_JOINT;
		wheelAngles[i]	= 0.0f;
	}
	steering[0]			= NULL;
	steering[1]			= NULL;
}

/*
================
idAFEntity_VehicleFourWheels::Spawn
================
*/
void idAFEntity_VehicleFourWheels::Spawn()
{
	int i;
	static const char* wheelBodyKeys[] =
	{
		"wheelBodyFrontLeft",
		"wheelBodyFrontRight",
		"wheelBodyRearLeft",
		"wheelBodyRearRight"
	};
	static const char* wheelJointKeys[] =
	{
		"wheelJointFrontLeft",
		"wheelJointFrontRight",
		"wheelJointRearLeft",
		"wheelJointRearRight"
	};
	static const char* steeringHingeKeys[] =
	{
		"steeringHingeFrontLeft",
		"steeringHingeFrontRight",
	};

	const char* wheelBodyName, *wheelJointName, *steeringHingeName;

	for( i = 0; i < 4; i++ )
	{
		wheelBodyName = spawnArgs.GetString( wheelBodyKeys[i], "" );
		if( !wheelBodyName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' no '%s' specified", name.c_str(), wheelBodyKeys[i] );
		}
		wheels[i] = af.GetPhysics()->GetBody( wheelBodyName );
		if( !wheels[i] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' can't find wheel body '%s'", name.c_str(), wheelBodyName );
		}
		wheelJointName = spawnArgs.GetString( wheelJointKeys[i], "" );
		if( !wheelJointName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' no '%s' specified", name.c_str(), wheelJointKeys[i] );
		}
		wheelJoints[i] = animator.GetJointHandle( wheelJointName );
		if( wheelJoints[i] == INVALID_JOINT )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' can't find wheel joint '%s'", name.c_str(), wheelJointName );
		}
	}

	for( i = 0; i < 2; i++ )
	{
		steeringHingeName = spawnArgs.GetString( steeringHingeKeys[i], "" );
		if( !steeringHingeName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' no '%s' specified", name.c_str(), steeringHingeKeys[i] );
		}
		steering[i] = static_cast<idAFConstraint_Hinge*>( af.GetPhysics()->GetConstraint( steeringHingeName ) );
		if( !steering[i] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s': can't find steering hinge '%s'", name.c_str(), steeringHingeName );
		}
	}

	memset( wheelAngles, 0, sizeof( wheelAngles ) );
	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_VehicleFourWheels::Think
================
*/
void idAFEntity_VehicleFourWheels::Think()
{
	int i;
	float force = 0.0f, velocity = 0.0f, steerAngle = 0.0f;
	idVec3 origin;
	idMat3 axis;
	idRotation rotation;

	if( thinkFlags & TH_THINK )
	{

		if( player )
		{
			// capture the input from a player
			velocity = veh_velocity;
			if( player->usercmd.forwardmove < 0 )
			{
				velocity = -velocity;
			}
			force = idMath::Fabs( player->usercmd.forwardmove * veh_force ) * ( 1.0f / 128.0f );
			steerAngle = GetSteerAngle();
		}

		// update the wheel motor force
		for( i = 0; i < 2; i++ )
		{
			wheels[2 + i]->SetContactMotorVelocity( velocity );
			wheels[2 + i]->SetContactMotorForce( force );
		}

		// adjust wheel velocity for better steering because there are no differentials between the wheels
		if( steerAngle < 0.0f )
		{
			wheels[2]->SetContactMotorVelocity( velocity * 0.5f );
		}
		else if( steerAngle > 0.0f )
		{
			wheels[3]->SetContactMotorVelocity( velocity * 0.5f );
		}

		// update the wheel steering
		steering[0]->SetSteerAngle( steerAngle );
		steering[1]->SetSteerAngle( steerAngle );
		for( i = 0; i < 2; i++ )
		{
			steering[i]->SetSteerSpeed( 3.0f );
		}

		// update the steering wheel
		animator.GetJointTransform( steeringWheelJoint, gameLocal.time, origin, axis );
		rotation.SetVec( axis[2] );
		rotation.SetAngle( -steerAngle );
		animator.SetJointAxis( steeringWheelJoint, JOINTMOD_WORLD, rotation.ToMat3() );

		// run the physics
		RunPhysics();

		// rotate the wheels visually
		for( i = 0; i < 4; i++ )
		{
			if( force == 0.0f )
			{
				velocity = wheels[i]->GetLinearVelocity() * wheels[i]->GetWorldAxis()[0];
			}
			wheelAngles[i] += velocity * MS2SEC( gameLocal.time - gameLocal.previousTime ) / wheelRadius;
			// give the wheel joint an additional rotation about the wheel axis
			rotation.SetAngle( RAD2DEG( wheelAngles[i] ) );
			axis = af.GetPhysics()->GetAxis( 0 );
			rotation.SetVec( ( wheels[i]->GetWorldAxis() * axis.Transpose() )[2] );
			animator.SetJointAxis( wheelJoints[i], JOINTMOD_WORLD, rotation.ToMat3() );
		}

		// spawn dust particle effects
		if( force != 0.0f && !( gameLocal.framenum & 7 ) )
		{
			int numContacts;
			idAFConstraint_Contact* contacts[2];
			for( i = 0; i < 4; i++ )
			{
				numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[i]->GetClipModel()->GetId(), contacts, 2 );
				for( int j = 0; j < numContacts; j++ )
				{
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[j]->GetContact().point, contacts[j]->GetContact().normal.ToMat3(), timeGroup /* D3XP */ );
				}
			}
		}
	}

	UpdateAnimation();
	if( thinkFlags & TH_UPDATEVISUALS )
	{
		Present();
		LinkCombat();
	}
}


/*
===============================================================================

  idAFEntity_VehicleSixWheels

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Vehicle, idAFEntity_VehicleSixWheels )
END_CLASS

/*
================
idAFEntity_VehicleSixWheels::idAFEntity_VehicleSixWheels
================
*/
idAFEntity_VehicleSixWheels::idAFEntity_VehicleSixWheels()
{
	int i;

	for( i = 0; i < 6; i++ )
	{
		wheels[i]		= NULL;
		wheelJoints[i]	= INVALID_JOINT;
		wheelAngles[i]	= 0.0f;
	}
	steering[0]			= NULL;
	steering[1]			= NULL;
	steering[2]			= NULL;
	steering[3]			= NULL;
}

/*
================
idAFEntity_VehicleSixWheels::Spawn
================
*/
void idAFEntity_VehicleSixWheels::Spawn()
{
	int i;
	static const char* wheelBodyKeys[] =
	{
		"wheelBodyFrontLeft",
		"wheelBodyFrontRight",
		"wheelBodyMiddleLeft",
		"wheelBodyMiddleRight",
		"wheelBodyRearLeft",
		"wheelBodyRearRight"
	};
	static const char* wheelJointKeys[] =
	{
		"wheelJointFrontLeft",
		"wheelJointFrontRight",
		"wheelJointMiddleLeft",
		"wheelJointMiddleRight",
		"wheelJointRearLeft",
		"wheelJointRearRight"
	};
	static const char* steeringHingeKeys[] =
	{
		"steeringHingeFrontLeft",
		"steeringHingeFrontRight",
		"steeringHingeRearLeft",
		"steeringHingeRearRight"
	};

	const char* wheelBodyName, *wheelJointName, *steeringHingeName;

	for( i = 0; i < 6; i++ )
	{
		wheelBodyName = spawnArgs.GetString( wheelBodyKeys[i], "" );
		if( !wheelBodyName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' no '%s' specified", name.c_str(), wheelBodyKeys[i] );
		}
		wheels[i] = af.GetPhysics()->GetBody( wheelBodyName );
		if( !wheels[i] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' can't find wheel body '%s'", name.c_str(), wheelBodyName );
		}
		wheelJointName = spawnArgs.GetString( wheelJointKeys[i], "" );
		if( !wheelJointName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' no '%s' specified", name.c_str(), wheelJointKeys[i] );
		}
		wheelJoints[i] = animator.GetJointHandle( wheelJointName );
		if( wheelJoints[i] == INVALID_JOINT )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' can't find wheel joint '%s'", name.c_str(), wheelJointName );
		}
	}

	for( i = 0; i < 4; i++ )
	{
		steeringHingeName = spawnArgs.GetString( steeringHingeKeys[i], "" );
		if( !steeringHingeName[0] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' no '%s' specified", name.c_str(), steeringHingeKeys[i] );
		}
		steering[i] = static_cast<idAFConstraint_Hinge*>( af.GetPhysics()->GetConstraint( steeringHingeName ) );
		if( !steering[i] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s': can't find steering hinge '%s'", name.c_str(), steeringHingeName );
		}
	}

	memset( wheelAngles, 0, sizeof( wheelAngles ) );
	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_VehicleSixWheels::Think
================
*/
void idAFEntity_VehicleSixWheels::Think()
{
	int i;
	idVec3 origin;
	idMat3 axis;
	idRotation rotation;

	if( thinkFlags & TH_THINK )
	{

		if( player )
		{
			// capture the input from a player
			velocity = veh_velocity;
			if( player->usercmd.forwardmove < 0 )
			{
				velocity = -velocity;
			}
			force = idMath::Fabs( player->usercmd.forwardmove * veh_force ) * ( 1.0f / 128.0f );
			steerAngle = GetSteerAngle();
		}

		// update the wheel motor force
		for( i = 0; i < 6; i++ )
		{
			wheels[i]->SetContactMotorVelocity( velocity );
			wheels[i]->SetContactMotorForce( force );
		}

		// adjust wheel velocity for better steering because there are no differentials between the wheels
		if( steerAngle < 0.0f )
		{
			for( i = 0; i < 3; i++ )
			{
				wheels[( i << 1 )]->SetContactMotorVelocity( velocity * 0.5f );
			}
		}
		else if( steerAngle > 0.0f )
		{
			for( i = 0; i < 3; i++ )
			{
				wheels[1 + ( i << 1 )]->SetContactMotorVelocity( velocity * 0.5f );
			}
		}

		// update the wheel steering
		steering[0]->SetSteerAngle( steerAngle );
		steering[1]->SetSteerAngle( steerAngle );
		steering[2]->SetSteerAngle( -steerAngle );
		steering[3]->SetSteerAngle( -steerAngle );
		for( i = 0; i < 4; i++ )
		{
			steering[i]->SetSteerSpeed( 3.0f );
		}

		// update the steering wheel
		animator.GetJointTransform( steeringWheelJoint, gameLocal.time, origin, axis );
		rotation.SetVec( axis[2] );
		rotation.SetAngle( -steerAngle );
		animator.SetJointAxis( steeringWheelJoint, JOINTMOD_WORLD, rotation.ToMat3() );

		// run the physics
		RunPhysics();

		// rotate the wheels visually
		for( i = 0; i < 6; i++ )
		{
			if( force == 0.0f )
			{
				velocity = wheels[i]->GetLinearVelocity() * wheels[i]->GetWorldAxis()[0];
			}
			wheelAngles[i] += velocity * MS2SEC( gameLocal.time - gameLocal.previousTime ) / wheelRadius;
			// give the wheel joint an additional rotation about the wheel axis
			rotation.SetAngle( RAD2DEG( wheelAngles[i] ) );
			axis = af.GetPhysics()->GetAxis( 0 );
			rotation.SetVec( ( wheels[i]->GetWorldAxis() * axis.Transpose() )[2] );
			animator.SetJointAxis( wheelJoints[i], JOINTMOD_WORLD, rotation.ToMat3() );
		}

		// spawn dust particle effects
		if( force != 0.0f && !( gameLocal.framenum & 7 ) )
		{
			int numContacts;
			idAFConstraint_Contact* contacts[2];
			for( i = 0; i < 6; i++ )
			{
				numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[i]->GetClipModel()->GetId(), contacts, 2 );
				for( int j = 0; j < numContacts; j++ )
				{
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[j]->GetContact().point, contacts[j]->GetContact().normal.ToMat3(), timeGroup /* D3XP */ );
				}
			}
		}
	}

	UpdateAnimation();
	if( thinkFlags & TH_UPDATEVISUALS )
	{
		Present();
		LinkCombat();
	}
}

/*
===============================================================================

idAFEntity_VehicleAutomated

===============================================================================
*/
const idEventDef EV_Vehicle_setVelocity( "setVelocity", "f" );
const idEventDef EV_Vehicle_setTorque( "setTorque", "f" );
const idEventDef EV_Vehicle_setSteeringSpeed( "setSteeringSpeed", "f" );
const idEventDef EV_Vehicle_setWaypoint( "setWaypoint", "e" );

CLASS_DECLARATION( idAFEntity_VehicleSixWheels, idAFEntity_VehicleAutomated )
EVENT( EV_PostSpawn,				idAFEntity_VehicleAutomated::PostSpawn )
EVENT( EV_Vehicle_setVelocity,		idAFEntity_VehicleAutomated::Event_SetVelocity )
EVENT( EV_Vehicle_setTorque,		idAFEntity_VehicleAutomated::Event_SetTorque )
EVENT( EV_Vehicle_setSteeringSpeed,	idAFEntity_VehicleAutomated::Event_SetSteeringSpeed )
EVENT( EV_Vehicle_setWaypoint,		idAFEntity_VehicleAutomated::Event_SetWayPoint )
END_CLASS

/*
================
idAFEntity_VehicleAutomated::Spawn
================
*/
void idAFEntity_VehicleAutomated::Spawn()
{

	velocity = force = steerAngle = 0.f;
	currentSteering = steeringSpeed = 0.f;
	originHeight = 0.f;
	waypoint = NULL;

	spawnArgs.GetFloat( "velocity", "150", velocity );
	spawnArgs.GetFloat( "torque", "200000", force );
	spawnArgs.GetFloat( "steeringSpeed", "1", steeringSpeed );
	spawnArgs.GetFloat( "originHeight", "0", originHeight );

	PostEventMS( &EV_PostSpawn, 0 );
}

/*
================
idAFEntity_VehicleAutomated::PostSpawn
================
*/
void idAFEntity_VehicleAutomated::PostSpawn()
{

	if( targets.Num() )
	{
		waypoint = targets[0].GetEntity();
	}
}

/*
================
idAFEntity_VehicleAutomated::Event_SetVelocity
================
*/
void idAFEntity_VehicleAutomated::Event_SetVelocity( float _velocity )
{
	velocity = _velocity;
}

/*
================
idAFEntity_VehicleAutomated::Event_SetTorque
================
*/
void idAFEntity_VehicleAutomated::Event_SetTorque( float _torque )
{
	force = _torque;
}

/*
================
idAFEntity_VehicleAutomated::Event_SetSteeringSpeed
================
*/
void idAFEntity_VehicleAutomated::Event_SetSteeringSpeed( float _steeringSpeed )
{
	steeringSpeed = _steeringSpeed;
}

/*
================
idAFEntity_VehicleAutomated::Event_SetWayPoint
================
*/
void idAFEntity_VehicleAutomated::Event_SetWayPoint( idEntity* _waypoint )
{
	waypoint = _waypoint;
}

/*
================
idAFEntity_VehicleAutomated::Think
================
*/
#define	HIT_WAYPOINT_THRESHOLD	80.f

void idAFEntity_VehicleAutomated::Think()
{

	// If we don't have a waypoint, coast to a stop
	if( !waypoint )
	{
		velocity = force = steerAngle = 0.f;
		idAFEntity_VehicleSixWheels::Think();
		return;
	}

	idVec3 waypoint_origin, vehicle_origin;
	idVec3 travel_vector;
	float distance_from_waypoint;

	// Set up the vector from the vehicle origin, to the waypoint
	vehicle_origin = GetPhysics()->GetOrigin();
	vehicle_origin.z -= originHeight;

	waypoint_origin = waypoint->GetPhysics()->GetOrigin();

	travel_vector = waypoint_origin - vehicle_origin;
	distance_from_waypoint = travel_vector.Length();

	// Check if we've hit the waypoint (within a certain threshold)
	if( distance_from_waypoint < HIT_WAYPOINT_THRESHOLD )
	{
		idStr				callfunc;
		const function_t*	func;
		idThread*			thread;

		// Waypoints can call script functions
		waypoint->spawnArgs.GetString( "call", "", callfunc );
		if( callfunc.Length() )
		{
			func = gameLocal.program.FindFunction( callfunc );
			if( func != NULL )
			{
				thread = new idThread( func );
				thread->DelayedStart( 0 );
			}
		}

		// Get next waypoint
		if( waypoint->targets.Num() )
		{
			waypoint = waypoint->targets[0].GetEntity();
		}
		else
		{
			waypoint = NULL;
		}

		// We are switching waypoints, adjust steering next frame
		idAFEntity_VehicleSixWheels::Think();
		return;
	}

	idAngles vehicle_angles, travel_angles;

	// Get the angles we need to steer towards
	travel_angles = travel_vector.ToAngles().Normalize360();
	vehicle_angles = this->GetPhysics()->GetAxis().ToAngles().Normalize360();

	float	delta_yaw;

	// Get the shortest steering angle towards the travel angles
	delta_yaw = vehicle_angles.yaw - travel_angles.yaw;
	if( idMath::Fabs( delta_yaw ) > 180.f )
	{
		if( delta_yaw > 0 )
		{
			delta_yaw = delta_yaw - 360;
		}
		else
		{
			delta_yaw = delta_yaw + 360;
		}
	}

	// Maximum steering angle is 35 degrees
	delta_yaw = idMath::ClampFloat( -35.f, 35.f, delta_yaw );

	idealSteering = delta_yaw;

	// Adjust steering incrementally so it doesn't snap to the ideal angle
	if( idMath::Fabs( ( idealSteering - currentSteering ) ) > steeringSpeed )
	{
		if( idealSteering > currentSteering )
		{
			currentSteering += steeringSpeed;
		}
		else
		{
			currentSteering -= steeringSpeed;
		}
	}
	else
	{
		currentSteering = idealSteering;
	}

	// DEBUG
	if( g_vehicleDebug.GetBool() )
	{
		gameRenderWorld->DebugBounds( colorRed, idBounds( idVec3( -4, -4, -4 ), idVec3( 4, 4, 4 ) ), vehicle_origin );
		gameRenderWorld->DebugBounds( colorRed, idBounds( idVec3( -4, -4, -4 ), idVec3( 4, 4, 4 ) ), waypoint_origin );
		gameRenderWorld->DrawText( waypoint->name.c_str(), waypoint_origin + idVec3( 0, 0, 16 ), 0.25f, colorYellow, gameLocal.GetLocalPlayer()->viewAxis );
		gameRenderWorld->DebugArrow( colorWhite, vehicle_origin, waypoint_origin, 12.f );
	}

	// Set the final steerAngle for the vehicle
	steerAngle = currentSteering;

	idAFEntity_VehicleSixWheels::Think();
}



