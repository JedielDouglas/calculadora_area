# Calculadora de área (retângulo e triângulo)

## Introdução
A seguir, desenvolvemos, passo a passo, as etapas de síntese de um programa em linguagem C para calcular a área de um retângulo e de um triângulo, utilizando a placa educacional BitDogLab que possui a Raspberry Pi Pico W integrada. O projeto integra diversos periféricos – botões, display OLED e matriz de LEDs – para proporcionar uma experiência interativa ao usuário.

Atividade:
O objetivo deste projeto é permitir que o usuário informe os valores de base e altura para calcular:

Área do Retângulo: Calculada como base × altura.
Área do Triângulo: Calculada como (base × altura) / 2.
O programa segue os seguintes passos:

Exibição Inicial:
Exibe a mensagem “Calculadora de Área” no display OLED e, simultaneamente, na matriz de LEDs forma rapidamente o desenho de um retângulo (na cor verde) e de um triângulo (na cor azul).

Entrada de Dados:

Base:
É exibida a mensagem “Informe a Base – Pressione A para iniciar”. Com o botão A o valor é incrementado (iniciando em 1) e o botão B salva o valor, confirmando com “Base Final Salvo”.
Altura:
Em seguida, é exibida a mensagem “Informe Altura – Pressione A para iniciar”. A seleção funciona de maneira semelhante: botão A para incrementar e botão B para salvar, confirmando com “Altura Final Salvo”.
Cálculo e Exibição dos Resultados:
Após a escolha dos valores, o programa calcula:

Área do Retângulo = base × altura.
Área do Triângulo = (base × altura) / 2.
Os resultados são exibidos no display OLED, enquanto a matriz de LEDs alterna os desenhos do retângulo (verde) e do triângulo (azul).
Desenvolvimento:
Para a implementação deste projeto foram utilizados os seguintes recursos:

Placa BitDogLab com Raspberry Pi Pico W: Responsável pelo controle dos periféricos.
Botões A e B: Usados para a entrada dos valores.
Display OLED: Exibe mensagens, instruções e resultados.
Matriz de LEDs NeoPixel: Utilizada para exibir os desenhos do retângulo e do triângulo.
Linguagem C: Empregada para integrar funções de GPIO, I2C, controle dos LEDs via PIO e interação com o display.
O código foi estruturado em módulos que:

Inicializam e configuram os periféricos (GPIO, I2C, display OLED, matriz de LEDs).
Exibem mensagens no display OLED.
Controlam a exibição de desenhos na matriz de LEDs.
Capturam interativamente os valores de base e altura através dos botões.
Realizam os cálculos das áreas e exibem os resultados.
