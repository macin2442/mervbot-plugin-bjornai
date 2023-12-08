/*
	MERVBot Spawn-callback by Catid@pacbell.net
*/

#ifndef SPAWN_H
#define SPAWN_H

#include "dllcore.h"
#include "clientprot.h"
#include "pluginmeta.h"

struct PlayerTag
{
	Player *p;
	int index;
	int data;
};

#define MAX_OBJECTS 20

class botInfo
{
	bool CONNECTION_DENIED;

	_linkedlist <PlayerTag> taglist;
	int get_tag(Player *p, int index);
	void set_tag(Player *p, int index, int data);
	void killTags(Player *p);
	void killTags();

	CALL_HANDLE handle;
	CALL_COMMAND callback;
	CALL_PLIST playerlist;
	CALL_FLIST flaglist;
	CALL_MAP map;
	CALL_BLIST bricklist;
	char *arena;
	arenaSettings *settings;
	Player *me;
	bool biller_online;

	objectInfo object_array[MAX_OBJECTS];
	int num_objects;
	Player *object_dest;

	Player *TempPlayer;

	int countdown[4];

	Vector predpos;
	Vector predvel;
	Vector predwork;


	struct projectiles
	{
		Vector pos;
		Vector vel;
		Vector work;
		int speed;
		Uint32 alivetime;
		Uint32 lasttime;
		bool alive;

		weaponInfo wi;
	};

	projectiles proj[11000];


	struct Warp
	{
	int x;
	int y;
	};

	Warp coords[10];

	int ypos;
	int maxproj;
	int minproj;


	int botenergy;
	bool botdead;

	double lasthit;


	int test;
	bool randomspawn;
	bool attachspawn;

	int timenow;


	//INI variables

	int ship;
	int team;
	int x,y;
	bool active;
	bool shooting;
	int spawnmode;
	int dodgenum;


	// Put bot data here


public:
	botInfo(CALL_HANDLE given)
	{
		handle = given;
		callback = 0;
		playerlist = 0;
		flaglist = 0;
		map = 0;
		countdown[0] = 0;
		countdown[1] = 0;
		countdown[2] = 0;
		countdown[3] = 0;
		CONNECTION_DENIED = false;
		me = 0;
		biller_online = true;
		num_objects = 0;
		object_dest = NULL;


		botenergy=850;
		botdead=0;

		lasthit=0;

		maxproj=0;
		minproj=0;

		ypos=0;

		randomspawn=1;
		attachspawn=0;

		timenow=0;





		// Put initial values here
	}

	void clear_objects();
	void object_target(Player *p);
	void toggle_objects();
	void queue_enable(int id);
	void queue_disable(int id);

	void gotEvent(BotEvent &event);

	void tell(BotEvent event);

	void move(Sint32 time, int proj);
	void GetRandomPlayer();
	void Reset();
	void movepred(Sint32 time,Player *p);
	BYTE Predict(Player *p, Sint32 scalar);	// use prediction

	void LoadINI();
	bool GetPilotName(String name);

	BYTE TriangulateFireAngle(const Vector &rel);
	BYTE TriangulateFireAngle(const Vector &pos, const Vector &vel, Sint32 scalar);
	BYTE oppositeDirection(BYTE d);


	void sendFreqs(char *msg);

	bool validate(CALL_HANDLE given) { return given == handle; }

	void sendPrivate(Player *player, const char *msg);
	void sendPrivate(Player *player, BYTE snd, const char *msg);

	void sendTeam(const char *msg);
	void sendTeam(BYTE snd, const char *msg);

	void sendTeamPrivate(Uint16 team, const char *msg);
	void sendTeamPrivate(Uint16 team, BYTE snd, const char *msg);

	void sendPublic(const char *msg);
	void sendPublic(BYTE snd, const char *msg);

	void sendPublicMacro(const char *msg);
	void sendPublicMacro(BYTE snd, const char *msg);

	void sendChannel(const char *msg);			// #;Message
	void sendRemotePrivate(const char *msg);		// :Name:Messsage
	void sendRemotePrivate(const char *name, const char *msg);

	void gotHelp(Player *p, Command *c);
	void gotCommand(Player *p, Command *c);
	void gotRemoteHelp(char *p, Command *c, Operator_Level l);
	void gotRemote(char *p, Command *c, Operator_Level l);
};


// The botlist contains every bot to ever be spawned of this type,
// some entries may no longer exist.

extern _linkedlist <botInfo> botlist;

botInfo *findBot(CALL_HANDLE handle);


#endif	// SPAWN_H
