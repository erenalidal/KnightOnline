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

#include <cstdlib>

CUIAutoLootDlg::CUIAutoLootDlg()
{
	m_bUnique      = false;
	m_pEditMinNoah = nullptr;
	m_pBtnUnique   = nullptr;
	m_pBtnOk       = nullptr;
	m_pBtnCancel   = nullptr;
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
	if (m_pBtnUnique != nullptr)
		m_pBtnUnique->SetState(bUnique ? UI_STATE_BUTTON_ON : UI_STATE_BUTTON_NORMAL);
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

	__ASSERT(iOffset <= (int) sizeof(byBuff), "Send Buffer OverFlow");
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

bool CUIAutoLootDlg::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;
	if (!IsVisible())
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
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
