# Gorsel alarm tanima sistemi hakkinda

Sistem kameralar uzerindeki istenen olaylari takip ederek istenen durumlarda alarm uretebilmektedir. Gorsel alarmlar bu dokumanda belirtilen API uzerinden baska sistemlere aktarilmak icin kullanilabilir.

API, analiz makinesi uzerinden 50043 baglanti noktasi uzerinden sunulmaktadir. Istekleri atmak icin:

> http://IP_ADRESI:50043 

uc noktasini kullanabilirsiniz. API'yi web uzerinden denemek icin uygun bir "swagger" [1] baglanti noktasi sunulmustur. Swagger erisimi icin:

> http://IP_ADRESI:50043/swagger/index.html

uc noktasini kullanabilirsiniz. API'de 5 farkli erisim uc noktasi bulunmaktadir:

## GET /EventSub/GetCurrentEvents

Bu uc nokta varolan olaylari listelemek icin kullanilabilir. Bu uc noktadan sadece cagri yapildigi an itibari ile aktif olan olaylar gelmektedir. Daha once gerceklesmis olaylara bu API'den erisim saglanamaz. API'nin herhangi bir parametresi yoktur, swagger uzerinden test edilebilecegi gibi su sekilde de test edilebilir:

```bash
curl -X GET "http://ip_adresi:50043/EventSub/GetCurrentEvents"
```

Ornek JSON cevabi:

```json
[
  {
    "startTs": 1698849683847,
    "cameraUuid": "5a4439ca",
    "cameraName": "Liman Gunduz Fixed",
    "type": "start",
    "event": "no safety vest",
    "image": "base64_image",
    "trackId": "3a8d63ce",
    "colorDistance": "63.993332"
  },
  {
    "startTs": 1698849637852,
    "cameraUuid": "7475571f",
    "cameraName": "OFIS_GENIS",
    "type": "start",
    "event": "no safety vest",
    "image": "",
    "trackId": "55f10e1e",
    "colorDistance": "57.275074"
  }
]
```

> Bu uc nokta ile calisirken event type olarak her zman 'start' gonderilir.

## GET /EventSub/GetEvents

Bu uc nokta GetCurrentEvents'dan farkli olarak gecmise donuk olarak olusmus olan alarmlari da almaya olanak tanir. Buradaki alarmlari anlamak icin 'type' anahtarinin degerlerini bilmek gereklidir. 2 turlu alarm tipi olabilir, baslangic (start) ve bitis (end). Her alarm durumu icin bir kez 'start' gonderilir, ve ilgili alarm kayboldugu zamanda bir kez 'end' gonderilir.

## POST /EventSub/Subscribe

GetEvents ve GetCurrentEvents GET tabanli uc noktalardir. Bu noktalardan alarm almak icin periyodik olarak sisteme istek atmaniz gereklidir. Buna karsin Subscribe/Unsubscribe uc noktalarini kullanarak, her yeni bir alarm tipinde sistemin istenen bir uzak noktaya notifikasyon gondermesini saglayabilirsiniz. Bunun icin bu uc noktaya 'url' alanina sahip bir json post etmeniz yeterli olacaktir. URL formati 'http://{ip_adres}:{port}/{path}' seklinde olmalidir.

Subscribe API'si ile post edilen event formatlari GetEvents uc noktasi ile bire bir aynidir.

## POST /EventSub/Unsubscribe

Bu uc noktaya var olan bir subscription'i kapatmak icin istek atabilirsiniz.

## POST /EventSub/ListSubscribes

Bu uc nokta sistem var olan subscription'lari listelemenize olanak verir.


[1] https://swagger.io/
