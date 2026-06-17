// UIAutoLootDlg.cpp: implementation of the CUIAutoLootDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIAutoLootDlg.h"

#include "GameProcedure.h"
#include "GameProcMain.h"
#include "UIManager.h"
#include "APISocket.h"
#include "PacketDef.h" // <shared/packets.h> -> WIZ_AUTOLOOT_SETTINGS

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>
#include <N3Base/LogWriter.h>

#include <cstdlib>

bool CUIAutoLootDlg::s_bEnabled = false;

CUIAutoLootDlg::CUIAutoLootDlg()
{
	m_bUnique      = false;
	m_pEditMinNoah = nullptr;
	m_pBtnUnique   = nullptr;
	m_pBtnOk       = nullptr;
	m_pBtnCancel   = nullptr;
	m_pLblUnique   = nullptr;
}

CUIAutoLootDlg::~CUIAutoLootDlg()
{
}

void CUIAutoLootDlg::Release()
{
	CN3UIBase::Release();
}

bool CUIAutoLootDlg::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	// Min-Noah value reuses the cloned edit field (id kept from template).
	N3_VERIFY_UI_COMPONENT(m_pEditMinNoah, GetChildByID<CN3UIEdit>("edit_trade"));
	N3_VERIFY_UI_COMPONENT(m_pBtnUnique, GetChildByID<CN3UIButton>("btn_unique"));
	N3_VERIFY_UI_COMPONENT(m_pBtnOk, GetChildByID<CN3UIButton>("btn_ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtnCancel, GetChildByID<CN3UIButton>("btn_cancel"));
	m_pLblUnique = GetChildByID<CN3UIString>("String_AutoLoot_Unique"); // opsiyonel

	// Min-Noah max 9 hane (taşmayı/overflow'u önle) + sadece rakam kabul et.
	if (m_pEditMinNoah != nullptr)
	{
		m_pEditMinNoah->SetMaxString(9);
		m_pEditMinNoah->SetNumberOnly(true); // harf/işaret girişini engelle (#13)
	}

	return true;
}

void CUIAutoLootDlg::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

void CUIAutoLootDlg::SetUnique(bool bUnique)
{
	m_bUnique = bUnique;
	// btn_unique artık gerçek bir tick-box: NORMAL görseli boş kutu, ON görseli işaretli
	// kutu (rookie-tip "don't show again" checkbox UV'leri, ui_message_us.dxt).
	// ON state = işaretli görsel, NORMAL = boş kutu.
	if (m_pBtnUnique != nullptr)
		m_pBtnUnique->SetState(bUnique ? UI_STATE_BUTTON_ON : UI_STATE_BUTTON_NORMAL);
	// Etiket additif anlamı taşır: işaretliyse unique item'lar fiyatı min-Noah'ın
	// ALTINDA olsa bile toplanır ("sadece unique" DEĞİL). Tick durumu görselden okunur.
	if (m_pLblUnique != nullptr)
		m_pLblUnique->SetString("Loot uniques anyway");
}

int CUIAutoLootDlg::GetMinNoah() const
{
	if (m_pEditMinNoah == nullptr)
		return 0;

	int iVal = atoi(m_pEditMinNoah->GetString().c_str());
	return (iVal > 0) ? iVal : 0;
}

void CUIAutoLootDlg::SetMinNoah(int iVal)
{
	if (m_pEditMinNoah == nullptr)
		return;

	std::string buff;
	if (iVal > 0)
		buff = std::to_string(iVal);
	m_pEditMinNoah->SetString(buff);
}

void CUIAutoLootDlg::Open()
{
	CLogWriter::Write("AutoLootDlg::Open called");
	// Başlık/etiket metinlerini autoloot'a uygun set et (.uif'ten gelen eski metin yerine).
	CN3UIString* pMsg = GetChildByID<CN3UIString>("String_PersonTradeEdit_Msg");
	if (pMsg != nullptr)
		pMsg->SetString("Auto-Loot: Min Noah");

	// Default each open to a clean state. SetMinNoah/SetUnique BEFORE SetFocus so
	// the macOS edit bridge (UpdateTextFromEditCtrl) doesn't overwrite it.
	SetMinNoah(0);
	SetUnique(false);

	SetVisible(true);

	if (m_pEditMinNoah != nullptr)
		m_pEditMinNoah->SetFocus();
}

void CUIAutoLootDlg::Close()
{
	SetVisibleWithNoSound(false);

	CN3UIEdit* pEdit = GetFocusedEdit();
	if (pEdit != nullptr)
		pEdit->KillFocus();
}

void CUIAutoLootDlg::SendSettings()
{
	uint8_t byBuff[16];
	int     iOffset = 0;

	int     iMinNoah = GetMinNoah();
	uint8_t byUnique = m_bUnique ? 1 : 0;

	// [byte WIZ_AUTOLOOT_SETTINGS][byte enable=1][DWORD minValue][byte uniqueOnly]
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_AUTOLOOT_SETTINGS);
	CAPISocket::MP_AddByte(byBuff, iOffset, 1); // enable
	CAPISocket::MP_AddDword(byBuff, iOffset, static_cast<uint32_t>(iMinNoah));
	CAPISocket::MP_AddByte(byBuff, iOffset, byUnique);

	CLogWriter::Write("AutoLootDlg::SendSettings min=%d unique=%d off=%d", iMinNoah, (int) byUnique, iOffset);
	__ASSERT(iOffset <= (int) sizeof(byBuff), "Send Buffer OverFlow");
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	s_bEnabled = true; // bir sonraki toggle artık kapatma (disable) yapsın
}

void CUIAutoLootDlg::DisableAutoLoot()
{
	uint8_t byBuff[16];
	int     iOffset = 0;

	// [byte WIZ_AUTOLOOT_SETTINGS][byte enable=0][DWORD 0][byte 0]
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_AUTOLOOT_SETTINGS);
	CAPISocket::MP_AddByte(byBuff, iOffset, 0); // enable=0 -> server m_bAutoLoot=false + "[Autoloot] OFF"
	CAPISocket::MP_AddDword(byBuff, iOffset, 0);
	CAPISocket::MP_AddByte(byBuff, iOffset, 0);

	CLogWriter::Write("AutoLootDlg::DisableAutoLoot");
	__ASSERT(iOffset <= (int) sizeof(byBuff), "Send Buffer OverFlow");
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);

	s_bEnabled = false; // bir sonraki toggle artık popup açsın
}

bool CUIAutoLootDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;
	if (!IsVisible())
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		CLogWriter::Write("AutoLootDlg::ReceiveMessage CLICK sender=%s",
			pSender == m_pBtnUnique ? "unique" : pSender == m_pBtnOk ? "ok"
			: pSender == m_pBtnCancel									? "cancel"
																		: "other");

		if (pSender == m_pBtnUnique)
		{
			// Toggle the unique-only flag (stay open).
			SetUnique(!m_bUnique);
			return true;
		}

		if (pSender == m_pBtnOk)
		{
			SendSettings();
			Close();
			return true;
		}

		if (pSender == m_pBtnCancel)
		{
			Close();
			return true;
		}
	}

	return true;
}

bool CUIAutoLootDlg::OnKeyPress(int iKey)
{
	CLogWriter::Write("AutoLootDlg::OnKeyPress key=%d visible=%d", iKey, (int) IsVisible());
	switch (iKey)
	{
		case DIK_RETURN:
			ReceiveMessage(m_pBtnOk, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_ESCAPE:
			ReceiveMessage(m_pBtnCancel, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
