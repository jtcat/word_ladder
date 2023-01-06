clear
close all
clc

% Obter dados do ficheiro
table = load("first.txt");
j = table(:,1);
new_size = table(:,2);
memory = table(:,3);
free_memory = table(:,4);
collisions = table(:,5);

% Ordenar arrays free_memory e collisions (com base no free_memory)
[free_memory_sorted,sortIdx] = sort(free_memory,'ascend');
collisions_sorted = collisions(sortIdx);

% Obter rácios
ratio_col_mem = collisions./memory;
ratio_col_free = collisions./free_memory;

% Plots
figure(1)
plot(memory,collisions)
title('Número de colisões em função da memória total')
xlabel('Memória total (bytes)')
ylabel('Número de colisões')
grid on

figure(2)
plot(free_memory_sorted,collisions_sorted)
title('Número de colisões em função da memória livre')
xlabel('Memória livre (bytes)')
ylabel('Número de colisões')
grid on
xlim([5000 20000])

figure(3)
plot(j,ratio_col_mem)
title('Rácio colisões/memória total em função do incremento')
xlabel('Incremento, j')
ylabel('Rácio colisões/memória total')
grid on

figure(4)
plot(j,ratio_col_free)
title('Rácio colisões/memória livre em função do incremento')
xlabel('Incremento, j')
ylabel('Rácio colisões/memória livre')
grid on

figure(5)
plot(memory,free_memory)
title('Memória livre em função da memória total')
xlabel('Memória total (bytes)')
ylabel('Número livre (bytes)')
grid on