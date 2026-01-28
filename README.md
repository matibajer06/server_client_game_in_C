PROJEKT ZALICZENIOWY - GRA STRATEGICZNA
1. INSTRUKCJA KOMPILACJI:

Aby skompilować projekt, należy otworzyć terminal w katalogu z plikami źródłowymi i wpisać komendę: "make"

Spowoduje to utworzenie dwóch plików wykonywalnych: `server` oraz `client`.

Aby wyczyścić pliki binarne (np. przed ponowną kompilacją), użyj: "make clean"

2. INSTRUKCJA URUCHOMIENIA

Grę należy uruchamiać w osobnych terminalach w następującej kolejności:

1. Terminal 1 (Serwer):  ./server

2. Terminal 2 (Gracz A): ./client A

3. Terminal 3 (Gracz B): ./client B

Aby zakończyć działanie serwera i wyczyścić zasoby systemowe (IPC), należy w terminalu serwera użyć kombinacji klawiszy Ctrl+C lub doprowadzić grę do końca (5 punktów zwycięstwa).

3. OPIS PLIKÓW:

3.1 server.c
- Główny silnik gry.
- Inicjalizacja zasobów IPC (pamięć, kolejki, semafory).
- Logika upływu czasu (przyrost surowców, produkcja jednostek).
- Obsługę walki i obliczanie strat.
- Synchronizację stanu gry między graczami.
                  
3.2 client.c
- Interfejs użytkownika. Odpowiada za:
- Wyświetlanie stanu gry (surowce, kolejka, punkty).
- Pobieranie komend od użytkownika (trening, atak).
- Wysyłanie rozkazów do serwera.
                  
3.3 structures.h  
- Plik nagłówkowy współdzielony przez klienta i serwer.
- Zawiera definicje struktur, stałe (ID kolejek) oraz konfigurację jednostek.
                  
3.4 Makefile      
- Skrypt automatyzujący proces kompilacji projektu z flagą -Wall.
                  
3.5 PROTOCOL      
- Opis techniczny protokołu komunikacyjnego.

Mateusz Bajer 28.01.2026
