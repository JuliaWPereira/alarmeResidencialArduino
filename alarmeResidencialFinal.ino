/*
 * Projeto de Alarme Residencial
 * 
 * created October 2019
 * by Júlia Wotzasek Pereira & Matheus Augusto dos Santos
 * final version 
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

// Variáveis do bargraph
const int qtdLeds = 10;
const int pinosLEDs[] = {26,27,28,29,30,31,32,33,34,35};
int bargraph = 0;


// Variáveis do sistema
int alarmeLigado = 0; // booleano para alarme ligado ou não
int primeiraVez = 1; // booleano para não ficar piscando o display LCD
int pir; // variável de leitura do sensor de presença
char digito; // variável de leitura do teclado
int leSenha = 0; // variável booleana para que se leia a senha (condição em ativação e desativação de alarme e mudança de senha)
int contadorDigitos; // variável de controle da leitura da senha
const int NDIGITOS = 4; // tamanho das senhas
int ligarAlarme = 0; // variável que indica a intenção de ligar o alarme. Precisa da confirmação da senha
int senhaDigitada[NDIGITOS];
int senhaSistema[NDIGITOS] = {1,2,3,4};
int desligarAlarme = 0; // variável que indica a intenção de desligar o alarme. Precisa da confirmação da senha
int mudarSenha = 0; // variável que indica a intenção de mudar a senha. Precisa da confirmação da senha
int senhaDeveMudar = 0; // variável que autoriza mudar a senha do Sistema
int tempo = 5; // tempo do timer - 5 segundos de tempo default
int recebeTempo = 0; // variável para receber tempo
int contadorDisplay = 0;
int bargraphAtivo = 0;
int terminouTimer = 0;
int contadorBargraph;
int decresceTimer = 0;
int deveLigarAlarme = 0;

// Procedimento para leitura do teclado
char leDigito(){
  char digit = teclado.getKey();
  if(!digit){
    return 'e'; // não é um digito
  }
  else{
    int d = (int)digit;
    tom = 420 + 100 * (d - 48);
    digitalWrite(buzzer,HIGH);
    delayMicroseconds(tom);
    delay(100);
    digitalWrite(buzzer,LOW);
    delayMicroseconds(tom);
    delay(100);
    //lcd.clear();
    return digit;
  }
}

// Procedimento para verificação de senha
int verificaSenha(){
  int i;
  for(i = 0; i < NDIGITOS; i++){
    if(senhaDigitada[i] != senhaSistema[i]){
      return 0; // senha incorreta
    }
  }
  return 1; // senha correta
}

// Procedimento que chama as opções do menu
void executaMenu(){
  switch(digito){
    case 'A': // botão para ligar o alarme
      if((leSenha == 0) && (alarmeLigado == 0)){
        leSenha = 1;
        contadorDigitos = NDIGITOS; // reseta o contador
        ligarAlarme = 1;
        lcd.setCursor(0,1);
        lcd.print("Senha:");
      }
      break;
    case 'B': // botão para desligar o alarme
      if((leSenha == 0) && (alarmeLigado == 1)){
        leSenha = 1;
        contadorDigitos = NDIGITOS;
        desligarAlarme = 1;
        lcd.setCursor(0,1);
        lcd.print("Senha:");
      }
      break;
    case 'C': // botão para mudar a senha
      if(alarmeLigado == 1){
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Alarme Ligado!");
        lcd.setCursor(1,1);
        lcd.print("Acesso Negado!");
        delay(3000);
        lcd.clear();
      }else{
        if(leSenha == 0){
          leSenha = 1;
          mudarSenha = 1;
          contadorDigitos = NDIGITOS;
          lcd.setCursor(0,1);
          lcd.print("Senha:");
        }
      }
      break;
    case 'D': // setar o timer de disparo do alarme
       if(alarmeLigado == 1){
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("Alarme Ligado!");
        lcd.setCursor(1,1);
        lcd.print("Acesso Negado!");
        delay(3000);
        lcd.clear();
       }else if(recebeTempo == 1){
        recebeTempo = 0;
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("Timer");
        lcd.setCursor(3,1);
        lcd.print("Alterado");
        delay(3000);
        lcd.clear();
       }else{
        lcd.setCursor(0,1);
        lcd.print("Tempo(s):");
        recebeTempo = 1;
        tempo = 0;
       }
       break;
    case '#':
      if(leSenha == 1){
        contadorDigitos = NDIGITOS;
        int i;
        for(i = 0; i < NDIGITOS; i++){
          lcd.setCursor(6 + i,1);
          lcd.print(" ");
        }
      }
      break;
    case '*':
      if(leSenha == 1){
        if(contadorDigitos > 0){
          lcd.setCursor(6 + (NDIGITOS - contadorDigitos - 1),1);
          contadorDigitos += 1;
          lcd.print(" ");
        }
      }
      break;
    default: // leitura dos números
       if((leSenha == 1) && (digito != 'e')){
         senhaDigitada[NDIGITOS - contadorDigitos] = (int)digito - 48; // arruma digito para int
         lcd.setCursor(6 + (NDIGITOS - contadorDigitos),1);
         lcd.print("*");
         contadorDigitos -= 1;
         if(contadorDigitos == 0){ // acabou de receber a senha digitada
           if(senhaDeveMudar == 1){
             leSenha = 0;
             int i;
             for(i = 0;i < NDIGITOS; i++){
               senhaSistema[i] = senhaDigitada[i];
             }
             senhaDeveMudar = 0;
             lcd.clear();
             lcd.setCursor(6,0);
             lcd.print("Senha");
             lcd.setCursor(4,1);
             lcd.print("Alterada");
             delay(3000);
             lcd.clear();
           }else{
             int senhaVerificada = verificaSenha();
             leSenha = 0;
             if(senhaVerificada == 1){
               lcd.clear();
               if(ligarAlarme == 1){
                 deveLigarAlarme = 1;
               }
               if(desligarAlarme == 1){
                 alarmeLigado = 0;
                 deveLigarAlarme = 0;
                 desligarAlarme = 0;
               }
               if(mudarSenha == 1){
                 leSenha = 1;
                 senhaDeveMudar = 1;
                 mudarSenha = 0;
                 contadorDigitos = NDIGITOS;
                 lcd.setCursor(0,1);
                 lcd.print("Senha:");
                 deveLigarAlarme = 0;
                 
               }
             }else{
               lcd.clear();
               lcd.setCursor(5,0);
               lcd.print("Senha");
               lcd.setCursor(3,1);
               lcd.print("Incorreta");
               delay(3000);
               lcd.clear();
             }
           }
         }
       }
       if((recebeTempo == 1) && (digito != 'e')){
         tempo = 10 * tempo + (int)digito - 48;
         lcd.setCursor(9, 1);
         lcd.print(tempo);
       }
  }
}

void acendeBargraph(){
  if(deveLigarAlarme == 1 && bargraphAtivo == 0){
    bargraphAtivo = 1;
    contadorBargraph = tempo;
    decresceTimer = 1;
  }if(bargraphAtivo == 1){
    bargraph = map(contadorBargraph,0,tempo,0,9);
    for (int i=0; i< qtdLeds; i++) {
      if (i < bargraph){
        digitalWrite(pinosLEDs[i], HIGH);
      }
      else{
        digitalWrite(pinosLEDs[i],LOW);
      }
    }
    contadorBargraph -= 1;
    delay(1000);
    if(contadorBargraph < 0){
      if(ligarAlarme == 1){
        alarmeLigado = 1;
        bargraphAtivo = 0;
        deveLigarAlarme = 0;
        decresceTimer = 0;
      }
    }
  }
}

// Setup do jogo
void setup(){
  // Buzzer
  pinMode(buzzer,OUTPUT);

  // Display LCD
  lcd.init();
  lcd.setBacklight(HIGH);

  // bargraph
  int i;
  for (i = 0; i < qtdLeds; i++) {
    pinMode(pinosLEDs[i], OUTPUT);
  }
  
}

// Loop Principal
void loop(){
  digito = leDigito(); // le o digito do teclado
  pir = digitalRead(PIR); // le o sensor de presença
  executaMenu();
  acendeBargraph();
  if(alarmeLigado == 1){
    if(primeiraVez == 1){
      lcd.clear();
      primeiraVez = 0;
    }
    lcd.setCursor(0,0);
    lcd.print("Alarme On!");
    if(pir){
      //                                                                                                                                                                              tocaAlarme();
      digitalWrite(buzzer,HIGH);
      delayMicroseconds(100);
    }else{
      digitalWrite(buzzer,LOW);
      delayMicroseconds(100);
    }
  }else{
    lcd.setCursor(0,0);
    lcd.print("Alarme Off!");
    digitalWrite(buzzer,LOW);
    delayMicroseconds(100);
    primeiraVez = 1;
  }
}
