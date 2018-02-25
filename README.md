essa caralha é pra ser um controle de SCR via arduino.

Por motivos de cagalhão e preguiçoso, a frequencia é fixa em 50hz pra ser gerado 3 sinais distintos de 5vpp.

os timers tão setados pra 100hz por nyquist (mentira, é pq é a unica frequencia harmonica que eu consigo usar os 3 timers do 328p)
nesses 100hz cada um só vai contar em pulso alternado

cada interr do timer vai somar num contador de cada fase
quando o contador da fase for igual ou maior que a leitura do ADC que é o caralho do angulo de corte
vai mandar o sinal pro MOC3020 PQ EU NÃO TENHO TEMPO PRA USAR UM TRANSFORMADOR DE PULSO

mt tempo gasto procrasturbando


o MOC3020 nesse exato momento tá sendo representado por outros 3 leds que tem vontade e desejos proprios

na real eu n sei bem qq ta acontecendo



.... mass eu faço alguma ideia


primeiro prototipo vai trocar os optoacoplador 4n25 por push button pra ficar mais facil de discernir e não precisar de um fucking gerador de função
inclusive to aceitando
