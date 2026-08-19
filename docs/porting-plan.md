# Plano de portabilidade

## Fase 0 — Preparação

1. Preservar o APK original fora do Git.
2. Calcular e registrar o SHA-256.
3. Criar um workspace separado por tentativa.
4. Registrar versões das ferramentas e do Android SDK.

## Fase 1 — Autópsia

- Decodificar Manifest e descriptor AIR.
- Inventariar SWF, CAB, ANE, JAR e SO.
- Identificar APIs ActionScript e extensões nativas.
- Analisar ABI, símbolos e dependências ELF.
- Mapear armazenamento, intents, rede, câmera, OpenGL e notificações.
- Produzir um relatório antes de modificar qualquer arquivo.

## Fase 2 — Prova de conceito

Testar, de forma isolada:

1. SWF original com runtime AIR compatível.
2. Reempacotamento mínimo.
3. Inicialização em dispositivo ARM com suporte a 32-bit.
4. Coleta de logcat e classificação do primeiro erro real.

Não fazer várias alterações ao mesmo tempo.

## Fase 3 — Portabilidade

Corrigir apenas problemas comprovados:

- permissões e armazenamento;
- intents e URI de arquivos;
- WebView e rede;
- Activity, Service e Receiver;
- OpenGL e aceleração;
- AIR APIs;
- ANEs;
- JNI e bibliotecas nativas;
- notificações e serviços remotos.

Se uma biblioteca não puder ser recompilada, registrar a limitação em vez de criar uma implementação especulativa.

## Fase 4 — Validação

Testar progressivamente em dispositivos/emuladores disponíveis. O mínimo é validar instalação, inicialização, criação de documento, importação, camadas, pincéis, filtros, transformação, salvar/abrir PSDX e exportação.

Para cada falha, registrar versão do Android, dispositivo/ABI, passos, logcat, hipótese, alteração e resultado.

## Fase 5 — Build final

Uma build só pode ser considerada candidata quando:

- o processo é reproduzível;
- a assinatura de teste está documentada;
- o APK instala e abre;
- as funções principais foram testadas;
- limitações e serviços indisponíveis estão listados;
- nenhum binário proprietário foi publicado neste repositório.

