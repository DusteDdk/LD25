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
guiContext* wonContext;
sprite_s* tcur_spr; //"Target" cursor
sprite_s* scur_spr; //"Select" cursor
sprite_s* dcur_spr; //"default" cursor
sprite_s* click_spr;
engObj_s* clickObj;

engObj_s* selSat=NULL;
engObj_s* selCty=NULL;

sound_s* strangeSnd;
sound_s* laserFireSnd;
sound_s* explSnd;

guiLabel_s* satCdLbl[3];
guiLabel_s* scoreLbl;

engObj_s* goatObj;
vboModel* goatMdl;
vboModel* ctyMdl;

engObj_s* helpCity;

int helpState=0;
int score;

void ctyColFunc( engObj_s* cty, engObj_s* nme );
void btnMainMenuCb( void* unused );
void ctyMouseEvent( engObj_s* cty, int bs );


typedef struct {
	vec3 target;	//Target direction
	int coolDown;	//Time left to it can fire again
	int fireNow;	//Fire asap!
	char* txtStatus;//Status txt label
} satState_s;

satState_s satS[3];

#define AIS_MOVING_TO_TARGET 1
#define AIS_LOOKING_FOR_TARGET 2

typedef struct {
	listItem* targets;
	int coolDown;
	GLfloat rotation;
	GLfloat rotationSpeed;
	GLfloat targetRot;
	vec3 lastTargetPos;
	vec3 newTargetPos;
	int state;
	GLfloat rotDir;
	GLfloat aisMspeed;
} aiState_s;

aiState_s aiS;

void btnClbQuit(void* notused)
{
  eoExec("quit 1");
}

void btnClbStartGame(void* notused)
 {
	eoSamplePlay(strangeSnd,128);
	score=0;
	eoExec("campos 32.7 15.72 45.43");
	eoExec("camlook 32.7 17.72 20.43");
	eoGuiContextSet( ingameContext );
	  int i;
	  for(i=0; i < 6; i++ )
	  {
		  engObj_s* cty = eoObjCreate( ENGOBJ_MODEL );
		  cty->colTeam = COLTEAM_CITIES;
		  cty->model = ctyMdl;
		  cty->colFunc = ctyColFunc;
		  if(i==0)
		  {
			  helpCity=cty;
		  }
		  cty->pos.x = i*10;
		  if( i > 2 ) cty->pos.x +=16;
		  cty->clickedFunc=ctyMouseEvent;
		  eoObjBake( cty );
		  eoObjAdd( cty );
      cty->rot.y = (GLfloat) ((rand()%90)-45);
	  }

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

void goatThink( engObj_s* g )
{
	//Rotate the shiny goat!
	static GLfloat rot = 0;
	rot += 0.01;
	g->rot.y = rot*57.29;
	g->pos.y = 34.267868+ sin( rot );

  g->rot.x += 0.012;
  g->rot.z += 0.006;

}

void removeMissile( engObj_s* m)
{
	if(!m->deleteMe)
	{
		listRemoveByData( aiS.targets, (void*)m );
		eoObjDel(m);
	}
}

void ctyColFunc( engObj_s* cty, engObj_s* nme )
{
	eoSamplePlay( explSnd, 128 );
	eoObjDel(cty);
	removeMissile(nme);
	explo( cty->pos );
	score++;
}


void frameStart()
{
	eoGuiSetCursor(dcur_spr, 0,0 );
	sprintf(scoreLbl->txt, "%i kills", score);

	if( goatObj )
	{
		if(goatObj->gameData==(void*)1)
		{
			//Kill the goat
			eoObjDel(goatObj);
			//Haha nasty, return to main menu here..
			goatObj=NULL;
			btnMainMenuCb(NULL);
		}
	}

	if( helpState==1 )
	{
		clickObj->pos=helpCity->pos;
		clickObj->pos.x += 15;
		clickObj->pos.z += 5;
		clickObj->pos.y += 4;
	}

	if( score == 6 )
	{
		//eoPauseSet(1);
		score=0;
		eoGuiContextSet( wonContext );


		eoExec("campos -11.694969 36.751045 -246.860229");
		eoExec("camlook -1.942898 31.639606 -224.415405");

		//Declare the goat!
		goatObj = eoObjCreate( ENGOBJ_MODEL );
		goatObj->model = goatMdl;
		goatObj->thinkFunc = goatThink;
    goatObj->colTeam = 123;
		eoObjBake(goatObj);
		eoObjAdd(goatObj);
		goatObj->pos.x = -10.660823;
		goatObj->pos.y = 34.267868;
		goatObj->pos.z = -239.942154;

	}
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
		removeMissile(m);
	}
	if( abs(m->pos.x) > 100 || m->pos.y > 100 )
	{
		if( m->colTeam != COLTEAM_GOODPC )
		{
			eoPrint("Bug: Missile %i should not be here..",m->id);
		} else {
			eoObjDel(m);
		}
	}
}

//only for missiles shot from cannon
void mCol( engObj_s* m, engObj_s* nme )
{
	//We were hit by the damn ai..
	explo(m->pos);
	removeMissile(nme);
	eoObjDel(m);
	eoSamplePlay( explSnd, 128  );

}

void spawnMissile( int colTeam, vec3 startPos, vec3 direction )
{
	engObj_s* m = eoObjCreate( ENGOBJ_PAREMIT );

	particleEmitter_s* emitter = eoPsysNewEmitter();
	emitter->addictive=1;
	emitter->numParticlesPerEmission = 4;
	emitter->ticksBetweenEmissions = 1;
	emitter->particleLifeMax = 100;
	emitter->particleLifeVariance = 80;
	emitter->shrink=0;
	emitter->fade=0;
	emitter->percentFlicker=70;
	emitter->sizeMax=0.07;
	emitter->sizeVariance=0.03;
	emitter->rotateParticles=0;
	emitter->colorVariance[0]=0.4;
	emitter->colorVariance[1]=0.0;
	emitter->colorVariance[2]=0.3;
	emitter->colorVariance[3]=0.0;
	emitter->color[0]=0.9;
	emitter->color[1]=0.0;
	emitter->color[2]=0.4;
	emitter->emitSpeedMax=0.1;
	emitter->emitSpeedVariance=0.1;
	m->emitter = emitter;
	m->colTeam=colTeam;
	m->pos=startPos;
	m->vel=eoVec3Scale( eoVec3Normalize(direction), 0.2);
	m->thinkFunc=mThink;

	if( colTeam==COLTEAM_EVILRAYS )
	{
		listAddData( aiS.targets, (void*)m );
	} else {
		emitter->colorVariance[0]=0.4;
		emitter->colorVariance[1]=0.3;
		emitter->colorVariance[2]=0.1;
		emitter->colorVariance[3]=0.0;
		emitter->color[0]=0.7;
		emitter->color[1]=0.3;
		emitter->color[2]=0.1;
		emitter->shrink=1;
		emitter->sizeMax=0.12;
		emitter->particleLifeMax = 250;
		emitter->numParticlesPerEmission = 16;
		m->vel=eoVec3Scale( eoVec3Normalize(direction), aiS.aisMspeed);
		m->colFunc=mCol;
	}

	eoPsysBake(emitter);
	eoObjBake(m);
	eoObjAdd(m);

}

void satThink( engObj_s* sat )
{
	//We won't change velocity if it's too extreme
	int i;
		GLfloat change = ((float)((rand()%10000)-5000))/1000000;

		if( (sat->pos.y > 45.5 && change > 0) || (sat->pos.y < 45.3 && change < 0 ))
			change = -change;

		if( (sat->pos.y > 45.7 && change > 0) || (sat->pos.y < 45.1 && change < 0 ))
		{
			sat->vel.y = 0;
		}

		sat->vel.y += change;

		satState_s* s = (satState_s*)sat->gameData;

		vec3 v = eoVec3FromPoints( sat->pos, s->target  );
		sat->rot.z = atan2( v.y, v.x ) * 57.29+90;

		if(s->coolDown > 0)
		{
			s->fireNow=0;
			 s->coolDown -= eoTicks();
			sprintf( s->txtStatus, "Charging %i left...", s->coolDown/1000+1);
		} else {
			for(i=0; i < 3;i++)
			{
				if( &satS[i]==s)
					sprintf( s->txtStatus, "Satellite %i Ready!", i);

			}
		}

		if(s->fireNow && s->coolDown < 1)
		{
			s->coolDown=5000;
			eoSamplePlay(laserFireSnd, 128);
			selCty=0;
			s->fireNow=0;
			spawnMissile( COLTEAM_EVILRAYS, sat->pos, v );
		}
		if( helpState== 0)
		{
			if(clickObj->pos.y == 1000 )
			{
				clickObj->pos=sat->pos;
				clickObj->pos.x += 15;
			}

		}

}

void aiLaunchMissile( GLfloat rot, vec3 pos )
{
	vec3 v;
	v.x = cos( (90+rot)/57.29 );
	v.y = sin( (90+rot)/57.29 );
	v.z = 0;

	spawnMissile(COLTEAM_GOODPC, pos, v );
}

void aiThink( engObj_s* top )
{
	//State is in aiS

	//Check if the target we are moving to is still the closest

return;

	//See which target would require least delta in Rotation.

	GLfloat bestDelta=360;
	GLfloat reqRot;
	GLfloat delta;
	GLfloat doRot;
	vec3 v;
	vec3 futureP;

	listItem* it=aiS.targets;
	engObj_s* obj;
	while( (it=it->next) )
	{
		obj=(engObj_s*)it->data;
		futureP = eoVec3Add( obj->pos, eoVec3Scale(obj->vel, 10 ));
		v = eoVec3FromPoints( top->pos, futureP );
		reqRot = atan2( v.y, v.x ) * 57.29 - 90;
		delta = abs(reqRot )+aiS.rotation;
		if( delta < bestDelta )
		{
			bestDelta = delta;
			doRot=reqRot;
			aiS.state = 1;
		}
	}
	if( bestDelta == 360 )
	{
		aiS.state = 0;
	}

	if( abs(doRot) < 60 )
	{
		aiS.targetRot=doRot;
		if( doRot < 0 )
			aiS.rotDir = -1;
		else
			aiS.rotDir = 1;

	}


	if( aiS.coolDown > 0 )
	{
		aiS.coolDown -= eoTicks();
	}

	vec3 len;
	len.x=cos( (aiS.rotation+90)/57.29)*10;
	len.z=0;
	len.y=sin( (aiS.rotation+90)/57.29)*10;
	if( aiS.rotDir < 0 )
	{
		if( aiS.rotation < aiS.targetRot )
		{
			//Fire!
			aiS.rotDir=0;
			if( bestDelta != 360 && aiS.coolDown < 1)
			{
				aiS.coolDown=300;
				aiLaunchMissile( aiS.rotation, eoVec3Add( top->pos, len ) );
			}
		}
	} else {
		if( aiS.rotation > aiS.targetRot )
		{
			aiS.rotDir=0;
			//Fiiiire!
			if( bestDelta != 360 && aiS.coolDown < 1)
			{
				aiS.coolDown=300;
				aiLaunchMissile( aiS.rotation, eoVec3Add( top->pos, len ) );
			}
		}
	}

	if(aiS.state)
		aiS.rotation += aiS.rotationSpeed*aiS.rotDir;

	top->rot.z = aiS.rotation;
}

void ctyMouseEvent( engObj_s* cty, int bs )
{

	if( bs == 1 && selSat)
	{
		if( helpState == 1 )
		{
			helpState++;
			eoObjDel(clickObj);
		}
		selCty=cty;

		satState_s* s = (satState_s*)selSat->gameData;
		s->target=selCty->pos;
		if( s->coolDown < 1 )
		{
			s->fireNow=1;
			selSat=NULL;
		}
	}

	eoGuiSetCursor(tcur_spr, -32,-32 );

}

void satMouseEvent( engObj_s* sat, int bs )
{
	if( bs == 1)
	{
		if( helpState == 0 )
		{
			helpState++;
		}
		selSat=sat;
	}

	eoGuiSetCursor(scur_spr, -32,-32 );
}

void btnGoat( void* unused )
{
	goatObj->gameData=(void*)1;
}

void btnMainMenuCb( void* unused )
{
	eoPauseSet(1);
	eoGuiContextSet( menuContext );

	  vec3 p;
	  p.x=43.838181;
	  p.y=20.039286;
	  p.z=-94.781746;
	  eoCamPosSet(p);

	  p.x=19.156969;
	  p.y=21.724607;
	  p.z=-98.386940;

	  eoCamTargetSet( p );


}


int main(int argc, char *argv[])
{

  eoInitAll(argc, argv, DATADIR);

  //Enable mouse-selection
  eoGameEnableMouseSelection(0.1);

  //Load the target cursor
  sprite_base* tcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor-target.spr"));
  sprite_base* scur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor-select.spr"));
  sprite_base* dcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor.spr"));

  tcur_spr = eoSpriteNew( tcur_sprb, 1, 1 );
  scur_spr = eoSpriteNew( scur_sprb, 1, 1 );
  dcur_spr = eoSpriteNew( dcur_sprb, 1, 1 );

  sprite_base* click_sprb = eoSpriteBaseLoad( Data("/data/gfx/", "click.spr") );
  click_spr = eoSpriteNew( click_sprb, 1, 1 );


  goatObj=NULL;

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


  //The "you won" screen...
  wonContext = eoGuiContextCreate();
  guiWindow_s* winWin= eoGuiAddWindow(wonContext, eoSetting()->res.x/2-100, eoSetting()->res.y/2+300, 200,100, "No title",0);
  winWin->showTitle=0;
  eoGuiAddLabel( winWin,0,0, "You won the game, great job.\nTake a goat, it's shiney.");
  eoGuiAddButton( wonContext, 10, 10, 100,50, "Thanks, let me go!", btnGoat );

  //Ingame HUD

  //The Exit button
  eoGuiAddButton( ingameContext, eoSetting()->res.x - 160, eoSetting()->res.y - 60, 150, 50, "Main Menu", btnMainMenuCb );

  satCdLbl[0] = eoGuiAddLabel(ingameContext, eoSetting()->res.x/2-300, eoSetting()->res.y-60, "Satellite 1 Ready!!");
  satCdLbl[1] = eoGuiAddLabel(ingameContext, eoSetting()->res.x/2-100, eoSetting()->res.y-60, "Satellite 2 Ready!!");
  satCdLbl[2] = eoGuiAddLabel(ingameContext, eoSetting()->res.x/2+100, eoSetting()->res.y-60, "Satellite 3 Ready!!");

  satCdLbl[0]->font=FONT_LARGE;
  satCdLbl[1]->font=FONT_LARGE;
  satCdLbl[2]->font=FONT_LARGE;

  scoreLbl = eoGuiAddLabel(ingameContext, eoSetting()->res.x/2,10, "          GOAT           ");
  scoreLbl->font=FONT_LARGE;
  scoreLbl->fontPos=TXT_CENTER;

  strangeSnd = eoSampleLoad(Data("/data/sound/","muwah.wav"));
  laserFireSnd =eoSampleLoad( Data("/data/sound/","laser0.wav"));
  explSnd = eoSampleLoad( Data("/data/sound/","expl0.wav") );


  vec3 p;
  p.x=43.838181;
  p.y=20.039286;
  p.z=-94.781746;
  eoCamPosSet(p);

  p.x=19.156969;
  p.y=21.724607;
  p.z=-98.386940;

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

  ctyMdl = eoModelLoad( "data/objs/", "cty.obj");
  vboModel* canBaseMdl = eoModelLoad( "data/objs/", "canbase.obj" );
  vboModel* canTopMdl = eoModelLoad( "data/objs/", "cantop.obj");
  vboModel* satMdl = eoModelLoad( "data/objs/", "sat.obj");


  vboModel* scapeMdl = eoModelLoad( "data/objs/", "scape.obj");
  vboModel* starsMdl = eoModelLoad( "data/objs/", "stars.obj" );
  vboModel* psyMdl = eoModelLoad( "data/objs/", "psy.obj" );

  goatMdl = eoModelLoad( "data/objs/", "goat.obj" );



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
  canObj->thinkFunc=aiThink;
  eoObjBake(canObj );
  eoObjAdd( canObj );

  memset( &aiS, 0, sizeof(aiState_s));
  aiS.targets = initList();
  aiS.rotationSpeed = 0.87;
  aiS.aisMspeed=0.85;

  //Sats
  memset( satS, 0, sizeof( satState_s) );

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 0;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
  canObj->gameData=&satS[0];
  satS[0].txtStatus=satCdLbl[0]->txt;
  eoObjBake(canObj );
  eoObjAdd( canObj );

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 33;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
  canObj->gameData=&satS[1];
  satS[1].txtStatus=satCdLbl[1]->txt;
  eoObjBake(canObj );
  eoObjAdd( canObj );

  canObj = eoObjCreate( ENGOBJ_MODEL );
  canObj->model = satMdl;
  canObj->pos.x = 66;
  canObj->pos.y = 45.4;
  canObj->clickedFunc=satMouseEvent;
  canObj->thinkFunc=satThink;
  canObj->gameData=&satS[2];
  satS[2].txtStatus=satCdLbl[2]->txt;
  eoObjBake(canObj );
  eoObjAdd( canObj );


  clickObj = eoObjCreate( ENGOBJ_SPRITE );
  clickObj->sprite = click_spr;
  clickObj->pos.y = 1000;

  eoObjBake( clickObj );
  eoObjAdd( clickObj );
  click_spr->scale.x=0.08;
  click_spr->scale.y=0.08;


  eoMainLoop();

  return 0;
}
