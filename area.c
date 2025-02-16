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
