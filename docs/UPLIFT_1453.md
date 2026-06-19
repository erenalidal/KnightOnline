# Protocol Uplift: 1.298 → 1453 (Fire Drake)

Amaç: native macOS client'ımızı (OpenKO 1.298 tabanı, Metal port) bir **1453 sunucusuna**
bağlanıp oynatacak şekilde yükseltmek. Yeni client kaynağı dünyada yok (sadece 1.298 sızdı),
ama bizim client zaten kısmen 1886-şeklinde paket okuyor → uplift beklenenden küçük.

Referans (hedef paket düzeni): `ko-refs/snoxd-koserver` (snoxd, `__VERSION` guard'larıyla 1298↔1886
yollarını içeriyor). 1453, snoxd'de `>=1453 && <1700` rejimi.

## Neden 1453 (1886 değil)
1453 deltası ~6-10 paket; 1886 ~10-15 + en riskli değişiklik (16→32-bit paket uzunluğu, 1800'de).
1453 bunları atlar (buying-merchant 1700, 32-bit uzunluk 1800, LAN-IP 1888 yok).

## Faz 0 — Test sunucusu (ÖNKOŞUL)
Uplift'i test etmek için çalışan bir 1453 sunucusu şart. Aday: snoxd'yi `__VERSION 1453` ile
Mac'te derlemek (autotools Linux build'i var). DB: mevcut KN_online'a DOKUNMA → yeni KN_1453.
> Durum: fizibilite ayrı bir analizde değerlendiriliyor.

## Faz 1 — Login + kripto (ilk bağlantı)
- [x] **Kripto özel anahtarı**: 1453 → `0x1257091582190465` (snoxd `>=1453` rejimi).
      `src/shared/JvCryption.cpp` artık `__VERSION`'a göre constexpr seçiyor (1298'de 0x1234... korunur).
      DÜZELTME: 1453 anahtarı 1298'inkinden (0x1234...) FARKLI — `<1453` koşulu 1453'ü dışlıyor.
- [ ] **LS_CRYPTION** login-server handshake'i (snoxd `LogInServer/LoginSession.cpp:127`,
      `#if __VERSION >= 1453`). Client login sahnesi (`GameProcLogIn_1298`) bunu yapmıyor:
      client LS_CRYPTION gönderir → public key alır → login soketinde EnableCrypto. + `LS_UNKF7`.
- [ ] **LS_SERVERLIST** echo alanı (+uint16, `>=1500`) ve genişlemiş server-list payload
      (serverID/groupID/playerCap/freeCap + king/notice). snoxd `LoginServer.cpp:81`.
- [ ] **MAX_PW_SIZE** 12 → 28 (`>=1453`, snoxd `globals.h`).

## Faz 2 — Karakter girişi (EN ZOR)
- [ ] **WIZ_MYINFO** item struct hizalaması. Server 19 byte/item yazıyor
      (`+uint32 unknown +uint32 expiry`); client 11 byte okuyor
      (`GameProcMain.cpp::MsgRecv_MyInfo_All:2009`). 1 byte kayarsa tüm stream desync.
      Hedef düzen: snoxd `Ebenezer/User.cpp::SendMyInfo`.
- [ ] **Envanter slot sayısı**: server `INVENTORY_TOTAL` (80+ slot; cospre + 2 magic bag);
      client `MAX_ITEM_INVENTORY=28`. Client UI yoksa bile tüm slotları TÜKETMELİ.

## Faz 3 — Oyun paketleri (kolaylar)
- [ ] **WIZ_NOTICE** yeni format (`>=1453`, sub-opcode 2, başlık+mesaj). Yeni UI widget'ı gerek.
- [ ] **WIZ_ITEM_UPGRADE** `+bType` byte (preview/upgrade).
- [ ] **WIZ_CLASS_CHANGE** stat point uint8 → uint16.

## Tamamen yeni sistemler (UI gerektirir, sadece 1453'te stub'lanabilir)
- Yeni-stil ekran bildirimleri (WIZ_NOTICE widget'ı — kaçınılmaz).
- Cospre (kostüm) + magic-bag envanter slotları — veri tüketilmeli, UI stub.
- Item rental + expiration (client flag'i okuyor, atıyor).
- Buying-merchant: **1700+, 1453'te YOK → atla.**

## Kritik dosyalar
- Client kripto/framing: `src/Client/WarFare/APISocket.cpp` (199-289)
- Client handshake: `src/Client/WarFare/GameProcedure.cpp` (845-944), `GameProcLogIn_1298.cpp`
- Client MyInfo parser (en zor): `src/Client/WarFare/GameProcMain.cpp` (1890-2018)
- Kripto anahtarı/sürüm: `src/shared/JvCryption.cpp`, `src/shared/version.h`
- Server referans: `ko-refs/snoxd-koserver/src/{LogInServer/LoginSession.cpp, Ebenezer/User.cpp::SendMyInfo, shared/{JvCryption.cpp,KOSocket.cpp,globals.h}}`

## Build notu
`__VERSION` (src/shared/version.h) şu an 1298. 1453 build için 1453'e çekilecek
(kripto anahtarı otomatik takip eder). Bu, 1298 sunucu/client çiftini bozar — uplift
branch'inde sunucu olarak snoxd@1453 kullanılır, kendi Ebenezer'imiz değil.
