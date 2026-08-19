# Matriz de compatibilidade

## Objetivo

Prioridade máxima: **Android 16 / API 36**.  
Matriz mínima: Android 10, 11, 12, 13, 14, 15 e 16.

## Estado atual

| Android | API | Dispositivo/ambiente | Instalação | Execução | Status |
|---|---:|---|---|---|---|
| 10 | 29 | Ainda não testado | — | — | Pendente |
| 11 | 30 | Ainda não testado | — | — | Pendente |
| 12 | 31 | Ainda não testado | — | — | Pendente |
| 13 | 33 | Ainda não testado | — | — | Pendente |
| 14 | 34 | Ainda não testado | — | — | Pendente |
| 15 | 35 | Ainda não testado | — | — | Pendente |
| 16 | 36 | Redmi Note 9S, ARM64/ARMv7 | AIR 51 target 36 instala | Runtime moderno inicia; ANEs antigas causam crash | Bloqueio nativo confirmado |

Detalhes da troca do runtime e dos hashes estão em [air51-modern-poc.md](air51-modern-poc.md).

## Critério de aprovação

Uma versão só será marcada como compatível quando:

1. instalar sem bypass;
2. abrir sem crash;
3. carregar o editor;
4. criar um documento;
5. importar imagem;
6. usar pincel, camada e pelo menos um filtro;
7. salvar e reabrir um projeto PSDX;
8. exportar uma imagem;
9. registrar o resultado em um dispositivo/ambiente reproduzível.

“Instala” sozinho não é compatibilidade.
