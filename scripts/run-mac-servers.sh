#!/usr/bin/env bash
# OpenKO — macOS server stack başlatıcı
# Tüm server'ları doğru sırada, ODBC env'leri ayarlı şekilde başlatır.
# Önkoşullar:
#   1. Server'lar derlenmiş olmalı: cmake --build build-mac
#   2. Docker SQL Server ayakta: bash docker/clean_setup.sh
#   3. FreeTDS kurulu + build-mac/odbc/{odbc.ini,odbcinst.ini} mevcut
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$REPO/build-mac/bin/Release"
export ODBCSYSINI="$REPO/build-mac/odbc"
export ODBCINI="$REPO/build-mac/odbc/odbc.ini"

cd "$BIN"
[ -f Notice.txt ] || echo "OpenKO Mac" > Notice.txt

echo "==> AIServer başlatılıyor (port 10020)..."
./AIServer > /tmp/aiserver.log 2>&1 &
AI_PID=$!
# AIServer dinlemeye başlayana kadar bekle
until lsof -nP -iTCP:10020 -sTCP:LISTEN >/dev/null 2>&1; do sleep 1; done

echo "==> Ebenezer başlatılıyor (port 15001)..."
./Ebenezer > /tmp/ebenezer.log 2>&1 &
EB_PID=$!
until grep -q "successfully initialized" /tmp/ebenezer.log 2>/dev/null; do sleep 1; done

echo "==> Aujard başlatılıyor (DB agent)..."
./Aujard > /tmp/aujard.log 2>&1 &
AJ_PID=$!

echo ""
echo "OpenKO server stack ayakta:"
echo "  AIServer  pid=$AI_PID  (10020)   log: /tmp/aiserver.log"
echo "  Ebenezer  pid=$EB_PID  (15001)   log: /tmp/ebenezer.log"
echo "  Aujard    pid=$AJ_PID  (DB agent) log: /tmp/aujard.log"
echo ""
echo "Durdurmak için: kill $AI_PID $EB_PID $AJ_PID"
