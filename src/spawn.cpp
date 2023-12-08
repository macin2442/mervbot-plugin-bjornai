#include "spawn.h"

#include "algorithms.h"

#if __linux__

uint16_t HIWORD(uint32_t x) {
	return (uint16_t)(x >> 16);
}

uint16_t LOWORD(uint32_t x) {
	return (uint16_t)x;
}

#define _declspec(x)

#endif

#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <math.h>
#include <time.h>

#pragma pack(1)
struct tilerecord
{
   unsigned int x     : 12;
   unsigned int y     : 12;
   unsigned int tile  : 8;
};

struct lvlheader
{
   unsigned short type; /* should be 19778, aka 'B' followed by 'M' */
   unsigned int length;
};
#pragma pack()


//////// Bot list ////////

_linkedlist <botInfo> botlist;

botInfo *findBot(CALL_HANDLE handle)
{
	_listnode <botInfo> *parse = botlist.head;

	while (parse)
	{
		botInfo *item = parse->item;

		if (item->validate(handle))
			return item;

		parse = parse->next;
	}

	return 0;
}


//////// DLL "import" ////////

void botInfo::tell(BotEvent event)
{
	if (callback && handle)
	{
		event.handle = handle;
		callback(event);
	}
}



bool closeto(Player *p, int x, int y, int tolerance)
{
	// return if player p is in area of square with center x,y and radius = tolerance
	return (abs((p->pos.x) - x) < tolerance) && (abs((p->pos.y) - y) < tolerance);
}


double Deg2Rad(float degrees)
{
   return degrees*TwoPI/360.0;
}

BYTE botInfo::oppositeDirection(BYTE d)
{
	return (d + 20) % 40;
}

BYTE botInfo::TriangulateFireAngle(const Vector &rel)
{
	double	dx = -rel.x,
			dy = rel.y;

	if (dy == 0)
		if (dx > 0)
			return 10;
		else
			return 30;

	double angle = atan(dx / dy) + PI;

	if (dy >= 0)
	{
		if (dx >= 0)
			angle -= PI;
		else
			angle += PI;
	}

	return BYTE(angle * 40.0 / TwoPI);
}

BYTE botInfo::TriangulateFireAngle(const Vector &pos, const Vector &vel, Sint32 scalar)
{
	// pos = relative positions
	// vel = relative velocities
	// scalar = velocity of bullets

	double a = vel.x * vel.x + vel.y * vel.y - scalar;
	double b = 2 * (pos.x * vel.x + pos.y * vel.y);
	double c = pos.x * pos.x + pos.y * pos.y;
	double time = (-b - sqrt((b * b) - (a * c * 2))) / (2.0 * a);
	if (time <= 0.0)
		   time = (-b + sqrt((b * b) - (a * c * 2))) / (2.0 * a);

	test=time*1000;

	return TriangulateFireAngle(Vector(pos.x + Sint32(double(vel.y) * time), pos.y + Sint32(double(vel.y) * time)));
}

void botInfo::movepred(Sint32 time, Player *p)
{
	predwork += ((predvel-p->vel) * time);
	predpos = predwork / 1000;
}

BYTE botInfo::Predict(Player *p, Sint32 scalar)
{
	Vector pred2;
	predpos=me->pos;
	predvel=me->vel;
	predwork=me->work;

	int bulletSpeed=scalar;

	int dist = p->pos.distance(me->pos);


	int time = (dist / (bulletSpeed/1000));

	//printf("%d\n",time);


	movepred(time,p);


	//printf("X:%d Y:%d\n",test.x,test.y);

	return BYTE(TriangulateFireAngle(p->pos - predpos));
}


void botInfo::move(Sint32 time, int p)
{
	proj[p].work += (proj[p].vel * time);

	proj[p].pos = proj[p].work / 1000;

	proj[p].lasttime=getTime();
}


void botInfo::Reset()
{
	coords[0].x=480;
	coords[0].y=540;

	coords[1].x=545;
	coords[1].y=575;

	coords[2].x=495;
	coords[2].y=450;

	coords[3].x=445;
	coords[3].y=480;

	coords[4].x=455;
	coords[4].y=540;

	coords[5].x=590;
	coords[5].y=560;

	coords[6].x=545;
	coords[6].y=540;

	coords[7].x=420;
	coords[7].y=450;

	coords[8].x=530;
	coords[8].y=480;

	coords[9].x=490;
	coords[9].y=490;
}



void botInfo::GetRandomPlayer()
{
	// get pilot p from a name as TempPlayer
	TempPlayer=NULL;

	int countplayers=0,getthis;
	int random = int(10.0 * rand()/(RAND_MAX+1.0));

	_listnode <Player> *parse = playerlist->head;

	while (parse)
	{
		Player *p = parse->item;

		// in freq data
		if (p->team==me->team && me!=p)
		{
		countplayers++;
		}

		parse = parse->next;
	}


	if (countplayers<random)
		getthis=countplayers;
	else
		getthis=random;

	countplayers=0;

	_listnode <Player> *parse2 = playerlist->head;

	while (parse2)
	{
		Player *p = parse2->item;

		// in freq data
		if (p->team==me->team && me!=p)
		{
			countplayers++;

			if (countplayers==getthis)
			{
				TempPlayer=p;
			}
		}

		parse2 = parse2->next;
	}
}



bool botInfo::GetPilotName(String name)
{
	// get pilot p from a name as TempPlayer
	_listnode <Player> *parse = playerlist->head;

	while (parse)
	{
		Player *p = parse->item;

		String compare=p->name;
		// in freq data
		if (name==compare)
		{
			me->target = p;
			return true;
		}

		parse = parse->next;
	}
	return false;
}


void botInfo::LoadINI()
{
	std::ifstream file("AI.ini");
	char line[256];
	while (file.getline(line, 256))
	{
		if (CMPSTART("Ship=", line))
			ship =  String(&line[5]).trim().toInteger();
		else if (CMPSTART("Team=", line))
			team =  String(&line[5]).trim().toInteger();
		else if (CMPSTART("X=", line))
			x =  String(&line[2]).trim().toInteger();
		else if (CMPSTART("Y=", line))
			y =  String(&line[2]).trim().toInteger();
		else if (CMPSTART("Active=", line))
			active =  String(&line[7]).trim().toInteger();
		else if (CMPSTART("Shooting=", line))
			shooting =  String(&line[9]).trim().toInteger();
		else if (CMPSTART("Spawnmode=", line))
			spawnmode =  String(&line[10]).trim().toInteger();
		else if (CMPSTART("DodgePredHit=", line))
			dodgenum =  String(&line[13]).trim().toInteger();
	}
	file.close();
}


void botInfo::gotEvent(BotEvent &event)
{
	if (CONNECTION_DENIED) return;

	/*for (int n = maxproj; n>0; n--)
	{
		move(GetTickCount()-proj[n].lasttime,n);
		printf("%d-%d\n",proj[n].pos.x,proj[n].pos.y);

		if (closeto(me, proj[n].pos.x, proj[n].pos.y, 50))
			sendFreqs("hit");
	}*/

	switch (event.code)
	{
//////// Periodic ////////
	case EVENT_Tick:
		{
			for (int i = 0; i < 4; ++i)
				--countdown[i];

			try
			{
				if (countdown[1]<-8)
				{
					if (me->dead && !botdead)
					{
						if (me->bounty<=0)
							me->bounty=50;
						tell(makeDeath(me->proj[me->killer].shooter));
						//tell(makePlayerDeath(me, me->proj[me->killer].shooter, 50, 0));
						me->energy=0;
						int delay=(settings->EnterDelay/100)+3;
						countdown[1]=delay;
						//printf("%d\n",delay);
						botdead=1;
					}
				}


				if (countdown[1]==1)
				{
					if (randomspawn)
					{
						int random = int(10.0 * rand()/(RAND_MAX+1.0));
						me->move(coords[random].x * 16, coords[random].y * 16);
					}
					else
						me->move(x * 16, y * 16);
				}

				if (countdown[1]==0)
				{
					botdead=0;
					me->dead=0;
					botenergy=settings->ships[me->ship].InitialEnergy;
					me->energy=botenergy;
				}

				if (attachspawn)
				{
					if (countdown[1]<-5 && closeto(me,512*16,512*16,1200) && countdown[2] < 0)
					{
						GetRandomPlayer();
						GetRandomPlayer();

						/*char test[10];
						strcpy(test,TempPlayer->name);
						printf(test);*/
						if (TempPlayer!=NULL)
						tell(makeAttach(TempPlayer));

						countdown[2]=2;
					}


					if (countdown[2]==1 && TempPlayer!=NULL)
					{
						me->move(TempPlayer->pos.x, TempPlayer->pos.y);
						me->vel=TempPlayer->vel;
					}

					if (countdown[2]==0 && TempPlayer!=NULL)
					{
						me->move(TempPlayer->pos.x, TempPlayer->pos.y);
						me->vel=TempPlayer->vel;
						tell(makeDetach());
					}
				}
				//RECHARGE
			}
			catch(...)
			{
				printf("\nERROR: Tick.");
			}
		}
		break;
//////// Arena ////////
	case EVENT_ArenaEnter:
		{
			arena = (char*)event.p[0];
			me = (Player*)event.p[1];	// if(me) {/*we are in the arena*/}
			bool biller_online = *(bool*)&event.p[2];

			sendPrivate(me,"*energy");
			//sendPublic("*nrg");
			sendPublic("*eletricgod");
			sendPublic("*hp");
		}
		break;
	case EVENT_ArenaSettings:
		{
			settings = (arenaSettings*)event.p[0];
		}
		break;
	case EVENT_ArenaLeave:
		{
			me = 0;
			killTags();
		}
		break;
	case EVENT_ArenaListEntry:
		{
			char *name = (char*)event.p[0];
			bool current = *(bool*)&event.p[1];
			int population = *(int*)&event.p[2];
		}
		break;
	case EVENT_ArenaListEnd:
		{
			char *name = (char*)event.p[0];
			bool current = *(bool*)&event.p[1];
			int population = *(int*)&event.p[2];
		}
		break;
//////// Flag ////////
	case EVENT_FlagGrab:
		{
			Player *p = (Player*)event.p[0];
			Flag *f = (Flag*)event.p[1];
		}
		break;
	case EVENT_FlagDrop:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_FlagMove:
		{
			Flag *f = (Flag*)event.p[0];
		}
		break;
	case EVENT_FlagVictory:
		{
			int team = *(int*)&event.p[0];
			int reward = *(int*)&event.p[1];
		}
		break;
	case EVENT_FlagGameReset:
		{
		}
		break;
	case EVENT_FlagReward:
		{
			int team = *(int*)&event.p[0];
			int reward = *(int*)&event.p[1];
		}
		break;
//////// Timed ////////
	case EVENT_TimedGameOver:
		{
			Player *p[5];
			for (int i = 0; i < 5; i++)
				p[i] = (Player*)event.p[i];
		}
		break;
//////// Soccer ////////
	case EVENT_SoccerGoal:
		{
			int team = *(int*)&event.p[0];
			int reward = *(int*)&event.p[1];
		}
		break;
	case EVENT_BallMove:
		{
			PBall *ball = (PBall*)event.p[0];
		}
		break;
//////// File ////////
	case EVENT_File:
		{
			char *name = (char*)event.p[0];
		}
		break;
//////// Player ////////
	case EVENT_PlayerEntering:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerMove:
		{
			Player *p = (Player*)event.p[0];

			try
			{
				if (me->dead && !botdead)
				{
					if (me->bounty<=0)
						me->bounty=50;
					tell(makeDeath(me->proj[me->killer].shooter));
					//tell(makePlayerDeath(me, me->proj[me->killer].shooter, 50, 0));
					me->energy=0;
					int delay=(settings->EnterDelay/100)+3;
					countdown[1]=delay;
					//printf("%d\n",delay);
					botdead=1;
				}
			}
			catch(...)
			{
				printf("\nERROR: Death (move).");
			}
		}
		break;
	case EVENT_PlayerWeapon:
		{
			Player *p = (Player*)event.p[0];
			weaponInfo wi;
			wi.n = *(Uint16*)&event.p[1];

			try
			{
				Uint32 dist = me->pos.distance(p->pos);

				if (dist < 3000 && !botdead)
				{
					if (dist<900)
						me->shooting=1;

					Vector bullet;
					int tempd = p->d;
					int degree=tempd*9;
					degree=degree-90 % 360;

					int doublebullets=1;
					int tempdegree;
					Vector temp,tempwork,temppos;
					tempwork=p->work;

					if (settings->ships[p->ship].DoubleBarrel && (wi.type==1 || wi.type==2))
						doublebullets=2;

					if (settings->ships[p->ship].EmpBomb)
						wi.emp=1;
					else
						wi.emp=0;


					for (int k=1;k<=doublebullets;k++)
					{

						if (wi.type==1 || wi.type==2)
						{
							bullet.x = settings->ships[p->ship].BulletSpeed * cos(Deg2Rad(degree));
							bullet.y = settings->ships[p->ship].BulletSpeed * sin(Deg2Rad(degree));
						}
						else if (wi.type==3 || wi.type==4)
						{
							bullet.x = settings->ships[p->ship].BombSpeed * cos(Deg2Rad(degree));
							bullet.y = settings->ships[p->ship].BombSpeed * sin(Deg2Rad(degree));
						}

						if (me->maxproj>8000000)
							me->maxproj=0;
						if (me->minproj>8000000)
							me->minproj=0;

						if (doublebullets==1)
						{
							me->proj[me->maxproj].pos.x=p->pos.x;
							me->proj[me->maxproj].pos.y=p->pos.y;
							me->proj[me->maxproj].work.x=p->work.x;
							me->proj[me->maxproj].work.y=p->work.y;
						}
						else
						{
							if (k==1)
								tempdegree=degree+90;
							else
								tempdegree=degree-90;

							temp.x = 18 * cos(Deg2Rad(tempdegree));
							temp.y = 18 * sin(Deg2Rad(tempdegree));
							tempwork += (temp * 1000);
							temppos = tempwork / 1000;

							me->proj[me->maxproj].pos.x=temppos.x;
							me->proj[me->maxproj].pos.y=temppos.y;
							me->proj[me->maxproj].work.x=tempwork.x;
							me->proj[me->maxproj].work.y=tempwork.y;
						}

						if ((wi.type==5) || ((wi.type==3 || wi.type==4) && wi.fireType==1))
						{
							me->proj[me->maxproj].vel.x=0;
							me->proj[me->maxproj].vel.y=0;
						}
						else
						{
							me->proj[me->maxproj].vel.x=p->vel.x;
							me->proj[me->maxproj].vel.y=p->vel.y;
							me->proj[me->maxproj].vel+=bullet;
						}

						if (wi.type==1 || wi.type==2)
							me->proj[me->maxproj].speed=settings->ships[p->ship].BulletSpeed;
						else if (wi.type==3 || wi.type==4)
						{
							me->proj[me->maxproj].speed=settings->ships[p->ship].BombSpeed;
							me->proj[me->maxproj].bounceCount=settings->ships[p->ship].BombBounceCount;
						}
						me->proj[me->maxproj].alivetime=getTime();
						me->proj[me->maxproj].lasttime=getTime();
						me->proj[me->maxproj].alive=true;
						me->proj[me->maxproj].shooter=p;
						me->proj[me->maxproj].type=wi.type;
						me->proj[me->maxproj].bouncing=wi.bouncing;
						me->proj[me->maxproj].level=wi.level;
						me->proj[me->maxproj].emp=wi.emp;
						me->proj[me->maxproj].fireType=wi.fireType;
						if (p->team == me->team)
							me->proj[me->maxproj].hostile=0;
						else
							me->proj[me->maxproj].hostile=1;

						me->maxproj++;
					}

					//DODGE PREDICTION
					int findnew = Predict(p,settings->ships[p->ship].BulletSpeed);

					int diff;
					if (dist>350)
						diff=0;
					else
						diff=1;

					if ((p->d >= (findnew-diff)) && (p->d <= (findnew+diff)))
					{
						if (getTime() - timenow < 100 && dist < 700)
						{
							me->hits++;
						}
						else if (dist < 700)
						{
							me->hits=1;
							timenow=getTime();
						}

						if (me->hits >= dodgenum && !me->dodge)
						{
							me->dodge=1;
						}
					}
				}
			}
			catch(...)
			{
				printf("\nERROR: Weapon handling.");
			}
		}
		break;
	case EVENT_WatchDamage:
		{
			Player *p = (Player*)event.p[0];	// player
			Player *k = (Player*)event.p[1];	// attacker
			weaponInfo wi;
			wi.n = *(Uint16*)&event.p[2];
			Uint16 energy = *(Uint16*)&event.p[3];
			Uint16 damage = *(Uint16*)&event.p[4];
		}
		break;
	case EVENT_PlayerDeath:
		{
			Player *p = (Player*)event.p[0],
				   *k = (Player*)event.p[1];
			Uint16 bounty = *(Uint16*)&event.p[2];
			Uint16 flags = *(Uint16*)&event.p[3];

			/*
			if (k->name == me->name)
			{
				int random = int(10.0 * rand()/(RAND_MAX+1.0));

				if (random == 0)
					sendPublic("ezpz");
				else if (random == 1)
					sendPublic("pwned ez gtfo!");
				else if (random == 2)
					sendPublic("your mom put up more resistance!");
				else if (random == 3)
					sendPublic("5-0! ez");
				else if (random == 4)
					sendPublic("can't even cheat to win.");
				else if (random == 5)
					sendPublic("get out of my kitchen!!");
				else if (random == 6)
					sendPublic("ole!.");
				else if (random == 7)
					sendPublic("now sit down and shut up!");
				else if (random == 8)
					sendPublic("lol, to easy!");
				else if (random == 9)
					sendPublic("wasting my time with you!");
			}


			if (p->name == me->name)
			{
				int random = int(10.0 * rand()/(RAND_MAX+1.0));

				if (random == 0)
					sendPublic("lagger");
				else if (random == 1)
					sendPublic("tank!");
				else if (random == 2)
					sendPublic("makron.exe!");
				else if (random == 3)
					sendPublic("gtfo!");
				else if (random == 4)
					sendPublic("luck");
				else if (random == 5)
					sendPublic("typekiller!!");
				else if (random == 6)
					sendPublic("that one doesnt count.");
				else if (random == 7)
					sendPublic("wasnt even looking");
				else if (random == 8)
					sendPublic("wtf!");
				else if (random == 9)
					sendPublic("bitch!");
			}*/
		}
		break;
	case EVENT_BannerChanged:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerScore:
		{
			Player *p = (Player*)event.p[0];
		}
		break;
	case EVENT_PlayerPrize:
		{
			Player *p = (Player*)event.p[0];
			int prize = *(int *)&event.p[1];
		}
		break;
	case EVENT_PlayerShip:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldship = *(Uint16*)&event.p[1];
			Uint16 oldteam = *(Uint16*)&event.p[2];

			if (p==me)
			{
				botenergy=settings->ships[me->ship].InitialEnergy;
				me->energy=botenergy;
			}

		}
		break;
	case EVENT_PlayerSpec:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = *(Uint16*)&event.p[1];
			Uint16 oldship = *(Uint16*)&event.p[2];
		}
		break;
	case EVENT_PlayerTeam:
		{
			Player *p = (Player*)event.p[0];
			Uint16 oldteam = *(Uint16*)&event.p[1];
			Uint16 oldship = *(Uint16*)&event.p[2];
		}
		break;
	case EVENT_PlayerLeaving:
		{
			Player *p = (Player*)event.p[0];

			killTags(p);
		}
		break;
//////// Selfish ////////
	case EVENT_SelfShipReset:
		{
		}
		break;
	case EVENT_SelfPrize:
		{
			Uint16 prize = *(Uint16*)&event.p[0];
			Uint16 count = *(Uint16*)&event.p[1];
		}
		break;
	case EVENT_SelfUFO:
		{
		}
		break;
	case EVENT_PositionHook:
		{
		}
		break;
//////// Bricks ////////
	case EVENT_BrickDropped:
		{
			int x1 = *(int*)&event.p[0];
			int y1 = *(int*)&event.p[1];
			int x2 = *(int*)&event.p[2];
			int y2 = *(int*)&event.p[3];
			int team = *(int*)&event.p[4];
		}
		break;
//////// LVZ ////////
	case EVENT_ObjectToggled:
		{
			objectInfo obj;
			obj.n = *(Uint16*)&event.p[0];
		}
		break;
//////// Turret ////////
	case EVENT_CreateTurret:
		{
			Player *turreter = (Player*)event.p[0];
			Player *turretee = (Player*)event.p[1];
		}
		break;
	case EVENT_DeleteTurret:
		{
			Player *turreter = (Player*)event.p[0];
			Player *turretee = (Player*)event.p[1];
		}
		break;
//////// Chat ////////
	case EVENT_Chat:
		{
			int type  = *(int*)&event.p[0];
			int sound = *(int*)&event.p[1];
			Player *p = (Player*)event.p[2];
			char *msg = (char*)event.p[3];

			switch (type)
			{
			case MSG_Arena:
				{
					String test=msg;
					if (test.left(18)=="Showing Energy OFF")
						sendPrivate(me,"*energy");
				}
				break;
			case MSG_PublicMacro:		if (!p) break;
				{
				}
				break;
			case MSG_Public:			if (!p) break;
				{
				}
				break;
			case MSG_Team:				if (!p) break;
				{
				}
				break;
			case MSG_TeamPrivate:		if (!p) break;
				{
				}
				break;
			case MSG_Private:			if (!p) break;
				{
				}
				break;
			case MSG_PlayerWarning:		if (!p) break;
				{
				}
				break;
			case MSG_RemotePrivate:
				{
				}
				break;
			case MSG_ServerError:
				{
				}
				break;
			case MSG_Channel:
				{
				}
				break;
			};
		}
		break;
	case EVENT_LocalCommand:
		{
			Player *p = (Player*)event.p[0];
			Command *c = (Command*)event.p[1];

			gotCommand(p, c);
		}
		break;
	case EVENT_LocalHelp:
		{
			Player *p = (Player*)event.p[0];
			Command *c = (Command*)event.p[1];

			gotHelp(p, c);
		}
		break;
	case EVENT_RemoteCommand:
		{
			char *p = (char*)event.p[0];
			Command *c = (Command*)event.p[1];
			Operator_Level o = *(Operator_Level*)&event.p[2];

			gotRemote(p, c, o);
		}
		break;
	case EVENT_RemoteHelp:
		{
			char *p = (char*)event.p[0];
			Command *c = (Command*)event.p[1];
			Operator_Level o = *(Operator_Level*)&event.p[2];

			gotRemoteHelp(p, c, o);
		}
		break;
//////// Containment ////////
	case EVENT_Init:
		{
			uint32_t major = HIWORD(*(uint32_t*)&event.p[0]);
			uint32_t minor = LOWORD(*(uint32_t*)&event.p[0]);
			callback	= (CALL_COMMAND)event.p[1];
			playerlist	= (CALL_PLIST)event.p[2];
			flaglist	= (CALL_FLIST)event.p[3];
			map			= (CALL_MAP)event.p[4];
			bricklist	= (CALL_BLIST)event.p[5];

			Reset();

			if (major > CORE_MAJOR_VERSION)
			{
				tell(makeEcho("DLL plugin cannot connect.  This plugin is out of date."));

				CONNECTION_DENIED = true;

				return;
			}
			else if ((major < CORE_MAJOR_VERSION) || (minor < CORE_MINOR_VERSION))
			{
				tell(makeEcho("DLL plugin cannot connect.  This plugin requires the latest version of MERVBot."));

				CONNECTION_DENIED = true;

				return;
			}
			else
				tell(makeEcho("DLL plugin connected."));

		}
		break;
	case EVENT_Term:
		{
			tell(makeEcho("DLL plugin disconnected."));
		}
		break;
	};
}


//////// DLL export ////////

_declspec(dllexport) void __stdcall talk(BotEvent &event)
{
	botInfo *bot;

	bot = findBot(event.handle);

	if (!bot)
	{
		bot = new botInfo(event.handle);
		botlist.append(bot);
	}

	if (bot) bot->gotEvent(event);
}


//////// Tag emulation ////////

int botInfo::get_tag(Player *p, int index)
{
	_listnode <PlayerTag> *parse = taglist.head;
	PlayerTag *tag;

	while (parse)
	{
		tag = parse->item;

		if (tag->p == p)
		if (tag->index == index)
			return tag->data;

		parse = parse->next;
	}

	return 0;
}

void botInfo::set_tag(Player *p, int index, int data)
{
	_listnode <PlayerTag> *parse = taglist.head;
	PlayerTag *tag;

	while (parse)
	{
		tag = parse->item;

		if (tag->p == p)
		if (tag->index == index)
		{
			tag->data = data;
			return;
		}

		parse = parse->next;
	}

	tag = new PlayerTag;
	tag->p = p;
	tag->index = index;
	tag->data = data;
	taglist.append(tag);
}

void botInfo::killTags(Player *p)
{
	_listnode <PlayerTag> *parse = taglist.head;
	PlayerTag *tag;

	while (parse)
	{
		tag = parse->item;
		parse = parse->next;

		if (tag->p != p) continue;

		taglist.kill(tag);
	}
}

void botInfo::killTags()
{
	taglist.clear();
}


//////// LVZ Object Toggling ////////

void botInfo::clear_objects()
{
	num_objects = 0;
}

void botInfo::object_target(Player *p)
{
	object_dest = p;
}

void botInfo::toggle_objects()
{
	Player *p = object_dest;

	if (!p)	tell(makeToggleObjects(UNASSIGNED, (Uint16 *)object_array, num_objects));
	else	tell(makeToggleObjects(p->ident, (Uint16 *)object_array, num_objects));

	num_objects = 0;
}

void botInfo::queue_enable(int id)
{
	if (num_objects == MAX_OBJECTS)
		toggle_objects();

	object_array[num_objects].id = id;
	object_array[num_objects].disabled = false;
	++num_objects;
}

void botInfo::queue_disable(int id)
{
	if (num_objects == MAX_OBJECTS)
		toggle_objects();

	object_array[num_objects].id = id;
	object_array[num_objects].disabled = true;
	++num_objects;
}


//////// Chatter ////////

void botInfo::sendFreqs(char *msg)
{
	String s = msg;

	s.prepend("*arena ",7);
	sendPublic(s);
}

void botInfo::sendPrivate(Player *player, BYTE snd, const char *msg)
{
	tell(makeSay(MSG_Private, snd, player->ident, msg));
}

void botInfo::sendPrivate(Player *player, const char *msg)
{
	tell(makeSay(MSG_Private, 0, player->ident, msg));
}

void botInfo::sendTeam(const char *msg)
{
	tell(makeSay(MSG_Team, 0, 0, msg));
}

void botInfo::sendTeam(BYTE snd, const char *msg)
{
	tell(makeSay(MSG_Team, snd, 0, msg));
}

void botInfo::sendTeamPrivate(Uint16 team, const char *msg)
{
	tell(makeSay(MSG_TeamPrivate, 0, team, msg));
}

void botInfo::sendTeamPrivate(Uint16 team, BYTE snd, const char *msg)
{
	tell(makeSay(MSG_TeamPrivate, snd, team, msg));
}

void botInfo::sendPublic(const char *msg)
{
	tell(makeSay(MSG_Public, 0, 0, msg));
}

void botInfo::sendPublic(BYTE snd, const char *msg)
{
	tell(makeSay(MSG_Public, snd, 0, msg));
}

void botInfo::sendPublicMacro(const char *msg)
{
	tell(makeSay(MSG_PublicMacro, 0, 0, msg));
}

void botInfo::sendPublicMacro(BYTE snd, const char *msg)
{
	tell(makeSay(MSG_PublicMacro, snd, 0, msg));
}

void botInfo::sendChannel(const char *msg)
{
	tell(makeSay(MSG_Channel, 0, 0, msg));
}

void botInfo::sendRemotePrivate(const char *msg)
{
	tell(makeSay(MSG_RemotePrivate, 0, 0, msg));
}

void botInfo::sendRemotePrivate(const char *name, const char *msg)
{
	String s;
	s += ":";
	s += name;
	s += ":";
	s += msg;

	sendRemotePrivate(s);
}
