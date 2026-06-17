#ifndef SERVER_EBENEZER_OPERATIONMESSAGE_H
#define SERVER_EBENEZER_OPERATIONMESSAGE_H

#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace Ebenezer
{

class EbenezerApp;
class CUser;
class OperationMessage
{
public:
	OperationMessage(EbenezerApp* main, CUser* srcUser);
	bool Process(const std::string_view command);

protected:
	void Pursue();
	void ActPursue();
	void MonPursue();
	void MonCatch();
	void Assault();
	void MonSummon();
	void MonSummonAll();
	void UnikMonster();
	void UserSeekReport();
	void MonKill();
	void Open();
	void Open2();
	void Open3();
	void MOpen();
	void ForbidUser();
	void ForbidConnect();
	void SnowOpen();
	void Close();
	void Captain();
	void TieBreak();
	void Auto();
	void AutoOff();
	void Down();
	void Discount();
	void FreeDiscount();
	void AllDiscount();
	void UnDiscount();
	void Santa();
	void Angel();
	void OffSanta();
	void LimitBattle();
	void OnSummonBlock();
	void OffSummonBlock();
	void ZoneChange();
	void SiegeWarfare();
	void ResetSiegeWar();
	void SiegeWarScheduleStart();
	void SiegeWarScheduleEnd();
	void SiegeWarBaseReport();
	void SiegeWarStatusReport();
	void SiegeWarCheckBase();
	void ServerTestMode();
	void ServerNormalMode();
	void MerchantMoney();
	void SiegeWarPunishKnights();
	void SiegeWarLoadTable();
	void MoneyAdd();
	void ExpAdd();
	void UserBonus();
	void Discount1();
	void Discount2();
	void Battle1();
	void Battle2();
	void Battle3();
	void BattleAuto();
	void BattleReport();
	void ChallengeOn();
	void ChallengeOff();
	void ChallengeKill();
	void ChallengeLevel();
	void RentalReport();
	void RentalStop();
	void RentalStart();
	void KingReport1();
	void KingReport2();
	void ReloadKing();
	void Kill();
	void ReloadNotice();
	void ReloadHacktool();
	void ServerDown();
	void WriteLog();
	void EventLog();
	void EventLogOff();
	void ItemDown();
	void ItemDownReset();
	void ChallengeStop();
	void Permanent();
	void OffPermanent();
	void GiveItem(); // +give_item itemId [count] — Release'de de etkin (GM authority gerekir)
	void ExpEvent();  // +exp_event <yüzde> <dakika> — sunucu-geneli süreli EXP bonusu (GM)
	void DropEvent(); // +drop_event <yüzde> <dakika> — süreli drop-şansı bonusu (GM)
	void CoinEvent(); // +coin_event <yüzde> <dakika> — süreli para bonusu (GM)
	void GodMode();   // +godmode on/off — GM 30000-hasar + no-aggro aç/kapa (oyun-içi GM)
	void AutoLoot();  // +autoloot [on/off] — mob loot'u direkt envantere (arg yoksa toggle)
	void Repair();    // +repair — tüm itemları onar (durability max)
	// Ortak yardımcı: byType 0=drop, 1=coin. label duyuru metni.
	void EventRateCmd(uint8_t byType, const char* label);

	bool ParseCommand(const std::string_view command, size_t& key);

	// Returns the number of arguments, excluding the command name.
	size_t GetArgCount() const;
	int ParseInt(size_t argIndex) const;
	float ParseFloat(size_t argIndex) const;
	const std::string& ParseString(size_t argIndex) const;

protected:
	EbenezerApp* _main;
	CUser* _srcUser;
	std::string _command;
	std::vector<std::string> _args;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_OPERATIONMESSAGE_H
