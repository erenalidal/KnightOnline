# Kalıcılık (Persistence) Mimarisi — Ebenezer ↔ Aujard ↔ DB

Bu doküman, sunucunun oyuncu verisini nasıl tuttuğunu, ne zaman/nasıl DB'ye yazdığını,
crash davranışını ve **ölçeklenir hedef mimariyi** kaydeder. Küçük grup (≈10 kişi) için
bugünkü hâl yeterli; "binlerce oyuncu" için ADIM 2→3→4 buraya göre uygulanır.

> Bağlam: bu analiz, dupe/torn-save (#23 M7) çalışması sırasında çıktı. ADIM 1 (aşağıda)
> uygulanıyor; ADIM 2/3/4 oyuncu sayısı gerçekten gerektirince yapılacak (premature optimization'dan kaçın).

---

## 1. Bugünkü mimari (kanıtlanmış gerçekler)

### Süreçler
- **Ebenezer** (oyun sunucusu): TÜM oyun logic'i. `asio::thread_pool` (hw*2 worker) AMA
  `TcpSocketManager::_mutex` (tek global recursive_mutex) ile **"effectively single-threaded"**
  — tüm Parsing/logic ondan geçer. DB'ye ASLA doğrudan dokunmaz.
- **Aujard** (DB agent): DB ile konuşan TEK süreç. Ebenezer'dan kuyrukla istek alır.
  Tek `_readQueueThread` + tek `_dbAgent` (şu an seri).
- **VersionManager**: login/patch. Oyun verisiyle ilgisiz.
- **AIServer**: mob/NPC AI (TCP). Persistence'a karışmaz.

### Paylaşılan bellek + kuyruklar (`/private/tmp/boost_interprocess/`)
- **KNIGHT_DB**: shared memory bloğu (24MB sabit), `_USER_DATA[]` dizisi. **lsof ile doğrulandı:**
  Ebenezer (creator, OpenOrCreate) ve Aujard (Open) AYNI fiziksel belleği (aynı inode) map'liyor.
  Ebenezer `CUser.m_pUserData` ve Aujard `DBAgent.UserData[userId]` aynı bloğa işaret eder.
- **KNIGHT_SEND**: Ebenezer → Aujard (save/load/logout istekleri). FIFO.
- **KNIGHT_RECV**: Aujard → Ebenezer (yanıtlar).
- **ITEMLOG_SEND**: item olay/audit log kuyruğu (op-log tohumu — ADIM 4'te kullanılır).

### `_USER_DATA` (kalıcı alanlar)
`m_id, m_Accountid, m_bZone, m_curx/z/y, m_iGold, m_iExp, m_bLevel, m_iBank,`
`m_sItemArray[42] (envanter), m_sWarehouseArray[192] (depo), m_iLoyalty, stats..., m_dwTime`

> **`m_dwTime` = per-user monotonik VERSİYON sayacı.** `UpdateUser` her save'de `++`, `CheckUserData`
> DB'deki dwTime ile karşılaştırıp mismatch'i tespit eder. Bu, sıralama/optimistic-concurrency
> backstop'u (ADIM 3 için kritik).

### Veri akışı / DB ne zaman okunur-yazılır
| Olay | DB okuma | DB yazma |
|------|----------|----------|
| Karakter seç (login) | ✔ `LoadUserData` (DB→shared) | — |
| Oyun (combat/item/depo) | — | — (sadece shared RAM) |
| Periyodik (30sn timer) | — | ✔ `UpdateUser`+`UpdateWarehouseData` |
| Logout / Alt+F4 | — | ✔ (anında) |

- Login'de Ebenezer karakter verisini SHARED'den okur (SelChar yanıtı sadece "ok" flag'i taşır)
  → shared'in gerçekten coherent paylaşıldığının kanıtı.
- Oyun-içi mutasyonlar doğrudan shared `m_pUserData`'ya yazılır (upstream/twostars kodu).

### Tarihsel "relog'da item kaybı" — iki ayrı fix (referans)
- `65692cfe` (logic): canlı envanteri logout/save mesajına ekleme. **Sonradan anlaşıldı ki
  asıl sebep bu değildi** (shared zaten canlıydı) — yanlış teşhis, ama zararsız.
- `25f1135f` (driver): ODBC `AutoTranslate=no` + `SQL_C_BINARY` — binary item blob'unun
  (null byte'lar) charset çevirisiyle bozulmasını engeller. **Asıl fix buydu.**

---

## 2. Crash semantiği (kanıtlanmış)

Shared memory = kernel'in adlandırılmış nesnesi (tmpfs). TEK sefer allocate, ömrü kernel'de
(shm_unlink/reboot/restart-script rm'e kadar). **Bir süreç çökerse blok diğeri için geçerli kalır**
(yeniden-allocate yok, bozulma yok; istisna: çökme-anı yazılan tek kullanıcı torn olabilir).

**Heartbeat + post-mortem flush (ZATEN VAR):**
- Ebenezer her **10sn** `DB_HEARTBEAT` yollar.
- Aujard `CheckHeartbeat` (her 40sn): >**30sn** heartbeat yoksa → `AllSaveRoutine()` →
  TÜM shared belleği tarayıp herkesi DB'ye kaydeder.
- Ebenezer çökünce bellek DONAR (yazan yok) → flush çökme-anı state'ini yakalar → **~0 kayıp**.

### Dayanıklılık tablosu
| Çökme | Olasılık | Kayıp |
|-------|----------|-------|
| Temiz çıkış (logout) | — | ~0 (anında save) |
| Ebenezer process (makine sağ) | YÜKSEK | ~0 (heartbeat→post-mortem flush) |
| Aujard process (makine sağ) | DÜŞÜK | ~0 (bellek sağlam, restart'ta devam) |
| Tüm makine (elektrik/panic) | NADİR | ≤30sn (periyodik-save tabanı) |

→ 30sn penceresi YALNIZCA tüm-makine çökmesinde ısırır. N parametresi seçilebilir.

---

## 3. Hedef mimari — dört katman

```
KATMAN 1  AUTHORITATIVE STATE (Ebenezer, shared)   — canlı _USER_DATA, tek logic-mutex → her op atomik
   │ snapshot (logic-mutex altında → asla torn)
KATMAN 2  CHANGE CAPTURE                            — periyodik (dirty) + event (trade/kritik) snapshot
   │ durable queue (geri-basınç tamponu)
KATMAN 3  WRITE-BEHIND (Aujard, worker POOL)        — userId-partition, per-worker DB conn, paralel
   │
KATMAN 4  AUDIT/RECOVERY (ITEMLOG)                  — item serial + op-log → dupe/kayıp onarımı
```

### Üç değişmez ilke
1. **Snapshot HER ZAMAN logic-mutex altında alınır** (torn yok). Ucuz: mutex zaten logic'te tutulu.
2. **Aujard shared'e DOKUNMAZ, snapshot'tan (mesajdan) kaydeder** (rollback yok, self-contained → paralelleşir).
3. **Çok-varlıklı op'lar (trade) TEK DB transaction'ında** (yarım-trade crash'i imkânsız).

---

## 4. Uygulama adımları

### ADIM 1 — Doğruluk çekirdeği  *(UYGULANIYOR)*
**Amaç:** dupe + rollback'i bitir, cross-process named mutex'i kaldır, save-path'i sadeleştir.
**Değişiklikler:**
- Ebenezer `UserDataSaveToAgent`: snapshot'ı **`TcpSocketManager::GetMutex()` (mevcut logic-mutex)**
  altında al (named mutex yerine). WarehouseProcess zaten o mutex altında → snapshot tutarlı.
- Ebenezer `WarehouseProcess`: M7 v1'de eklenen `InterprocessMutexGuard`'ı KALDIR.
- Aujard `UserDataSave`: snapshot'ı SHARED'e YAZMA; LOCAL bir `_USER_DATA` snapshot'a parse et.
- Aujard `DBAgent::UpdateUser` / `UpdateWarehouseData`: `UserData[userId]` yerine **LOCAL snapshot'tan**
  oku. `m_dwTime` versiyon artışı: snapshot üstünde yapıp DB'ye yaz; shared `m_dwTime`'ı da güncel tut
  (CheckUserData/verify tutarlılığı için tek alan, dikkatli).
- `InterprocessMutex.*` artık kullanılmaz (dosya silinebilir veya bırakılır).
- `MAX_MSG_SIZE=4096` KALIR (snapshot büyük).
**Sonuç:** dupe yok, rollback yok, semaphore/deadlock-mitigation makinesi yok. Tek-worker'la bile geçerli.
**Doğrulama:** build temiz; server'lar kalkar; bir save round-trip eder; (canlı) warehouse deposit/
withdraw + trade dupe testi kullanıcı tarafından.

### ADIM 2 — Trade/depo atomikliği (event-transactional)  *(ERTELENDİ — binlerce için)*
**Sorun:** Bugün A↔B trade bellekte atomik ama persist koordinesiz (A ve B ayrı 30sn timer'da).
Crash A-sonrası/B-öncesi → yarım trade (dupe/kayıp).
**Çözüm:**
- `ExchangeDecide` (ve warehouse-op) → bellekte takas sonrası **HEMEN** A+B snapshot'ını
  "transactional pair" tek mesajda yolla.
- Aujard: `BEGIN TRAN; UpdateUser(A); UpdateUser(B); COMMIT;` — ya hep ya hiç.
- Pair routing: `worker(hash(min(A,B), max(A,B)))` → tek worker, tek transaction.
- item serial (int64, benzersiz) → ikinci savunma (dupe tespiti).
**Not:** warehouse (envanter↔depo, iki tablo) için de aynı tek-transaction kuralı.

### ADIM 3 — Ölçek: Aujard worker pool  *(ERTELENDİ — binlerce için)*
**Sorun:** Tek `_readQueueThread` + tek `_dbAgent` → DB yazımı seri (~125–500 save/sn tavan).
Ayrıca `AllSaveRoutine`'de 100ms/user sleep → 3000 user = 5 DAKİKA flush (post-mortem ölçeklenmiyor).
**Çözüm (Kafka partition-by-key mantığı):**
- Tek FIFO **dispatcher** KNIGHT_SEND'i okuyup `userId % N` ile worker'a route eder.
- N worker, her birinde **KENDİ DB bağlantısı** (`db::ConnectionManager` per-thread; ODBC thread-safe değil).
- Aynı user → hep aynı worker → **per-user FIFO korunur**. Farklı user'lar paralel → N× throughput.
- **Sıralama backstop:** `m_dwTime` versiyonu — reorder olsa bile stale (düşük versiyon) save
  reddedilir (mekanizma var; stale-reject'e bağlanır).
- Trade pair (ADIM 2): tek deterministik worker + tek transaction; versiyon cross-partition'ı güvenli yapar.
- `AllSaveRoutine`: 100ms sleep KALDIR + işi worker pool'a dağıt (paralel flush).
**Self-contained mesaj (ADIM 1) olmadan worker'lar güvenle paralelleşemez → sıra ADIM 1 sonrası.**

### ADIM 4 — (Opsiyonel) ~0 kayıp: write-ahead-log  *(ERTELENDİ — gerekirse)*
30sn tüm-makine penceresini ~0'a indirmek için: ITEMLOG'u durable append-only op-log'a çevir,
crash sonrası replay. Daha büyük inşaat; sadece ~0 kayıp şartı varsa.

---

## 5. Sıra ve gerekçe
```
ADIM 1 (doğruluk + sadeleştirme)  → her ölçekte doğru, gerisinin ön koşulu.   [şimdi]
ADIM 2 (trade atomikliği)          → yarım-trade'i bitirir.                    [yüzlerce+]
ADIM 3 (worker pool + flush ölçeği)→ DB tavanını kaldırır.                     [binlerce]
ADIM 4 (WAL)                       → ~0 kayıp.                                 [gerekirse]
```
ADIM 1 olmadan 2/3 güvensiz. 10 kişilik grup için ADIM 1 yeterli; "binlerce" için 1→2→3.
```
