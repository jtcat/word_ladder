clear
close all
clc

table = load("first.txt");
j = table(:,1);
memory = table(:,2);
collisions = table(:,3);
ratio_col_mem = collisions./memory;

figure(1)
plot(memory,collisions)
title('Número de colisões em função da memória')
xlabel('Memória ocupada (bytes)')
ylabel('Número de colisões')
grid on

figure(2)
plot(j,ratio_col_mem)
title('Rácio colisões/memória em função do incremento')
xlabel('Incremento, j')
ylabel('Rácio colisões/memória')
grid on