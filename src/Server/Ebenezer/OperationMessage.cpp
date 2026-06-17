#include "pch.h"
#include "OperationMessage.h"
#include "EbenezerApp.h"
#include "EbenezerResourceFormatter.h"
#include "User.h"
#include "db_resources.h"

#include <djb2/djb2_hasher.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

#include <ctime>
#include <sstream>
#include <stdexcept>

namespace Ebenezer
{

extern bool g_serverdown_flag;

OperationMessage::OperationMessage(EbenezerApp* main, CUser* srcUser) :
	_main(main), _srcUser(srcUser)
{
}

bool OperationMessage::Process(const std::string_view command)
{
	size_t key = 0;
	if (!ParseCommand(command, key))
		return false;

	try
	{
		switch (key)
		{
#if 0 // TODO
			case "+pursue"_djb2:
				Pursue();
				break;

			case "+actpursue"_djb2:
				ActPursue();
				break;

			case "+monpursue"_djb2:
				MonPursue();
				break;

			case "+moncatch"_djb2:
				MonCatch();
				break;

			case "+assault"_djb2:
				Assault();
				break;

			case "+monsummon"_djb2:
				MonSummon();
				break;

			case "+monsummonall"_djb2:
				MonSummonAll();
				break;

			case "+unikmonster"_djb2:
				UnikMonster();
				break;

			case "+user_seek_report"_djb2:
				UserSeekReport();
				break;

			case "+monkill"_djb2:
				MonKill();
				break;
#endif

			case "/open"_djb2:
			case "+open"_djb2:
				Open();
				break;

#if 0 // TODO
			case "/open2"_djb2:
			case "+open2"_djb2:
				Open2();
				break;

			case "/open3"_djb2:
			case "+open3"_djb2:
				Open3();
				break;

			case "/mopen"_djb2:
			case "+mopen"_djb2:
				MOpen();
				break;

			case "+forbiduser"_djb2:
				ForbidUser();
				break;

			case "+forbidconnect"_djb2:
				ForbidConnect();
				break;
#endif

			case "/snowopen"_djb2:
			case "+snowopen"_djb2:
				SnowOpen();
				break;

			case "/close"_djb2:
			case "+close"_djb2:
				Close();
				break;

			case "/captain"_djb2:
			case "+captain"_djb2:
				Captain();
				break;

#if 0 // TODO
			case "/tiebreak"_djb2:
			case "+tiebreak"_djb2:
				TieBreak();
				break;

			case "/auto"_djb2:
			case "+auto"_djb2:
				Auto();
				break;

			case "/auto_off"_djb2:
			case "+auto_off"_djb2:
				AutoOff();
				break;
#endif

			case "/down"_djb2:
			case "+down"_djb2:
				Down();
				break;

			case "/discount"_djb2:
			case "+discount"_djb2:
				Discount();
				break;

#if 0 // TODO
			case "/freediscount"_djb2:
			case "+freediscount"_djb2:
				FreeDiscount();
				break;
#endif

			case "/alldiscount"_djb2:
			case "+alldiscount"_djb2:
				AllDiscount();
				break;

			case "/undiscount"_djb2:
			case "+undiscount"_djb2:
				UnDiscount();
				break;

			case "/santa"_djb2:
			case "+santa"_djb2:
				Santa();
				break;

			case "/angel"_djb2:
			case "+angel"_djb2:
				Angel();
				break;

			case "/offsanta"_djb2:
			case "+offsanta"_djb2:
				OffSanta();
				break;

#if 0 // TODO
			case "/limitbattle"_djb2:
			case "+limitbattle"_djb2:
				LimitBattle();
				break;

			case "/onsummonblock"_djb2:
			case "+onsummonblock"_djb2:
				OnSummonBlock();
				break;

			case "/offsummonblock"_djb2:
			case "+offsummonblock"_djb2:
				OffSummonBlock();
				break;
#endif

			// +zonechange: {int: zoneId} [float: x] [float: z]
			// NOTE: Coordinates are unofficial.
			case "+zonechange"_djb2:
				ZoneChange();
				break;

#if 0 // TODO
			case "+siegewarfare"_djb2:
				SiegeWarfare();
				break;

			case "+resetsiegewar"_djb2:
				ResetSiegeWar();
				break;

			case "+siegewarschedule-start"_djb2:
				SiegeWarScheduleStart();
				break;

			case "+siegewarschedule-end"_djb2:
				SiegeWarScheduleEnd();
				break;

			case "+siegewar_base_report"_djb2:
				SiegeWarBaseReport();
				break;

			case "+siegewar_status_report"_djb2:
				SiegeWarStatusReport();
				break;

			case "+siegewar_check_base"_djb2:
				SiegeWarCheckBase();
				break;

			case "/server_testmode"_djb2:
			case "+server_testmode"_djb2:
				ServerTestMode();
				break;

			case "/server_normalmode"_djb2:
			case "+server_normalmode"_djb2:
				ServerNormalMode();
				break;

			case "+merchant_money"_djb2:
				MerchantMoney();
				break;

			case "+siegewar_punish_knights"_djb2:
				SiegeWarPunishKnights();
				break;

			case "+siegewar_load_table"_djb2:
				SiegeWarLoadTable();
				break;

			case "/money_add"_djb2:
			case "+money_add"_djb2:
				MoneyAdd();
				break;

			case "/exp_add"_djb2:
			case "+exp_add"_djb2:
				ExpAdd();
				break;

			case "+user_bonus"_djb2:
				UserBonus();
				break;

			case "/discount1"_djb2:
			case "+discount1"_djb2:
				Discount1();
				break;

			case "/discount2"_djb2:
			case "+discount2"_djb2:
				Discount2();
				break;

			case "/battle1"_djb2:
			case "+battle1"_djb2:
				Battle1();
				break;

			case "/battle2"_djb2:
			case "+battle2"_djb2:
				Battle2();
				break;

			case "/battle3"_djb2:
			case "+battle3"_djb2:
				Battle3();
				break;

			case "/battle_auto"_djb2:
			case "+battle_auto"_djb2:
				BattleAuto();
				break;

			case "+battle_report"_djb2:
				BattleReport();
				break;

			case "/challenge_on"_djb2:
			case "+challenge_on"_djb2:
				ChallengeOn();
				break;

			case "/challenge_off"_djb2:
			case "+challenge_off"_djb2:
				ChallengeOff();
				break;

			case "/challenge_kill"_djb2:
			case "+challenge_kill"_djb2:
				ChallengeKill();
				break;

			case "/challenge_level"_djb2:
			case "+challenge_level"_djb2:
				ChallengeLevel();
				break;

			case "/rental_report"_djb2:
			case "+rental_report"_djb2:
				RentalReport();
				break;

			case "/rental_stop"_djb2:
			case "+rental_stop"_djb2:
				RentalStop();
				break;

			case "/rental_start"_djb2:
			case "+rental_start"_djb2:
				RentalStart();
				break;

			case "/king_report1"_djb2:
			case "+king_report1"_djb2:
				KingReport1();
				break;

			case "/king_report2"_djb2:
			case "+king_report2"_djb2:
				KingReport2();
				break;

			case "/reload_king"_djb2:
			case "+reload_king"_djb2:
				ReloadKing();
				break;
#endif

			case "/kill"_djb2:
				Kill();
				break;

#if 0
			case "/reload_notice"_djb2:
				ReloadNotice();
				break;

			case "/reload_hacktool"_djb2:
				ReloadHacktool();
				break;

			case "/serverdown"_djb2:
				ServerDown();
				break;

			case "/writelog"_djb2:
				WriteLog();
				break;

			case "/eventlog"_djb2:
				EventLog();
				break;

			case "/eventlog_off"_djb2:
				EventLogOff();
				break;

			case "/itemdown"_djb2:
				ItemDown();
				break;

			case "/itemdownreset"_djb2:
				ItemDownReset();
				break;

			case "/challengestop"_djb2:
				ChallengeStop();
				break;
#endif

			case "/permanent"_djb2:
				Permanent();
				break;

			case "/offpermanent"_djb2:
				OffPermanent();
				break;

			// +give_item itemId [count] — GM item oluşturma (Release'de de etkin; authority gerekir)
			case "+give_item"_djb2:
				GiveItem();
				break;

			// +exp_event <yüzde> <dakika> — sunucu-geneli süreli EXP bonusu (GM)
			case "+exp_event"_djb2:
				ExpEvent();
				break;

			// +drop_event / +coin_event <yüzde> <dakika> — süreli drop/para bonusu (GM)
			case "+drop_event"_djb2:
				DropEvent();
				break;

			case "+coin_event"_djb2:
				CoinEvent();
				break;

			// +godmode on/off — GM 30000-hasar + no-aggro aç/kapa (oyun-içi)
			case "+godmode"_djb2:
				GodMode();
				break;

			// +autoloot [on/off] — mob loot'u direkt envantere (arg yoksa toggle)
			case "+autoloot"_djb2:
				AutoLoot();
				break;

			// +repair — tüm itemların durability'sini max yap (eskimiş silah hasarı yarılıyordu)
			case "+repair"_djb2:
				Repair();
				break;

			// Unhandled command.
			default:
				return false;
		}
	}
	catch (const std::invalid_argument& ex)
	{
		if (_srcUser != nullptr)
		{
			spdlog::warn("OperationMessage::Process: argument could not be parsed from GM "
						 "[charId={} command='{}' exception='{}']",
				_srcUser->m_pUserData->m_id, _command, ex.what());
		}
		else
		{
			spdlog::warn("OperationMessage::Process: argument could not be parsed from server "
						 "[command='{}' exception='{}']",
				_command, ex.what());
		}
	}
	catch (const std::out_of_range& ex)
	{
		if (_srcUser != nullptr)
		{
			spdlog::warn("OperationMessage::Process: parsed argument out of range from GM "
						 "[charId={} command='{}' exception='{}']",
				_srcUser->m_pUserData->m_id, _command, ex.what());
		}
		else
		{
			spdlog::warn("OperationMessage::Process: parsed argument out of range from server "
						 "[command='{}' exception='{}']",
				_command, ex.what());
		}
	}

	// Command was handled, even if it errored.
	return true;
}

void OperationMessage::Pursue()
{
	// TODO
}

void OperationMessage::ActPursue()
{
	// TODO
}

void OperationMessage::MonPursue()
{
	// TODO
}

void OperationMessage::MonCatch()
{
	// TODO
}

void OperationMessage::Assault()
{
	// TODO
}

// +monsummon <monsterSid> [adet] — GM, bulunduğu yere mob spawn eder (test/araç).
// Komut Ebenezer'da işlenir ama NPC'ler AIServer'da; AG_MONSTER_SUMMON ile AIServer'a iletilir.
void OperationMessage::MonSummon()
{
	if (_srcUser == nullptr) // telnet'te kaynak konum yok → oyun-içi GM gerekir
		return;

	if (GetArgCount() < 1)
	{
		_srcUser->SendSysMsg("[GM] kullanim: +monsummon <monsterId> [adet]");
		return;
	}

	int sid   = ParseInt(0);
	int count = (GetArgCount() >= 2) ? ParseInt(1) : 1;
	if (sid <= 0)
	{
		_srcUser->SendSysMsg("[GM] gecersiz monsterId");
		return;
	}
	if (count < 1)
		count = 1;
	if (count > 20)
		count = 20; // tek seferde guvenli ust sinir

	_USER_DATA* pData = _srcUser->m_pUserData;

	char sendBuffer[64];
	int  sendIndex = 0;
	SetByte(sendBuffer, AG_MONSTER_SUMMON, sendIndex);
	SetShort(sendBuffer, static_cast<int16_t>(sid), sendIndex);
	SetByte(sendBuffer, static_cast<uint8_t>(count), sendIndex);
	SetByte(sendBuffer, pData->m_bZone, sendIndex);
	SetFloat(sendBuffer, pData->m_curx, sendIndex);
	SetFloat(sendBuffer, pData->m_cury, sendIndex);
	SetFloat(sendBuffer, pData->m_curz, sendIndex);
	_main->Send_AIServer(pData->m_bZone, sendBuffer, sendIndex);

	_srcUser->SendSysMsg(fmt::format("[GM] Monster summon istegi gonderildi: sid={} x{}", sid, count));
}

void OperationMessage::MonSummonAll()
{
	// TODO
}

void OperationMessage::UnikMonster()
{
	// TODO
}

void OperationMessage::UserSeekReport()
{
	// TODO
}

void OperationMessage::MonKill()
{
	// TODO
}

void OperationMessage::Open()
{
	_main->BattleZoneOpen(BATTLEZONE_OPEN);
}

void OperationMessage::Open2()
{
	// TODO
}

void OperationMessage::Open3()
{
	// TODO
}

void OperationMessage::MOpen()
{
	// TODO
}

void OperationMessage::ForbidUser()
{
	// TODO
}

void OperationMessage::ForbidConnect()
{
	// TODO
}

void OperationMessage::SnowOpen()
{
	_main->BattleZoneOpen(SNOW_BATTLEZONE_OPEN);
}

void OperationMessage::Close()
{
	_main->m_byBanishFlag = 1;
	// _main->WithdrawUserOut();
}

void OperationMessage::Captain()
{
	_main->LoadKnightsRankTable();
}

void OperationMessage::TieBreak()
{
	// TODO
}

void OperationMessage::Auto()
{
	// TODO
}

void OperationMessage::AutoOff()
{
	// TODO
}

void OperationMessage::Down()
{
	g_serverdown_flag = true;
	_main->_serverSocketManager.StopAccept();
	_main->KickOutAllUsers();
}

void OperationMessage::Discount()
{
	_main->m_sDiscount = 1;
}

void OperationMessage::FreeDiscount()
{
	// TODO
}

void OperationMessage::AllDiscount()
{
	_main->m_sDiscount = 2;
}

void OperationMessage::UnDiscount()
{
	_main->m_sDiscount = 0;
}

void OperationMessage::Santa()
{
	_main->m_bySanta = 1; // Make Motherfucking Santa Claus FLY!!!
}

void OperationMessage::Angel()
{
	_main->m_bySanta = 2;
}

void OperationMessage::OffSanta()
{
	_main->m_bySanta = 0; // SHOOT DOWN Motherfucking Santa Claus!!!
}

void OperationMessage::LimitBattle()
{
	// TODO
}

void OperationMessage::OnSummonBlock()
{
	// TODO
}

void OperationMessage::OffSummonBlock()
{
	// TODO
}

// +zonechange: {int: zoneId} [float: x] [float: z]
// NOTE: Coordinates are unofficial.
void OperationMessage::ZoneChange()
{
	// Requires a user.
	if (_srcUser == nullptr || GetArgCount() < 1)
		return;

	int zoneId = ParseInt(0);
	float x    = _srcUser->m_pUserData->m_curx;
	float z    = _srcUser->m_pUserData->m_curz;

	if (GetArgCount() >= 3)
	{
		x = ParseFloat(1);
		z = ParseFloat(2);
	}

	_srcUser->ZoneChange(zoneId, x, z);
}

void OperationMessage::SiegeWarfare()
{
	// TODO
}

void OperationMessage::ResetSiegeWar()
{
	// TODO
}

void OperationMessage::SiegeWarScheduleStart()
{
	// TODO
}

void OperationMessage::SiegeWarScheduleEnd()
{
	// TODO
}

void OperationMessage::SiegeWarBaseReport()
{
	// TODO
}

void OperationMessage::SiegeWarStatusReport()
{
	// TODO
}

void OperationMessage::SiegeWarCheckBase()
{
	// TODO
}

void OperationMessage::ServerTestMode()
{
	// TODO
}

void OperationMessage::ServerNormalMode()
{
	// TODO
}

void OperationMessage::MerchantMoney()
{
	// TODO
}

void OperationMessage::SiegeWarPunishKnights()
{
	// TODO
}

void OperationMessage::SiegeWarLoadTable()
{
	// TODO
}

void OperationMessage::MoneyAdd()
{
	// TODO
}

void OperationMessage::ExpAdd()
{
	// TODO
}

void OperationMessage::UserBonus()
{
	// TODO
}

void OperationMessage::Discount1()
{
	// TODO
}

void OperationMessage::Discount2()
{
	// TODO
}

void OperationMessage::Battle1()
{
	// TODO
}

void OperationMessage::Battle2()
{
	// TODO
}

void OperationMessage::Battle3()
{
	// TODO
}

void OperationMessage::BattleAuto()
{
	// TODO
}

void OperationMessage::BattleReport()
{
	// TODO
}

void OperationMessage::ChallengeOn()
{
	// TODO
}

void OperationMessage::ChallengeOff()
{
	// TODO
}

void OperationMessage::ChallengeKill()
{
	// TODO
}

void OperationMessage::ChallengeLevel()
{
	// TODO
}

void OperationMessage::RentalReport()
{
	// TODO
}

void OperationMessage::RentalStop()
{
	// TODO
}

void OperationMessage::RentalStart()
{
	// TODO
}

void OperationMessage::KingReport1()
{
	// TODO
}

void OperationMessage::KingReport2()
{
	// TODO
}

void OperationMessage::ReloadKing()
{
	// TODO
}

void OperationMessage::Kill()
{
	if (GetArgCount() < 1)
		return;

	const std::string& charId = ParseString(0);
	_main->KillUser(charId.c_str());
}

void OperationMessage::ReloadNotice()
{
	// TODO
}

void OperationMessage::ReloadHacktool()
{
	// TODO
}

void OperationMessage::ServerDown()
{
	// TODO
}

void OperationMessage::WriteLog()
{
	// TODO
}

void OperationMessage::EventLog()
{
	// TODO
}

void OperationMessage::EventLogOff()
{
	// TODO
}

void OperationMessage::ItemDown()
{
	// TODO
}

void OperationMessage::ItemDownReset()
{
	// TODO
}

void OperationMessage::ChallengeStop()
{
	// TODO
}

void OperationMessage::Permanent()
{
	_main->m_bPermanentChatMode = true;
	_main->m_bPermanentChatFlag = true;
}

void OperationMessage::OffPermanent()
{
	_main->m_bPermanentChatMode = false;
	_main->m_bPermanentChatFlag = false;

	char sendBuffer[1024] {};
	int sendIndex = 0;

	SetByte(sendBuffer, WIZ_CHAT, sendIndex);
	SetByte(sendBuffer, END_PERMANENT_CHAT, sendIndex);

	SetByte(sendBuffer, 0x01, sendIndex); // nation
	SetShort(sendBuffer, -1, sendIndex);  // sid
	SetByte(sendBuffer, 0, sendIndex);    // sender name length
	SetString2(sendBuffer, "", sendIndex);
	_main->Send_All(sendBuffer, sendIndex);

	sendIndex = 0;
	memset(sendBuffer, 0, 1024);
	SetByte(sendBuffer, STS_CHAT, sendIndex);
	SetString2(sendBuffer, _command, sendIndex);

	for (const auto& [_, pInfo] : _main->m_ServerArray)
	{
		if (pInfo != nullptr && pInfo->sServerNo != _main->m_nServerNo)
			_main->m_pUdpSocket->SendUDPPacket(pInfo->strServerIP, sendBuffer, sendIndex);
	}
}

// +give_item [charName] itemId [count] — GM item oluşturma (Release'de de etkin).
// Chat'ten: srcUser hedeftir (+give_item itemId [count]).
// Telnet'ten (srcUser yok): ilk argüman hedef karakter adıdır (+give_item charName itemId [count]).
void OperationMessage::GiveItem()
{
	if (GetArgCount() < 1)
		return;

	CUser*                 pTarget = _srcUser;
	std::shared_ptr<CUser> pTargetHold; // telnet hedefinin ömrünü tut
	size_t                 base = 0;

	if (pTarget == nullptr)
	{
		if (GetArgCount() < 2)
			return;
		pTargetHold = _main->GetUserPtr(ParseString(0).c_str(), NameType::Character);
		pTarget     = pTargetHold.get();
		base        = 1;
	}

	if (pTarget == nullptr)
		return;

	int itemId = ParseInt(base);
	int count  = (GetArgCount() >= base + 2) ? ParseInt(base + 1) : 1;

	bool isSuccess = false;
	if (itemId > 0 && count > 0)
		isSuccess = pTarget->GiveItem(itemId, count);

	spdlog::warn("OperationMessage::GiveItem: invoked [target={} itemId={} count={} success={}]",
		pTarget->m_pUserData->m_id, itemId, count, isSuccess);
}

// +exp_event <yüzde> <dakika> — sunucu-geneli süreli EXP bonusu. Örn. "+exp_event 200 60" → 1 saat
// boyunca tüm oyunculara x2 exp. Yüzde 100 ya da dakika<=0 → kapatır. Süre dolunca otomatik normal.
void OperationMessage::ExpEvent()
{
	if (_main == nullptr || GetArgCount() < 1)
		return;

	int rate    = ParseInt(0);
	int minutes = (GetArgCount() >= 2) ? ParseInt(1) : 0;

	if (rate < 0)
		rate = 0;
	if (rate > 10000)
		rate = 10000; // güvenlik (x100 üst sınır)

	if (rate == 100 || minutes <= 0)
	{
		_main->m_nExpEventRate = 100;
		_main->m_tExpEventEnd  = 0;
		spdlog::warn("OperationMessage::ExpEvent: EXP bonusu KAPATILDI");
		_main->NoticeAll("EXP Event has ended. Experience rate is back to normal.");
		return;
	}

	_main->m_nExpEventRate = rate;
	_main->m_tExpEventEnd  = time(nullptr) + static_cast<time_t>(minutes) * 60;
	spdlog::warn("OperationMessage::ExpEvent: EXP bonusu ACIK rate={}% sure={}dk", rate, minutes);
	_main->NoticeAll(fmt::format("EXP Event! x{} experience for the next {} minutes. Happy hunting!",
		rate / 100, minutes));
}

// +drop_event / +coin_event <yüzde> <dakika> — süreli drop/para bonusu. Loot AIServer'da rollanır;
// ayar oraya AG_EVENT_RATE ile gönderilir. rate 100 ya da dakika<=0 → kapatır. Tüm oyunculara duyuru.
void OperationMessage::DropEvent()
{
	EventRateCmd(0, "DROP");
}

void OperationMessage::CoinEvent()
{
	EventRateCmd(1, "COIN");
}

// +godmode on/off — GM 30000-hasar + no-aggro'yu aç/kapa. Oyun-içi GM gerekir (srcUser).
// Arg yoksa varsayılan "on". Kapatınca gear'ınla gerçek hasar verir, mob saldırır, exp kazanır.
void OperationMessage::GodMode()
{
	if (_srcUser == nullptr) // yalnızca oyun-içi (telnet'te hedef kullanıcı yok)
		return;

	bool bGod;
	if (GetArgCount() >= 1)
	{
		const std::string& a = ParseString(0);
		bGod                 = !(a == "off" || a == "0" || a == "kapat");
	}
	else
	{
		bGod = !_srcUser->m_bGodMode; // arg yoksa toggle (command penceresi tek komutla aç/kapa)
	}

	_srcUser->m_bGodMode = bGod;
	_srcUser->SendGmToggleToAI(bGod);
	_srcUser->SendSysMsg(bGod ? "[GM] Godmode: ON  (30000 hasar + no-aggro)"
							  : "[GM] Godmode: OFF  (normal hasar/aggro)");
	spdlog::warn("OperationMessage::GodMode: charId={} godmode={}", _srcUser->m_pUserData->m_id,
		bGod ? "ON" : "OFF");
}

// +autoloot [on/off] — mob loot'u yere düşürmek yerine direkt envantere. Arg yoksa toggle
// (command penceresi tek komutla aç/kapa için). Oyun-içi gerekir.
void OperationMessage::AutoLoot()
{
	if (_srcUser == nullptr)
		return;

	// +autoloot            → aç/kapa toggle
	// +autoloot off|0|kapat → kapat
	// +autoloot <sayı>     → aç + min Noah (BuyPrice) eşiği (çöp filtresi)
	// +autoloot unique     → aç + unique (değerden bağımsız) topla toggle
	// (Popup #13 gelene kadar test/komut yolu.)
	if (GetArgCount() >= 1)
	{
		const std::string& a = ParseString(0);
		if (a == "off" || a == "0" || a == "kapat")
		{
			_srcUser->m_bAutoLoot = false;
		}
		else if (a == "unique")
		{
			_srcUser->m_bAutoLootUniqueOnly = !_srcUser->m_bAutoLootUniqueOnly;
			_srcUser->m_bAutoLoot           = true;
		}
		else
		{
			int v                          = atoi(a.c_str());
			_srcUser->m_nAutoLootMinValue  = (v > 0) ? v : 0;
			_srcUser->m_bAutoLoot          = true;
		}
	}
	else
	{
		_srcUser->m_bAutoLoot = !_srcUser->m_bAutoLoot; // toggle
	}

	if (_srcUser->m_bAutoLoot)
	{
		_srcUser->SendSysMsg(fmt::format("[GM] Autoloot: ON  (min {} Noah{})",
			_srcUser->m_nAutoLootMinValue,
			_srcUser->m_bAutoLootUniqueOnly ? ", +unique" : ""));
	}
	else
	{
		_srcUser->SendSysMsg("[GM] Autoloot: OFF");
	}
	spdlog::warn("OperationMessage::AutoLoot: charId={} autoloot={} minVal={} unique={}",
		_srcUser->m_pUserData->m_id, _srcUser->m_bAutoLoot, _srcUser->m_nAutoLootMinValue,
		_srcUser->m_bAutoLootUniqueOnly);
}

// +repair — tüm itemların durability'sini max yapar (eskimiş silah hasarı yarılıyordu) + atağı yeniler.
void OperationMessage::Repair()
{
	if (_srcUser == nullptr)
		return;

	int n = _srcUser->RepairAllItems();
	_srcUser->SendSysMsg(fmt::format("[GM] {} item onarildi (durability max). Atak yeniden hesaplandi.", n));
	spdlog::warn("OperationMessage::Repair: charId={} repaired={}", _srcUser->m_pUserData->m_id, n);
}

void OperationMessage::EventRateCmd(uint8_t byType, const char* label)
{
	if (_main == nullptr || GetArgCount() < 1)
		return;

	int rate    = ParseInt(0);
	int minutes = (GetArgCount() >= 2) ? ParseInt(1) : 0;
	if (rate < 0)
		rate = 0;
	if (rate > 10000)
		rate = 10000;

	bool off = (rate == 100 || minutes <= 0);
	_main->SendEventRate(byType, off ? 100 : rate, off ? 0 : minutes * 60);

	if (off)
		_main->NoticeAll(fmt::format("{} Event has ended.", label));
	else
		_main->NoticeAll(
			fmt::format("{} Event! x{} for the next {} minutes!", label, rate / 100, minutes));
	spdlog::warn("OperationMessage::EventRateCmd: {} rate={}% sure={}dk", label, off ? 100 : rate,
		off ? 0 : minutes);
}

bool OperationMessage::ParseCommand(const std::string_view command, size_t& key)
{
	_command.assign(command.data(), command.length());
	_args.clear();

	// Split string into parts.
	// Delimit by whitespace.
	// Empty spaces are ignored.
	// This:
	// +cmd arg1    arg2     arg3
	// Will become:
	// [0] = +cmd, [1] = arg1, [2] = arg3
	std::istringstream ss(_command);
	std::string part;
	while (ss >> part)
		_args.push_back(part);

	// Expect at least one "argument" (the command name).
	if (_args.empty())
		return false;

	// Extract and transform the command name to lowercase.
	std::string& commandNameLowercase = _args.front();
	strtolower(commandNameLowercase);

	// Hash the lowercase key name for returning.
	key = hashing::djb2::hash(commandNameLowercase);

	// Strip it from the args list for consistency; we don't need it anymore.
	_args.erase(_args.begin());

	return true;
}

// Returns the number of arguments, excluding the command name.
size_t OperationMessage::GetArgCount() const
{
	return _args.size();
}

int OperationMessage::ParseInt(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return std::stoi(_args[argIndex]);
}

float OperationMessage::ParseFloat(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return std::stof(_args[argIndex]);
}

const std::string& OperationMessage::ParseString(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return _args[argIndex];
}

} // namespace Ebenezer
