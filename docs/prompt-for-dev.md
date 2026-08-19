# Prompt para dev ou IA

```text
Você é um engenheiro especializado em Android legado, Adobe AIR, engenharia reversa autorizada e portabilidade de APKs.

Quero analisar e, se for tecnicamente possível, portar para Android moderno o APK do Adobe Photoshop Touch 1.3.7. O objetivo é uso pessoal e autorizado. Não faça bypass de DRM, licenciamento, autenticação ou serviços da Adobe, e não publique o aplicativo nem seus binários.

O APK é baseado em Adobe AIR. Dados iniciais:

- ID: com.adobe.pstouchphone
- Versão: 1.3.7
- Descriptor AIR: namespace 16.0
- Conteúdo principal: assets/TTPixel.swf
- minSdkVersion: 14
- targetSdkVersion: 12
- ANEs: com.adobe.ttpixel.extension, air.com.adobe.cc.sync.SyncEngine e air.com.adobe.cc.PushNotification
- Bibliotecas nativas somente em armeabi-v7a

REGRAS

1. Não modifique o APK original.
2. Faça uma cópia de trabalho e registre o SHA-256.
3. Comece com uma autópsia técnica completa.
4. Não altere apenas targetSdkVersion.
5. Antes de cada alteração, informe arquivo, alteração, motivo, problema resolvido e teste.
6. Não declare sucesso apenas porque o APK instalou.
7. Se o código-fonte ou uma biblioteca necessária não estiver disponível, documente o bloqueio.

FASE 1 — AUTÓPSIA

Use apktool, aapt2, apkanalyzer, JADX, baksmali, strings, readelf/objdump, JPEXS/FFDec e unzip quando apropriado.

Identifique:

- AndroidManifest.xml;
- assets/META-INF/AIR/application.xml;
- versão/formato AIR;
- estrutura e dependências do TTPixel.swf;
- todos os SWF, CAB, CABMOD, CABRES, FS e recursos;
- extension.xml e catalog.xml de cada ANE;
- JARs, SOs, JNI e dependências ELF;
- ABI, min API e símbolos nativos;
- permissões, Activities, Services e BroadcastReceivers;
- armazenamento, content://, file://, câmera, galeria, WebView e OpenGL;
- endpoints, autenticação e serviços remotos;
- assinatura e certificado do APK.

Entregue primeiro um relatório de viabilidade. Não gere o APK final antes disso.

FASE 2 — PROVA DE CONCEITO

Avalie, nesta ordem:

A. SWF original com AIR SDK/runtime compatível.
B. Reempacotamento mínimo.
C. Wrapper moderno preservando o SWF.
D. Substituição/portabilidade dos ANEs.
E. Recompilação legítima das bibliotecas para ARM64, somente se houver código-fonte ou solução autorizada.

Não presuma que um SWF descompilado equivale ao código-fonte original.

FASE 3 — COMPATIBILIDADE ANDROID

Investigue permissões, Scoped Storage, URI de arquivos, câmera, galeria, WebView, OpenGL, hardware acceleration, PendingIntent, Services, Receivers, notificações C2DM/GCM, JNI, carregamento de SOs e suporte a ARMv7/ARM64.

FASE 4 — BUILD E TESTES

Crie scripts reproduzíveis para desmontar, modificar, empacotar, assinar, instalar via ADB, coletar logcat e limpar artefatos. Use uma chave de teste própria. Explique que a chave original da Adobe não está disponível e que a build modificada não será uma atualização oficial.

Teste instalação, abertura, novo documento, JPEG/PNG/GIF, camadas, pincéis, filtros, crop, transformação, texto, PSDX, câmera, galeria, rotação, stylus, exportação e funções de rede.

Para cada crash, classifique a origem como Java/Android, AIR, ActionScript/SWF, ANE, JNI, biblioteca nativa ou serviço remoto.

ENTREGÁVEIS

1. Relatório de autópsia.
2. Matriz de dependências e incompatibilidades.
3. Classificação de viabilidade.
4. Scripts/código de build, quando possível.
5. APK modificado somente se funcional.
6. Hash SHA-256 dos artefatos.
7. Instruções ADB.
8. Funcionalidades testadas.
9. Limitações conhecidas.
10. Logs e evidências.

Não diga que o aplicativo foi atualizado se ele apenas foi recompilado ou instalado.
```

