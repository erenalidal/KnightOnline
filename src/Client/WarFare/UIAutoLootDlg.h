// UIAutoLootDlg.h: interface for the CUIAutoLootDlg class.
//
// Auto-loot filter popup (macOS port). Opened from the command window via the
// "autoloot" GM command. Lets the player set a Min-Noah threshold and a
// "unique-only" toggle, then sends WIZ_AUTOLOOT_SETTINGS to the server.
//
// Modeled on CCountableItemEditDlg. Reuses the personaltradeedit .uif layout
// cloned into ka_autolootset_us.uif / el_autolootset_us.uif.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "N3UIWndBase.h"

class CN3UIEdit;
class CN3UIButton;
class CN3UIString;

class CUIAutoLootDlg : public CN3UIBase
{
	bool m_bUnique; // "unique-only" toggle state

public:
	CN3UIEdit* m_pEditMinNoah;
	CN3UIButton* m_pBtnUnique;
	CN3UIButton* m_pBtnOk;
	CN3UIButton* m_pBtnCancel;
	CN3UIString* m_pLblUnique; // "Unique: ON/OFF" — toggle durumunu gösterir

public:
	CUIAutoLootDlg();
	~CUIAutoLootDlg() override;

	bool Load(File& file) override;
	void Release() override;

	bool OnKeyPress(int iKey) override;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;

	void SetVisible(bool bVisible) override;

	void Open();
	void Close();

	int GetMinNoah() const;
	void SetMinNoah(int iVal);

	void SetUnique(bool bUnique);
	bool IsUnique() const
	{
		return m_bUnique;
	}

	// Akıllı toggle: autoloot zaten açıksa popup yerine direkt kapat (disable).
	// s_bEnabled, en son uygulanan ON/OFF durumunu client tarafında izler.
	static bool IsEnabled()
	{
		return s_bEnabled;
	}
	void DisableAutoLoot(); // WIZ_AUTOLOOT_SETTINGS enable=0 gönderir, s_bEnabled=false

private:
	static bool s_bEnabled;
	void SendSettings(); // builds + sends WIZ_AUTOLOOT_SETTINGS (enable=1)
};
