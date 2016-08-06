/*
 * staticParticles.h
 *
 *  Created on: 24/07/2016
 *      Author: biel
 */

#ifndef D3XP_STATICPARTICLES_H_
#define D3XP_STATICPARTICLES_H_


class blStaticParticles {
public:
	blStaticParticles();
	virtual ~blStaticParticles();

	void						Save( idSaveGame* savefile ) const;
	void						Restore( idRestoreGame* savefile );

	void						Init();
	void						Shutdown();
	void						UpdateJointTransforms(jointHandle_t, idVec3&, idMat3&);;
	bool						EmitParticles( const idDeclParticle* smoke, const int startTime, const float diversity,
			   	   	   	   	   	   	   	   	   const idVec3& origin, const idMat3& axis, int timeGroup /*_D3XP*/ );
	void						KillParticles(); // erases them from the game
	void						ShowParticles();
	void						HidePArticles();
	void						updateParms();

	virtual void				WriteToSnapshot( idBitMsg& msg ) const;
	virtual void				ReadFromSnapshot( const idBitMsg& msg );

protected:
	bool						Hidden;
	renderEntity_t				renderEntity;
	int							h_renderEntity;

	bool						UpdateRenderEntity( renderEntity_s* renderEntity, const renderView_t* renderView );
	static bool					ModelCallback( renderEntity_s* renderEntity, const renderView_t* renderView );


};

#endif /* D3XP_STATICPARTICLES_H_ */
