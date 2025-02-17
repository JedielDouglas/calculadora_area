# Calculadora de área (retângulo e triângulo)

## Introdução
O presente projeto consiste em um programa em linguagem C capaz de calcular a área de um retângulo e de um triângulo, ao mesmo tempo, conforme os valores informados pelo usuário. O projeto utiliza a placa educacional BitDogLab que possui a Raspberry Pi Pico W integrada. 

### O projeto utiliza os seguintes componentes da placa: 
- Botões A e B: Utilizados para a entrada dos valores.
- Display OLED: Utilizado para exibir as instruções e os resultados dos cálculos 
- Matriz de LEDs : Utilizada para exibir os desenhos do retângulo e do triângulo.

Objetivo:
O objetivo deste projeto é permitir que o usuário informe os valores de base e altura para calcular:

Área do Retângulo: Calculada como base × altura.
Área do Triângulo: Calculada como (base × altura) / 2.

## Desenvolvimento:
### Etapa 1: Entendendo o projeto 
O programa segue os seguintes passos:

1. Exibição Inicial:
Exibe a mensagem "Calculadora de Área" no display OLED e, simultaneamente, na matriz de LEDs forma rapidamente o desenho de um retângulo (na cor verde) e de um triângulo (na cor azul).

2. Entrada de Dados:

- Base:
É exibida a mensagem "Informe a Base – Pressione A para iniciar". Com o botão A o valor é incrementado (iniciando em 1) e o botão B salva o valor, confirmando com a mensagem "Base Final Salvo".
- Altura:
Em seguida, após a escolha da base, é exibida a mensagem "Informe Altura – Pressione A para iniciar". A seleção funciona de maneira semelhante: botão A para incrementar e botão B para salvar, confirmando com a mensagem "Altura Final Salvo".

3. Cálculo e Exibição dos Resultados:
Após a escolha dos valores, o programa calcula:

- Área do Retângulo = base × altura.
- Área do Triângulo = (base × altura) / 2.

4. Exibição final:
Os resultados da área do retângulo e do triângulo, respectivamente, são exibidos no display OLED, enquanto a matriz de LEDs mostra os desenhos do retângulo (na cor verde) e do triângulo (na cor azul) para ilustrar.

## Etapa 2: Linguagem C

**1. Inclusão de biblioteca:** Inclui as bibliotecas padrão (stdio, string, stdlib) e também diversas bibliotecas específicas para a plataforma Pico, como "pico/stdlib.h" e "pico/binary_info.h". Em seguida, são incluídas bibliotecas para o display OLED (inc/ssd1306.h), para a comunicação I2C (hardware/i2c.h) e para a leitura do ADC (hardware/adc.h).
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
```
Além disso, há a inclusão da biblioteca "ws2818b.pio.h", que é gerada durante a compilação pelo arquivo .pio.
```c
// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"
```
**2. Definições e Configurações:** Define constantes para os pinos do I2C (SDA no pino 14 e SCL no 15), para os botões (A no pino 5 e B no 6) e para os LEDs (verde, azul e vermelho nos pinos 11, 12 e 13, respectivamente). Também são definidas variáveis globais para armazenar os valores da base e da altura, e outras constantes relacionadas a tempo e dimensões do display.
```c
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Defina os pinos dos botões e dos LEDs
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define LEDV 11  // Verde
#define LEDA 12  // Azul
#define LEDVM 13 // Vermelho

// Variáveis globais para armazenar a base e a altura
int base = 0;
int height = 0;

#define tempo_minimo 100
#define tempo_maximo 2900

#define DISPLAY_WIDTH 128  // Largura do display em pixels

int delay = 100;
```
Estrutura e Inicialização da Matriz de LEDs

Para controlar a matriz de LEDs, o código define:
- Uma constante LED_COUNT com valor 25, representando uma matriz 5x5.
- A estrutura pixel_t (renomeada como npLED_t) que armazena as componentes de cor em ordem GRB (importante para os LEDs WS2818B).
- Um vetor global leds[LED_COUNT] que guarda os valores de cor de cada LED.
```c
// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];
```
A função npInit inicializa a máquina PIO para enviar os sinais aos LEDs. Ela adiciona o programa ws2818b_program ao PIO e aloca um state machine. Ao final, o buffer de LEDs é limpo (todos com valor zero).
```c
// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {
    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
  
    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }
  
    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  
    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i) {
      leds[i].R = 0;
      leds[i].G = 0;
      leds[i].B = 0;
    }
  }
```
As funções auxiliares npSetLED, npClear e npWrite são responsáveis, respectivamente, por atribuir uma cor a um LED específico, limpar todo o buffer de LEDs e escrever o buffer para os LEDs. Em npWrite, cada componente (G, R, B) é enviada via pio_sm_put_blocking e, após enviar os dados de todos os LEDs, há uma pausa de 100 µs, sinal de RESET do datasheet.
Em seguida, a função getIndex converte coordenadas (linha e coluna) para o índice linear correspondente na matriz 5x5 (índice = linha * 5 + coluna).
```c
 /**
   * Atribui uma cor RGB a um LED.
   */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
  
  /**
   * Limpa o buffer de pixels.
   */
  void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
      npSetLED(i, 0, 0, 0);
  }

  /**
   * Escreve os dados do buffer nos LEDs.
   */
void npWrite() {
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i) {
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

/**
 * Converte coordenadas de linha e coluna para índice no buffer.
 */
int getIndex(int row, int col) {
    return row * 5 + col; // Converte coordenadas (linha, coluna) em índice linear.
}
```
**3. Funções para Desenhos na Matriz de LEDs**
```c
// desenhando um quadrado de led
void quadrado(uint8_t r, uint8_t g, uint8_t b) {
    npClear();
    // desenha um quadrado
    // Mapeamento corrigido do coração na matriz 5x5.
    int heart[5][5] = {
      {1, 1, 1, 1, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1}
    };
  
    // Define os LEDs que formam o desenho.
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (heart[row][col]) {
          int index = getIndex(row, col);
          npSetLED(index, r, g, b);
        }
      }
    }
    npWrite();
}

/**
 * Desenha na matriz quadrado
 */
void triangulo(uint8_t r, uint8_t g, uint8_t b) {
    npClear();
    // desenha sinal de mais +
    // Mapeamento corrigido do coração na matriz 5x5.
    int heart[5][5] = {
      {1, 1, 1, 1, 1},
      {0, 1, 1, 1, 0},
      {0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}
    };
  
    // Define os LEDs que formam o desenho.
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (heart[row][col]) {
          int index = getIndex(row, col);
          npSetLED(index, r, g, b);
        }
      }
    }
    npWrite();
  }

  
// apaga todos os leds
void apaga(uint8_t r, uint8_t g, uint8_t b) {
    npClear();
    // desenha um quadrado
    // Mapeamento corrigido do coração na matriz 5x5.
    int heart[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}
    };
  
    // Define os LEDs que formam o desenho.
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (heart[row][col]) {
          int index = getIndex(row, col);
          npSetLED(index, r, g, b);
        }
      }
    }
    npWrite();
}
```
**4. Função display_text** 

A função display_text é responsável por exibir mensagens no display OLED. Onde ela:
- Define uma área de renderização (frame_area) que cobre todo o display.
- Calcula o tamanho necessário do buffer para essa área.
- Zera o buffer (preenchendo com zeros) e desenha até quatro linhas de texto em posições fixas (por exemplo, y = 8, 38, 48 e 58).
- Chama render_on_display para atualizar o display e espera 100 ms para garantir a atualização visual.
```c
// Exibe um texto no display OLED
void display_text(const char *text1, const char *text2, const char *text3, const char *text4) {
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Zera o display antes de exibir novos textos
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, sizeof(ssd));  

    ssd1306_draw_string(ssd, 5, 8, text1);
    ssd1306_draw_string(ssd, 5, 38, text2);
    ssd1306_draw_string(ssd, 5, 48, text3);
    ssd1306_draw_string(ssd, 5, 58, text4);

    render_on_display(ssd, &frame_area);
    
    sleep_ms(100); // Pequeno atraso para garantir atualização
}
```
**5. Configuração dos Botões e Leitura dos Valores:** 

A função init_gpio configura os pinos dos botões A e B como entrada com resistor de pull-up. 

As funções get_base e get_height são responsáveis por ler os valores digitados pelo usuário:
- Ambas exibem uma mensagem inicial solicitando o valor (base ou altura) e aguardam a ação do usuário.
- Quando o botão A é pressionado (gpio_get retorna 0, pois há pull-up), o valor é incrementado e uma mensagem é exibida com o novo valor.
- Quando o botão B é pressionado, o valor atual é "salvo" (confirmado), uma mensagem de confirmação é exibida e a função retorna o valor após um atraso de 3 segundos.
```c
// Inicializa os GPIOs dos botões
void init_gpio() {
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
}

// Lê o valor da base
int get_base() {
    int value = 0;
    display_text("Informe a Base", "Pressione A", "Iniciar", "");

    while (true) {
        if (!gpio_get(BUTTON_A_PIN)) {
            value++;
            char message[20];
            sprintf(message, "Base: %d", value);
            display_text(message, "A INCREMENTAR", "B para salvar","");
            sleep_ms(200);
        }
        if (!gpio_get(BUTTON_B_PIN)) {
            char message[20];
            sprintf(message, "Base final: %d", value);
            display_text(message, "SALVO", "","");
            sleep_ms(3000);
            return value;
        }
    }
}

// Lê o valor da altura
int get_height() {
    int value = 0;
    display_text("Informe Altura","Pressione A", "Iniciar", "");

    while (true) {
        if (!gpio_get(BUTTON_A_PIN)) {
            value++;
            char message[20];
            sprintf(message, "Altura: %d", value);
            display_text(message, "A INCREMENTAR", "B para salvar","");
            sleep_ms(200);
        }
        if (!gpio_get(BUTTON_B_PIN)) {
            char message[20];
            sprintf(message, "Altura final: %d", value);
            display_text(message, "SALVO", "","");
            sleep_ms(3000);
            return value;
        }
    }
}
```
**6. Função principal (main):** 
Na função main, o fluxo do programa funciona da seguinte forma:

- Inicializa a comunicação padrão (stdio) e configura os GPIOs dos botões.
- Inicializa a comunicação I2C e configura os pinos SDA e SCL para o display OLED, além de habilitar os pull-ups.
- Inicializa o display OLED chamando ssd1306_init e exibe uma mensagem de boas-vindas ("Calculadora de Area").
- Inicializa a matriz de LEDs (npInit) e a limpa com npClear.
- Exibe dois desenhos iniciais: primeiro o quadrado (com cor verde, parâmetros 0,160,0) por 2 segundos e depois o "triângulo" (com cor azul, parâmetros 0,0,160) por 2 segundos. Em seguida, apaga a matriz.
- Chama get_base e get_height para obter os valores de base e altura do usuário.
- Calcula a área do retângulo (base * altura) e a área do triângulo (metade da área do retângulo).
- Prepara mensagens com os resultados e exibe no display OLED.
- Entra em um loop infinito onde alterna novamente os desenhos na matriz de LEDs (quadrado e triangulo) a cada 2 segundos, proporcionando uma exibição contínua dos símbolos das áreas calculadas.
```c
// Função principal
int main() {
    stdio_init_all();
    init_gpio();

    // Inicializa o display
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
 
    ssd1306_init();

    display_text("Calculadora", "de Area", "","");

    // Inicializa matriz de LEDs NeoPixel.
    npInit(LED_PIN);
    npClear();

    quadrado(0,160,0);
    sleep_ms (2000);
    
    triangulo(0,0,160);
    sleep_ms (2000);

    apaga(0,0,0);

    sleep_ms(1000);

    // Obter a base
    base = get_base();

    // Obter a altura
    height = get_height();

    // Calcular a área do retângulo
    int area_rectangle = base * height;

    // Calcular a área do triângulo
    int area_triangle = area_rectangle / 2;

    char message1[20], message2[20];
    sprintf(message1, "Retangulo: %d", area_rectangle);
    sprintf(message2, "Triangulo: %d", area_triangle);

    // Exibir resultados no display
    display_text("Resultados:", message1, message2,"");
   

    while (1) {
        quadrado(0,160,0);
        sleep_ms (2000);
    
        triangulo(0,0,160);
        sleep_ms (2000);
    }

    
}
```
## Código completo em C:
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Defina os pinos dos botões e dos LEDs
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define LEDV 11  // Verde
#define LEDA 12  // Azul
#define LEDVM 13 // Vermelho

// Variáveis globais para armazenar a base e a altura
int base = 0;
int height = 0;

#define tempo_minimo 100
#define tempo_maximo 2900

#define DISPLAY_WIDTH 128  // Largura do display em pixels

int delay = 100;

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {
    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
  
    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }
  
    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  
    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i) {
      leds[i].R = 0;
      leds[i].G = 0;
      leds[i].B = 0;
    }
  }
  
  /**
   * Atribui uma cor RGB a um LED.
   */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
  
  /**
   * Limpa o buffer de pixels.
   */
  void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
      npSetLED(i, 0, 0, 0);
  }

  /**
   * Escreve os dados do buffer nos LEDs.
   */
void npWrite() {
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i) {
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}


/**
 * Converte coordenadas de linha e coluna para índice no buffer.
 */
int getIndex(int row, int col) {
    return row * 5 + col; // Converte coordenadas (linha, coluna) em índice linear.
}
  
// desenhando um quadrado de led
void quadrado(uint8_t r, uint8_t g, uint8_t b) {
    npClear();
    // desenha um quadrado
    // Mapeamento corrigido do coração na matriz 5x5.
    int heart[5][5] = {
      {1, 1, 1, 1, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1}
    };
  
    // Define os LEDs que formam o desenho.
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (heart[row][col]) {
          int index = getIndex(row, col);
          npSetLED(index, r, g, b);
        }
      }
    }
    npWrite();
}

/**
 * Desenha na matriz quadrado
 */
void triangulo(uint8_t r, uint8_t g, uint8_t b) {
    npClear();
    // desenha sinal de mais +
    // Mapeamento corrigido do coração na matriz 5x5.
    int heart[5][5] = {
      {1, 1, 1, 1, 1},
      {0, 1, 1, 1, 0},
      {0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}
    };
  
    // Define os LEDs que formam o desenho.
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (heart[row][col]) {
          int index = getIndex(row, col);
          npSetLED(index, r, g, b);
        }
      }
    }
    npWrite();
  }

  
// apaga todos os leds
void apaga(uint8_t r, uint8_t g, uint8_t b) {
    npClear();
    // desenha um quadrado
    // Mapeamento corrigido do coração na matriz 5x5.
    int heart[5][5] = {
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}
    };
  
    // Define os LEDs que formam o desenho.
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 5; col++) {
        if (heart[row][col]) {
          int index = getIndex(row, col);
          npSetLED(index, r, g, b);
        }
      }
    }
    npWrite();
}


// Exibe um texto no display OLED
void display_text(const char *text1, const char *text2, const char *text3, const char *text4) {
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Zera o display antes de exibir novos textos
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, sizeof(ssd));  

    ssd1306_draw_string(ssd, 5, 8, text1);
    ssd1306_draw_string(ssd, 5, 38, text2);
    ssd1306_draw_string(ssd, 5, 48, text3);
    ssd1306_draw_string(ssd, 5, 58, text4);

    render_on_display(ssd, &frame_area);
    
    sleep_ms(100); // Pequeno atraso para garantir atualização
}

// Inicializa os GPIOs dos botões
void init_gpio() {
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
}

// Lê o valor da base
int get_base() {
    int value = 0;
    display_text("Informe a Base", "Pressione A", "Iniciar", "");

    while (true) {
        if (!gpio_get(BUTTON_A_PIN)) {
            value++;
            char message[20];
            sprintf(message, "Base: %d", value);
            display_text(message, "A INCREMENTAR", "B para salvar","");
            sleep_ms(200);
        }
        if (!gpio_get(BUTTON_B_PIN)) {
            char message[20];
            sprintf(message, "Base final: %d", value);
            display_text(message, "SALVO", "","");
            sleep_ms(3000);
            return value;
        }
    }
}

// Lê o valor da altura
int get_height() {
    int value = 0;
    display_text("Informe Altura","Pressione A", "Iniciar", "");

    while (true) {
        if (!gpio_get(BUTTON_A_PIN)) {
            value++;
            char message[20];
            sprintf(message, "Altura: %d", value);
            display_text(message, "A INCREMENTAR", "B para salvar","");
            sleep_ms(200);
        }
        if (!gpio_get(BUTTON_B_PIN)) {
            char message[20];
            sprintf(message, "Altura final: %d", value);
            display_text(message, "SALVO", "","");
            sleep_ms(3000);
            return value;
        }
    }
}

// Função principal
int main() {
    stdio_init_all();
    init_gpio();

    // Inicializa o display
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
 
    ssd1306_init();

    display_text("Calculadora", "de Area", "","");

    // Inicializa matriz de LEDs NeoPixel.
    npInit(LED_PIN);
    npClear();

    quadrado(0,160,0);
    sleep_ms (2000);
    
    triangulo(0,0,160);
    sleep_ms (2000);

    apaga(0,0,0);

    sleep_ms(1000);

    // Obter a base
    base = get_base();

    // Obter a altura
    height = get_height();

    // Calcular a área do retângulo
    int area_rectangle = base * height;

    // Calcular a área do triângulo
    int area_triangle = area_rectangle / 2;

    char message1[20], message2[20];
    sprintf(message1, "Retangulo: %d", area_rectangle);
    sprintf(message2, "Triangulo: %d", area_triangle);

    // Exibir resultados no display
    display_text("Resultados:", message1, message2,"");
   

    while (1) {
        quadrado(0,160,0);
        sleep_ms (2000);
    
        triangulo(0,0,160);
        sleep_ms (2000);
    }

    
}
```
## O vídeo de demonstração do funcionamento do código na placa pode ser acessado no link abaixo:
https://drive.google.com/file/d/1b0W4Djss8ov6PAhjsvUbE55ue-PUtAHG/view?usp=drive_link

