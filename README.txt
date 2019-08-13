PLIKI:
files\files.txt - plik z bezwzglednymi sciezkami do plikow tekstowych z ktorych rysowane sa widma
config.txt - ustawienie stalych dla programu parametrow
e0.txt - plik z przewidywanymi wartosciami e0 do ktorych dopasowywane sa gaussy (moze byc edytowany podczas dzialania programu)
gaussy\"nazwa pliku"_g[numer widma].txt - plik z dopasowanymi parametrami dla zadanych gaussow w e0

KLAWISZE:
w,s,a,d - przesuwanie wykresu widma (gora, dol, lewo, prawo)
q - przywrocenie domyslnego widoku wykresu widma
-,+ - zmniejszenie, powiekszenie wykresu widma
e,r - wczytanie poprzedniego/ nastepnego wykresu widma
f - dopasowanie gaussow zgodnie z zadanymi parametrami z config i e0
z - pokazuje/ ukrywa sciezke aktualnie wyswietlanego widma
space - postawienie nowego znacznika/ zaznaczenie istniejacego juz znacznika
backspace - usuniecie wskazanego myszka znacznika

PRZYCISKI:
DOPASUJ - dopasowanie gaussow zgodnie z zadanymi parametrami z config i e0
USTAWIENIA - otwiera ustawienia stalych dla programu parametrow
USTAW E0 - otwiera plik z przewidywanymi wartosciami e0 do ktorych dopasowywane sa gaussy (moze byc edytowany podczas dzialania programu)
<,> - wczytanie poprzedniego/ nastepnego wykresu widma
DODAJ WIDMO - dodaje do pliku files.txt kolejna sciezke
USUN WIDMO - usuwa aktualnie wyswietlane widmo z files\files.txt 

przeciagniecie myszy z wcisnietym prawym przyciskiem powoduje przyblizenie zaznaczonego obszaru wykresu widma 

UWAGI:
-Przy wiekszej liczbie zliczen zalecane jest zwiekszenie liczby iteracji i/lub A w config.txt lub klikajac USTAWIENIA w programie
-Przy mniejszej liczbie zliczen zalecane jest zmniejszenie liczby iteracji i/lub A w config.txt lub klikajac USTAWIENIA w programie
-Przytrzymanie spacji moze powodowac bledy z pozniejszym usuwaniem znacznikow 

Program napisany zostal z wykorzystaniem biblioteki OPENGL i biblioteki rozszerzajacej jej funkcjonalnosc GLUT (ang. GL Utility Toolkit). Wykorzystano takze biblioteke GTK+ 2.2.

Kod zrodlowy skompilowany zostal za pomoca programu Code::Blocks 17.12.

KONTAKT:
mat.nerkowski@gmail.com