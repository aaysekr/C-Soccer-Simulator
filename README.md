# ⚽ ASCII Soccer Simulator (C-AI)

Bu proje, C programlama dili kullanılarak geliştirilmiş, tamamen konsol üzerinde çalışan interaktif bir futbol simülasyonudur. Oyuncular, top fiziği ve maç dinamikleri gerçek zamanlı bir döngü (Game Loop) üzerinden yönetilir.

## 🌟 Öne Çıkan Özellikler
- **Otonom Takım AI:** Oyuncular topun konumuna göre savunma, hücum ve pas verme kararlarını kendi başlarına alırlar.
- **Dinamik Spiker Sistemi:** Maçın gidişatına göre (gol, top kapma, şut) alt kısımda anlık Türkçe yorumlar güncellenir.
- **Top Fiziği:** Şut hızı, sürtünme ve saha kenarlarından sekme gibi temel fizik kuralları uygulanmıştır.
- **Maç Yönetimi:** İki devre (45'er dk), skor tablosu ve devre arası mekanikleri mevcuttur.
- **Görsel Arayüz:** `windows.h` ve ASCII karakterleri kullanılarak tasarlanmış detaylı bir futbol sahası.

## 🛠️ Teknik Altyapı
- **Dil:** C
- **Algoritmalar:**
  - **Player AI:** Öklid mesafesi tabanlı hedef belirleme ve "Best Pass Target" seçimi.
  - **Collision Detection:** Oyuncu-top ve saha sınırları arası çarpışma kontrolleri.
- **Veri Yapıları:** `struct` yapıları ile yönetilen oyuncu ve top nesneleri.
- **Grafik:** `WriteConsoleOutput` ve `screenBuffer` ile flicker-free (titremesiz) görüntü renderlama.

## 🕹️ Nasıl Oynanır?
1. Kod derlendikten sonra konsol ekranı otomatik olarak 120x34 boyutuna ayarlanır.
2. **SPACE** tuşu ile maçı başlatabilir, duraklatabilir veya devre aralarını geçebilirsiniz.
3. Maçın gidişatını spiker servisinden takip edebilirsiniz!

## 👨‍💻 Geliştirici Notu
Bu proje, düşük seviyeli programlama dillerinde oyun döngüsü mantığını ve basit yapay zeka karar mekanizmalarını anlamak amacıyla geliştirilmiştir.
<img width="1457" height="708" alt="image" src="https://github.com/user-attachments/assets/24571623-59ce-453e-b13f-85787523039e" />

