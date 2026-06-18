-- #21: Weapon/Armor Enchant Scroll'ları çalışır hale getir (geçici buff)
--
-- SORUN: "Weapon Enchant Scroll" (item 389155000, Kind=97, magic 500049) ve "Armor Enchant
-- Scroll" (item 389156000, magic 500050) kullanılınca "using" diyor ama hiçbir etki olmuyordu.
-- KÖK NEDEN: MAGIC tablosunda bu magic'ler Type1=1 (MAGIC_TYPE1/melee) + Type2=4 (MAGIC_TYPE4/
-- buff) diye işaretliydi, AMA ne MAGIC_TYPE1 ne MAGIC_TYPE4'te o iNum için EFEKT SATIRI vardı.
-- Üstelik MagicProcess dispatch'i Type2'yi (buff) yalnızca Type1 başarılı olursa çalıştırıyor
-- (initial_result != 0); MAGIC_TYPE1 satırı olmadığı için Type1 fail → buff hiç koşmuyordu → no-op.
--
-- ÇÖZÜM (data-only, kod değişikliği yok): magic'i doğrudan buff yap (Type1=4, Type2=0 → ExecuteType4
-- koşulsuz çalışır) ve MAGIC_TYPE4'e buff efekt satırını ekle. Tasarım niyeti zaten buff'tı (Type2=4).
--   * Weapon Enchant Scroll  -> BuffType 4 (AttackPower), Attack=120  => +%20 saldırı (baseline 100)
--   * Armor  Enchant Scroll  -> BuffType 2 (Armor/AC),   AC=100      => +100 düz AC
--   * Duration=1800 sn (30 dk). Mevcut sınıf buff'ları daha güçlü (Attack 120-150, AC 400-800) ama
--     kısa süreli (15-300sn); enchant scroll uzun süreli consumable olduğu için mütevazı tutuldu.
-- Değerler kolayca ayarlanabilir (Attack/AC/Duration güncelle + Ebenezer restart → magic tablosu reload).
--
-- NOT: Bu magic'ler client'ın "use item" akışıyla cast ediliyor; buff slotu (BuffType 4/2) sınıf
-- attack/AC buff'larıyla ortaktır (ikisi aynı anda olmaz — normal KO davranışı).

-- 1) Magic'i koşulsuz buff'a çevir (Type1=4 buff doğrudan çalışır, Type2 bağımlılığı kalkar)
UPDATE MAGIC SET Type1 = 4, Type2 = 0 WHERE MagicNum IN (500049, 500050, 500051);

-- 2) Buff efekt satırlarını (yeniden) oluştur — idempotent
DELETE FROM MAGIC_TYPE4 WHERE iNum IN (500049, 500050, 500051);

-- iNum, Name, Description, BuffType, Radius, Duration, AttackSpeed, Speed, AC, ACPct, Attack,
-- MagicAttack, MaxHP, MaxHpPct, MaxMP, MaxMpPct, HitRate, AvoidRate, Str, Sta, Dex, Intel, Cha,
-- FireR, ColdR, LightningR, MagicR, DiseaseR, PoisonR, ExpPct
INSERT INTO MAGIC_TYPE4 (iNum, Name, Description, BuffType, Radius, Duration, AttackSpeed, Speed,
  AC, ACPct, Attack, MagicAttack, MaxHP, MaxHpPct, MaxMP, MaxMpPct, HitRate, AvoidRate,
  Str, Sta, Dex, Intel, Cha, FireR, ColdR, LightningR, MagicR, DiseaseR, PoisonR, ExpPct)
VALUES
  -- Weapon Enchant Scroll: +%20 saldırı, 30 dk
  (500049, 'Weapon Enchant Scroll', '+20% attack 30m', 4, 0, 1800, 0, 0, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
  -- Armor Enchant Scroll: +100 AC, 30 dk
  (500050, 'Armor Enchant Scroll', '+100 AC 30m',      2, 0, 1800, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
  -- Weapon Enchant Scroll (varyant 800061000): +%20 saldırı, 30 dk
  (500051, 'Weapon Enchant Scroll', '+20% attack 30m', 4, 0, 1800, 0, 0, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
