#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

// Arquivo .pio
#include "pio_matrix.pio.h"

// =============================================
// DEFINIÇÕES DE HARDWARE E CONSTANTES
// =============================================

#define NUM_LEDS 25   // Número total de LEDs na matriz
#define PINO_SAIDA 7  // Pino de controle da matriz de LEDs

// Definição dos botões
const uint BOTAO_DESCE = 6;    // Botão para diminuir o andar
const uint BOTAO_SOBE = 5;     // Botão para aumentar o andar
const uint LED_INDICADOR = 11;  // LED RGB para indicação de movimento
const uint LED_VERMELHO = 13;  // Novo LED vermelho piscante

// =============================================
// VARIÁVEIS GLOBAIS E ESTADOS
// =============================================

volatile int andar_atual = 0;  // Armazena o andar atual do elevador

// Estados do LED indicador
typedef enum { LED_DESLIGADO, LED_PISCANDO } estado_led_t;

volatile estado_led_t estado_led = LED_DESLIGADO;
volatile uint32_t proximo_alternar_led = 0;
volatile int contador_piscadas = 0;

// Padrões dos números para a matriz de LEDs (0-9)
double desenhos_numeros[10][NUM_LEDS] = {
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},

    {0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0},

    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},

    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1},

    {1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1},

    {1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1},

    {1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1},

    {1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},

    {1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1},

    {1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1}};

// =============================================
// FUNÇÕES DE CONTROLE DE LEDS
// =============================================

// Converte valores RGB para formato de 32 bits
uint32_t definir_cor_rgb(double azul, double vermelho, double verde) {
  return (((uint32_t)(verde * 255) << 24) | ((uint32_t)(vermelho * 255) << 16) |
          ((uint32_t)(azul * 255) << 8));
}

// Exibe um número na matriz de LEDs
void exibir_numero_matriz(double *desenho, uint32_t valor_led, PIO pio, uint sm,
                          double vermelho, double verde, double azul) {
  for (int i = 0; i < NUM_LEDS; i++) {
    valor_led = definir_cor_rgb(desenho[24 - i], 0.0, 0.0);
    pio_sm_put_blocking(pio, sm, valor_led);
  }
}

// =============================================
// CONTROLE DE INTERRUPÇÕES E DEBOUNCE
// =============================================

// Função de callback para tratamento de debounce
static int64_t tratar_debounce(alarm_id_t id, void *dados) {
  uint pino = (uint)(uintptr_t)dados;

  if (!gpio_get(pino)) {  // Verifica se o botão ainda está pressionado
    if (pino == BOTAO_DESCE && andar_atual > 0) {
      andar_atual--;
      estado_led = LED_PISCANDO;
      contador_piscadas = 0;
      printf("Andar atual: %d\n", andar_atual);
    } else if (pino == BOTAO_SOBE && andar_atual < 9) {
      andar_atual++;
      estado_led = LED_PISCANDO;
      contador_piscadas = 0;
      printf("Andar atual: %d\n", andar_atual);
    }
  }
  // Detectar pressionamento do botão
  gpio_set_irq_enabled(pino, GPIO_IRQ_EDGE_FALL, true);
  return 0;
}

// Manipulador de interrupção dos GPIOs
static void manipulador_interrupcao_gpio(uint pino, uint32_t eventos) {
  if (eventos & GPIO_IRQ_EDGE_FALL) {
    if (pino == BOTAO_DESCE || pino == BOTAO_SOBE) {
      gpio_set_irq_enabled(pino, GPIO_IRQ_EDGE_FALL, false);
      // Agenda uma verificação posterior
      add_alarm_in_ms(50, tratar_debounce, (void *)(uintptr_t)pino, false);
    }
  }
}

// =============================================
// FUNÇÃO PRINCIPAL E INICIALIZAÇÃO
// =============================================

int main() {
  PIO pio = pio0;
  uint32_t valor_led;
  double vermelho = 0.0, verde = 0.0, azul = 0.0;

  // Configuração do sistema
  set_sys_clock_khz(128000, false);
  stdio_init_all();

  // Inicialização do controlador PIO para a matriz
  uint offset = pio_add_program(pio, &pio_matrix_program);
  uint sm = pio_claim_unused_sm(pio, true);
  pio_matrix_program_init(pio, sm, offset, PINO_SAIDA);

  // Configuração do LED indicador
  gpio_init(LED_INDICADOR);
  gpio_set_dir(LED_INDICADOR, GPIO_OUT);
  gpio_put(LED_INDICADOR, 0);

  // Configuração dos botões com pull-up
  gpio_init(BOTAO_DESCE);
  gpio_set_dir(BOTAO_DESCE, GPIO_IN);
  gpio_pull_up(BOTAO_DESCE);

  gpio_init(BOTAO_SOBE);
  gpio_set_dir(BOTAO_SOBE, GPIO_IN);
  gpio_pull_up(BOTAO_SOBE);

// Inicialização do LED vermelho
  gpio_init(LED_VERMELHO);
  gpio_set_dir(LED_VERMELHO, GPIO_OUT);
  gpio_put(LED_VERMELHO, 0);

  uint32_t proximo_toggle_vermelho = 0;
  bool estado_led_vermelho = false;

  // Habilita interrupções nos botões
  gpio_set_irq_enabled(BOTAO_DESCE, GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_enabled(BOTAO_SOBE, GPIO_IRQ_EDGE_FALL, true);
  gpio_set_irq_callback(manipulador_interrupcao_gpio);
  irq_set_enabled(IO_IRQ_BANK0, true);

  // Loop principal
  while (true) {
    // Atualiza a exibição do andar na matriz
    exibir_numero_matriz(desenhos_numeros[andar_atual], valor_led, pio, sm,
                         vermelho, verde, azul);

    // Controle do LED indicador de movimento
    if (estado_led == LED_PISCANDO) {
      if (contador_piscadas == 0) {
        gpio_put(LED_INDICADOR, 1);
        proximo_alternar_led = to_ms_since_boot(get_absolute_time()) + 50;
        contador_piscadas = 1;
      } else {
        uint32_t agora = to_ms_since_boot(get_absolute_time());
        if (agora >= proximo_alternar_led) {
          gpio_put(LED_INDICADOR, !gpio_get(LED_INDICADOR));
          contador_piscadas++;
          if (contador_piscadas >= 6) {  // 3 ciclos completos
            gpio_put(LED_INDICADOR, 0);
            estado_led = LED_DESLIGADO;
          } else {
            proximo_alternar_led = agora + 50;
          }
        }
      }
    }

    // =============================================
    // Controle do LED vermelho
    // =============================================
    uint32_t agora = to_ms_since_boot(get_absolute_time());
    if (agora >= proximo_toggle_vermelho) {
      estado_led_vermelho = !estado_led_vermelho;
      gpio_put(LED_VERMELHO, estado_led_vermelho);
      proximo_toggle_vermelho = agora + 100;  // 100ms = 5Hz (1000ms/5/2)
    }

    sleep_ms(10);
  }
}