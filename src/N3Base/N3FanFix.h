// N3FanFix.h — D3DPT_TRIANGLEFAN → D3DPT_TRIANGLELIST dönüşüm yardımcıları (macOS/MoltenVK).
//
// Apple GPU (Metal) triangle fan'i native desteklemez. MoltenVK her fan draw'ında render pass'i
// durdurup compute ile fan→list dönüştürür ve render pass'i RESTART eder. Apple'in TBDR GPU'sunda
// her render-pass restart = tam tile flush (çok pahalı). KO yoğun fan kullandığından kalabalık/UI
// sahnelerde GPU 10x yavaşlar (GPU frame-capture ile doğrulandı: "Metal renderpass restart").
//
// Çözüm: fan'i, birebir aynı üçgenleri veren ama Metal-native olan triangle list'e çevirmek.
// fan(v0,v1,..,vn) ≡ list (v0,v1,v2),(v0,v2,v3),...,(v0,v(n-1),vn).
//
// Windows'ta da doğru çalışır (sadece topology farkı), bu yüzden platformdan bağımsız kullanılır.

#pragma once

// Bound vertex buffer'daki 4-köşe quad'i (D3DPT_TRIANGLEFAN, 0, 2 yerine) indexed list olarak çizer.
void KO_DrawQuadAsList();

// Bound vertex buffer'daki değişken boyutlu fan'i (D3DPT_TRIANGLEFAN, startVertex, numVertex-2
// yerine) indexed list olarak çizer. numVertex = fan köşe sayısı (>= 3).
void KO_DrawTriFanIndexed(unsigned int startVertex, unsigned int numVertex);

// DrawPrimitiveUP(D3DPT_TRIANGLEFAN, primCount, pVerts, stride) yerine: fan köşelerini list'e
// genişletip DrawPrimitiveUP(D3DPT_TRIANGLELIST, ...) ile çizer. primCount = üçgen sayısı.
void KO_DrawTriFanUP(unsigned int primCount, const void* pVerts, unsigned int stride);
