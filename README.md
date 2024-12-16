# Embarcatech
Repositório do Embarcatech

# COMO EXECUTAR

## INSTALANDO O COMPILADOR

é necessario instalar um compilador no windows é o minGW a maneira mais pratica de instalar é utilizando linha de comando
no meu caso estou usando o <a href='https://chocolatey.org/install'> chocolatey <a> esse é o comando necessario:

OBS:você deve esta como administrador

```
    choco install mingw
```

Após executar o comando reinicie o computador

## Extensões VScode

- C/C++ Extension Pack

## Executando

Para rodar basta está com o arquivo aberto e apertar F5 para iniciar o compilador
em seguida o VScode vai pedir para selecionar o compilador escolhar a opção 'C++ (GDB/LLDB)''

### Arquivo de configuração

Pode ocorrer do VScode solicitar uma configuração, caso ocorrar ele vai criar um arquivo chamado settings.json
e adicione o seguinte no arquivo:

```
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build C++",
      "type": "shell",
      "command": "g++",
      "args": [
        "-g",
        "-o",
        "build/Debug/outDebug",
        "source.cpp"
      ],
      "group": {
        "kind": "build",
        "isDefault": true
      }
    }
  ]
}
```
