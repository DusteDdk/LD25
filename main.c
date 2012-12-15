#include <stdio.h>
#include <stdlib.h>

#include "libeo/eng.h"

#ifndef DATADIR
  #define DATADIR "."
#endif

#define COLTEAM_CITIES 1
#define COLTEAM_EVILRAYS 2
#define COLTEAM_GOODPC 3

void btnClbQuit(void* notused)
{
  eoExec("quit 1");
}

void btnClbStartGame(void* notused)
 {
	eoExec("campos 32.7 15.72 45.43");
	eoExec("camlook 32.7 17.72 20.43");
	eoGuiHide();
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

//width=20, height=10,length=30;
void _bthink(engObj_s* b)
{
	if( b->pos.x > 20 || b->pos.x < -20 )
	{
		b->vel.x *= -1;
		explo( b->pos );
	}

	if( b->pos.y > 10 || b->pos.y < -10 )
	{
		b->vel.y *= -1;
		explo( b->pos );
	}

	if( b->pos.z > 30 || b->pos.z < -30 )
	{
		b->vel.z *= -1;
		explo( b->pos );
	}
}

void frameStart()
{

}

void starsThink( engObj_s* stars )
{
	stars->pos = eoCamPosGet();
	stars->rot.x += 0.02;
	stars->rot.z += 0.06;
}

void psyThink( engObj_s* psy )
{
	psy->pos = eoCamPosGet();
	psy->rot.x -= 0.042;
	psy->rot.z -= 0.05;
	psy->rot.y += 0.07;
}

int main(int argc, char *argv[])
{

  eoInitAll(argc, argv, DATADIR);

  //Load the target cursor
  sprite_base* tcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor-target.spr"));
  sprite_s* tcur_spr = eoSpriteNew( tcur_sprb, 1, 1 );
  eoGuiSetCursor(tcur_spr, -32,-32 );

  //Setup a window so we can Start Game or Exit.
  guiContext* winRoot = eoGuiContextCreate();
  eoGuiContextSet(winRoot);

  guiWindow_s* winMainMenu = eoGuiAddWindow( winRoot, eoSetting()->res.x/2-100, eoSetting()->res.y/2-45, 200,90, "killCity",0 );
  //Add button to exit the game.
  eoGuiAddButton(winMainMenu,GUI_POS_CENTER,5, 190, 20, "Start!", btnClbStartGame );
  eoGuiAddButton(winMainMenu,GUI_POS_CENTER,35, 190, 20, "Quit..", btnClbQuit );

  vec3 p;
  p.x=1;
  p.y=1;
  p.z=1;
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


  int i;
  for(i=0; i < 6; i++ )
  {
	  engObj_s* cty = eoObjCreate( ENGOBJ_MODEL );
	  cty->colTeam = COLTEAM_CITIES;
	  cty->model = ctyMdl;
	  cty->colFunc = ctyColFunc;

	  cty->pos.x = i*10;
	  if( i > 2 ) cty->pos.x +=16;

	  eoObjBake( cty );
	  eoObjAdd( cty );
  }

  eoMainLoop();

  return 0;
}
