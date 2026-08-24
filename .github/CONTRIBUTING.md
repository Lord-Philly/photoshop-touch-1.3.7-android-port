# Guia de contribuição

Obrigado pelo interesse em contribuir! Este projeto é mantido por uma pessoa e qualquer ajuda da comunidade faz diferença. ❤️

## Formas de contribuir

| Tipo | Onde |
|---|---|
| Reportar bug | [Issue → Bug report](https://github.com/Lord-Philly/photoshop-touch-1.3.7-android-port/issues/new?template=bug_report.md) |
| Sugerir melhoria | [Issue → Feature request](https://github.com/Lord-Philly/photoshop-touch-1.3.7-android-port/issues/new?template=feature_request.md) |
| Testar em aparelhos | Preencha os resultados na [`docs/compatibility-matrix.md`](../docs/compatibility-matrix.md) e abra issue com os dados |
| Melhorar documentação / código | Pull Request (fluxo abaixo) |

## Fluxo para Pull Requests

### 1. Fork

Faça um fork do repositório clicando no botão **Fork** no canto superior direito.

### 2. Clone e branch

```bash
git clone https://github.com/<seu-usuario>/photoshop-touch-1.3.7-android-port.git
cd photoshop-touch-1.3.7-android-port
git checkout -b minha-contribuicao
```

Use nomes descritivos de branch, por exemplo:

- `docs/corrige-typo-matriz`
- `native/ane-compat-novo-stub`
- `scripts/automacao-montagem`

### 3. Faça as alterações

- Siga o estilo do conteúdo existente (documentação em **português do Brasil**, código comentado em inglês);
- Commits pequenos e com mensagem clara, ex.: `docs: adiciona teste Android 14 à matriz`.

### 4. Teste

- Mudanças em `scripts/*.ps1`: execute o script contra uma cópia de teste e confira a saída;
- Mudanças em `native/ane-compat/`: compile com NDK 25.2 conforme o [README do diretório](../native/ane-compat/README.md);
- Mudanças em documentação: confira links internos e caminhos de imagem.

### 5. Abra o Pull Request

```bash
git push origin minha-contribuicao
```

Descreva no PR: **o que** mudou, **por que**, e **como testou**.

## Boas práticas

- ⚠️ **Nunca faça commit** de APKs, SWFs, ANEs, JARs, `.so` ou outros binários proprietários — veja a [política de segurança](../SECURITY.md);
- Não invente informações: tudo documentado deve ser verificável (hashes, logs, resultados de teste);
- Registre resultados de teste com dispositivo, versão do Android e data;
- Dúvida sobre escopo? Abra uma issue antes de investir tempo em algo grande.

## Reportando bugs de forma útil

Quanto mais dados, mais rápido o diagnóstico:

- Modelo do aparelho + versão do Android;
- Qual APK (`ps137` ou `ps177`) e de qual release;
- Passos exatos para reproduzir;
- Logcat se possível: `adb logcat -b main -b crash > log.txt`;
- Screenshots ou vídeo quando o problema for visual.
