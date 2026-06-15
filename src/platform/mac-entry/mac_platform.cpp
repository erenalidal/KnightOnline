// macOS port compat layer — see docs/MAC_PORT_ROADMAP.md
//
// mac_platform.cpp — macOS giriş noktası (main) + geçici D3D9 fabrika stub'ı.
//
// DURUM (Faz 1/2 sınırı):
//  - main(): C runtime'ın beklediği giriş noktası. Şimdilik doğrudan WinMain'i
//    çağırır. Faz 2'de burada SDL2 penceresi açılıp olay döngüsü kurulacak.
//  - Direct3DCreate9: dxvk-native kütüphanesi henüz build edilmediği için geçici
//    stub (nullptr döndürür → motor cihaz oluşturamaz, düzgünce çıkar). Faz 3'te
//    gerçek dxvk-native libdxvk_d3d9 ile değiştirilecek.

#include <win_types.h>
#include <d3d9.h>

// NOT: Direct3DCreate9 artık dxvk-native'in libdxvk_d3d9.dylib'inden gelir
// (Faz 3 tamamlandı). Geçici stub kaldırıldı.

// WarFareMain.cpp içindeki WinMain (aynı tip imzasıyla bildiriliyor → aynı mangling).
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);

int main(int /*argc*/, char** /*argv*/)
{
	static char emptyCmdLine[] = "";
	return WinMain(nullptr, nullptr, emptyCmdLine, 1 /*SW_SHOWNORMAL*/);
}
