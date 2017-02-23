1. Treść zadania:
Program wykonujący komendę na X komputerach równolegle, topologia płaska oraz drzewiasta.

2. Protokół komunikacji
Brak skomplikowanego protokołu, komenda jest wysyłana, odbierana i konsumowana natychmiastowo.

3. Ogólnie przyjęty model komunikacji
Każdy proces (oprócz matki) jest jednocześnie klientem i serwerem.
Kiedy włączamy program podajemy ip oraz port do których chcemy się podłączyć (użyto INADDR_ANY, u mnie działa ip 127.0.0.1.)
Z serwera matki podajemy komendy które propagowane są na klienty, które propagują je na swoje klienty,i tak dalej, i tak dalej..

4. Używamy zgodnie
Jako ip podawać 127.0.0.1, port taki jaki podalismy w procesie do ktorego chcemy się podłączyć.
Wysłanie komendy end kończy pracę wszystkich procesów.
Kompilacja: 'make -f makefile'
Testowane na ubuntu 14.04.
