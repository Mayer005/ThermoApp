# Ötlet

Lehetne olyan dolog, ami azt csinálja, hogy 3 módban tud lenni.
## 1. Periféria
- Ebben a módban azt tudja, hogy rácsatlakozik a gépre amin van bluetooth és hangvezérléssel lehet irányítani mondjuk a spotify nextet, stb.
- Ebben a módban 5 percenként logol egy humidity-t és egy temperature-t valamint egy légnyomást, számol belőlük magasságot, szól ha bizonyos értékek úgy jönnek ki, hogy kellene szellőztetni (bluetoothon küld egy olyan parancsot, amit az alkalmazás, ha van akkor szól, valamint a LED-et villogtatja ilyenkor valamilyen szinben) 

## 2. Otthoni hőmérés mód
- Ebben a módban szintén 5 percenként mér, de csak óránként számol magasságot, amúgy felesleges, feltételezi hogy nincs mozgatva. 
- Bluetoothon amennyiben csatlakozva van eszköz akkor az alkalmazásnak elküldi a dolgokat, az szép grafikont rajzol belőle, ugyan úgy szól ha szellőztetni kellene.  

## 3. Túra mód
- Ebben a módban gyakrabban, kb 10 mp-ként logol, csinál mindent is, mér sebességet, magasságot számol, hőmérsékletet el logolja, humidity-t szintén. Valamint az AI itt arra lesz alkalmatos, hogy lépéseket számolja az alapján hogyan rászkódik, így lehet kvázi lépésszámot is számolni. 

# Lehetséges optimalizálás
- Amennyiben van patika képlet a magassághoz, annyiban megfelelő ha csak az android applikáció számolja majd ki és más nem, szóval nem kell a chipnek ezt számolnia.

### Kiegészítési lehetőségek
- Amennyiben van rá mód meg lehetőség, jó lenne ha androidra csatlakozásnál le szinkronizálná az időt, majd utána a logoláshoz ezáltal lenne egy timestamp és akkor cpu-n tudná követni az időt, nem kell hogy oramu pontos legyen, ugyis szinkronizal amikor andrioid alkalmazásra csatlakozik, de azért jó lenne ha mondjuk 1 hét után még ilyen 10p pontosságot tudna.

### Logolás
- A logolás egy XML fájlokban történne meg, vagy JSON-ben attol fugg melyik takarekosabb. ugy kepzelem el, hogy a filesystem mogotte igy lenne 
```
./logs
    Mode1
        1.json
        2.json
    Mode2
        1.jsons
    Mode3
        1.json
```
Természetesen, nem végtelen a tárhely, ezért úgy tervezem, hogy timestamp alapján, minden héten egyszer lefut az, hogy az 1 évnél idősebb logokat eldobja. 
Viszont android alkalmazásba ha kiirok egy logot, akkor lesz gomb rá, hogy androidon elmentsem ezt.

## **VÁLTOZÁS**

- logolás az binárisan structban lesz, android app alakitja jsonbe.