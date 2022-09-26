# Iot Device Gateway - IoT  Navigation System
A [rtos](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html) navigation system based on HTTP request. Developed with my computer engineering students at the [FIAP](https://www.fiap.com.br/) university - São Paulo, Brasil.

- [Setup](#local-setup)
- [Development](#development)
- [Contribution Guide](#contribution-guide)


## Local Setup

The hardware used id a robotic kit provided by [robocore](https://www.robocore.net/), and the sensor MPU6060:

![explorer-kit](https://s3-sa-east-1.amazonaws.com/robocore-lojavirtual/1444/pag_solos_RoboExplorer.jpg)

The controller is an esp32 embedded on board VESPA developed by robocore for robotic projects, and the the follow diagram details the  pinout:

![vespa-conections](https://s3-sa-east-1.amazonaws.com/robocore-lojavirtual/1439/vespa_esquematica-01.jpg)

It is necesary to install the [esp-idf] on your computer, we recomend using the vscode plugin.

The requirements about the system are:
```bash
- Full control based on HTTP request.
- Multithreading API requisitions.
- Navigation based on lineal translation.
- Trajectory based on inercial aceleration.
```
The code strategy are listed below:
![coding-strategy](docs/code_startegy.png)

## Contribution Guide

You will need to create a new branch, follow this convention:

```bash
<turma>_<checkpoint>_<rm>
```
for example:

```bash
5ECR_Checkpoint2-A_rm3456
```

Create a Pull Request and add me to revise and aprove the features/changes.

## Checkpoint2-B (3 ptos) - due 27/09 00:00

We had developed the code to comunicate with the Thingspeak API, but we are developing a hardware API based on flask, so please updating the code to comunicate with flask, to inicializate the service follow this steps:

```bash
pip3 install -r requirements.txt
```

```bash
export FLASK_APP=gateway #on the project's root
```

```bash
flask run --host=xxx.xxx.x.x #on the project's root
```
You should see something like that:

![server-example](docs/flask-server.png)

Use postman to ensure that the esp32 is doing a HTTP POST on the API .
