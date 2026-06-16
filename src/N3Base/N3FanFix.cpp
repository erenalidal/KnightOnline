// N3FanFix.cpp — D3DPT_TRIANGLEFAN → D3DPT_TRIANGLELIST dönüşüm yardımcıları. Bkz. N3FanFix.h.

#include "StdAfxBase.h"
#include "N3FanFix.h"
#include "N3Base.h"

#include <vector>

namespace
{
// Quad fan ([0,1,2, 0,2,3]) için paylaşılan index buffer.
LPDIRECT3DINDEXBUFFER9 GetQuadIB()
{
	static LPDIRECT3DINDEXBUFFER9 s_pIB = nullptr;
	if (s_pIB == nullptr && CN3Base::s_lpD3DDev != nullptr)
	{
		if (SUCCEEDED(CN3Base::s_lpD3DDev->CreateIndexBuffer(
				6 * sizeof(uint16_t), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED,
				&s_pIB, nullptr)))
		{
			void* pIdx = nullptr;
			if (SUCCEEDED(s_pIB->Lock(0, 0, &pIdx, 0)))
			{
				static const uint16_t kIdx[6] = { 0, 1, 2, 0, 2, 3 };
				memcpy(pIdx, kIdx, sizeof(kIdx));
				s_pIB->Unlock();
			}
		}
	}
	return s_pIB;
}

// Değişken boyutlu fan için index buffer: [0,1,2, 0,2,3, 0,3,4, ...]. İhtiyaç büyüdükçe büyür.
LPDIRECT3DINDEXBUFFER9 GetTriFanIB(unsigned int triCount)
{
	static LPDIRECT3DINDEXBUFFER9 s_pIB     = nullptr;
	static unsigned int           s_maxTris = 0;
	if (CN3Base::s_lpD3DDev == nullptr || triCount == 0)
		return nullptr;

	if (triCount > s_maxTris)
	{
		if (s_pIB != nullptr)
		{
			s_pIB->Release();
			s_pIB = nullptr;
		}
		// fan köşe sayısı = triCount + 2; 16-bit index sınırı (65535) içinde kalmalı.
		unsigned int n = triCount;
		if (n + 2 > 65535)
			n = 65533;
		if (FAILED(CN3Base::s_lpD3DDev->CreateIndexBuffer(
				n * 3 * sizeof(uint16_t), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED,
				&s_pIB, nullptr)))
			return nullptr;
		void* pIdx = nullptr;
		if (SUCCEEDED(s_pIB->Lock(0, 0, &pIdx, 0)))
		{
			uint16_t* idx = static_cast<uint16_t*>(pIdx);
			for (unsigned int i = 0; i < n; i++)
			{
				idx[i * 3 + 0] = 0;
				idx[i * 3 + 1] = static_cast<uint16_t>(i + 1);
				idx[i * 3 + 2] = static_cast<uint16_t>(i + 2);
			}
			s_pIB->Unlock();
		}
		s_maxTris = n;
	}
	return s_pIB;
}
} // namespace

void KO_DrawQuadAsList()
{
	LPDIRECT3DINDEXBUFFER9 pIB = GetQuadIB();
	if (pIB != nullptr)
	{
		CN3Base::s_lpD3DDev->SetIndices(pIB);
		CN3Base::s_lpD3DDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	}
	else
	{
		CN3Base::s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	}
}

void KO_DrawTriFanIndexed(unsigned int startVertex, unsigned int numVertex)
{
	if (numVertex < 3)
		return;
	unsigned int triCount = numVertex - 2;
	LPDIRECT3DINDEXBUFFER9 pIB = GetTriFanIB(triCount);
	if (pIB != nullptr)
	{
		CN3Base::s_lpD3DDev->SetIndices(pIB);
		CN3Base::s_lpD3DDev->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, startVertex, 0, numVertex, 0, triCount);
	}
	else
	{
		CN3Base::s_lpD3DDev->DrawPrimitive(D3DPT_TRIANGLEFAN, startVertex, triCount);
	}
}

void KO_DrawTriFanUP(unsigned int primCount, const void* pVerts, unsigned int stride)
{
	if (primCount == 0 || pVerts == nullptr || stride == 0)
		return;
	// fan: primCount+2 köşe → list: primCount*3 köşe (v0, v(i+1), v(i+2)).
	static std::vector<uint8_t> s_buf;
	s_buf.resize(static_cast<size_t>(primCount) * 3 * stride);
	const uint8_t* v   = static_cast<const uint8_t*>(pVerts);
	uint8_t*       dst = s_buf.data();
	for (unsigned int i = 0; i < primCount; i++)
	{
		memcpy(dst + (static_cast<size_t>(i) * 3 + 0) * stride, v, stride);
		memcpy(dst + (static_cast<size_t>(i) * 3 + 1) * stride, v + static_cast<size_t>(i + 1) * stride, stride);
		memcpy(dst + (static_cast<size_t>(i) * 3 + 2) * stride, v + static_cast<size_t>(i + 2) * stride, stride);
	}
	CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, primCount, s_buf.data(), stride);
}
