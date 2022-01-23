#include <random>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <fstream>

/* N - Niente: caselle che non spostano le pedine
 * L - Luoghi: strade, proprietà
 * P - Probabilità
 * S - Stazioni
 * I - Imprevisti
 * E - Società: Elettrica o Acqua potabile
 * R - pRigione: Vai in prigione */

//Versione originale
/*char spots[40] = {'N','L','P','L','N','S','L','I','L','L',
                  'N','L','E','L','L','S','L','P','L','L',
                  'N','L','I','L','L','S','L','L','E','L',
                  'R','L','L','P','L','S','I','L','N','L'};  */
//Versione "Città d'Italia"
char spots[40] = {'N','L','P','L','N','S','L','I','L','L',
                  'N','L','E','L','L','S','L','L','P','L',
                  'N','L','I','L','L','S','L','L','E','L',
                  'R','L','L','P','L','S','I','L','N','L'};

/* -1: Nothing: Carte che non spostano le pedine
 * -2: Avanzare fino alla società più vicina
 * -3: Fai 3 passi indietro (con tanti auguri)
 * -4: Avanzare fino alla prossima stazione
 * -5: ...in alternativa prendi un imprevisti (lo prende al 70%)
 * Altro (n >= 0): Spostarsi nella casella (spot) n */

//Versione originale 
//int probs[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 10};
//int imprs[16] = {-4, -4, -3, -2, -1, -1, -1, -1, -1, -1, -1, 0, 5, 11, 24, 39};

//Versione speciale "Città d'Italia"
int probs[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -5, 0, 10, 1};
int imprs[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -3, 11, 24, 0, 25, 39, 10};

int pIndex = 0;
int iIndex = 0;

std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> dice(1,6);

int position = 0;
bool inPrison = false;


void shuffleDeks() {
  std::shuffle(&probs[0], &probs[16], generator);
  std::shuffle(&imprs[0], &imprs[16], generator);
}

int rollDice(int* same = nullptr) {
  int first = dice(generator);
  int second = dice(generator);
  if (same) {
    if (first == second)
      *same++;
    else
      *same = 0;
  }
  
  return first + second;
}

void move(int num) {
  if (num < 2 || num > 12) {
    std::cerr << "STRANGE ERROR 1\n";
    return;
  }
  position += num;
  if (position > 39)
    position -= 40;
}

//calcola lo spostamento dopo imprevisti, probabilità o prigione. Ritorna true se la pedina si sposta
bool calculate() {
  switch (spots[position])
  {
  case 'P': {     //probabilità 
    int card = probs[pIndex]; //As fast as possible, accessing an array takes some little
    pIndex++;
    if (pIndex > 15) {
      pIndex -= 16;
    }

    if (card >= 0) {
      position = card;
      if (card == 10) 
        inPrison = true;

      return true;  //si sposta
    }

    if (card == -5) { //..in alternativa prendi imprevisti
      std::uniform_real_distribution<double> perc(0.1,1.0);
      if (perc(generator) >= 0.7) {
        return false;
      }
    }
    else {
      return false;   //non si sposta
    }
  }
  case 'I': {     //imprevisti
    int card = imprs[iIndex];
    iIndex++;
    if (iIndex > 15) {
      iIndex -= 16;
    }
    
    if(card >= 0) {
      position = card;
      return true;
    }
    
    switch (card)
    {
    case -2:  //società più vicina
      while(spots[position] != 'E') {
        position ++;
        if (position > 39)
          position -= 40;
      }
      return true;
    case -3:  //3 passi indietro
      position -= 3;
      if (position < 0)
        position += 40;
      return true;
    case -4:  //prossima stazione
      while(spots[position] != 'S') {
        position++;
        if (position > 39)
          position -= 40;
      }
      return true;
    }
    return false;
  }
  case 'R':       //prigione
    position = 10;
    inPrison = true;
    return true;
  }

  return false;
}

int main() {
  int count[40] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  std::cout << "Starting new simulation\n";
  //k partite
  for (int k = 0; k < 1000000; k++) {
    position = 0;
    shuffleDeks();
    int same = 0;

    //i mosse ogni partita
    for (int i = 0; i < 40; i++) {
      
      if (inPrison) {
        int same = 0;
        //Provo solo due volte a fare doppio perché la terza anche se non faccio doppio esco comunque di prigione (pagando)
        for (int i = 0; i < 2; i++) {
          int dice = rollDice(&same);
          if (same != 0) {
            inPrison = false;
            move(dice);
            count[position]++;
            break;
          }
        }
        inPrison = false;
      }
      
      move(rollDice(&same));
     
      if (same == 3) {
        position = 10;
        inPrison = true;
        same = 0;
      }

      count[position]++;

      if (calculate())
        count[position]++;
    }
  }
  
  for (int i = 0; i < 40; i++) {
    std::cout << count[i] << '\n';
  }

  std::ofstream out("data.txt");
  for (int i = 0; i < 40; i++) {
    out << count[i] << '\n';
  }
  out.close();

  std::cout << "Data saved.\n";
}