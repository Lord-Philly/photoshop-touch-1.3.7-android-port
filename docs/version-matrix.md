# Matriz de versões — PS Touch

Catálogo oficial das builds publicadas nas Releases deste repositório. Duas famílias:

| Família | Arquivos | Público-alvo |
|---|---|---|
| **Bases originais** (`pstouchbase_*`) | `pstouchbase_1.3.7.apk`, `pstouchbase_1.7.7.apk` | Dispositivos **antigos** (era Android 2.x–4.x). Em Android moderno só instalam via ferramentas que contornam o bloqueio de `targetSdk` baixo (ex.: MT Manager) |
| **Ports modernos** (`psNNN`) | `ps137.apk`, `ps177.apk` | Dispositivos **modernos** (Android 10–16), instalação pelo gerenciador de arquivos normal |

## Tabela-resumo

| Arquivo | Conteúdo | Runtime | Orientação | Pacote Android | `targetSdk` |
|---|---|---|---|---|---|
| `pstouchbase_1.3.7.apk` | PS Touch 1.3.7 (phone) | AIR legado 16.0 (stagefright) | retrato nativo | `air.com.adobe.pstouchphone` | 12 |
| `pstouchbase_1.7.7.apk` | PS Touch 1.7.7 (tablet) | AIR legado 16.0 (stagefright) | paisagem nativa | `air.com.adobe.pstouch` | 12 |
| `ps137.apk` | PS Touch 1.3.7 (phone) | **AIR 51 moderno** | **retrato travado** + autocorreção | `air.com.adobe.pstouchphone` | 34 |
| `ps177.apk` | PS Touch 1.7.7 (tablet) | **AIR 51 moderno** (híbrido) | **paisagem travada** + autocorreção | `air.com.adobe.pstouch` | 28 |

Os dois ports usam **pacotes diferentes** e podem coexistir instalados no mesmo aparelho.

## Instalação (ports modernos)

1. Desinstale qualquer build anterior **do mesmo pacote** (cada port é independente).
2. Toque no APK e instale pelo instalador padrão do Android — não precisa de ferramenta especial.
3. Na primeira abertura, conceda a permissão de armazenamento quando pedida (**uma única vez**).
4. `ps137` abre em pé; `ps177` abre deitado — ambos mantêm a orientação fixa.

## Notas técnicas dos ports

- **Troca de runtime**: classes Java e bibliotecas nativas (`libCore.so` etc.) do AIR SDK 51 aplicadas sobre o conteúdo original do jogo (SWF, recursos, extensões). O descriptor original permanece intocado no disco (integridade de licença) e um descriptor equivalente é alimentado em memória no boot (*split-feed*) — técnica documentada em [orientation-heal-poc.md](orientation-heal-poc.md).
- **Autocorreção de composição** (`OrientHeal`): o recalculo nativo de orientação não existe no boot do AIR; um ciclo sintético de eventos corrige o estado da superfície automaticamente após o lançamento e a cada retomada de foco. Um toque na tela também dispara a cura manualmente.
- **Trava de orientação**: aplicada programaticamente nos gatilhos do ciclo de vida (`onResume`, primeira superfície pós-boot).
- **Permissões**: pedido único em tempo de execução; sem telas repetidas de "gerenciar todos os arquivos".
- **Compatibilidade de ANEs**: as 230 classes das três extensões proprietárias do 1.7.7 estão presentes no dex híbrido; a camada PIC ([native/ane-compat](../native/ane-compat)) atende os contratos JNI.
- **Glitch conhecido**: leve tremor/piscadas ao navegar menus e tocar botões — cosmético, ligado à composição GPU do runtime híbrido.

## Integridade (SHA-256)

```text
85c9baaa6a321f268bc47224517a5e74c7bf16ed6ba822bf416aa7b2386064dc  pstouchbase_1.3.7.apk
36bf167edb48f05303a078a120ba2c4dc027c2e3da2ba3ffcccec05856bc6654  pstouchbase_1.7.7.apk
c5baf4e4f4f582315d7d5de2f0506d3fc92b7aa7d6cc4878207eb06f5af92081  ps137.apk
91787bb23470306c26bb32c6b5902e8261c8b8b9b7629e4bf45beccf5fc2a76f  ps177.apk
```

> As bases originais pertencem à Adobe. São arquivadas aqui como referência de análise, com a origem registrada, nos mesmos termos do APK da release [`v1.3.7-original`](https://github.com/Lord-Philly/photoshop-touch-1.3.7-android-port/releases/tag/v1.3.7-original).
