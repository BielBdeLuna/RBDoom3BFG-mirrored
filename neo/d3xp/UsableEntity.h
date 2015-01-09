/*
 * UsableEntity.h
 *	the source code contained in this file is under the terms of the GPL v3 license
 *  Created on: 09/01/2015
 *      Author: biel
 */

#ifndef USABLEENTITY_H_
#define USABLEENTITY_H_

#include "physics/Physics_Parametric.h"
#include "Entity.h"

extern const idEventDef EV_Use;

class blUsableEntity : public idEntity {
public:
	CLASS_PROTOTYPE( blUsableEntity );

                            blUsableEntity( void );
//                            ~blUsableEntity( void );

	void					Spawn( void );
	void			        Event_Use( void );
    void			        Event_Interact( void );
protected:

    bool                    activated;

    idPhysics_Parametric	physicsObj;

private:

    idAngles				dest_angles;
	idVec3					dest_position;
};

/*

class blUsableEntityTarget : public blUsableEntity {
public:
	CLASS_PROTOTYPE( blUsableEntityTarget );

                            blUsableEntityTarget( void );
							~blUsableEntityTarget( void );

	void					Spawn( void );
	void			        Event_Use( void );
    void                    Target( void );
};

class blUsableEntityMounted : public blUsableEntity {
public:
	CLASS_PROTOTYPE( blUsableEntityMounted );

                            blUsableEntityMounted( void );
							~blUsableEntityMounted( void );

	void					Spawn( void );
    void                    Event_Use( idPlayer *player );

protected:
	idPlayer *				player;
    jointHandle_t			eyesJoint;
};
*/
#endif /* USABLEENTITY_H_ */
