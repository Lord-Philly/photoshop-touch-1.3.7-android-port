# Autópsia inicial do APK

Data da análise: 2026-08-19  
Escopo: inspeção somente leitura; nenhum byte do APK original foi alterado.

## Artefato analisado

Arquivo local:

```text
C:\Users\Mike\Downloads\air.com.adobe.pstouchphone_1.3.7-1003007_minAPI14(armeabi-v7a)(240,320,480,560,640dpi)_apkmirror.com.apk
```

| Propriedade | Valor |
|---|---|
| Tamanho | 21,792,085 bytes |
| SHA-256 | `C57E409493F46F4B9404678A777E23CEE7FEDCF921427435F4767EFDA5823A85` |
| Formato | APK/ZIP |
| Assinatura encontrada | `META-INF/ATTOOLS.SF` e `META-INF/ATTOOLS.RSA` |
| Repositório | O APK não é versionado neste projeto |

O hash acima identifica exatamente o arquivo analisado. Sempre compare o hash antes de repetir a análise.

## Descriptor Adobe AIR

Arquivo: `assets/META-INF/AIR/application.xml`

| Campo | Valor observado |
|---|---|
| Namespace AIR | `http://ns.adobe.com/air/application/16.0` |
| ID | `com.adobe.pstouchphone` |
| Filename | `PSTouchPhone` |
| Nome | `PS Touch` |
| Versão | `1.3.7` |
| Conteúdo inicial | `TTPixel.swf` |
| Render mode | `direct` |
| Auto-orients | `false` |
| Full screen | `true` |
| Visible | `true` |
| Color depth | `32bit` |
| Extensões | Pixel, SyncEngine e PushNotification |

O namespace indica o formato/runtime AIR esperado pelo descriptor; não prova, isoladamente, que um AIR moderno será compatível com o SWF e com os ANEs.

## Manifest Android

O Manifest embutido no descriptor declara:

- `minSdkVersion=14`.
- `targetSdkVersion=12`.
- `android:installLocation="auto"`.
- OpenGL ES 2.0 obrigatório.
- `android:hardwareAccelerated="false"`.
- `android:largeHeap="true"`.
- Permissões de Internet, armazenamento externo, câmera, rede Wi-Fi, WakeLock e estado de rede.
- Filtros de tela/densidade específicos para normal, hdpi, xhdpi, 480, 560 e 640 dpi.
- Activity principal com filtros para abrir imagens e arquivos `.psdx`.
- Service/receiver legado para `air.com.adobe.cc.notifications.c2dm.C2DMReceiver`.

Esses dados indicam que a compatibilidade não depende apenas de trocar o target SDK: armazenamento, permissões, intents, notificações, AIR e código nativo precisam ser testados.

## ANEs identificados

| ID | Plataforma incluída | Biblioteca declarada | Risco |
|---|---|---|---|
| `com.adobe.ttpixel.extension` | Android-ARM | `library.jar` | Alto; provavelmente usado pelo editor Pixel |
| `air.com.adobe.cc.sync.SyncEngine` | Android-ARM | `libAdobeSyncEngineBridge.so` | Alto; sincronização e dependências nativas |
| `air.com.adobe.cc.PushNotification` | Android-ARM | `AdobeC2DMLibrary.jar` | Alto; API C2DM legada |

Cada ANE também contém `extension.xml`, `catalog.xml` e SWF de biblioteca. O empacotamento final coloca bibliotecas nativas em `lib/armeabi-v7a`.

## Bibliotecas nativas

Arquitetura observada: somente `armeabi-v7a`; não foi encontrada uma pasta `arm64-v8a`.

```text
lib/armeabi-v7a/libCore.so
lib/armeabi-v7a/libstagefright_froyo.so
lib/armeabi-v7a/libstagefright_honeycomb.so
lib/armeabi-v7a/libstlport_shared.so
lib/armeabi-v7a/libysshared.so
lib/armeabi-v7a/libair.com.adobe.cc.sync.SyncEngine.so
lib/armeabi-v7a/libsibsynclib.so
lib/armeabi-v7a/libgdx-audio.so
lib/armeabi-v7a/libTTPixelExtensionAndroid.so
```

Consequência: não se deve prometer suporte ARM64 sem verificar se o dispositivo aceita código nativo 32-bit e sem analisar cada biblioteca. Portar para ARM64 provavelmente exige código-fonte, recompilação ou substituição legítima das bibliotecas.

## Conteúdo principal e recursos

Além do SWF inicial, o APK contém módulos e recursos do editor, incluindo:

- `assets/resource/application/application.cabapp`.
- `assets/resource/application/main.cabprf` e `main.cabres`.
- Módulos de câmera, crop, curvas, níveis, gradiente, transformação, texto, pincel, camadas e filtros.
- Shaders `.fs`, `.vsh` e recursos de fontes.
- SWFs de tema, painéis, galeria, mídia, tutorial e boas-vindas.
- Arquivos de localização para alemão, inglês, espanhol, francês, italiano, japonês, português e russo.

O tamanho e a quantidade de módulos reforçam que não se trata de um simples aplicativo Java que possa ser atualizado por edição superficial do Manifest.

## Avaliação preliminar

Classificação atual: **difícil, ainda não inviável**.

| Questão | Situação |
|---|---|
| Manifest/target SDK | Pode ser investigado, mas não é o principal bloqueio |
| AIR antigo | Bloqueador potencial; exige prova de compatibilidade |
| SWF proprietário | Código-fonte não disponível; reconstrução pode não ser possível |
| ANE Pixel | Provavelmente essencial para edição |
| ANE SyncEngine | Pode depender de APIs/serviços encerrados |
| Push/C2DM | Tecnologia legada; pode exigir remoção ou substituição |
| Bibliotecas ARMv7 | Não há ARM64 no APK |
| Assinatura | Chave Adobe não disponível; build modificada não será atualização oficial |
| Serviços remotos | Ainda precisam ser identificados e testados |

## O que ainda não foi feito

- Descompilação/análise detalhada do ActionScript do `TTPixel.swf`.
- Análise ELF de todas as bibliotecas `.so` e de suas dependências.
- Teste em dispositivo Android moderno.
- Coleta de `adb logcat` durante instalação e execução.
- Verificação funcional de PSDX, câmera, galeria, filtros, sincronização e publicação.
- Prova de conceito com um AIR SDK compatível.

Nenhum desses itens deve ser marcado como concluído sem evidência reproduzível.

Para a análise executada com ferramentas de engenharia reversa, veja [deep-autopsy.md](deep-autopsy.md).
