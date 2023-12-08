#include "spawn.h"

#include "algorithms.h"

#include "time.h"
#include <fstream>
#include <string>
#include <iomanip>

#include <math.h>

double Deg2Rad2(float degrees)
{
	return degrees * TwoPI / 360.0;
}

void botInfo::gotHelp(Player *p, Command *c)
{
	// List commands
	if (!*c->final)
	{
		switch (p->access)
		{
		case OP_Duke:
		case OP_Baron:
		case OP_King:
		case OP_Emperor:
		case OP_RockStar:
		case OP_Q:
		case OP_God:
		case OP_Owner:
			{	// Owner-level commands
//				sendPrivate(p, "Ext: ");
			}
		case OP_SysOp:	/* FALL THRU */
			{	// SysOp-level commands
//				sendPrivate(p, "Ext: ");
			}
		case OP_SuperModerator:	/* FALL THRU */
			{	// SuperModerator-level commands
//				sendPrivate(p, "Ext: ");
			}
		case OP_Moderator:	/* FALL THRU */
			{	// Moderator-level commands
//				sendPrivate(p, "Ext: ");
			}
		case OP_Limited:	/* FALL THRU */
			{	// Limited-level commands
				sendPrivate(p, "Ext: !start !active0 !active1 !shoot0 !shoot1");
			}
		case OP_Player:	/* FALL THRU */
			{	// Player-level commands
				sendPrivate(p, "Ext: !about");
			}
		}

		return;
	}

	// Specific command help

	switch (p->access)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		{	// Owner-level commands
			if (c->checkParam("owner") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_SysOp:
		{	// SysOp-level commands
			if (c->checkParam("sop") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
			if (c->checkParam("smod") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_Moderator:
		{	// Moderator-level commands
			if (c->checkParam("mod") || c->checkParam("all"))
			{
//				sendPrivate(p, "Ext: ");
			}
		}
	case OP_Limited:
		{	// Limited-level commands
			if (c->checkParam("start"))
			{
				sendPrivate(p, "!start (starts the bot if sent twice)");
			}
			else if (c->checkParam("active0"))
			{
				sendPrivate(p, "!active0 (deactivates the bot)");
			}
			else if (c->checkParam("active1"))
			{
				sendPrivate(p, "!active1 (activates the bot)");
			}
			else if (c->checkParam("shoot0"))
			{
				sendPrivate(p, "!shoot0 (deactivates the shooting)");
			}
			else if (c->checkParam("shoot1"))
			{
				sendPrivate(p, "!shoot1 (activates the shooting)");
			}
		}
	case OP_Player:
		{	// Player-level commands
			if (c->checkParam("about"))
			{
				sendPrivate(p, "!about (query me about my function)");
			}
		}
	}
}

void botInfo::gotCommand(Player *p, Command *c)
{
	if (!p) return;
	if (!c) return;

	switch (p->access)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		{	// Owner-level commands
		}
	case OP_SysOp:
		{	// SysOp-level commands
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
			if (c->check("version"))
			{
				String s = "[name:";
				s += plugin_name;
				s += "] [maker:";
				s += plugin_author;
				s += "] [build:";
				s += plugin_version;
				s += "]";
				sendPrivate(p, s);
			}
		}
	case OP_Moderator:
		{	// Moderator-level commands
		}
	case OP_Limited:
		{	// Limited-level commands
			if (c->check("active0"))
			{
				me->active = 0;
			}
			if (c->check("active1"))
			{
				me->active = 1;
			}
			if (c->check("shoot0"))
			{
				me->shooting = 0;
			}
			if (c->check("shoot1"))
			{
				me->shooting = 1;
			}
			if (c->check("target"))
			{
				/*String name=c->final;

				if (GetPilotName(name))
				{
				sendPrivate(p,"Target set!");
				}
				else
				{
				sendPrivate(p,"Target not found!");
				me->target=NULL;
				}*/

				sendPrivate(p, "This command is disabled");
			}

			if (c->check("test"))
			{
				String test = " X:";
				test += p->pos.x;
				test += "  Y:";
				test += p->pos.y;
				sendFreqs(test);

				Vector temp, tempwork, temppos;
				tempwork = p->work;

				int tempd = p->d;
				int degree = tempd * 9;
				degree = degree - 90 % 360;
				degree = degree + 90;

				temp.x = 18 * cos(Deg2Rad2(degree));
				temp.y = 18 * sin(Deg2Rad2(degree));

				tempwork += (temp * 1000);
				temppos = tempwork / 1000;

				test = "lX:";
				test += temppos.x;
				test += " lY:";
				test += temppos.y;
				sendFreqs(test);
			}
			if (c->check("start"))
			{
				LoadINI();
				// me->target=NULL;

				tell(makeShip((ship - 1)));
				tell(makeTeam(team));

				me->active = active;
				me->shooting = shooting;

				if (spawnmode == 1)
				{
					attachspawn = 0;
					randomspawn = 1;
				}
				else if (spawnmode == 2)
				{
					attachspawn = 1;
					randomspawn = 0;
				}
				else
				{
					attachspawn = 0;
					randomspawn = 0;
				}

				countdown[1] = -1;

				tell(makeSendPosition(false));
				tell(makeFollowing(false));
				tell(makeFlying(false));

				me->move(x * 16, y * 16);
			}
		}
	case OP_Player:
		{	// Player-level commands
			if (c->check("about"))
			{
				sendPrivate(p, "AI 1.0 by Bjorn");
			}
		}
	}
}

void botInfo::gotRemoteHelp(char *p, Command *c, Operator_Level l)
{
	// List commands

	if (!*c->final)
	{
		switch (l)
		{
		case OP_Duke:
		case OP_Baron:
		case OP_King:
		case OP_Emperor:
		case OP_RockStar:
		case OP_Q:
		case OP_God:
		case OP_Owner:
			{	// Owner-level commands
			}
		case OP_SysOp:
			{	// SysOp-level commands
			}
		case OP_SuperModerator:
			{	// SuperModerator-level commands
			}
		case OP_Moderator:
			{	// Moderator-level commands
			}
		case OP_Limited:
			{	// Limited-level commands
			}
		case OP_Player:
			{	// Player-level commands
			}
		}

		return;
	}

	// Specific command help

	switch (l)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		{	// Owner-level commands
		}
	case OP_SysOp:
		{	// SysOp-level commands
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
		}
	case OP_Moderator:
		{	// Moderator-level commands
		}
	case OP_Limited:
		{	// Limited-level commands
		}
	case OP_Player:
		{	// Player-level commands
		}
	}
}

void botInfo::gotRemote(char *p, Command *c, Operator_Level l)
{
	if (!c) return;

	switch (l)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		{	// Owner-level commands
		}
	case OP_SysOp:
		{	// SysOp-level commands
		}
	case OP_SuperModerator:
		{	// SuperModerator-level commands
		}
	case OP_Moderator:
		{	// Moderator-level commands
		}
	case OP_Limited:
		{	// Limited-level commands
		}
	case OP_Player:
		{	// Player-level commands
		}
	}
}
