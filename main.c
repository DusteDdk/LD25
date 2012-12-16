/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libeo/eng.h"

#ifndef DATADIR
  #define DATADIR "."
#endif

#define COLTEAM_CITIES 1
#define COLTEAM_EVILRAYS 2
#define COLTEAM_GOODPC 3

guiContext* menuContext;
guiContext* ingameContext;
sprite_s* tcur_spr; //"Target" cursor
sprite_s* scur_spr; //"Select" cursor
sprite_s* dcur_spr; //"default" cursor

engObj_s* selSat=NULL;
engObj_s* selCty=NULL;

sound_s* strangeSnd;
sound_s* laserFireSnd;
sound_s* explSnd;

typedef struct {
	vec3 target;
	int hasTarget;
	int coolDown;
	int fireNow;
} satState_s;

void btnClbQuit(void* notused)
{
  eoExec("quit 1");
}

void btnClbStartGame(void* notused)
 {
	eoSamplePlay(strangeSnd,128);
	eoExec("campos 32.7 15.72 45.43");
	eoExec("camlook 32.7 17.72 20.43");
	eoGuiContextSet( ingameContext );
	eoPauseSet(0);
 }


void explo( vec3 p )
{
	static particleEmitter_s* emitter = NULL;
	if( emitter == NULL )
	{
      emitter = eoPsysNewEmitter();
	  emitter->addictive=1;
	  emitter->numParticlesPerEmission = 250;
	  emitter->ticksBetweenEmissions = 0;
	  emitter->particleLifeMax = 200;
	  emitter->particleLifeVariance = 400;
	  emitter->shrink=0;
	  emitter->fade=0;
	  emitter->percentFlicker=70;
	  emitter->sizeMax=0.25;
	  emitter->sizeVariance=0.04;
	  emitter->rotateParticles=1;
	  emitter->colorVariance[0]=0.4;
	  emitter->colorVariance[1]=0.5;
	  emitter->colorVariance[2]=0.2;
	  emitter->colorVariance[3]=0.0;
	  emitter->color[0]=0.8;
	  emitter->color[1]=0.4;
	  emitter->color[2]=0.2;
	  emitter->emitSpeedMax=5;
	  emitter->emitSpeedVariance=1;
	  eoPsysBake(emitter);
	}


	  eoPsysEmit(emitter, p);
}

void ctyColFunc( engObj_s* cty, engObj_s* nme )
{
	eoSamplePlay( explSnd, 128 );
	eoObjDel(nme);
	eoObjDel(cty);
	explo( cty->pos );
}


void frameStart()
{
	eoGuiSetCursor(dcur_spr, 0,0 );
}

void starsThink( engObj_s* stars )
{
	stars->pos = eoCamPosGet();
	stars->rot.x += 0.01;
	stars->rot.z += 0.008;
}

void psyThink( engObj_s* psy )
{
	psy->pos = eoCamPosGet();
	psy->rot.x -= 0.004;
	psy->rot.z -= 0.005;
	psy->rot.y += 0.007;
}

void mThink( engObj_s* m )
{
	if( m->pos.y < 1 )
	{
		eoSamplePlay( explSnd, 128 );
		explo(m->pos);
		eoObjDel(m);
	}
}

void spawnMissile( int colTeam, vec3 startPos, vec3 direction )
{
	engObj_s* m = eoObjCreate( ENGOBJ_PAREMIT );

	particleEmitter_s* emitter = eoPsysNewEmitter();
	emitter->addictive=1;
	emitter->numParticlesPerEmission = 4;
	emitter->ticksBetweenEmissions = 1;
	emitter->particleLifeMax = 100;
	emitter->particleLifeVariance = 100;
	emitter->shrink=0;
	emitter->fade=0;
	emitter->percentFlicker=70;
	emitter->sizeMax=0.07;
	emitter->sizeVariance=0.03;
	emitter->rotateParticles=0;
	emitter->colorVariance[0]=0.4;
	emitter->colorVariance[1]=0.5;
	emitter->colorVariance[2]=0.6;
	emitter->colorVariance[3]=0.0;
	emitter->color[0]=0.8;
	emitter->color[1]=0.6;
	emitter->color[2]=0.5;
	emitter->emitSpeedMax=0.1;
	emitter->emitSpeedVariance=0.1;
	eoPsysBake(emitter);
	m->emitter = emitter;
	m->colTeam=colTeam;
	m->pos=startPos;
	m->vel=eoVec3Scale( eoVec3Normalize(direction), 0.2);
	m->thinkFunc=mThink;
	eoObjBake(m);
	eoObjAdd(m);
}

void satThink( engObj_s* sat )
{
	//We won't change velocity if it's too extreme
		GLfloat change = ((float)((rand()%10000)-5000))/1000000;

		if( (sat->pos.y > 45.5 && change > 0) || (sat->pos.y < 45.3 && change < 0 ))
			change = -change;

		if( (sat->pos.y > 45.7 && change > 0) || (sat->pos.y < 45.1 && change < 0 ))
		{
			sat->vel.y = 0;
		}

		sat->vel.y += change;

		satState_s* s = (satState_s*)sat->gameData;
		if( s->hasTarget )
		{
			vec3 v = eoVec3FromPoints( sat->pos, s->target  );
			sat->rot.z = atan2( v.y, v.x ) * 57.29+90;

			if(s->fireNow)
			{
				eoSamplePlay(laserFireSnd, 128);
				selCty=0;
				s->fireNow=0;
				spawnMissile( COLTEAM_EVILRAYS, sat->pos, v );
			}
		}

}

void ctyMouseEvent( engObj_s* cty, int bs )
{

	if( bs == 1)
	{
		selCty=cty;
		if( selSat )
		{
			satState_s* s = (satState_s*)selSat->gameData;
			s->target=selCty->pos;
			s->hasTarget=1;
		}
		explo( cty->pos );
	}

	eoGuiSetCursor(tcur_spr, -32,-32 );

}

void satMouseEvent( engObj_s* sat, int bs )
{
	if( bs == 1)
	{
		selSat=sat;
		if( selCty )
		{
			satState_s* s = (satState_s*)selSat->gameData;
			s->target=selCty->pos;
			s->hasTarget=1;
		}
		explo( sat->pos );
	}

	eoGuiSetCursor(scur_spr, -32,-32 );
}

void btnMainMenuCb( void* unused )
{
	eoPauseSet(1);
	eoGuiContextSet( menuContext );

	  vec3 p;
	  p.x=100;
	  p.y=10;
	  p.z=0;
	  eoCamPosSet(p);

	  p.x=100;
	  p.y=100;
	  p.z=3;

	  eoCamTargetSet( p );

}

void fireSats()
{
	if( !eoPauseGet() )
	{
		if( selSat )
		{
			satState_s* s = (satState_s*)selSat->gameData;
			s->fireNow=1;
		}
	}
}

void btnFireCb( void* unused )
{
	fireSats();
}

void keyFireCb(inputEvent* e)
{
	fireSats();
}

int main(int argc, char *argv[])
{

  eoInitAll(argc, argv, DATADIR);

  //Load the target cursor
  sprite_base* tcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor-target.spr"));
  sprite_base* scur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor-select.spr"));
  sprite_base* dcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor.spr"));

  tcur_spr = eoSpriteNew( tcur_sprb, 1, 1 );
  scur_spr = eoSpriteNew( scur_sprb, 1, 1 );
  dcur_spr = eoSpriteNew( dcur_sprb, 1, 1 );



  //Setup a window so we can Start Game or Exit.
  menuContext = eoGuiContextCreate();
  ingameContext = eoGuiContextCreate();
  eoGuiContextSet(menuContext);

  guiWindow_s* winHalp = eoGuiAddWindow( menuContext, eoSetting()->res.x/2-150, eoSetting()->res.y/2-180 , 300,90, "Instructions", 0 );
  eoGuiAddLabel(winHalp, 0,0, "Destroy the cities using your laser shooting space satellites!\nSelect a sat. and tell it where to shoot.");


  guiWindow_s* winMainMenu = eoGuiAddWindow( menuContext, eoSetting()->res.x/2-100, eoSetting()->res.y/2-45, 200,90, "killCity",0 );
  //Add button to exit the game.
  eoGuiAddButton(winMainMenu,GUI_POS_CENTER,5, 190, 20, "Start!", btnClbStartGame );
  eoGuiAddButton(winMainMenu,GUI_POS_CENTER,35, 190, 20, "Quit..", btnClbQuit );

  //Ingame HUD

  //The Exit button
  eoGuiAddButton( ingameContext, eoSetting()->res.x - 160, eoSetting()->res.y - 60, 150, 50, "Main Menu", btnMainMenuCb );
  //The Fire button
  eoGuiAddButton( ingameContext, 10, eoSetting()->res.y - 60, 150, 50, "Fire! (Spacebar)", btnFireCb );

  eoInpAddHook( INPUT_EVENT_KEY, INPUT_FLAG_DOWN, SDLK_SPACE ,keyFireCb );

  strangeSnd = eoSampleLoad(Data("/data/sound/","muwah.wav"));
  laserFireSnd =eoSampleLoad( Data("/data/sound/","laser0.wav"));
  explSnd = eoSampleLoad( Data("/data/sound/","expl0.wav") );

  //Rounds left
  vec3 p;
  p.x=100;
  p.y=10;
  p.z=0;
  eoCamPosSet(p);

  p.x=100;
  p.y=100;
  p.z=3;

  eoCamTargetSet( p );

  eoGuiShow();
  eoPauseSet(1);

  eoRegisterStartFrameFunc( frameStart );


  GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 2.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

  GLfloat pos[] = { 20,60,38,1 }; //Last pos: 0 = dir, 1=omni
  glLightfv( GL_LIGHT0, GL_POSITION, pos );

  GLfloat specular[] = {0.0, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  vboModel* ctyMdl = eoModelLoad( "data/objs/", "cty.obj");
  vboModel* canBaseMdl = eoModelLoad( "data/objs/", "canbase.obj" );
  vboModel* canTopMdl = eoModelLoad( "data/objs/", "cantop.obj");
  vboModel* satMdl = eoModelLoad( "data/objs/", "sat.obj");


  vboModel* scapeMdl = eoModelLoad( "data/objs/", "scape.obj");
  vboModel* starsMdl = eoModelLoad( "data/objs/", "stars.obj" );
  vboModel* psyMdl = eoModelLoad( "data/objs/", "psy.obj" );

  engObj_s* sphereObj;

  sphereObj = eoObjCreate( ENGOBJ_MODEL );
  sphereObj->model = psyMdl;
  sphereObj->thinkFunc = psyThink;
  eoObjBake(sphereObj);
  eoObjAdd( sphereObj );

  sphereObj = eoObjCreate( ENGOBJ_MODEL );
  sphereObj->model = starsMdl;
  sphereObj->thinkFunc = starsThink;
  eoObjBake(sphereObj);
  eoObjAdd( sphereObj );


  engObj_s* scapeObj = eoObjCreate( ENGOBJ_MODEL );
  scapeObj->model = scapeMdl;
  scapeObj->pos.x=0;
  scapeObj->pos.y=-10;
  scapeObj->pos.z=-100;
  scapeObj->rot.y=43;

  eoObjBake(scapeObj);
  eoObjAdd(scapeObj);

  engObj_s* canObj;

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = canBaseMdl;
  canObj->pos.x = 33;

  eoObjBake(canObj );
  eoObjAdd( canObj );

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = canTopMdl;
  canObj->pos.x = 33;
  canObj->pos.y = 5;

  eoObjBake(canObj );
  eoObjAdd( canObj );


  //Sats
  satState_s* satState;

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 0;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
  satState=malloc(sizeof(satState_s));
  memset( satState, 0, sizeof(satState_s));
  canObj->gameData=satState;
  eoObjBake(canObj );
  eoObjAdd( canObj );

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 33;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
  satState=malloc(sizeof(satState_s));
  memset( satState, 0, sizeof(satState_s));
  canObj->gameData=satState;
  eoObjBake(canObj );
  eoObjAdd( canObj );

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 66;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
  satState=malloc(sizeof(satState_s));
  memset( satState, 0, sizeof(satState_s));
  canObj->gameData=satState;
  eoObjBake(canObj );
  eoObjAdd( canObj );


  int i;
  for(i=0; i < 6; i++ )
  {
	  engObj_s* cty = eoObjCreate( ENGOBJ_MODEL );
	  cty->colTeam = COLTEAM_CITIES;
	  cty->model = ctyMdl;
	  cty->colFunc = ctyColFunc;

	  cty->pos.x = i*10;
	  if( i > 2 ) cty->pos.x +=16;
	  cty->clickedFunc=ctyMouseEvent;
	  eoObjBake( cty );
	  eoObjAdd( cty );
  }

  eoMainLoop();

  return 0;
}
