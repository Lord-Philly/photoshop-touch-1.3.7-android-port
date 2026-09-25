# Plano B — reconstrução nativa

O runtime AIR 51 já carrega no Android 16, mas o SWF original fica parado
porque os contratos de edição das ANEs ainda não foram implementados. O Plano B
começa em paralelo como um aplicativo novo, sem dependência do AIR.

## Primeiro marco

`native-app/` entrega uma fatia vertical testável:

1. abrir uma tela nativa;
2. desenhar com o dedo;
3. desfazer e limpar;
4. exportar PNG localmente.

Isso comprova a base de entrada, renderização Skia-backed e armazenamento
compatível com Android 10–16 antes de portarmos o restante do produto.

## Próximos marcos

### M1 — documento e camadas

- modelo `Document`/`Layer`;
- criar, importar e redimensionar documento;
- múltiplas camadas com visibilidade e ordem;
- autosave local.

### M2 — ferramentas essenciais

- pincel com tamanho/opacidade/cor;
- borracha;
- conta-gotas;
- mover, cortar e transformar;
- importação de imagem usando Photo Picker/SAF.

### M3 — formatos

- PNG/JPEG;
- formato de projeto nativo versionado;
- compatibilidade progressiva com PSDX, se a especificação puder ser
  implementada sem redistribuir código proprietário.

### M4 — filtros e desempenho

- filtros básicos em tiles;
- shaders/GPU onde houver ganho real;
- testes de memória em aparelhos 32-bit ARMv7 e 64-bit ARM64.

### M5 — Android 10–16

- validar instalação e fluxo essencial em API 29, 30, 31, 33, 34, 35 e 36;
- manter o editor offline como caminho principal;
- adicionar nuvem apenas depois do editor local estar estável.

## Limite de licenciamento

O código nativo é novo. O SWF, os assets e o comportamento do aplicativo antigo
servem apenas como referência quando houver autorização adequada; não entram
automaticamente no novo APK.
