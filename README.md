# Projeto: Indicador de Andar para Elevador com RP2040

Este projeto simula o painel de um elevador utilizando o microcontrolador RP2040 e a placa de desenvolvimento BitDogLab. O objetivo é criar um sistema onde uma matriz de LEDs WS2812 exibe o andar atual, enquanto botões permitem simular comandos para subir ou descer andares. O LED RGB é utilizado para indicar quando o elevador está em movimento, piscando durante a transição entre andares.

## Requisitos do Projeto

O projeto foi desenvolvido com base nos seguintes requisitos:

1. **Uso de Interrupções**: Todas as funcionalidades relacionadas aos botões foram implementadas utilizando rotinas de interrupção (IRQ).
2. **Debouncing**: O tratamento do bouncing dos botões foi implementado via software.
3. **Controle de LEDs**: O projeto demonstra o controle de LEDs comuns e LEDs WS2812, incluindo a criação de efeitos visuais.
4. **Organização do Código**: O código está estruturado e comentado para facilitar o entendimento.

## Funcionalidades

O projeto implementa as seguintes funcionalidades:

1. **LED Vermelho Piscante**: O LED vermelho do LED RGB pisca continuamente a uma taxa de 5 vezes por segundo, simulando o estado de "elevador em movimento".
2. **Botões de Incremento e Decremento**:
   - O botão A incrementa o número exibido na matriz de LEDs WS2812, simulando o comando para subir um andar.
   - O botão B decrementa o número exibido na matriz de LEDs WS2812, simulando o comando para descer um andar.
3. **Matriz de LEDs WS2812**: Os LEDs WS2812 são usados para exibir números de 0 a 9 em um formato fixo, com estilo de caracteres digitais, representando o andar atual do elevador.
4. **Feedback Visual de Movimento**: O LED RGB pisca durante a transição entre andares, indicando que o elevador está em movimento.

## Estrutura do Projeto

O projeto está organizado da seguinte forma:
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── pio_matrix.pio
├── pio_matrix.pio.h
├── README.md
├── ws2812.c
└── ws2812.pio.h


### Descrição dos Arquivos

- **`CMakeLists.txt`**: Arquivo de configuração do CMake para compilar o projeto.
- **`pico_sdk_import.cmake`**: Importa o SDK do Raspberry Pi Pico para o projeto.
- **`pio_matrix.pio`**: Contém o código PIO para controlar a matriz de LEDs WS2812.
- **`pio_matrix.pio.h`**: Header file gerado a partir do código PIO.
- **`ws2812.c`**: Arquivo principal do projeto, contendo a lógica de controle dos LEDs e botões.
- **`ws2812.pio.h`**: Header file gerado a partir do código PIO para controle dos LEDs WS2812.

### Funções Principais

O arquivo `ws2812.c` contém as seguintes funções principais:

1. **`definir_cor_rgb(double azul, double vermelho, double verde)`**:
   - Converte valores RGB para o formato de 32 bits utilizado pelos LEDs WS2812.

2. **`exibir_numero_matriz(double *desenho, uint32_t valor_led, PIO pio, uint sm, double vermelho, double verde, double azul)`**:
   - Exibe um número na matriz de LEDs WS2812 com base em um padrão pré-definido.

3. **`tratar_debounce(alarm_id_t id, void *dados)`**:
   - Função de callback para tratamento de debounce dos botões.

4. **`manipulador_interrupcao_gpio(uint pino, uint32_t eventos)`**:
   - Manipulador de interrupção para os botões, que agenda o debounce.

5. **`main()`**:
   - Função principal que inicializa o hardware, configura interrupções e executa o loop de controle dos LEDs e botões.

## Como Compilar e Executar

1. Clone o repositório para o seu ambiente de desenvolvimento.
2. Configure o ambiente para o Raspberry Pi Pico seguindo as instruções oficiais.
3. Compile o projeto usando o CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
4. Carregue o arquivo .uf2 gerado na pasta build para a placa RP2040.

## Destaques do Projeto

1. **Incremento e Decremento de Valores com Feedback Visual:** Os botões permitem simular a subida e descida do elevador, com feedback visual imediato na matriz de LEDs.
2. **Integração de Efeitos Visuais:** O LED RGB pisca durante a transição entre andares, simulando o movimento do elevador.
3. **Simplicidade e Eficiência:** O uso de interrupções e debouncing via software garante uma resposta rápida e confiável aos comandos do usuário.

## Licença

Este projeto está licenciado sob a licença MIT. Consulte o arquivo LICENSE para mais detalhes.