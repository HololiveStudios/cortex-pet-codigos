# Cortex Pet - Núcleo Neural e Controle para Pet Robótico

## 1. Objetivo e Finalidade
Motor computacional em C de alto desempenho para simulação de córtex biológico, redes neurais e controle de agentes robóticos autônomos.

## 2. Tecnologias Utilizadas
- C, CMake, GCC / Clang, POSIX Threads, Math

## 3. Arquitetura e Estrutura de Diretórios
Projeto C nativo modularizado construído com CMake. Código-fonte organizado em `src/` com suporte a patches de otimização em `patches/`.

## 4. Principais Arquivos e Responsabilidades
- `CMakeLists.txt`: Configuração do build executável `cortex_pet`.
- `BUILD.md`: Instruções de compilação e dependências de ambiente.
- `src/`: Módulos de simulação neural, controle de estado e atuadores.
- `patches/`: Correções e otimizações de baixo nível.

## 5. Funcionalidades Identificadas
- Simulação de redes neurais artificiais de tempo contínuo
- Processamento ultra-rápido em código C nativo sem dependências pesadas
- Estrutura pronta para compilação cruzada para sistemas embarcados (ARM/RISC-V)

## 6. Como Compilar ou Executar
```bash
mkdir -p build && cd build
cmake ..
make
./cortex_pet
```

## 7. Estado Atual do Projeto
**Completo**

## 8. Pendências e Limitações Identificadas
Implementar driver de comunicação serial para envio de comandos a microcontroladores ESP32/Arduino.
