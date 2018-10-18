/* Jakub Cichy, Przetwarzanie obrazow */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define Max 512                 /* maksymalny rozmiar wczytywanego obrazu */
#define DL_LINII 1024           /* dlugosc buforow pomocnicznych */
#define PLIK "nowyobraz.txt"    /* definiujemy plik do ktorego zostanie zapisany obraz po wykonanych operacjach */

/************************************************************************************
 * Funkcja wczytuje obraz PGM z pliku do tablicy       	       	       	       	    *
 *                                                                                  *
 * \param[in] plik_we uchwyt do pliku z obrazem w formacie PGM                      *
 * \param[out] obraz_pgm tablica, do ktorej zostanie zapisany obraz                 *
 * \param[out] wymx szerokosc obrazka                                               *
 * \param[out] wymy wysokosc obrazka                                                *
 * \param[out] szarosci liczba odcieni szarosci                                     *
 * \return liczba wczytanych pikseli                                                *
 ************************************************************************************/

int czytaj(FILE *plik_we, int obraz_pgm[][Max], int *wymx, int *wymy, int *szarosci){
    char buf[DL_LINII];     /* bufor pomocniczy do czytania nagłówka i komentarzy */
    int znak;               /* zmienna pomocniczna do czytania komentarzy */
    int koniec=0;           /* czy napotkano koniec danych w pliku */
    int i, j;
    
    /* czy podano prawdilowy uchwyt do pliku? */
    if(plik_we==NULL){
        fprintf(stderr, "Blad. Nie podano uchwytu do pliku.\n");
        return 0;
    }
    
    /* Sprawdzanie "numeru magicznego" - powinno być P2 */
    if(fgets(buf, DL_LINII, plik_we)==NULL){            /* Wczytanie pierwszej linii do bufora */
        koniec=1;                                       /* Nie udało się - koniec danych */
    }
    
    if(buf[0]!='P' && buf[1]!='2'){                     /* Czy mamy "numer magiczny"? */
        fprintf(stderr, "Blad. To nie jest plik typu PGM\n");
        return 0;
    }
    
    /* Pominiecie komentarzy */
    do{
        if((znak=fgetc(plik_we)=='#')){                 /* Czy linia rozpoczyna się od '#'? */
            if(fgets(buf, DL_LINII, plik_we)==NULL){    /* Wczytujemy do bufora */
                koniec=1;                               /* Zapamietaj ewentualny koniec */
            }
        }
        else{
            ungetc(znak, plik_we);                      /* Gdy przeczytany znak nie jest '#', zwroc go */
        }
    } while(znak=='#' && !koniec);                      /* Powtarzaj tak dlugo jak sa linie komentarza i nie ma konca danych */
    
    /* Pobranie wymiarów i odcieni szarosci */
    if(fscanf(plik_we, "%d %d %d", wymx, wymy, szarosci)!=3){
        fprintf(stderr, "Blad. Brak wymiarow obrazu lub liczby stopni szarosci.\n");
        koniec=1;
    }
    
    /* Pobranie obrazu i zapisanie do talibcy: obraz_pgm */
    for(i=0; i<*wymy; i++){         /* po wierszach */
        for(j=0; j<*wymx; j++){     /* po kolumnach */
            if(fscanf(plik_we, "%d", &obraz_pgm[i][j])!=1){
                fprintf(stderr, "Blad. Niepoprawne wymiary obrazu.\n");
                return 0;
            }
        }
    }
    
    return (*wymx)*(*wymy);                              /* Zwraca liczbe wczytanych pikseli i oznacza ze czytanie zakonczylo sie sukcesem */
}

/* Wyswietlenie obrazu o zadanej nazwie za pomoca programu "display"   */
void wyswietl(char *n_pliku){
    char polecenie[DL_LINII];      /* bufor pomocniczy do zestawienia polecenia */
    
    strcpy(polecenie,"display ");  /* konstrukcja polecenia postaci */
    strcat(polecenie,n_pliku);     /* display "nazwa_pliku" &       */
    strcat(polecenie," &");
    printf("%s\n",polecenie);      /* wydruk kontrolny polecenia */
    system(polecenie);             /* wykonanie polecenia        */
}

/* Funkcja wykonujaca PROGOWANIE obrazu. Pozwala na odroznienie obiektu od tła              *
 * param[in] tablica dwuwymiarowa z wartosciami pikseli obrazu                              *
 * param[out] tablica z przeliczonymi wartosciami                                           */
void progowanie(int obraz_pgm[][Max], int *wymx, int *wymy, int *szarosci, float prog){
    prog = (prog/100)*(*szarosci);  /* wyliczenie wartosci progowej */
    for(int i=0; i<*wymy; i++){
        for(int j=0; j<*wymx; j++){
            if(obraz_pgm[i][j]>prog)
                obraz_pgm[i][j]=*szarosci;
            else
                obraz_pgm[i][j]=0;
        }
    }
}

/* Funkcja wykonujaca KONTUROWANIE obrazu, co pozwala wyostrzyc obrys przedmiotow na obrazie    *
 * param[in] tablica dwuwymiarowa z wartosciami pikseli obrazu                                  *
 * param[out] tablica z przeliczonymi wartosciami                                               */
void konturowanie(int obraz_pgm[][Max], int *wymx, int *wymy, int *szarosci){
    int i, j;
    
    /* Konturujemy piksele obrazu poza ramka */
    for(i=1; i<(*wymy-1); i++){
        for(j=1; j<(*wymx-1); j++){
            obraz_pgm[i][j]=(abs(obraz_pgm[i+1][j] - obraz_pgm[i][j]) + abs(obraz_pgm[i][j+1] - obraz_pgm[i][j]));
        }
    }
    
    /* Tworzymy ramke dla obrazu, wyczerniajac jego graniczne piksele */
    for(i=0, j=0; j<*wymx; j++){
        obraz_pgm[i][j]=*szarosci;
    }
    for(i=*wymy-1, j=0; j<*wymx; j++){
        obraz_pgm[i][j]=*szarosci;
    }
    for(j=0, i=1; i<*wymy-1; i++){
        obraz_pgm[i][j]=*szarosci;
    }
    for(j=*wymx-1, i=1; i<*wymy-1; i++){
        obraz_pgm[i][j]=*szarosci;
    }
}

/* Funkcja ROZCIAGANIE HISTOGRAMU. Nastepuje konwersja zakresu wartości, aby jasnosci pikseli obejmowały cały zakres jasnosci dostępny w obrazie    *
 * param[in] tablica dwuwymiarowa z wartosciami pikseli obrazu                                                                                      *
 * param[out] tablica z przeliczonymi wartosciami                                                                                                   */
void histogram(int obraz_pgm[][Max], int *wymx, int *wymy, int *szarosci){
    
    /* Szukamy wartosci min i max w tablicy obraz_pgm, porownujac ze soba kolejne jej elementy */
    int min, max, i, j;
    max=obraz_pgm[0][0];
    for(i=0; i<*wymy; i++){
        for(j=0; j<*wymx; j++){
            if(obraz_pgm[i][j]>max){
                max=obraz_pgm[i][j];
            }
        }
    }
    
    min=obraz_pgm[0][0];
    for(i=0; i<*wymy; i++){
        for(j=0; j<*wymx; j++){
            if(obraz_pgm[i][j]<min){
                min=obraz_pgm[i][j];
            }
        }
    }
    
    /* Wyliczamy nowe warotsci pikselow na podstawie odpowedniego wzoru */
    
    for(i=0; i<*wymy; i++){
        for(j=0; j<*wymx; j++){
            obraz_pgm[i][j]=(obraz_pgm[i][j] - min)*((int)((*szarosci)/(max-min)));
        }
    }
    
}

/* Funkcja KOREKCJA GAMMA. Przeskalowuje wartosci jasnosci pikseli obrazu.                  *
 * param[in] tablica dwuwymiarowa z wartosciami pikseli obrazu                              *
 * param[in] podanie liczbowej wartosci zmiennej "wspolczynnik"                             *
 * pram[out] tablica z przeliczonymi wartosciami                                            */
void Gamma(int obraz_pgm[][Max], int *wymx, int *wymy, int *szarosci, float wspolczynnik){
    int i, j;
    
    float a, b, x;
        /* Wyliczamy nowe wartosci */
    for(i=0; i<*wymy; i++){
        for(j=0; j<*wymx; j++){
            a=(float)obraz_pgm[i][j]/(*szarosci);
            b=1/wspolczynnik;
            x=pow(a, b);
            obraz_pgm[i][j]=x*(*szarosci);
        }
    }
}

/* Funckja pozwalajaca ZAPISAC tablice wynikowa do nowego pliku reprezentujacego nowy obraz                                     *
 * param[in] uchwyt do pliku w ktorym zapisany zostanie obraz                                                                   *
 * param[out] nowy plik                                                                                                         */
void zapisz(FILE *nowyplik, int obraz_pgm[][Max], int *wymx, int *wymy, int *szarosci){
    int i, j;
    
    /* piszemy do pliku */
    fprintf(nowyplik, "P2\n%d %d %d\n", *wymx, *wymy, *szarosci);
    for(i=0; i<*wymy; i++){
        for(j=0; j<*wymx; j++){
            fprintf(nowyplik, "%d ", obraz_pgm[i][j]);
            if(j==*wymx-1){
                fprintf(nowyplik, "\n");
            }
        }
    }
}

int main(){
    
    int obraz[Max][Max];                    /* do wczytywania */
    int wymx, wymy, szarosci;               /* dane obrazu */
    int odczytano = 0;
    FILE *plik;
    char nazwa[100];                        /* do wpisania nazwy pliku */
    char *dzialanie =" ";                   /* zmienna ulatwia komunikowanie */
    
    printf("Program do przetwarzania obrazow. Wybierz sposrod opcji:\n");

    char wybor[2]="4";                      /* wstepna deklaracja zmiennej pozwalajaca wejsc w petle */
    
    while (wybor[0] != '8'){
    printf("1 - Wczytaj plik\n");
    printf("2 - Zapisz plik\n");
    printf("3 - Progowanie\n");
    printf("4 - Konturowanie\n");
    printf("5 - Rozciaganie histogramu\n");
    printf("6 - Korekcja gamma\n");
    printf("7 - Wyswietl obraz\n");
    printf("8 - Zakoncz dzialanie\n");
    printf("Twoj wybor:\n");
    scanf("%1s", wybor);
    
    switch (wybor[0]){
            
        /* Wczytywanie pliku */
        case '1':{
            printf("Podaj nazwe pliku:\n");
            scanf("%s", nazwa);
            plik=fopen(nazwa, "r"); /* otwieramy plik w trybie czytania */
            odczytano=czytaj(plik, obraz, &wymx, &wymy, &szarosci);
            if(odczytano!=0){
            dzialanie="Pomyslnie wczytano plik";
            } else{
                dzialanie="Brak pliku. Ponownie wybierz opcje 1.";
            }
            fclose(plik);
            printf("%s\n", dzialanie);
        } break;

        /* Zapisywanie pliku */
        case '2':{
            if(odczytano!=0){
                printf("Podaj nazwe pliku:\n");
                scanf("%s", nazwa);
                plik=fopen(nazwa, "w"); /* otwieramy plik w trybie pisania */
                zapisz(plik, obraz, &wymx, &wymy, &szarosci);
                fclose(plik);
                dzialanie="Pomyslnie zapisano plik";
            }
            else{
                dzialanie="Brak pliku. Ponownie wybierz opcje 1.";}
            
            printf("%s\n", dzialanie);
        } break;
            
        /* Progowanie */
        case '3':{
            if(odczytano!=0){
                float prog;
                printf("Podaj wartosc progu (0-100):\n");
                scanf("%f", &prog);
                progowanie(obraz, &wymx, &wymy, &szarosci, prog);
                dzialanie="Progowanie zakonczone";
            }
            else{
                dzialanie="Brak pliku. Ponownie wybierz opcje 1.";}
            
            printf("%s\n", dzialanie);
        } break;
            
        /* Konturowanie */
        case '4':{
            if(odczytano!=0){
                konturowanie(obraz, &wymx, &wymy, &szarosci);
                dzialanie="Konturowanie zakonczone";
            }
            else{
                dzialanie="Brak pliku. Ponownie wybierz opcje 1.";}
            
            printf("%s\n", dzialanie);
        } break;
        
        /* Rozciaganie histogramu */
        case '5':{
            if(odczytano!=0){
                histogram(obraz, &wymx, &wymy, &szarosci);
                dzialanie="Rozciaganie histogramu zakonczone";
            }
            else{
                dzialanie="Brak pliku. Ponownie wybierz opcje 1.";}
            
            printf("%s\n", dzialanie);
        } break;
            
        /* korekcja gamma */
        case'6':{
            if(odczytano!=0){
                float wspolczynnik;
                printf("Podaj wspolczynnik (<1 przyciemnienie, >1 rozjasnienie)\n");
                scanf("%f", &wspolczynnik);
                Gamma(obraz, &wymx, &wymy, &szarosci, wspolczynnik);
                dzialanie="Korekcja gamma zakonczona";
            }
            else{
                dzialanie="Brak pliku. Ponownie wybierz opcje 1.";}
            
            printf("%s\n", dzialanie);
        } break;
            
            /* Wyswietlanie obrazu */
        case '7':{
            if(odczytano!=0){
                dzialanie="Obraz zostanie wyswietlony";
                wyswietl(nazwa);
            } else{
                    dzialanie="Brak pliku. Ponownie wybierz opcje 1.";
            }
            printf("%s\n", dzialanie);
        } break;
            
        case '8':{
            printf("Koniec\n");
        } break;
}
}
    return 0;
}
