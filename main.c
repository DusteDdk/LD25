#include <stdio.h>
#include <stdlib.h>

#include "libeo/eng.h"

#ifndef DATADIR
  #define DATADIR "."
#endif

void btnClbQuit(void* notused)
{
  eoExec("quit 1");
}

void btnClbStartGame(void* notused)
 {
   eoExec("level 1");
 }

int conClbLoadLevel( const char* arg, void* unused )
{
  eoPrint("I was asked to load level number %s", arg);
  //eoExec("camfree 1");
  eoExec("testbox 1");
  //eoExec("campos -10 0 0");
  eoExec("camlook 0 0 0");
  //eoExec("echo Hello World");
  eoGuiHide();
  return(CON_CALLBACK_HIDE_RETURN_VALUE);
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
	static GLfloat rot=0;
	rot += 0.001;
	vec3 p;
	p.y = 16.0;
	p.x = sin( rot )*70.0;
	p.z = cos( rot )*70.0;
	eoCamPosSet( p );
}

int main(int argc, char *argv[])
{

  eoInitAll(argc, argv, DATADIR);

  //Register the "level" console command to load game levels.
  eoFuncAdd( conClbLoadLevel,NULL, "level" );


  //Setup a window so we can Start Game or Exit.
  guiContext* winRoot = eoGuiContextCreate();
  eoGuiContextSet(winRoot);

  guiWindow_s* winMainMenu = eoGuiAddWindow( winRoot, eoSetting()->res.x/2-100, eoSetting()->res.y/2-45, 200,90, "Where am I ?",0 );
  //Add button to exit the game.
  eoGuiAddButton(winMainMenu,GUI_POS_CENTER,5, 190, 20, "Start!", btnClbStartGame );
  eoGuiAddButton(winMainMenu,GUI_POS_CENTER,35, 190, 20, "Quit..", btnClbQuit );

  eoGuiShow();

  eoRegisterStartFrameFunc( frameStart );
  //Load stuff
  sprite_base* bugspic = eoSpriteBaseLoad( Data("/data/gfx/", "bugs.spr") );
  sprite_s* spr = eoSpriteNew( bugspic, 0,0  );

  //Add to game world
  engObj_s* bugs = eoObjCreate( ENGOBJ_SPRITE );
  bugs->sprite = spr;
  eoObjBake( bugs );
 // eoObjAdd( bugs );


  int i=0;
  engObj_s* bouncey;
  for(i=0; i <50; i++ )
  {
	  bouncey = eoObjCreate( ENGOBJ_PAREMIT );
	  bouncey->emitter = eoPsysNewEmitter();
	  bouncey->emitter->addictive=1;
	  bouncey->emitter->numParticlesPerEmission = 2;
	  bouncey->emitter->ticksBetweenEmissions = 1;
	  bouncey->emitter->particleLifeMax = 1000;
	  bouncey->emitter->particleLifeVariance = 60;
	  bouncey->emitter->shrink=1;
	  bouncey->emitter->sizeMax=0.06;
	  bouncey->emitter->sizeVariance=0;
	  bouncey->emitter->rotateParticles=0;
	  bouncey->emitter->colorVariance[0]=1.0;
	  bouncey->emitter->colorVariance[1]=1.0;
	  bouncey->emitter->colorVariance[2]=1.0;
	  bouncey->emitter->colorVariance[3]=0.0;
	  bouncey->emitter->emitSpeedMax=1;
	  bouncey->emitter->emitSpeedVariance=1;
	  eoPsysBake(bouncey->emitter);

	  bouncey->pos.x=0;
	  bouncey->pos.y=0;
	  bouncey->pos.z=0;

	  bouncey->vel.x = ((float)(rand()%30))/100.0;
	  bouncey->vel.y = ((float)(rand()%30))/100.0;
	  bouncey->vel.z = ((float)(rand()%30))/100.0;

	  bouncey->thinkFunc = _bthink;

	  eoObjBake( bouncey );
	  eoObjAdd(bouncey);
  }

  bouncey = eoObjCreate( ENGOBJ_MODEL );
  bouncey->model = eoModelLoad( "data/objs/", "untitled.obj" );
  bouncey->thinkFunc = _bthink;
  eoObjBake(bouncey);
  eoObjAdd(bouncey);



//  eoPsysEmit( )

  eoMainLoop();

  return 0;
}
