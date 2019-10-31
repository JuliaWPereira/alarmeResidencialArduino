/*
 * Projeto de Alarme Residencial
 * 
 * created October 2019
 * by Júlia Wotzasek Pereira & Matheus Augusto dos Santos
 * 
 */

// Bibliotecas Utilizadas
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// Definição das constantes referentes aos componetes do circuito
const int buzzer = 8;
const int PIR = 12;

// Inicialização do teclado
const byte linhas = 4; // 4 linhas
const byte colunas = 4; // 4 colunas

// teclado matricial
char matrizteclado[linhas][colunas] {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinoslinhas[linhas] = {46,47,48,49};
byte pinoscolunas[colunas] = {50,51,52,53};

// inicializa o teclado
Keypad teclado = Keypad( makeKeymap(matrizteclado), pinoslinhas, pinoscolunas, linhas, colunas );
int tom;


// Procedimento para leitura do teclado
int leDigito(){
  char digito = teclado.getKey();
  if(!digito){
    return -1; // não é um digito
  }
  else{
    //lcd.clear();
    lcd.setCursor(1,0);
    lcd.print(digito);
    int d = (int)digito;
    tom = 420 + 100 * (d - 48);
    digitalWrite(buzzer,HIGH);
    delayMicroseconds(tom);
    delay(100);
    digitalWrite(buzzer,LOW);
    delayMicroseconds(tom);
    delay(100);
    //lcd.clear();
    return d - 48;
  }
}

// Setup do jogo
void setup(){
  // Buzzer
  pinMode(buzzer,OUTPUT);

  // Display LCD
  lcd.init();
  lcd.setBacklight(HIGH);
  
}

// Loop Principal
void loop(){
  // Le teclado
  int digito,pir;
  digito = leDigito();
  pir = digitalRead(PIR);
  lcd.setCursor(1,1);
  lcd.print(pir);

  if(pir){
    digitalWrite(buzzer,HIGH);
  }else{
    digitalWrite(buzzer,LOW);
  }



  
}
