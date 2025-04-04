#Human depth perception test device#


Hardware:
- ESP32
- 9g hobby servo
- steel wire (i used welding filler rod)
- 3D printed parts to guide the wire
-   thingiverse: 


# Stereonäkötestilaitteen Katseluetäisyys

Tässä dokumentissa lasketaan suositeltu katseluetäisyys stereonäkötestilaitteelle, jotta sillä voidaan mitata stereonäön tarkkuutta noin 10 kaarisekunnin tasolle asti.

## Testilaitteen Rakenne

Testi perustuu klassiseen Howard-Dolman -testin periaatteeseen. Laitteessa on kolme pystysuoraa tolppaa (tai sauvaa) rivissä. Koehenkilö katsoo tolppia tietytä etäisyydeltä. Keskimmäistä tolppaa voidaan liikuttaa servo-ohjatusti eteen- tai taaksepäin suhteessa reunimmaisiin tolppiin. Koehenkilön tehtävä on arvioida, onko keskimmäinen tolppa lähempänä vai kauempana kuin reunimmaiset.

![Testin periaatekuva](stereonäkötesti.png)


**Laitteiston parametrit:**
* Tolppien välinen sivuttaisetäisyys: n. 10 mm
* Keskimmäisen tolpan liikerata (syvyyssuunnassa): n. 18 mm (oletetaan ±9 mm keskikohdasta)

## Laskennan Perusteet

Stereonäön tarkkuus (η) kuvaa pienintä syvyyseroa, jonka henkilö pystyy havaitsemaan kahden eri etäisyydellä olevan kohteen välillä. Se ilmaistaan usein kulmayksiköissä (kaarisekunteina) ja riippuu silmien välisestä etäisyydestä (IPD), kohteiden välisestä fyysisestä syvyyserosta (Δd) ja katseluetäisyydestä (D).

**Kaava:**
Stereotarkkuus radiaaneina voidaan approksimoida kaavalla:

`η (rad) ≈ (IPD * Δd) / D²`

Missä:
* `η`: Stereotarkkuus radiaaneina
* `IPD`: Silmien välinen etäisyys (Interpupillary distance) metreinä
* `Δd`: Kohteiden välinen syvyysero metreinä
* `D`: Katseluetäisyys metreinä

## Parametrit ja Laskenta

Tavoitteena on laskea etäisyys `D`, jolla laitteen suurin mahdollinen syvyysero `Δd_max` tuottaa juuri 10 kaarisekunnin parallaksieron.

1.  **Tavoiteltu stereotarkkuus (η):**
    * `η = 10 kaarisekuntia`
    * Muunnos radiaaneiksi: `10" = (10 / 3600) * (π / 180) rad ≈ 4.848 * 10⁻⁵ rad`

2.  **Silmien välinen etäisyys (IPD):**
    * Käytetään yleistä keskiarvoa: `IPD ≈ 65 mm = 0.065 m`
    * *Huom: Tämä on oletus, todellinen IPD vaihtelee.*

3.  **Suurin syvyysero (Δd_max):**
    * Laitteen liikerata on 18 mm, joten suurin siirtymä keskikohdasta on puolet siitä.
    * `Δd_max = 18 mm / 2 = 9 mm = 0.009 m`

4.  **Katseluetäisyyden (D) ratkaiseminen:**
    * Muokataan kaavaa: `D² ≈ (IPD * Δd) / η`
    * `D ≈ √[ (IPD * Δd_max) / η ]`
    * Sijoitetaan arvot:
        `D ≈ √[ (0.065 m * 0.009 m) / (4.848 * 10⁻⁵ rad) ]`
        `D ≈ √[ 0.000585 m² / 4.848 * 10⁻⁵ ]`
        `D ≈ √[ 12.067 m² ]`
        `D ≈ 3.47 m`

## Johtopäätös

Laskelman perusteella testilaitteen suositeltu katseluetäisyys on noin **3,5 metriä**.

Tällä etäisyydellä laitteen suurin fyysinen syvyyssiirtymä (9 mm) vastaa noin 10 kaarisekunnin stereotarkkuutta (olettaen 65 mm IPD). Tämä mahdollistaa testauksen tavoitellulla tarkkuustasolla laitteen mekaanisten rajoitusten puitteissa. Testiprotokollassa pyritään löytämään pienin havaittava siirtymä, mutta tämä laskettu etäisyys varmistaa, että 10 kaarisekunnin taso on saavutettavissa.
