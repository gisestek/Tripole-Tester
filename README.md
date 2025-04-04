#Human depth perception test device#


Hardware:
- ESP32
- 9g hobby servo
- steel wire (i used welding filler rod)
- 3D printed parts to guide the wire
-   thingiverse: 



Kaava:
Stereotarkkuus (η, radiaaneina) ≈ (Silmien välinen etäisyys (IPD) * Syvyysero (Δd)) / (Katseluetäisyys (D))²

Tiedossa olevat ja oletetut arvot:

Tavoiteltu stereotarkkuus (η): 10 kaarisekuntia.
Muunnetaan radiaaneiksi:
1 aste = 3600 kaarisekuntia
10 kaarisekuntia = 10 / 3600 astetta ≈ 0.002778 astetta
Radiaaneina: η = (10 / 3600) * (π / 180) ≈ 4.848 * 10⁻⁵ radiaania.
Silmien välinen etäisyys (IPD): Tätä ei annettu, mutta käytetään yleistä keskiarvoa 65 mm = 0.065 m. Huomaa, että tämä on oletus ja todellinen IPD vaihtelee henkilöittäin.
Syvyysero (Δd): Tämä on se fyysinen etäisyys etu- tai takasuunnassa, jonka keskimmäinen tolppa liikkuu suhteessa reunimmaisiin tolppiin. Kysymyksessä mainitaan keskimmäisen tolpan liikerata olevan noin 18 mm. Oletetaan, että tämä tarkoittaa ±9 mm liikettä keskikohdasta (reunimmaisten tolppien tasosta). Jotta voisimme testata vähintään 10 kaarisekunnin tarkkuutta, meidän tulee laskea etäisyys, jolla laitteen suurin tuottama syvyysero (Δd_max = 9 mm) vastaa 10 kaarisekunnin parallaksia.
Δd_max = 9 mm = 0.009 m.
Laskenta:
Ratkaistaan kaavasta katseluetäisyys D:
D² ≈ (IPD * Δd) / η
D ≈ √[ (IPD * Δd) / η ]

Sijoitetaan arvot kaavaan käyttäen laitteen suurinta syvyyseroa (Δd_max) ja tavoitetarkkuutta (η):
D ≈ √[ (0.065 m * 0.009 m) / (4.848 * 10⁻⁵) ]
D ≈ √[ (0.000585 m²) / (4.848 * 10⁻⁵) ]
D ≈ √[ 12.067 ] m
D ≈ 3.47 m

Tulkinta:
Laskelman mukaan testilaitteen tulisi olla noin 3,5 metrin etäisyydellä koehenkilöstä. Tällä etäisyydellä keskimmäisen tolpan liikuttaminen sen maksimiasentoon (9 mm eteen tai taakse reunimmaisiin nähden) aiheuttaa silmissä noin 10 kaarisekunnin suuruisen parallaksieron.

Jos henkilö pystyy tällä etäisyydellä erottamaan, onko maksimisiirtymällä oleva keskimmäinen tolppa lähempänä vai kauempana, hänen stereonäkönsä tarkkuus on vähintään 10 kaarisekuntia. Testi itsessään pyrkii löytämään pienemmän siirtymän (ja siten pienemmän parallaksikulman), jonka henkilö vielä erottaa, mutta 3,5 metriä on etäisyys, jolla 10 kaarisekunnin tavoitetaso on saavutettavissa laitteen liikeradan puitteissa.
