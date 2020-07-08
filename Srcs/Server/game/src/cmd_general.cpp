#include "stdafx.h"
#ifdef __FreeBSD__
#include <md5.h>
#else
#include "../../libthecore/include/xmd5.h"
#endif

#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "dev_log.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "threeway_war.h"
#include "log.h"
#include "../../common/VnumHelper.h"
#ifdef __AUCTION__
#include "auction_manager.h"
#endif

extern int g_server_id;

extern int g_nPortalLimitTime;

ACMD(do_user_horse_ride)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{
		// ¸»ÀÌ ¾Æ´Ñ ´Ù¸¥Å»°ÍÀ» Å¸°íÀÖ´Ù.
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Å»°ÍÀ» ÀÌ¿ëÁßÀÔ´Ï´Ù."));
			return;
		}

		if (ch->GetHorse() == NULL)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != NULL)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» µ¹·Áº¸³Â½À´Ï´Ù."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡¼­ ¸ÕÀú ³»·Á¾ß ÇÕ´Ï´Ù."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
	}
}

ACMD(do_user_horse_feed)
{
	// °³ÀÎ»óÁ¡À» ¿¬ »óÅÂ¿¡¼­´Â ¸» ¸ÔÀÌ¸¦ ÁÙ ¼ö ¾ø´Ù.
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == NULL)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» Åº »óÅÂ¿¡¼­´Â ¸ÔÀÌ¸¦ ÁÙ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	DWORD dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô %s%s ÁÖ¾ú½À´Ï´Ù."), 
				ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName,
				g_iUseLocale ? "" : under_han(ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName) ? LC_TEXT("À»") : LC_TEXT("¸¦"));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¾ÆÀÌÅÛÀÌ ÇÊ¿äÇÕ´Ï´Ù"), ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int		subcmd;
	int         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo() 
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == NULL )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("¼Ë´Ù¿îÀÌ %dÃÊ ³²¾Ò½À´Ï´Ù."), *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), LC_TEXT("%dÃÊ ÈÄ °ÔÀÓÀÌ ¼Ë´Ù¿î µË´Ï´Ù."), iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	if (NULL == ch)
	{
		sys_err("Accept shutdown command from %s.", ch->GetName());
	}
	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );
	
	if ( info == NULL )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == NULL) { // <Factor>
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = NULL;

		if (true == LC_IsEurope() || true == LC_IsYMIR() || true == LC_IsKorea())
		{
			switch (info->subcmd)
			{
				case SCMD_LOGOUT:
				case SCMD_QUIT:
				case SCMD_PHASE_SELECT:
					{
						TPacketNeedLoginLogInfo acc_info;
						acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

						db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

						LogManager::instance().DetailLoginLog( false, ch );
					}
					break;
			}
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d)
					d->SetPhase(PHASE_CLOSE);
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ³²¾Ò½À´Ï´Ù."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	/* RECALL_DELAY
	   if (ch->m_pkRecallEvent != NULL)
	   {
	   ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãë¼Ò µÇ¾ú½À´Ï´Ù."));
	   event_cancel(&ch->m_pkRecallEvent);
	   return;
	   }
	// END_OF_RECALL_DELAY */

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãë¼Ò µÇ¾ú½À´Ï´Ù."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·Î±×ÀÎ È­¸éÀ¸·Î µ¹¾Æ °©´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°ÔÀÓÀ» Á¾·á ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ä³¸¯ÅÍ¸¦ ÀüÈ¯ ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;
	}

	int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()) &&
	   	(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}
	
	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

ACMD(do_mount)
{
	/*
	   char			arg1[256];
	   struct action_mount_param	param;

	// ÀÌ¹Ì Å¸°í ÀÖÀ¸¸é
	if (ch->GetMountingChr())
	{
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	return;

	param.x		= atoi(arg1);
	param.y		= atoi(arg2);
	param.vid	= ch->GetMountingChr()->GetVID();
	param.is_unmount = true;

	float distance = DISTANCE_SQRT(param.x - (DWORD) ch->GetX(), param.y - (DWORD) ch->GetY());

	if (distance > 600.0f)
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á» ´õ °¡±îÀÌ °¡¼­ ³»¸®¼¼¿ä."));
	return;
	}

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	return;
	}

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	return;

	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(atoi(arg1));

	if (!tch->IsNPC() || !tch->IsMountable())
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å±â¿¡´Â Å» ¼ö ¾ø¾î¿ä."));
	return;
	}

	float distance = DISTANCE_SQRT(tch->GetX() - ch->GetX(), tch->GetY() - ch->GetY());

	if (distance > 600.0f)
	{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á» ´õ °¡±îÀÌ °¡¼­ Å¸¼¼¿ä."));
	return;
	}

	param.vid		= tch->GetVID();
	param.is_unmount	= false;

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	 */
}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (NULL == ch->m_pkDeadEvent)
		return;

	int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		if (!test_server)
		{
			if (ch->IsHack())
			{
				//¼ºÁö ¸ÊÀÏ°æ¿ì¿¡´Â Ã¼Å© ÇÏÁö ¾Ê´Â´Ù.
				if (false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
					return;
				}
			}

			if (iTimeToDead > 170)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - 170);
				return;
			}
		}
	}

	//PREVENT_HACK
	//DESC : Ã¢°í, ±³È¯ Ã¢ ÈÄ Æ÷Å»À» »ç¿ëÇÏ´Â ¹ö±×¿¡ ÀÌ¿ëµÉ¼ö ÀÖ¾î¼­
	//		ÄðÅ¸ÀÓÀ» Ãß°¡ 
	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			//±æµå¸Ê, ¼ºÁö¸Ê¿¡¼­´Â Ã¼Å© ÇÏÁö ¾Ê´Â´Ù.
			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
			   	false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
		}

		if (iTimeToDead > 173)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ¸¶À»¿¡¼­ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%d ÃÊ ³²À½)"), iTimeToDead - 173);
			return;
		}
	}
	//END_PREVENT_HACK

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
	ch->StartRecoveryEvent();

	//FORKED_LOAD
	//DESC: »ï°Å¸® ÀüÅõ½Ã ºÎÈ°À» ÇÒ°æ¿ì ¸ÊÀÇ ÀÔ±¸°¡ ¾Æ´Ñ »ï°Å¸® ÀüÅõÀÇ ½ÃÀÛÁöÁ¡À¸·Î ÀÌµ¿ÇÏ°Ô µÈ´Ù.
	if (1 == quest::CQuestManager::instance().GetEventFlag("threeway_war"))
	{
		if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE)
		{
			if (true == CThreeWayWar::instance().IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
					false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->ReviveInvisible(5);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				return;
			}

			//¼ºÁö 
			if (true == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				if (CThreeWayWar::instance().GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºÁö¿¡¼­ ºÎÈ° ±âÈ¸¸¦ ¸ðµÎ ÀÒ¾ú½À´Ï´Ù! ¸¶À»·Î ÀÌµ¿ÇÕ´Ï´Ù!"));
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
				}
				else
				{
					ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
				}

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(5);

				return;
			}
		}
	}
	//END_FORKED_LOAD

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		DWORD dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
					break;

				case SCMD_RESTART_HERE:
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
					break;
			}

			return;
		}
	}

	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
			sys_log(0, "do_restart: restart town");
			PIXEL_POSITION pos;

			if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(1);
			break;

		case SCMD_RESTART_HERE:
			sys_log(0, "do_restart: restart here");
			ch->RestartAtSamePos();
			//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(0);
			ch->ReviveInvisible(5);
			break;
	}
}

#define MAX_STAT 90

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetPoint(POINT_STAT) <= 0)
		return;

	BYTE idx = 0;
	
	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -1);
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	if (ch->GetArena() != NULL || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	if (pkVictim->GetArena() != NULL)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ´ë·ÃÁßÀÔ´Ï´Ù."));
		return;
	}

	CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿¡ ¼ÓÇØÀÖÁö ¾Ê½À´Ï´Ù."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		DWORD vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå ½ºÅ³ ·¹º§À» º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

//
// @version	05/06/20 Bang2ni - Ä¿¸Çµå Ã³¸® Delegate to CHARACTER class
//
ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

//
// @version	05/06/20 Bang2ni - Ä¿¸Çµå Ã³¸® Delegate to CHARACTER class
//
ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	if (LC_IsBrazil() == true)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (arg2[i] == '\0')
				break;

			if (isalpha(arg2[i]) == false)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> ºñ¹Ð¹øÈ£´Â ¿µ¹®ÀÚ¸¸ °¡´ÉÇÕ´Ï´Ù."));
				return;
			}
		}
	}

	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í°¡ ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10) // 10ÃÊ¿¡ ÇÑ¹ø¸¸ ¿äÃ» °¡´É
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í¸¦ ´ÝÀºÁö 10ÃÊ ¾È¿¡´Â ¿­ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ ³ª°¥ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

ACMD(do_close_shop)
{
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	//³» ±æµå Á¤º¸¸¦ ¾ò¾î¿À°í
	CGuild * g = ch->GetGuild();

	if (!g)
		return;

	//ÀüÀïÁßÀÎÁö Ã¼Å©ÇÑ¹ø!
	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ´Ù¸¥ ÀüÀï¿¡ ÂüÀü Áß ÀÔ´Ï´Ù."));
		return;
	}

	//ÆÄ¶ó¸ÞÅÍ¸¦ µÎ¹è·Î ³ª´©°í
	char arg1[256], arg2[256];
	int type = GUILD_WAR_TYPE_FIELD;
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}

	//±æµåÀÇ ¸¶½ºÅÍ ¾ÆÀÌµð¸¦ ¾ò¾î¿ÂµÚ
	DWORD gm_pid = g->GetMasterPID();

	//¸¶½ºÅÍÀÎÁö Ã¼Å©(±æÀüÀº ±æµåÀå¸¸ÀÌ °¡´É)
	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	//»ó´ë ±æµå¸¦ ¾ò¾î¿À°í
	CGuild * opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	//»ó´ë±æµå¿ÍÀÇ »óÅÂ Ã¼Å©
	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
					return;
				}

				int iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Àüºñ°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ Àüºñ°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ¼±ÀüÆ÷°í ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀïÀÌ ¿¹¾àµÈ ±æµå ÀÔ´Ï´Ù."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀï ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{
		// ±æµåÀüÀ» ÇÒ ¼ö ÀÖ´Â Á¶°ÇÀ» ¸¸Á·ÇÏÁö¾Ê´Â´Ù.
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀüÀ» ÇÏ±â À§ÇØ¼± ÃÖ¼ÒÇÑ %d¸íÀÌ ÀÖ¾î¾ß ÇÕ´Ï´Ù."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	// ÇÊµåÀü Ã¼Å©¸¸ ÇÏ°í ¼¼¼¼ÇÑ Ã¼Å©´Â »ó´ë¹æÀÌ ½Â³«ÇÒ¶§ ÇÑ´Ù.
	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµå¿ø ¼ö°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != NULL)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(g->GetMasterPID());

		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != NULL)
			break;
		
		CCI *pCCI = P2P_MANAGER::instance().FindByPID(opp_g->GetMasterPID());
		
		if (pCCI != NULL)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	g->RequestDeclareWar(opp_g->GetID(), type);
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	ch->DetailLog();
}

ACMD(do_monsterlog)
{
	ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	BYTE mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);

	if (answer != 'y')
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ¸·Î ºÎÅÍ Ä£±¸ µî·ÏÀ» °ÅºÎ ´çÇß½À´Ï´Ù."), ch->GetName());
	}

	MessengerManager::instance().AuthToAdd(ch->GetName(), arg2, answer == 'y' ? false : true); // DENY
}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		BYTE flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding(); 
		}
	}
	else
	{
		ch->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("ÀÎº¥Åä¸®°¡ ²Ë Â÷¼­ ³»¸± ¼ö ¾ø½À´Ï´Ù."));
	}

}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(NULL);

		if (ch->GetArena() != NULL || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != NULL)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(NULL);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else
		{
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (ch->GetLevel() <= 14)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Diese Funktion steht dir ab Level 15 zu verfügung."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		DWORD vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;
		
		   int iSPCost = ch->GetMaxSP() / 3;

		   if (ch->GetSP() < iSPCost)
		   {
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Du hast nicht genug MP."));
				return;
		   }
		   ch->PointChange(POINT_SP, -iSPCost);
		 
		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ÆÄÆ¼¿¡ ¼ÓÇØ ÀÖÀ¸¹Ç·Î °¡ÀÔ½ÅÃ»À» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	DWORD vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

ACMD(do_monarch_warpto)
{
	if (true == LC_IsYMIR() || true == LC_IsKorea())
		return;

	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
		return;
	}
	
	//±ºÁÖ ÄðÅ¸ÀÓ °Ë»ç
	if (!ch->IsMCOK(CHARACTER::MI_WARP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_WARP));
		return;
	}

	//±ºÁÖ ¸÷ ¼ÒÈ¯ ºñ¿ë 
	const int WarpPrice = 10000;
	
	//±ºÁÖ ±¹°í °Ë»ç 
	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, WarpPrice);
		return;	
	}

	int x = 0, y = 0;
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë¹ý: warpto <character name>"));
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("Å¸Á¦±¹ À¯Àú¿¡°Ô´Â ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù"));
				return;
			}

			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç À¯Àú´Â %d Ã¤³Î¿¡ ÀÖ½À´Ï´Ù. (ÇöÀç Ã¤³Î %d)"), pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			PIXEL_POSITION pos;
	
			if (!SECTREE_MANAGER::instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
				ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", pkCCI->lMapIndex);
			else
			{
				//ch->ChatPacket(CHAT_TYPE_INFO, "You warp to (%d, %d)", pos.x, pos.y);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¿¡°Ô·Î ÀÌµ¿ÇÕ´Ï´Ù"), arg1);
				ch->WarpSet(pos.x, pos.y);
				
				//±ºÁÖ µ· »è°¨	
				CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

				//ÄðÅ¸ÀÓ ÃÊ±âÈ­ 
				ch->SetMC(CHARACTER::MI_WARP);
			}
		}
		else if (NULL == CHARACTER_MANAGER::instance().FindPC(arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
		}

		return;
	}
	else
	{
		if (tch->GetEmpire() != ch->GetEmpire())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Å¸Á¦±¹ À¯Àú¿¡°Ô´Â ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù"));
			return;
		}
		if (!IsMonarchWarpZone(tch->GetMapIndex()))
		{
			ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return;
		}
		x = tch->GetX();
		y = tch->GetY();
	}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¿¡°Ô·Î ÀÌµ¿ÇÕ´Ï´Ù"), arg1);
	ch->WarpSet(x, y);
	ch->Stop();

	//±ºÁÖ µ· »è°¨	
	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

	//ÄðÅ¸ÀÓ ÃÊ±âÈ­ 
	ch->SetMC(CHARACTER::MI_WARP);
}

ACMD(do_monarch_transfer)
{
	if (true == LC_IsYMIR() || true == LC_IsKorea())
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë¹ý: transfer <name>"));
		return;
	}
	
	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
		return;
	}
	
	//±ºÁÖ ÄðÅ¸ÀÓ °Ë»ç
	if (!ch->IsMCOK(CHARACTER::MI_TRANSFER))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_TRANSFER));	
		return;
	}

	//±ºÁÖ ¿öÇÁ ºñ¿ë 
	const int WarpPrice = 10000;

	//±ºÁÖ ±¹°í °Ë»ç 
	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, WarpPrice);
		return;	
	}


	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ Á¦±¹ À¯Àú´Â ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀº %d Ã¤³Î¿¡ Á¢¼Ó Áß ÀÔ´Ï´Ù. (ÇöÀç Ã¤³Î: %d)"), arg1, pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}
			if (!IsMonarchWarpZone(ch->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ» ¼ÒÈ¯ÇÏ¿´½À´Ï´Ù."), arg1);
			
			//±ºÁÖ µ· »è°¨	
			CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
			//ÄðÅ¸ÀÓ ÃÊ±âÈ­ 
			ch->SetMC(CHARACTER::MI_TRANSFER);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÔ·ÂÇÏ½Å ÀÌ¸§À» °¡Áø »ç¿ëÀÚ°¡ ¾ø½À´Ï´Ù."));
		}

		return;
	}


	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚ½ÅÀ» ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (tch->GetEmpire() != ch->GetEmpire())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ Á¦±¹ À¯Àú´Â ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	if (!IsMonarchWarpZone(tch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	if (!IsMonarchWarpZone(ch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());
	
	//±ºÁÖ µ· »è°¨	
	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
	//ÄðÅ¸ÀÓ ÃÊ±âÈ­ 
	ch->SetMC(CHARACTER::MI_TRANSFER);
}

ACMD(do_monarch_info)
{
	if (CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))	
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³ªÀÇ ±ºÁÖ Á¤º¸"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			if (n == ch->GetEmpire())
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  º¸À¯±Ý¾× %lld "), EMPIRE_NAME(n), p->name[n], p->money[n]);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  "), EMPIRE_NAME(n), p->name[n]);
				
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ Á¤º¸"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  "), EMPIRE_NAME(n), p->name[n]);
				
		}
	}
	
}	

ACMD(do_elect)
{
	db_clientdesc->DBPacketHeader(HEADER_GD_COME_TO_VOTE, ch->GetDesc()->GetHandle(), 0);
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	BYTE 	empire;
	int 	mapIndex;
	DWORD 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}

	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

extern void BroadcastNotice(const char * c_pszBuf);

ACMD(do_monarch_tax)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: monarch_tax <1-50>");
		return;
	}

	// ±ºÁÖ °Ë»ç	
	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ëÇÒ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}

	// ¼¼±Ý¼³Á¤ 
	int tax = 0;
	str_to_number(tax,  arg1);

	if (tax < 1 || tax > 50)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("1-50 »çÀÌÀÇ ¼öÄ¡¸¦ ¼±ÅÃÇØÁÖ¼¼¿ä"));

	quest::CQuestManager::instance().SetEventFlag("trade_tax", tax); 

	// ±ºÁÖ¿¡°Ô ¸Þ¼¼Áö ÇÏ³ª
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼¼±ÝÀÌ %d %·Î ¼³Á¤µÇ¾ú½À´Ï´Ù"));

	// °øÁö 
	char szMsg[1024];	

	snprintf(szMsg, sizeof(szMsg), "±ºÁÖÀÇ ¸íÀ¸·Î ¼¼±ÝÀÌ %d %% ·Î º¯°æµÇ¾ú½À´Ï´Ù", tax);
	BroadcastNotice(szMsg);

	snprintf(szMsg, sizeof(szMsg), "¾ÕÀ¸·Î´Â °Å·¡ ±Ý¾×ÀÇ %d %% °¡ ±¹°í·Î µé¾î°¡°ÔµË´Ï´Ù.", tax);
	BroadcastNotice(szMsg);

	// ÄðÅ¸ÀÓ ÃÊ±âÈ­ 
	ch->SetMC(CHARACTER::MI_TAX); 
}

static const DWORD cs_dwMonarchMobVnums[] =
{
	191, //	»ê°ß½Å
	192, //	Àú½Å
	193, //	¿õ½Å
	194, //	È£½Å
	391, //	¹ÌÁ¤
	392, //	ÀºÁ¤
	393, //	¼¼¶û
	394, //	ÁøÈñ
	491, //	¸ÍÈ¯
	492, //	º¸¿ì
	493, //	±¸ÆÐ
	494, //	ÃßÈç
	591, //	ºñ·ù´Ü´ëÀå
	691, //	¿õ±Í Á·Àå
	791, //	¹Ð±³±³ÁÖ
	1304, // ´©··¹ü±Í
	1901, // ±¸¹ÌÈ£
	2091, // ¿©¿Õ°Å¹Ì
	2191, // °Å´ë»ç¸·°ÅºÏ
	2206, // È­¿°¿Õi
	0,
};

ACMD(do_monarch_mob)
{
	char arg1[256];
	LPCHARACTER	tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ëÇÒ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}
	
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mmob <mob name>");
		return;
	}

	BYTE pcEmpire = ch->GetEmpire();
	BYTE mapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(ch->GetMapIndex());

	if (LC_IsYMIR() == true || LC_IsKorea() == true)
	{
		if (mapEmpire != pcEmpire && mapEmpire != 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚ±¹ ¿µÅä¿¡¼­¸¸ »ç¿ëÇÒ ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
			return;
		}
	}

	// ±ºÁÖ ¸÷ ¼ÒÈ¯ ºñ¿ë 
	const int SummonPrice = 5000000;

	// ±ºÁÖ ÄðÅ¸ÀÓ °Ë»ç
	if (!ch->IsMCOK(CHARACTER::MI_SUMMON))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_SUMMON));	
		return;
	}
	
	// ±ºÁÖ ±¹°í °Ë»ç 
	if (!CMonarch::instance().IsMoneyOk(SummonPrice, ch->GetEmpire()))
	{
		int NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, SummonPrice);
		return;	
	}

	const CMob * pkMob;
	DWORD vnum = 0;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == NULL)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	DWORD count;

	// ¼ÒÈ¯ °¡´É ¸÷ °Ë»ç
	for (count = 0; cs_dwMonarchMobVnums[count] != 0; ++count)
		if (cs_dwMonarchMobVnums[count] == vnum)
			break;

	if (0 == cs_dwMonarchMobVnums[count])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÈ¯ÇÒ¼ö ¾ø´Â ¸ó½ºÅÍ ÀÔ´Ï´Ù. ¼ÒÈ¯°¡´ÉÇÑ ¸ó½ºÅÍ´Â È¨ÆäÀÌÁö¸¦ ÂüÁ¶ÇÏ¼¼¿ä"));
		return;
	}

	tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum, 
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750), 
			ch->GetY() - number(200, 750), 
			ch->GetX() + number(200, 750), 
			ch->GetY() + number(200, 750), 
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE,
			true);

	if (tch)
	{
		// ±ºÁÖ µ· »è°¨	
		CMonarch::instance().SendtoDBDecMoney(SummonPrice, ch->GetEmpire(), ch);

		// ÄðÅ¸ÀÓ ÃÊ±âÈ­ 
		ch->SetMC(CHARACTER::MI_SUMMON); 
	}
}

static const char* FN_point_string(int apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("ÃÖ´ë »ý¸í·Â +%d");
		case POINT_MAX_SP:	return LC_TEXT("ÃÖ´ë Á¤½Å·Â +%d");
		case POINT_HT:		return LC_TEXT("Ã¼·Â +%d");
		case POINT_IQ:		return LC_TEXT("Áö´É +%d");
		case POINT_ST:		return LC_TEXT("±Ù·Â +%d");
		case POINT_DX:		return LC_TEXT("¹ÎÃ¸ +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("°ø°Ý¼Óµµ +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("ÀÌµ¿¼Óµµ %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("ÄðÅ¸ÀÓ -%d");
		case POINT_HP_REGEN:	return LC_TEXT("»ý¸í·Â È¸º¹ +%d");
		case POINT_SP_REGEN:	return LC_TEXT("Á¤½Å·Â È¸º¹ +%d");
		case POINT_POISON_PCT:	return LC_TEXT("µ¶°ø°Ý %d");
		case POINT_STUN_PCT:	return LC_TEXT("½ºÅÏ +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("½½·Î¿ì +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% È®·ü·Î Ä¡¸íÅ¸ °ø°Ý");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("»ó´ëÀÇ Ä¡¸íÅ¸ È®·ü %d%% °¨¼Ò");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% È®·ü·Î °üÅë °ø°Ý");
		case POINT_RESIST_PENETRATE: return LC_TEXT("»ó´ëÀÇ °üÅë °ø°Ý È®·ü %d%% °¨¼Ò");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("ÀÎ°£·ù ¸ó½ºÅÍ Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("µ¿¹°·ù ¸ó½ºÅÍ Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("¿õ±ÍÁ· Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("¹Ð±³·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("½ÃÃ¼·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("¾Ç¸¶·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("Å¸°ÝÄ¡ %d%% ¸¦ »ý¸í·ÂÀ¸·Î Èí¼ö");
		case POINT_STEAL_SP:		return LC_TEXT("Å¸·ÂÄ¡ %d%% ¸¦ Á¤½Å·ÂÀ¸·Î Èí¼ö");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% È®·ü·Î Å¸°Ý½Ã »ó´ë Àü½Å·Â ¼Ò¸ð");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% È®·ü·Î ÇÇÇØ½Ã Á¤½Å·Â È¸º¹");
		case POINT_BLOCK:			return LC_TEXT("¹°¸®Å¸°Ý½Ã ºí·° È®·ü %d%%");
		case POINT_DODGE:			return LC_TEXT("È° °ø°Ý È¸ÇÇ È®·ü %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("ÇÑ¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("¾ç¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("µÎ¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("¹æ¿ï ¹æ¾î %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("ºÎÃ¤ ¹æ¾î %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("È°°ø°Ý ÀúÇ× %d%%");
		case POINT_RESIST_FIRE:		return LC_TEXT("È­¿° ÀúÇ× %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("Àü±â ÀúÇ× %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("¸¶¹ý ÀúÇ× %d%%");
		case POINT_RESIST_WIND:		return LC_TEXT("¹Ù¶÷ ÀúÇ× %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("³Ã±â ÀúÇ× %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("´ëÁö ÀúÇ× %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("¾îµÒ ÀúÇ× %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("Á÷Á¢ Å¸°ÝÄ¡ ¹Ý»ç È®·ü : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("ÀúÁÖ µÇµ¹¸®±â È®·ü %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("µ¶ ÀúÇ× %d%%");
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã Á¤½Å·Â È¸º¹");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã °æÇèÄ¡ Ãß°¡ »ó½Â");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã µ· 2¹è µå·Ó");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã ¾ÆÀÌÅÛ 2¹è µå·Ó");
		case POINT_POTION_BONUS:	return LC_TEXT("¹°¾à »ç¿ë½Ã %d%% ¼º´É Áõ°¡");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã »ý¸í·Â È¸º¹");
//		case POINT_IMMUNE_STUN:	return LC_TEXT("±âÀýÇÏÁö ¾ÊÀ½ %d%%");
//		case POINT_IMMUNE_SLOW:	return LC_TEXT("´À·ÁÁöÁö ¾ÊÀ½ %d%%");
//		case POINT_IMMUNE_FALL:	return LC_TEXT("³Ñ¾îÁöÁö ¾ÊÀ½ %d%%");
//		case POINT_SKILL:	return LC_TEXT("");
//		case POINT_BOW_DISTANCE:	return LC_TEXT("");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("°ø°Ý·Â +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("¹æ¾î·Â +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("¸¶¹ý °ø°Ý·Â +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("¸¶¹ý ¹æ¾î·Â +%d");
//		case POINT_CURSE_PCT:	return LC_TEXT("");
		case POINT_MAX_STAMINA:	return LC_TEXT("ÃÖ´ë Áö±¸·Â +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("¹«»ç¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("ÀÚ°´¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("¼ö¶ó¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("¹«´ç¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("¸ó½ºÅÍ¿¡°Ô °­ÇÔ +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("°ø°Ý·Â +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("¹æ¾î·Â +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("°æÇèÄ¡ %d%%");
		case POINT_MALL_ITEMBONUS:		return LC_TEXT("¾ÆÀÌÅÛ µå·ÓÀ² %.1f¹è");
		case POINT_MALL_GOLDBONUS:		return LC_TEXT("µ· µå·ÓÀ² %.1f¹è");
		case POINT_MAX_HP_PCT:			return LC_TEXT("ÃÖ´ë »ý¸í·Â +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("ÃÖ´ë Á¤½Å·Â +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("½ºÅ³ µ¥¹ÌÁö %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("ÆòÅ¸ µ¥¹ÌÁö %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("½ºÅ³ µ¥¹ÌÁö ÀúÇ× %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("ÆòÅ¸ µ¥¹ÌÁö ÀúÇ× %d%%");
//		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT("");
//		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT("");
//		case POINT_EXTRACT_HP_PCT:	return LC_TEXT("");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("¹«»ç°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("ÀÚ°´°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_SURA:		return LC_TEXT("¼ö¶ó°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("¹«´ç°ø°Ý¿¡ %d%% ÀúÇ×");
		default:					return NULL;
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (NULL == ch || NULL == buf)
		return false;

	CAffect* aff = NULL;
	time_t expire = 0;
	struct tm ltm;
	int	year, mon, day;
	int	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (NULL == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	// set apply string
	offset = snprintf(buf, bufsiz, FN_point_string(aff->bApplyOn), aff->lApplyValue);

	if (offset < 0 || offset >= (int) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT(" (¸¸·áÀÏ : %d³â %d¿ù %dÀÏ)"), year, mon, day);

	return true;
}

ACMD(do_costume)
{
	char buf[512];
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);

	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				snprintf(buf, bufferSize, FN_point_string(attr.bType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int	index = 0;
	int	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
						index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

//gift notify quest command
ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	dev_log(LOG_DEB0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int cube_index = 0, inven_index = 0;
	const char *line;

	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info     ==> (Client -> Server) ÇöÀç NPC°¡ ¸¸µé ¼ö ÀÖ´Â ·¹½ÃÇÇ ¿äÃ»
	//					    (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3   ==> (Client -> Server) ÇöÀç NPC°¡ ¸¸µé¼ö ÀÖ´Â ·¹½ÃÇÇ Áß 3¹øÂ° ¾ÆÀÌÅÛÀ» ¸¸µå´Â µ¥ ÇÊ¿äÇÑ Á¤º¸¸¦ ¿äÃ»
	// /cube r_info 3 5 ==> (Client -> Server) ÇöÀç NPC°¡ ¸¸µé¼ö ÀÖ´Â ·¹½ÃÇÇ Áß 3¹øÂ° ¾ÆÀÌÅÛºÎÅÍ ÀÌÈÄ 5°³ÀÇ ¾ÆÀÌÅÛÀ» ¸¸µå´Â µ¥ ÇÊ¿äÇÑ Àç·á Á¤º¸¸¦ ¿äÃ»
	//					   (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o':	// open
			Cube_open(ch);
			break;

		case 'c':	// close
			Cube_close(ch);
			break;

		case 'l':	// list
			Cube_show_list(ch);
			break;

		case 'a':	// add cue_index inven_index
			{
				if (0 == arg2[0] || !isdigit(*arg2) ||
					0 == arg3[0] || !isdigit(*arg3))
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
				Cube_add_item (ch, cube_index, inven_index);
			}
			break;

		case 'd':	// delete
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item (ch, cube_index);
			}
			break;

		case 'm':	// make
			if (0 != arg2[0])
			{
				while (true == Cube_make(ch))
					dev_log (LOG_DEB0, "cube make success");
			}
			else
				Cube_make(ch);
			break;

		default:
			return;
	}
}

ACMD(do_in_game_mall)
{
	if (LC_IsYMIR() == true || LC_IsKorea() == true)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "mall http://metin2.co.kr/04_mall/mall/login.htm");
		return;
	}

	if (true == LC_IsTaiwan())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "mall http://203.69.141.203/mall/mall/item_main.htm");
		return;
	}

	// ¤Ð_¤Ð Äèµµ¼­¹ö ¾ÆÀÌÅÛ¸ô URL ÇÏµåÄÚµù Ãß°¡
	if (true == LC_IsWE_Korea())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "mall http://metin2.co.kr/50_we_mall/mall/login.htm");
		return;
	}

	if (LC_IsJapan() == true)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "mall http://mt2.oge.jp/itemmall/itemList.php");
		return;
	}
	
	if (LC_IsNewCIBN() == true && test_server)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "mall http://218.99.6.51/04_mall/mall/login.htm");
		return;
	}
	
	/*
	if (LC_IsCanada() == true)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "mall http://mall.z8games.com/mall_entry.aspx?tb=m2");
		return;
	}*/

	if (LC_IsEurope() == true)
	{
		char country_code[3];

		switch (LC_GetLocalType())
		{
			case LC_GERMANY:	country_code[0] = 'd'; country_code[1] = 'e'; country_code[2] = '\0'; break;
			case LC_FRANCE:		country_code[0] = 'f'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_ITALY:		country_code[0] = 'i'; country_code[1] = 't'; country_code[2] = '\0'; break;
			case LC_SPAIN:		country_code[0] = 'e'; country_code[1] = 's'; country_code[2] = '\0'; break;
			case LC_UK:			country_code[0] = 'e'; country_code[1] = 'n'; country_code[2] = '\0'; break;
			case LC_TURKEY:		country_code[0] = 't'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_POLAND:		country_code[0] = 'p'; country_code[1] = 'l'; country_code[2] = '\0'; break;
			case LC_PORTUGAL:	country_code[0] = 'p'; country_code[1] = 't'; country_code[2] = '\0'; break;
			case LC_GREEK:		country_code[0] = 'g'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_RUSSIA:		country_code[0] = 'r'; country_code[1] = 'u'; country_code[2] = '\0'; break;
			case LC_DENMARK:	country_code[0] = 'd'; country_code[1] = 'k'; country_code[2] = '\0'; break;
			case LC_BULGARIA:	country_code[0] = 'b'; country_code[1] = 'g'; country_code[2] = '\0'; break;
			case LC_CROATIA:	country_code[0] = 'h'; country_code[1] = 'r'; country_code[2] = '\0'; break;
			case LC_MEXICO:		country_code[0] = 'm'; country_code[1] = 'x'; country_code[2] = '\0'; break;
			case LC_ARABIA:		country_code[0] = 'a'; country_code[1] = 'e'; country_code[2] = '\0'; break;
			case LC_CZECH:		country_code[0] = 'c'; country_code[1] = 'z'; country_code[2] = '\0'; break;
			case LC_ROMANIA:	country_code[0] = 'r'; country_code[1] = 'o'; country_code[2] = '\0'; break;
			case LC_HUNGARY:	country_code[0] = 'h'; country_code[1] = 'u'; country_code[2] = '\0'; break;
			case LC_NETHERLANDS: country_code[0] = 'n'; country_code[1] = 'l'; country_code[2] = '\0'; break;
			case LC_USA:		country_code[0] = 'u'; country_code[1] = 's'; country_code[2] = '\0'; break;
			case LC_CANADA:	country_code[0] = 'c'; country_code[1] = 'a'; country_code[2] = '\0'; break;
			default:
				if (test_server == true)
				{
					country_code[0] = 'd'; country_code[1] = 'e'; country_code[2] = '\0';
				}
				break;
		}

		char buf[512+1];
		char sas[33];
		MD5_CTX ctx;
		const char sas_key[] = "GF9001";

		snprintf(buf, sizeof(buf), "%u%u%s", ch->GetPlayerID(), ch->GetAID(), sas_key);

		MD5Init(&ctx);
		MD5Update(&ctx, (const unsigned char *) buf, strlen(buf));
#ifdef __FreeBSD__
		MD5End(&ctx, sas);
#else
		static const char hex[] = "0123456789abcdef";
		unsigned char digest[16];
		MD5Final(digest, &ctx);
		int i;
		for (i = 0; i < 16; ++i) {
			sas[i+i] = hex[digest[i] >> 4];
			sas[i+i+1] = hex[digest[i] & 0x0f];
		}
		sas[i+i] = '\0';
#endif

		snprintf(buf, sizeof(buf), "mall http://%s/ishop?pid=%u&c=%s&sid=%d&sas=%s",
				g_strWebMallURL.c_str(), ch->GetPlayerID(), country_code, g_server_id, sas);

		ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
	}
}

// ÁÖ»çÀ§
ACMD(do_dice) 
{
	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int n = number(start, end);
	
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s´ÔÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ç½ÅÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), n, start, end);
}

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
    dev_log(LOG_DEB0, "[DO_RIDE] start");
    if (ch->IsDead() || ch->IsStun())
	return;

    // ³»¸®±â
    {
	if (ch->IsHorseRiding())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] stop riding");
	    ch->StopRiding(); 
	    return;
	}

	if (ch->GetMountVnum())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] unmount");
	    do_unmount(ch, NULL, 0, 0);
	    return;
	}
    }

    // Å¸±â
    {
	if (ch->GetHorse() != NULL)
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] start riding");
	    ch->StartRiding();
	    return;
	}

	for (BYTE i=0; i<INVENTORY_MAX_NUM; ++i)
	{
	    LPITEM item = ch->GetInventoryItem(i);
	    if (NULL == item)
		continue;

	    // À¯´ÏÅ© Å»°Í ¾ÆÀÌÅÛ
		if (item->IsRideItem())
		{
			if (NULL==ch->GetWear(WEAR_UNIQUE1) || NULL==ch->GetWear(WEAR_UNIQUE2))
			{
				dev_log(LOG_DEB0, "[DO_RIDE] USE UNIQUE ITEM");
				//ch->EquipItem(item);
				ch->UseItem(TItemPos (INVENTORY, i));
				return;
			}
		}

	    // ÀÏ¹Ý Å»°Í ¾ÆÀÌÅÛ
	    // TODO : Å»°Í¿ë SubType Ãß°¡
	    switch (item->GetVnum())
	    {
		case 71114:	// Àú½ÅÀÌ¿ë±Ç
		case 71116:	// »ê°ß½ÅÀÌ¿ë±Ç
		case 71118:	// ÅõÁö¹üÀÌ¿ë±Ç
		case 71120:	// »çÀÚ¿ÕÀÌ¿ë±Ç
		    dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
		    ch->UseItem(TItemPos (INVENTORY, i));
		    return;
	    }

		// GF mantis #113524, 52001~52090 ¹ø Å»°Í
		if( (item->GetVnum() > 52000) && (item->GetVnum() < 52091) )	{
			dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
			ch->UseItem(TItemPos (INVENTORY, i));
		    return;
		}
	}
    }


    // Å¸°Å³ª ³»¸± ¼ö ¾øÀ»¶§
    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
}

#ifdef __AUCTION__
// temp_auction
ACMD(do_get_item_id_list)
{
	for (int i = 0; i < INVENTORY_MAX_NUM; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != NULL)
			ch->ChatPacket(CHAT_TYPE_INFO, "name : %s id : %d", item->GetProto()->szName, item->GetID());
	}
}

// temp_auction

ACMD(do_enroll_auction)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	char arg4[256];
	two_arguments (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));
	
	DWORD item_id = strtoul(arg1, NULL, 10);
	BYTE empire = strtoul(arg2, NULL, 10);
	int bidPrice = strtol(arg3, NULL, 10);
	int immidiatePurchasePrice = strtol(arg4, NULL, 10);

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item == NULL)
		return;

	AuctionManager::instance().enroll_auction(ch, item, empire, bidPrice, immidiatePurchasePrice);
}

ACMD(do_enroll_wish)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	one_argument (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));
	
	DWORD item_num = strtoul(arg1, NULL, 10);
	BYTE empire = strtoul(arg2, NULL, 10);
	int wishPrice = strtol(arg3, NULL, 10);

	AuctionManager::instance().enroll_wish(ch, item_num, empire, wishPrice);
}

ACMD(do_enroll_sale)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	one_argument (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));
	
	DWORD item_id = strtoul(arg1, NULL, 10);
	DWORD wisher_id = strtoul(arg2, NULL, 10);
	int salePrice = strtol(arg3, NULL, 10);

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item == NULL)
		return;

	AuctionManager::instance().enroll_sale(ch, item, wisher_id, salePrice);
}

// temp_auction
// packetÀ¸·Î Åë½ÅÇÏ°Ô ÇÏ°í, ÀÌ°Ç »èÁ¦ÇØ¾ßÇÑ´Ù.
ACMD(do_get_auction_list)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	two_arguments (one_argument (argument, arg1, sizeof(arg1)), arg2, sizeof(arg2), arg3, sizeof(arg3));

	AuctionManager::instance().get_auction_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10), strtoul(arg3, NULL, 10));
}
//
//ACMD(do_get_wish_list)
//{
//	char arg1[256];
//	char arg2[256];
//	char arg3[256];
//	two_arguments (one_argument (argument, arg1, sizeof(arg1)), arg2, sizeof(arg2), arg3, sizeof(arg3));
//
//	AuctionManager::instance().get_wish_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10), strtoul(arg3, NULL, 10));
//}
ACMD (do_get_my_auction_list)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_my_auction_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_get_my_purchase_list)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_my_purchase_list (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_auction_bid)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().bid (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_auction_impur)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().immediate_purchase (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_get_auctioned_item)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_auctioned_item (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_buy_sold_item)
{
	char arg1[256];
	char arg2[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().get_auctioned_item (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_cancel_auction)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_auction (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_cancel_wish)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_wish (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_cancel_sale)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_sale (ch, strtoul(arg1, NULL, 10));
}

ACMD (do_rebid)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().rebid (ch, strtoul(arg1, NULL, 10), strtoul(arg2, NULL, 10));
}

ACMD (do_bid_cancel)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().bid_cancel (ch, strtoul(arg1, NULL, 10));
}
#endif
