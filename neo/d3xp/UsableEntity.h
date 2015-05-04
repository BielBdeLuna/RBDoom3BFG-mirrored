/*
 * UsableEntity.h
 *	the source code contained in this file is under the terms of the GPL v3 license
 *  Created on: 09/01/2015
 *      Author: biel
 */

/*
===============================================================================

	blUsable

===============================================================================
*/

class blUsable : public idEntity
{
public:
	CLASS_PROTOTYPE( blUsable  );

	blUsable ( void );
	//                    	~blUsable ( void );

	void					Spawn( void );
	void					TestFireTargets( void );
	void					Interact( void );
	//void					Event_PostSpawn();
	void					Event_Activate( idEntity* activator );
protected:

    bool                    activated;
    bool					turned;
    bool					powered;

    //idPhysics_Parametric	physicsObj;

private:
    //void					ToggleState( void );
    //idAngles				dest_angles;
	//idVec3					dest_position;
};

/*

class blUsableTarget : public blUsable {
public:
	CLASS_PROTOTYPE( blUsableTarget );

                            blUsableTarget( void );
							~blUsableTarget( void );

	void					Spawn( void );
	void			        Event_Use( void );
    void                    Target( void );
};

class blUsableMounted : public blUsable {
public:
	CLASS_PROTOTYPE( blUsableMounted );

                            blUsableMounted( void );
							~blUsableMounted( void );

	void					Spawn( void );
    void                    Event_Use( idPlayer *player );

protected:
	idPlayer *				player;
    jointHandle_t			eyesJoint;
};
*/
