# Cortex Pet — códigos extraídos

Arquivos-fonte completos extraídos dos blocos de código do documento original.

## Build (Linux/macOS)

1. Gere o GLAD2 em https://gen.glad.sh com OpenGL 3.3 Core, linguagem C/C++ e loader ativado.
2. Extraia o GLAD em `external/glad/` (os arquivos `include/glad/gl.h` e `src/gl.c`).
3. Execute:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./cortex_pet
```

Dependências de desenvolvimento do GLFW/OpenGL devem estar instaladas no sistema.

## Controles

`C` carinho · `P` cutucar · `F` alimentar · `T` conversar · `R` reset de habituação · `Esc` fechar.

## Observações importantes

- Os arquivos do motor SNN citados no documento (`neuro.c/.h`, `behavior.c/.h`, `homeostasis.c/.h`, `conditioning.c/.h`, `memory.c/.h`, `sensors.c/.h`, `runtime_types.h`, `network_data.h`) não estavam incluídos como blocos de código completos no material extraído; precisam ser fornecidos separadamente.
- O GLAD2 é uma dependência gerada externamente e não veio incorporado no documento.
- A pasta `patches/` contém diffs/trechos incrementais extraídos separadamente. Eles não foram aplicados automaticamente aos arquivos completos, para evitar sobrescrever versões sem validação.
