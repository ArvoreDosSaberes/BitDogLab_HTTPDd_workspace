# BitDogLab_HTTPDd_workspace

![visitors](https://visitor-badge.laobi.icu/badge?page_id=ArvoreDosSaberes.BitDogLab_HTTPDd_workspace)
[![Build](https://img.shields.io/github/actions/workflow/status/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace/ci.yml?branch=main)](https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace/actions)
[![Issues](https://img.shields.io/github/issues/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace)](https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace/issues)
[![Stars](https://img.shields.io/github/stars/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace)](https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace/stargazers)
[![Forks](https://img.shields.io/github/forks/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace)](https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace/network/members)
[![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-brightgreen.svg)]()
[![HTTPd](https://img.shields.io/badge/Server-HTTPd-orange.svg)]()
[![License: CC BY 4.0](https://img.shields.io/badge/license-CC%20BY%204.0-blue.svg)](https://creativecommons.org/licenses/by/4.0/)

---

## Descrição

Workspace para testes da BitDogLab, uma interface HTTP que facilita o teste geral da placa, desde a conexão WiFi, porta serial USB e demais recursos externalizados via Joystick, matrix de LED, OLED, Buzzer e Botões.

---

## Como Clonar o Repositório

Para obter uma cópia local do projeto, utilize o comando abaixo:

```bash
git clone --recursive https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace.git
cd BitDogLab_HTTPDd_workspace
```

> **Nota:** O parâmetro `--recursive` é importante para clonar também os submódulos necessários.

Se você já clonou o repositório sem os submódulos, execute:

```bash
git submodule update --init --recursive
```

---

## Pré-requisitos

Antes de compilar o projeto, certifique-se de ter instalado:

- **Pico SDK** - SDK oficial da Raspberry Pi para RP2040
- **CMake** (versão 3.13 ou superior)
- **GCC ARM Embedded** - Compilador para ARM Cortex-M
- **Git** - Para controle de versão

---

## Como Usar

### 1. Configurar o ambiente

Configure a variável de ambiente `PICO_SDK_PATH` apontando para o diretório do Pico SDK:

```bash
export PICO_SDK_PATH=/caminho/para/pico-sdk
```

### 2. Compilar o projeto

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 3. Carregar o firmware

Após a compilação, o arquivo `.uf2` será gerado na pasta `build`. Para carregar na BitDogLab:

1. Conecte a placa ao computador segurando o botão **BOOTSEL**
2. Solte o botão após conectar - a placa aparecerá como um dispositivo de armazenamento USB
3. Copie o arquivo `.uf2` para o dispositivo

```bash
cp build/*.uf2 /media/$USER/RPI-RP2/
```

### 4. Acessar a interface HTTP

Após o firmware ser carregado, a placa se conectará à rede WiFi configurada e disponibilizará uma interface HTTP para testes. Acesse pelo navegador usando o IP atribuído à placa.

---

## Recursos Disponíveis

- **Conexão WiFi** - Configuração e gerenciamento de rede sem fio
- **Porta Serial USB** - Comunicação via USB CDC
- **Joystick** - Leitura de entradas do joystick
- **Matrix de LED** - Controle da matriz de LEDs
- **Display OLED** - Interface com display OLED
- **Buzzer** - Geração de sons e alertas
- **Botões** - Leitura de entradas digitais

---

## Autor

**Carlos Delfino**

- 🌐 Website: [https://carlosdelfino.eti.br](https://carlosdelfino.eti.br)
- 💻 GitHub: [https://github.com/CarlosDelfino](https://github.com/CarlosDelfino)
- 📧 Email: [consultoria@carlosdelfino.eti.br](mailto:consultoria@carlosdelfino.eti.br)

---

## Licença

Este projeto está licenciado sob a licença **Creative Commons Attribution 4.0 International (CC BY 4.0)**.

Você é livre para:
- **Compartilhar** — copiar e redistribuir o material em qualquer meio ou formato
- **Adaptar** — remixar, transformar e criar a partir do material para qualquer finalidade

Desde que atribua o crédito apropriado ao autor.

Consulte [LICENSE](LICENSE) ou visite [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) para mais detalhes.

---

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para:

1. Fazer um **fork** do projeto
2. Criar uma **branch** para sua feature (`git checkout -b feature/nova-feature`)
3. Fazer **commit** das suas alterações (`git commit -m 'Adiciona nova feature'`)
4. Fazer **push** para a branch (`git push origin feature/nova-feature`)
5. Abrir um **Pull Request**

---

## Suporte

Para dúvidas, sugestões ou reportar problemas:

- Abra uma [issue](https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace/issues)
- Entre em contato pelo email: [consultoria@carlosdelfino.eti.br](mailto:consultoria@carlosdelfino.eti.br)
