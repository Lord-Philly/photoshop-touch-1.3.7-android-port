# Adobe Photoshop Touch 1.3.7 — Android Modern Port Investigation

Investigação técnica para avaliar a portabilidade autorizada do Adobe Photoshop Touch 1.3.7 para versões modernas do Android.

> Status: **builds funcionais publicadas**. A fase de prova de conceito foi concluída: os ports modernos instalam pelo instalador padrão, iniciam, corrigem a composição automaticamente e mantêm a orientação travada. Veja [Downloads](#downloads).

O alvo principal é **Android 16 / API 36**. As builds usam o runtime AIR 51 com a camada ANE ARMv7 PIC e o ciclo de autocorreção de orientação descrito em [docs/air51-modern-poc.md](docs/air51-modern-poc.md) e [docs/orientation-heal-poc.md](docs/orientation-heal-poc.md).

## Downloads

Todos os APKs estão na release [`Ps Touch (versões)`](https://github.com/Lord-Philly/photoshop-touch-1.3.7-android-port/releases/tag/versoes):

| Arquivo | Para quê |
|---|---|
| `ps137.apk` | PS Touch 1.3.7 **em dispositivos modernos** (Android 10–16), abre em pé |
| `ps177.apk` | PS Touch 1.7.7 (tablet) **em dispositivos modernos** (Android 10–16), abre deitado |
| `pstouchbase_1.3.7.apk` | Base original 1.3.7 para **dispositivos antigos** |
| `pstouchbase_1.7.7.apk` | Base original 1.7.7 para **dispositivos antigos** |

Detalhes, requisitos e hashes em [docs/version-matrix.md](docs/version-matrix.md).

## Objetivo

Determinar se o APK antigo pode ser executado em Android moderno preservando o máximo possível das funções originais, identificando os componentes que exigiriam atualização, substituição ou recompilação.

## Achados iniciais

- Aplicativo empacotado com Adobe AIR; o descriptor usa o namespace AIR `16.0`.
- Conteúdo principal: `assets/TTPixel.swf`.
- `minSdkVersion=14` e `targetSdkVersion=12`.
- Três ANEs proprietários: Pixel, SyncEngine e PushNotification.
- Bibliotecas nativas presentes apenas em `armeabi-v7a`.
- O Manifest usa permissões e componentes Android legados, incluindo C2DM.
- A assinatura original da Adobe não está disponível; uma build modificada exigirá uma chave diferente.

Veja o relatório completo em [docs/initial-apk-autopsy.md](docs/initial-apk-autopsy.md).

## APK original

O APK original está hospedado na Release [`v1.3.7-original`](https://github.com/Lord-Philly/photoshop-touch-1.3.7-android-port/releases/tag/v1.3.7-original). O SHA-256 do asset é:

```text
C57E409493F46F4B9404678A777E23CEE7FEDCF921427435F4767EFDA5823A85
```

O fato de o aplicativo estar descontinuado não implica que o software tenha se tornado domínio público. O asset é mantido aqui como cópia de análise/arquivo, com a origem registrada.

## Como reproduzir a inspeção

Não coloque o APK no Git. Mantenha-o fora do repositório e execute:

```powershell
.\scripts\inspect-apk.ps1 `
  -ApkPath 'C:\caminho\para\air.com.adobe.pstouchphone_1.3.7.apk'
```

O script faz uma leitura somente, calcula o SHA-256, lista o conteúdo do ZIP/APK e gera um relatório em `analysis-output/`.

## Estrutura

```text
docs/
  initial-apk-autopsy.md   # evidências e achados da primeira análise
  air51-modern-poc.md      # PoC AIR 51, Android 16 e bloqueio das ANEs
  orientation-heal-poc.md  # retrato travado, causa raiz e ciclo de autocorreção
  version-matrix.md        # catálogo das builds: bases antigas x ports modernos
native/ane-compat/         # camada PIC experimental para os contratos JNI das ANEs
  porting-plan.md          # fases e critérios de decisão
  prompt-for-dev.md        # prompt operacional para dev ou IA
scripts/
  inspect-apk.ps1          # inspeção reproduzível e não destrutiva
```

## Limites e segurança

Este repositório contém documentação e automação de análise, não o aplicativo proprietário nem seus binários. O trabalho deve ser feito somente com autorização adequada, sem burlar DRM, licenciamento, autenticação ou serviços remotos. Não publicar APKs, SWFs, ANEs, JARs ou bibliotecas nativas da Adobe.
