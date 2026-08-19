# Autópsia profunda

Data: 2026-08-19  
Entrada: APK original da Release `v1.3.7-original`  
Método: análise somente leitura; os diretórios gerados permanecem em `work/` e são ignorados pelo Git.

## Ferramentas

| Ferramenta | Versão/origem |
|---|---|
| Android `aapt2` | Build Tools 36.0.0 |
| Apktool | 3.0.3 |
| JADX | 1.5.6 |
| Java | OpenJDK 21.0.10, JBR do Android Studio |
| LLVM readelf/nm | Android NDK 25.2.9519653 |

## Manifest compilado

`aapt2 dump badging` confirmou:

```text
package: air.com.adobe.pstouchphone
versionCode: 1003007
versionName: 1.3.7
launchable-activity: air.com.adobe.pstouchphone.AppEntry
minSdkVersion: 14
targetSdkVersion: 12
native-code: armeabi-v7a
uses-gl-es: 0x20000
```

O ID final do Android é `air.com.adobe.pstouchphone`, enquanto o ID declarado no `application.xml` do AIR é `com.adobe.pstouchphone`. Essa diferença é normal em pacotes AIR e precisa ser preservada ou tratada explicitamente em qualquer rebuild.

O Manifest também contém:

- `AppEntry` como Activity de entrada, com `launchMode=singleTask`, orientação fixa e `configChanges` legado.
- `android:hardwareAccelerated=false` e `android:largeHeap=true`.
- Meta-dados `namespaceVersion=16` e `initialcontent=TTPixel.swf`.
- Filtros para JPEG, PNG, GIF e MIME `application/vnd.adobe.pstouch`.
- Filtros para arquivos `.psdx` usando `file`, `http`, `https` e `content`.
- Service/receiver `air.com.adobe.cc.notifications.c2dm.C2DMReceiver`.
- OpenGL ES 2.0 obrigatório.

O `aapt2` emitiu avisos para referências de recursos sem arquivo correspondente (`drawable/icon`, `raw/debugger`, `raw/mms_cfg`, `raw/ss_cfg` e `raw/ss_sgn`). Isso deve ser verificado no processo de rebuild, mas não prova sozinho que o aplicativo não inicia.

## Runtime AIR: cativo e fallback legado

O `classes.dex` contém classes `com.adobe.air`, inclusive `AndroidActivityWrapper`. A `AppEntry` tenta primeiro carregar essa classe diretamente (`loadCaptiveRuntimeClasses`). Se não conseguir, procura o runtime compartilhado `com.adobe.air`, tenta carregar `AndroidActivityWrapper` via `DexClassLoader` e pode iniciar `com.adobe.air.AIRService`.

Quando o runtime não é encontrado, o código ainda tenta abrir:

```text
market://details?id=com.adobe.air
http://airdownload2.adobe.com/air?
```

Conclusão: este APK foi empacotado com lógica de runtime cativo, mas mantém caminhos de fallback para o AIR compartilhado. Uma portabilidade moderna precisa eliminar ou substituir esses caminhos, além de atualizar o runtime embutido.

## DEX e classes decompiladas

O JADX processou 612 unidades/classes do APK e gerou aproximadamente:

| Grupo observado | Arquivos Java decompilados |
|---|---:|
| Total | 419 |
| `com.adobe.air` | 108 |
| `com.adobe.ttpixel` | 211 |
| `air.com.adobe.pstouchphone` | 3 |
| Declarações `native` | 247 ocorrências |

Esses números descrevem a saída decompilada e não devem ser tratados como código-fonte original compilável.

## ANE TTPixel

O inicializador Java de `com.adobe.ttpixel.extension.TTPixelExtension` carrega, nesta ordem:

```text
System.loadLibrary("sibsynclib")
System.loadLibrary("air.com.adobe.cc.sync.SyncEngine")
System.loadLibrary("TTPixelExtensionAndroid")
initIDs()
```

Os contextos AIR expostos incluem:

```text
gl, am, ss, cameraroll, email, impexp, utils,
camera[static], camera, printer, bigdata, httpd,
activityui, bonjour, pressure.jaja, quickselection
```

Isso confirma que a extensão não é acessória: ela cobre renderização OpenGL, filtros, codificação JPEG/PNG/Zlib/LZ4, câmera, importação/exportação, seleção rápida, pressão/stylus, impressão, Bonjour e utilidades de imagem.

## ELF e ABI

Todas as bibliotecas foram identificadas como `ELF32`, máquina `ARM`, dentro de `lib/armeabi-v7a`. Não há `arm64-v8a`.

Dependências relevantes:

| Biblioteca | Dependências ou observação |
|---|---|
| `libCore.so` | `libstagefright`, `libutils`, `libui`, `libGLESv2`, `libEGL`, `libjnigraphics`, `libstlport_shared` |
| `libTTPixelExtensionAndroid.so` | `libEGL`, `libGLESv2`, `libandroid`, `libjnigraphics`, `liblog`, `libz` |
| `libair.com.adobe.cc.sync.SyncEngine.so` | `libCore`, `libstdc++`, `libm`, `libc`, `libdl` |
| `libstagefright_froyo.so` | APIs internas/antigas de `libstagefright`, `libutils` e `libcutils` |
| `libstagefright_honeycomb.so` | Também depende de `libgui` e `libui` antigos |
| `libsibsynclib.so` | Exporta uma grande superfície de criptografia/SSL e símbolos de sincronização |

O `libTTPixelExtensionAndroid.so` exporta `JNI_OnLoad` e muitos símbolos `Java_com_adobe_ttpixel_*`, incluindo operações OpenGL, codecs, buffers, alpha matting, seleção rápida e pressão. Portar somente o package name ou o target SDK não substitui essa ABI.

## SWF principal

`assets/TTPixel.swf`:

```text
assinatura: CWS (SWF comprimido com zlib)
tamanho no APK: 6,639,060 bytes
tamanho descomprimido: 7,980,100 bytes
comprimento declarado no cabeçalho: 7,980,100 bytes
```

Strings do SWF indicam dependências e funções para:

- Adobe Creative Cloud, IMS e licenciamento;
- sincronização de arquivos;
- Dropbox, Flickr e Facebook;
- câmera, galeria e impressão;
- PSDX, JPEG, PNG e exportação;
- analytics/telemetria Adobe;
- shaders e processamento Stage3D/OpenGL.

Foram observados endpoints/hosts legados como `creative.adobe.com`, `ims-na1-cc1.adobelogin.com`, `api.adbecrsl.com`, `storage-stage.adobe.com`, endpoints Dropbox/Flickr/Facebook e `adobemobile.112.2o7.net`. A presença de uma string não comprova que o endpoint ainda esteja operacional; cada fluxo deve ser testado ou marcado como indisponível.

## Estado de execução

```text
adb devices -l
```

O dispositivo físico conectado é um Redmi Note 9S com Android 16/API 36; os resultados estão em [test-results-android16.md](test-results-android16.md). Não há AVD configurado para os demais níveis da matriz.

## Rebuild baseline

O projeto desmontado pelo Apktool foi recompilado sem alterações de comportamento e assinado com a chave de debug local. O resultado preservou `targetSdkVersion=12`, `minSdkVersion=14`, package `air.com.adobe.pstouchphone` e ABI `armeabi-v7a`. O `apksigner` confirmou as assinaturas v1, v2 e v3.

Esse resultado valida a cadeia de empacotamento, mas não valida execução.

## Conclusão atual

Classificação: **portabilidade difícil, mas com uma rota de prova de conceito**.

O próximo teste técnico deve ser um rebuild mínimo que preserve o `TTPixel.swf` e valide a compatibilidade do runtime AIR/ANE em um dispositivo ARM com suporte a código 32-bit. Se o runtime moderno rejeitar o ANE ou se alguma biblioteca depender de APIs internas removidas, a alternativa realista será portar/substituir a extensão TTPixel e revisar os fluxos de Creative Cloud/sincronização.
