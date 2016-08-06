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

#ifndef D3XP_VEHICLE_H_
#define D3XP_VEHICLE_H_

/*
===============================================================================

idAFEntity_Vehicle

===============================================================================
*/

class idAFEntity_Vehicle : public idAFEntity_Base
{
public:
	CLASS_PROTOTYPE( idAFEntity_Vehicle );

	idAFEntity_Vehicle();

	void					Spawn();
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );
	bool					Collide( const trace_t& collision, const idVec3& velocity );
	void					Use( idPlayer* player );
	void					ToggleHeadLights();
	void					ToggleEngine();
	void					ToggleBrakes();
	float					GetThirdPersonRange();
	float					GetThirdPersonAngle();
	float					GetThirdPersonHeight();
	bool					TestDriveable();
	void					ForceAbandonVehicle();
    bool                    GetGlobalJointTransform(jointHandle_t, idVec3&, idMat3&);
    void                    UpdatelightPositions();


protected:
	idPlayer* 				player;
	//idEntityPtr<idPlayer>	player;
	jointHandle_t			eyesJoint;
	jointHandle_t			steeringWheelJoint;
	jointHandle_t			headlightsJoint;
	float					wheelRadius;
	float					steerAngle;
	float					steerSpeed;
	int						headLights; // -1 broken, 0 off, 1 on
	int						engine;	//-1 off, 0 idle, 1 gas
	float                  	brakes;
	bool					gasSound;
	bool					idleMotor;
	bool					backAlarm;
	bool					klaxonEngaged;
	bool					hasBackAlarm;
	bool					driveable;

	//instead of a cvar controlling all cars...
	float                   veh_velocity;           //"1000"
	float                   veh_force;              //"50000"
	float                   veh_suspensionUp;       //"32"
	float                   veh_suspensionDown;     //"20"
	float                   veh_suspensionKCompress;//"200"
	float                   veh_suspensionDamping;  //"400"
	float                   veh_tireFriction;       //"0.9"
	void					AbandonVehicle();
	float					GetSteerAngle();

	void 					TireTrack( const idVec3 &origin, float angle, const idMaterial* material );
	const idMaterial* 		trackDecal;
	const idDeclParticle* 	dustSmoke;

	void					setupEmitterParticles( const char* lightParticles );
	void					updateEmitterPos();
	const idFuncEmitter*	lightShaft;

	renderLight_s			l_headlights;

	idVec3					headlightsColor;
	int						headlightsEnd;
	int						headlightsTime;
	int						headlightsHandle;
	idVec3					headlightsOrigin;
	idMat3					headlightsAxis;
};


/*
===============================================================================

idAFEntity_VehicleSimple

===============================================================================
*/

class idAFEntity_VehicleSimple : public idAFEntity_Vehicle
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleSimple );

	idAFEntity_VehicleSimple();
	~idAFEntity_VehicleSimple();

	void					Spawn();
	virtual void			Think();

protected:
	idClipModel* 				wheelModel;
	idAFConstraint_Suspension* 	suspension[4];
	jointHandle_t				wheelJoints[4];
	float						wheelAngles[4];
};


/*
===============================================================================

idAFEntity_VehicleFourWheels

===============================================================================
*/

class idAFEntity_VehicleFourWheels : public idAFEntity_Vehicle
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleFourWheels );

	idAFEntity_VehicleFourWheels();

	void					Spawn();
	virtual void			Think();

protected:
	idAFBody* 				wheels[4];
	idAFConstraint_Hinge* 	steering[2];
	jointHandle_t			wheelJoints[4];
	float					wheelAngles[4];
};


/*
===============================================================================

idAFEntity_VehicleSixWheels

===============================================================================
*/

class idAFEntity_VehicleSixWheels : public idAFEntity_Vehicle
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleSixWheels );

	idAFEntity_VehicleSixWheels();

	void					Spawn();
	virtual void			Think();

	float					force;
	float					velocity;
	float					steerAngle;

private:
	idAFBody* 				wheels[6];
	idAFConstraint_Hinge* 	steering[4];
	jointHandle_t			wheelJoints[6];
	float					wheelAngles[6];
};

/*
===============================================================================

idAFEntity_VehicleAutomated

===============================================================================
*/

class idAFEntity_VehicleAutomated : public idAFEntity_VehicleSixWheels
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleAutomated );

	void					Spawn();
	void					PostSpawn();
	virtual void			Think();

private:

	idEntity*	waypoint;
	float		steeringSpeed;
	float		currentSteering;
	float		idealSteering;
	float		originHeight;

	void		Event_SetVelocity( float _velocity );
	void		Event_SetTorque( float _torque );
	void		Event_SetSteeringSpeed( float _steeringSpeed );
	void		Event_SetWayPoint( idEntity* _waypoint );
};

#endif /* D3XP_VEHICLE_H_ */
