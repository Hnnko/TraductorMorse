/*
 * PROYECTO: GUANTE TRADUCTOR MORSE (v4.1 - LCD)
 * PLACA: Arduino Nano
 * DESC: Lee un pulsador para traducir morse a una LCD 16x02,
 * con borrado por pulsación larga.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Configuración Global ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int pinBoton = 2;

const long tiempoDIT = 250;
const long tiempoPausaLetra = 1000;
const long tiempoPausaPalabra = 3000;
const long tiempoBorrarPantalla = 5000;
const long tiempoDebounce = 50;

// --- Variables Globales ---
bool botonEstaPresionado = false;
unsigned long tiempoPresionado = 0;
unsigned long tiempoLiberado = 0;
String bufferLetraActual = "";
bool comandoBorrarEjecutado = false;
bool pausaLetraYaEjecutada = true;
bool pausaPalabraYaEjecutada = true;
int cursorCol = 0;
int cursorRow = 0;

// --- Diccionario Morse (Hardcodeado) ---
const char diccionarioCaracteres[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
};
const String diccionarioMorse[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
  "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
  ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----"
};
const int tamanoDiccionario = sizeof(diccionarioCaracteres) / sizeof(diccionarioCaracteres[0]);


/**
 * @brief Configura el hardware inicial (LCD y Pin del botón).
 */
void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(pinBoton, INPUT_PULLUP);
  tiempoLiberado = millis();
}

/**
 * @brief Bucle principal: gestiona la máquina de estados
 * para DIT, DAH, Pausas y Borrado.
 */
void loop() {
  bool estadoActual = (digitalRead(pinBoton) == LOW);
  unsigned long tiempoActual = millis();

  // --- 1. Lógica de PRESIÓN ---
  if (estadoActual == true) {
    if (botonEstaPresionado == false) {
      if (tiempoActual - tiempoLiberado > tiempoDebounce) {
        tiempoPresionado = tiempoActual;
        botonEstaPresionado = true;
        comandoBorrarEjecutado = false;
        pausaLetraYaEjecutada = false;
        pausaPalabraYaEjecutada = false;
      }
    } else {
      unsigned long duracionPresion = tiempoActual - tiempoPresionado;
      
      // Comando de Borrado por 5 segundos
      if (duracionPresion > tiempoBorrarPantalla && !comandoBorrarEjecutado) {
        limpiarPantallaCompleta();
        comandoBorrarEjecutado = true;
        bufferLetraActual = "";
      }
    }
  }

  // --- 2. Lógica de LIBERACIÓN (Guardando DIT/DAH) ---
  else {
    if (botonEstaPresionado == true) {
      if (!comandoBorrarEjecutado) {
        unsigned long duracionPresion = tiempoActual - tiempoPresionado;
        if (duracionPresion < tiempoDIT) {
          bufferLetraActual += ".";
        } else {
          bufferLetraActual += "-";
        }
      }
      tiempoLiberado = tiempoActual;
      botonEstaPresionado = false;
    }
  }

  // --- 3. Lógica de PAUSA (Traduciendo) ---
  if (botonEstaPresionado == false) {
    unsigned long duracionPausa = tiempoActual - tiempoLiberado;

    // Pausa de Palabra (3 seg)
    if (duracionPausa > tiempoPausaPalabra && !pausaPalabraYaEjecutada) {
      if (!pausaLetraYaEjecutada) {
        traducirYImprimir();
        pausaLetraYaEjecutada = true;
      }
      imprimirEnLCD(' ');
      pausaPalabraYaEjecutada = true;
    }
    // Pausa de Letra (1 seg)
    else if (duracionPausa > tiempoPausaLetra && !pausaLetraYaEjecutada) {
      traducirYImprimir();
      pausaLetraYaEjecutada = true;
    }
  }
}

/**
 * @brief Limpia la LCD y resetea los cursores.
 */
void limpiarPantallaCompleta() {
  lcd.clear();
  cursorCol = 0;
  cursorRow = 0;
}

/**
 * @brief Imprime un caracter en la LCD, gestionando el salto
 * de línea y el límite de pantalla.
 */
void imprimirEnLCD(char c) {
  // Salto de línea
  if (cursorCol > 15 && cursorRow == 0) {
    cursorCol = 0;
    cursorRow = 1;
  }

  // Pantalla llena
  if (cursorRow == 1 && cursorCol > 15) {
    return;
  }

  lcd.setCursor(cursorCol, cursorRow);
  lcd.print(c);
  cursorCol++;
}

/**
 * @brief Busca el buffer morse actual en el diccionario
 * e imprime el caracter correspondiente.
 */
void traducirYImprimir() {
  if (bufferLetraActual.length() == 0) {
    return;
  }

  // Busca en el diccionario
  for (int i = 0; i < tamanoDiccionario; i++) {
    if (diccionarioMorse[i] == bufferLetraActual) {
      imprimirEnLCD(diccionarioCaracteres[i]);
      bufferLetraActual = "";
      return;
    }
  }

  // Si no se encuentra (Error)
  imprimirEnLCD('?');
  bufferLetraActual = "";
}