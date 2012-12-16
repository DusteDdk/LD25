
/*
 * License: WTFPL
 */

#include <stdio.h>
#include <stdlib.h>

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



void btnClbQuit(void* notused)
{
  eoExec("quit 1");
}

void btnClbStartGame(void* notused)
 {
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
	  emitter->numParticlesPerEmission = 50;
	  emitter->ticksBetweenEmissions = 0;
	  emitter->particleLifeMax = 1000;
	  emitter->particleLifeVariance = 500;
	  emitter->shrink=0;
	  emitter->fade=0;
	  emitter->percentFlicker=70;
	  emitter->sizeMax=0.08;
	  emitter->sizeVariance=0.04;
	  emitter->rotateParticles=1;
	  emitter->colorVariance[0]=0.4;
	  emitter->colorVariance[1]=0.5;
	  emitter->colorVariance[2]=0.6;
	  emitter->colorVariance[3]=0.0;
	  emitter->color[0]=0.8;
	  emitter->color[1]=0.6;
	  emitter->color[2]=0.5;
	  emitter->emitSpeedMax=1;
	  emitter->emitSpeedVariance=1;
	  eoPsysBake(emitter);
	}


	  eoPsysEmit(emitter, p);
}

void ctyColFunc( engObj_s* cty, engObj_s* nme )
{
	eoPrint("Something hit me!");
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

void satThink( engObj_s* sat )
{
	GLfloat change = ((float)((rand()%10000)-5000))/1000000;

	if( (sat->pos.y > 45.5 && change > 0) || (sat->pos.y < 45.3 && change < 0 ))
		change = -change;

	sat->vel.y += change;
}

void ctyMouseEvent( engObj_s* cty, int bs )
{

	if( bs != 0)
		explo( cty->pos );

	eoGuiSetCursor(tcur_spr, -32,-32 );

}

void satMouseEvent( engObj_s* sat, int bs )
{
		  eoGuiSetCursor(scur_spr, -32,-32 );
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


  vec3 p;
  p.x=0;
  p.y=10;
  p.z=0;
  eoCamPosSet(p);

  p.x=2;
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

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 33;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
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
